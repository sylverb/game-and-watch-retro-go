#!/usr/bin/env python3
import argparse
import hashlib
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import List

try:
    from tqdm import tqdm
except ImportError:
    tqdm = None

ROM_ENTRIES_TEMPLATE = """
const retro_emulator_file_t {name}[] EMU_DATA = {{
{body}
}};
const uint32_t {name}_count = {rom_count};
"""

# Note: this value is not easily changed as it's assumed in some memory optimizations
MAX_CHEAT_CODES = 16

ROM_ENTRY_TEMPLATE = """\t{{
#if CHEAT_CODES == 1
\t\t.id = {rom_id},
#endif
\t\t.name = "{name}",
\t\t.ext = "{extension}",
\t\t.address = {rom_entry},
\t\t.size = {size},
\t\t#if COVERFLOW != 0
\t\t.img_address = {img_entry},
\t\t.img_size = {img_size},
\t\t#endif
\t\t.save_address = {save_entry},
\t\t.save_size = {save_size},
\t\t.system = &{system},
\t\t.region = {region},
\t\t.mapper = {mapper},
\t\t.game_config = {game_config},
#if CHEAT_CODES == 1
\t\t.cheat_codes = {cheat_codes},
\t\t.cheat_descs = {cheat_descs},
\t\t.cheat_count = {cheat_count},
#endif
\t}},"""

SYSTEM_PROTO_TEMPLATE = """
#if !defined (COVERFLOW)
  #define COVERFLOW 0
#endif /* COVERFLOW */
#if !defined (BIG_BANK)
#define BIG_BANK 1
#endif
#if (BIG_BANK == 1) && (EXTFLASH_SIZE <= 128*1024*1024)
#define EMU_DATA 
#else
#define EMU_DATA __attribute__((section(".extflash_emu_data")))
#endif
extern const rom_system_t {name};
"""

SYSTEM_TEMPLATE = """
const rom_system_t {name} EMU_DATA = {{
\t.system_name = "{system_name}",
\t.roms = {variable_name},
\t.extension = "{extension}",
\t#if COVERFLOW != 0
\t.cover_width = {cover_width},
\t.cover_height = {cover_height},
\t#endif 
\t.roms_count = {roms_count},
}};
"""

SAVE_SIZES = {
    "nes": 24 * 1024, # only when using nofrendo, elseway it's given by nesmapper script
    "sms": 60 * 1024,
    "gg": 60 * 1024,
    "col": 60 * 1024,
    "sg": 60 * 1024,
    "pce": 76 * 1024,
    "msx": 272 * 1024,
    "gw": 4 * 1024,
    "wsv": 28 * 1024,
    "md": 144 * 1024,
    "a7800": 36 * 1024,
    "amstrad": 132 * 1024,
    "zelda3": 272 * 1024,
    "smw": 264 * 1024,
    "tama": 8 * 1024,
}


# TODO: Find a better way to find this before building
MAX_COMPRESSED_NES_SIZE = 0x00080010 #512kB + 16 bytes header
MAX_COMPRESSED_PCE_SIZE = 0x00049000
MAX_COMPRESSED_WSV_SIZE = 0x00080000
MAX_COMPRESSED_SG_COL_SIZE = 60 * 1024
MAX_COMPRESSED_A7800_SIZE = 131200
MAX_COMPRESSED_MSX_SIZE = 136*1024

"""
All ``compress_*`` functions must be decorated ``@COMPRESSIONS`` and have the
following signature:

Positional argument:
    data : bytes

Optional argument:
    level : ``None`` for default value,  depends on compression algorithm.
            Can be the special ``DONT_COMPRESS`` sentinel value, in which the
            returned uncompressed data is properly framed to be handled by the
            decompressor.

And return compressed bytes.
"""

DONT_COMPRESS = object()


class CompressionRegistry(dict):
    prefix = "compress_"

    def __call__(self, f):
        name = f.__name__
        assert name.startswith(self.prefix)
        key = name[len(self.prefix) :]
        self[key] = f
        self["." + key] = f
        return f


COMPRESSIONS = CompressionRegistry()


@COMPRESSIONS
def compress_lzma(data, level=None):
    if level == DONT_COMPRESS:
        if args.compress_gb_speed:
            raise NotImplementedError
        # This currently assumes this will only be applied to GB Bank 0
        return data
    import lzma

    compressed_data = lzma.compress(
        data,
        format=lzma.FORMAT_ALONE,
        filters=[
            {
                "id": lzma.FILTER_LZMA1,
                "preset": 6,
                "dict_size": 16 * 1024,
            }
        ],
    )

    compressed_data = compressed_data[13:]

    return compressed_data

def sha1_for_file(filename):
    sha1 = hashlib.sha1()
    if os.path.exists(filename):
        with open(filename, 'rb') as f:
            while True:
                data = f.read(16*1024)
                if not data:
                    break
                sha1.update(data)

        return sha1.hexdigest()
    else:
        return ""


def parse_msx_bios_files():
    #check that required MSX bios files are present
    if (sha1_for_file("roms/msx_bios/MSX2P.rom") != "e90f80a61d94c617850c415e12ad70ac41e66bb7"):
        print("Bad or missing roms/msx_bios/MSX2P.rom, check roms/msx_bios/README.md for info")
        return 0

    if (sha1_for_file("roms/msx_bios/MSX2PEXT.rom") != "fe0254cbfc11405b79e7c86c7769bd6322b04995"):
        print("Bad or missing roms/msx_bios/MSX2PEXT.rom, check roms/msx_bios/README.md for info")
        return 0

    if (sha1_for_file("roms/msx_bios/MSX2PMUS.rom") != "6354ccc5c100b1c558c9395fa8c00784d2e9b0a3"):
        print("Bad or missing roms/msx_bios/MSX2PMUS.rom, check roms/msx_bios/README.md for info")
        return 0

    if (sha1_for_file("roms/msx_bios/MSX2.rom") != "6103b39f1e38d1aa2d84b1c3219c44f1abb5436e"):
        print("Bad or missing roms/msx_bios/MSX2.rom, check roms/msx_bios/README.md for info")
        return 0

    if (sha1_for_file("roms/msx_bios/MSX2EXT.rom") != "5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02"):
        print("Bad or missing roms/msx_bios/MSX2EXT.rom, check roms/msx_bios/README.md for info")
        return 0

    if (sha1_for_file("roms/msx_bios/MSX.rom") != "e998f0c441f4f1800ef44e42cd1659150206cf79"):
        print("Bad or missing roms/msx_bios/MSX.rom, check roms/msx_bios/README.md for info")
        return 0

    # We revert previously patched PANASONICDISK if needed as we changed how it is done
    if (sha1_for_file("roms/msx_bios/PANASONICDISK.rom") == "b9bce28fb74223ea902f82ebd107279624cf2aba"):
        print("Reverting patch on roms/msx_bios/PANASONICDISK.rom")
        with open("roms/msx_bios/PANASONICDISK.rom", 'rb+') as f:
            f.seek(0x17ec)
            f.write(b'\x02')

    if (sha1_for_file("roms/msx_bios/PANASONICDISK.rom") != "7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6"):
        print("Bad or missing roms/msx_bios/PANASONICDISK.rom, check roms/msx_bios/README.md for info")
        return 0

    # PANASONICDISK_.rom is a patched version of PANASONICDISK.rom to disable the 2nd FDD
    # this is allowing to free some ram, which is needed for some games. It could be done by pressing
    # ctrl key at boot using original bios, but using this patched version, the user will have nothing
    # to do. Unfortunately this version can't be used in all cases because some games (from Micro Cabin)
    # like Fray, XAK III, 
    if (sha1_for_file("roms/msx_bios/PANASONICDISK_.rom") != "b9bce28fb74223ea902f82ebd107279624cf2aba"):
        shutil.copy("roms/msx_bios/PANASONICDISK.rom","roms/msx_bios/PANASONICDISK_.rom")
        if (sha1_for_file("roms/msx_bios/PANASONICDISK_.rom") == "7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6"):
            print("Patching roms/msx_bios/PANASONICDISK_.rom to disable 2nd FDD controller (= more free RAM)")
            with open("roms/msx_bios/PANASONICDISK_.rom", 'rb+') as f:
                f.seek(0x17ec)
                f.write(b'\x00')
        else:
            print("Bad or missing roms/msx_bios/PANASONICDISK.rom, check roms/msx_bios/README.md for info")
            return 0

    return 1


def compare_versions(version1, version2):
    """
    Compare two versions represented as strings.
    Returns:
        -1 if version1 < version2
         0 if version1 == version2
         1 if version1 > version2
    """
    v1_parts = [int(part) for part in version1.split('.')]
    v2_parts = [int(part) for part in version2.split('.')]

    for v1, v2 in zip(v1_parts, v2_parts):
        if v1 < v2:
            return -1
        elif v1 > v2:
            return 1

    # If all parts are equal, compare the lengths
    if len(v1_parts) < len(v2_parts):
        return -1
    elif len(v1_parts) > len(v2_parts):
        return 1
    else:
        return 0

def write_covart(srcfile, fn, w, h, jpg_quality):
    from PIL import Image, ImageOps
    if compare_versions(Image.__version__, '7.0') >= 0:
        dither = Image.Resampling.LANCZOS
    else:
        dither = Image.ANTIALIAS
    img = Image.open(srcfile).convert(mode="RGB").resize((w, h), dither)
    img.save(fn,format="JPEG",optimize=True,quality=jpg_quality)

# def write_rgb565(srcfile, fn, v):
#     from PIL import Image, ImageOps
#     #print(srcfile)
#     img = Image.open(srcfile).convert(mode="RGB")
#     img = img.resize((w, h), Image.ANTIALIAS)
#     pixels = list(img.getdata())
#     with open(fn, "wb") as f:
#         #no file header
#         for pix in pixels:
#             r = (pix[0] >> 3) & 0x1F
#             g = (pix[1] >> 2) & 0x3F
#             b = (pix[2] >> 3) & 0x1F
#             f.write(struct.pack("H", (r << 11) + (g << 5) + b))

class NoArtworkError(Exception):
    """No artwork found for this ROM"""


class ROM:
    def __init__(self, system_name: str, filepath: str, extension: str, romdefs: dict):
        filepath = Path(filepath)

        self.rom_id = 0 
        self.path = filepath
        self.filename = filepath
        # Remove compression extension from the name in case it ends with that
        if filepath.suffix in COMPRESSIONS or filepath.suffix == '.cdk':
            self.filename = filepath.with_suffix("").stem
        else :
            self.filename = filepath.stem
        romdefs.setdefault(self.filename, {})
        self.romdef = romdefs[self.filename]
        self.romdef.setdefault('name', self.filename)
        self.romdef.setdefault('publish', '1')
        self.romdef.setdefault('embed', '1')
        self.romdef.setdefault('enable_save', '0')
        self.publish = (self.romdef['publish'] == '1')
        self.embed = (self.romdef['embed'] == '1')
        self.enable_save = (self.romdef['enable_save'] == '1') or args.save
        self.system_name = system_name
        self.name = self.romdef['name']
        print("Found rom " + self.filename +" will display name as: " + self.romdef['name'])
        if not (self.publish):
            print("& will not Publish !")
        if not (self.embed):
            print("& will not embed !")
            self.symbol = "0";
        obj_name = "".join([i if i.isalnum() else "_" for i in self.path.name])
        self.obj_path = "build/roms/" + obj_name + ".o"
        symbol_path = str(self.path.parent) + "/" + obj_name
        if (self.embed):
            self.symbol = (
                "_binary_"
                + "".join([i if i.isalnum() else "_" for i in symbol_path])
                + "_start"
            )

        self.img_path = self.path.parent / (self.filename + ".img")
        obj_name = "".join([i if i.isalnum() else "_" for i in self.img_path.name])
        symbol_path = str(self.path.parent) + "/" + obj_name
        self.obj_img = "build/roms/" + obj_name + "_" + extension + ".o"
        self.img_symbol = (
            "_binary_"
            + "".join([i if i.isalnum() else "_" for i in symbol_path])
            + "_start"
        )

    def __str__(self) -> str:
        return f"id: {self.rom_id} name: {self.name} size: {self.size} ext: {self.ext}"

    def __repr__(self):
        return str(self)

    def read(self):
        return self.path.read_bytes()

    def get_rom_patchs(self):
        #get pce rompatchs files
        pceplus = Path(self.path.parent, self.filename + ".pceplus")

        if not os.path.exists(pceplus):
            return []

        codes_and_descs = []
        for line in pceplus.read_text().splitlines():
            line = line.strip()
            if not line:
                continue
            if line.startswith("#"):
                continue
            parts = line.split(',')
            cmd_count = 0
            cmd_str = ""
            for i in range(len(parts) - 1):
                part = parts[i].strip()
                #get cmd byte count
                x_str = part[0:2]
                x = (int(x_str, 16) >> 4) + 1
                one_cmd = ""
                for y in range(3):
                    one_cmd = one_cmd + "\\x" + part[y * 2: y * 2 + 2] 
                for y in range(x):
                    one_cmd = one_cmd + "\\x" + part[y * 2 + 6: y * 2 + 8]
                #got one cmd
                cmd_count += 1
                cmd_str = cmd_str + one_cmd
            cmd_str = "\\x%x" % (cmd_count) + cmd_str
            desc = parts[len(parts) - 1]
            if desc is not None:
                desc = desc[:40]
                desc = desc.replace('\\', r'\\\\')
                desc = desc.replace('"', r'\"')
                desc = desc.strip()

            codes_and_descs.append((cmd_str, desc))

        if len(codes_and_descs) > MAX_CHEAT_CODES:
            print(
                f"INFO: {self.name} has more than {MAX_CHEAT_CODES} cheat codes. Truncating..."
            )
            codes_and_descs = codes_and_descs[:MAX_CHEAT_CODES]

        return codes_and_descs

    def get_cheat_codes(self):
        # Get game genie code file path
        gg_path = Path(self.path.parent, self.filename + ".ggcodes")

        if os.path.exists(gg_path):
            codes_and_descs = []
            for line in gg_path.read_text().splitlines():
                line = line.strip()
                if not line:
                    continue
                parts = line.split(',', 1)
                code = parts[0]
                desc = None
                if len(parts)>1:
                    desc = parts[1]

                # Remove whitespace
                code = "".join(code.split())
                # Remove empty lines
                if code == "":
                    continue
                # Capitalize letters
                code = code.upper()

                # Shorten description
                if desc is not None:
                    desc = desc[:40]
                    desc = desc.replace('\\', r'\\\\')
                    desc = desc.replace('"', r'\"')
                    desc = desc.strip()

                codes_and_descs.append((code, desc))

            if len(codes_and_descs) > MAX_CHEAT_CODES:
                print(
                    f"INFO: {self.name} has more than {MAX_CHEAT_CODES} cheat codes. Truncating..."
                )
                codes_and_descs = codes_and_descs[:MAX_CHEAT_CODES]

            return codes_and_descs

        pceplus = Path(self.path.parent, self.filename + ".pceplus")
        if os.path.exists(pceplus):
            return self.get_rom_patchs()

        mfc_path = Path(self.path.parent, self.filename + ".mcf")
        if os.path.exists(mfc_path):
            codes_and_descs = []
            for line in mfc_path.read_text(encoding="cp1252").splitlines():
                line = line.strip()
                if not line:
                    continue
                # Check if it's a comment
                if line[0] == '!':
                    continue
                parts = line.split(',', 4)
                if len(parts) == 5:
                    length = 1
                    if int(parts[2]) > 0xff:
                        length = 2
                    elif int(parts[2]) > 0xffff:
                        length = 4
                    code = parts[1]+','+parts[2]+','+str(length)
                    desc = None
                    if len(parts)>4:
                        desc = parts[4]

                    # Remove whitespace
                    code = "".join(code.split())
                    # Remove empty lines
                    if code == "":
                        continue
                    # Capitalize letters
                    code = code.upper()

                    # Shorten description
                    if desc is not None:
                        desc = desc[:40]
                        desc = desc.replace('\\', r'\\\\')
                        desc = desc.replace('"', r'\"')
                        desc = desc.strip()

                    codes_and_descs.append((code, desc))
                elif len(parts) == 1:
                    parts = line.split(':', 4)
                    if int(parts[2]) == 0:
                        length = 1
                    elif int(parts[2]) == 1:
                        length = 2
                    elif int(parts[2]) == 2:
                        length = 4

                    code = str(int(parts[0], base=16))+','+str(int(parts[1], base=16))+','+str(length)
                    desc = None
                    if len(parts)>4:
                        desc = parts[4]

                    # Remove whitespace
                    code = "".join(code.split())
                    # Remove empty lines
                    if code == "":
                        continue
                    # Capitalize letters
                    code = code.upper()

                    # Shorten description
                    if desc is not None:
                        desc = desc[:40]
                        desc = desc.replace('\\', r'\\\\')
                        desc = desc.replace('"', r'\"')
                        desc = desc.strip()

                    codes_and_descs.append((code, desc))


            if len(codes_and_descs) > MAX_CHEAT_CODES:
                print(
                    f"INFO: {self.name} has more than {MAX_CHEAT_CODES} cheat codes. Truncating..."
                )
                codes_and_descs = codes_and_descs[:MAX_CHEAT_CODES]

            return codes_and_descs

        # No cheat file found
        return []

    @property
    def ext(self):
        return self.path.suffix[1:].lower()

    @property
    def size(self):
        return self.path.stat().st_size

    @property
    def mapper(self):
        mapper = 0
        if self.system_name == "MSX":
            mapper = int(subprocess.check_output([sys.executable, "./tools/findblueMsxMapper.py", "roms/msx_bios/msxromdb.xml", str(self.path).replace('.dsk.cdk','.dsk').replace('.lzma','')]))
        if self.system_name == "Nintendo Entertainment System":
            mapper = int(subprocess.check_output([sys.executable, "./fceumm-go/nesmapper.py", "mapper", str(self.path).replace('.lzma','')]))
        return mapper

    @property
    def game_config(self):
        value = 0xff
        if self.system_name == "MSX":
            # MSX game_config structure :
            # b7-b0 : Controls profile
            # b8 : Does the game require to press ctrl at boot ?
            sp_output = subprocess.check_output([sys.executable, "./tools/findblueMsxControls.py", "roms/msx_bios/msxromdb.xml", str(self.path).replace('.dsk.cdk','.dsk').replace('.lzma','')]).splitlines()
            value = int(sp_output[0]) + (int(sp_output[1]) << 8)
            if int(sp_output[0]) == 0xff :
                print(f"Warning : {self.name} has no controls configuration in roms/msx_bios/msxromdb.xml, default controls will be used")
        return value
    @property
    def img_size(self):
        try:
            return self.img_path.stat().st_size
        except FileNotFoundError:
            return 0


class ROMParser:
    global sms_reserved_flash_size
    def find_roms(self, system_name: str, folder: str, extension: str, romdefs: dict) -> [ROM]:
        extension = extension.lower()
        ext = extension
        if not extension.startswith("."):
            extension = "." + extension

        script_path = Path(__file__).parent
        roms_folder = script_path / "roms" / folder

        # find all files that end with extension (case-insensitive)
        rom_files = list(roms_folder.iterdir())
        rom_files = [r for r in rom_files if r.name.lower().endswith(extension)]
        rom_files.sort()
        found_roms = [ROM(system_name, rom_file, ext, romdefs) for rom_file in rom_files]
        for rom in found_roms:
            suffix = "_no_save"
            if rom.name.endswith(suffix) :
                rom.name = rom.name[:-len(suffix)]
                rom.enable_save = False

        return found_roms

    def generate_rom_entries(
        self, name: str, roms: [ROM], save_prefix: str, system: str, cheat_codes_prefix: str
    ) -> str:
        body = ""
        pubcount = 0
        for i in range(len(roms)):
            rom = roms[i]
            if not (rom.publish):
                continue
            is_pal = any(
                substring in rom.filename
                for substring in [
                    "(E)",
                    "(Europe)",
                    "(Sweden)",
                    "(Germany)",
                    "(Italy)",
                    "(France)",
                    "(A)",
                    "(Australia)",
                ]
            )
            region = "REGION_PAL" if is_pal else "REGION_NTSC"
            gg_count_name = "%s%s_COUNT" % (cheat_codes_prefix, i)
            gg_code_array_name = "%sCODE_%s" % (cheat_codes_prefix, i)
            gg_desc_array_name = "%sDESC_%s" % (cheat_codes_prefix, i)
            body += ROM_ENTRY_TEMPLATE.format(
                rom_id=rom.rom_id,
                name=str(rom.name),
                size=rom.size,
                rom_entry=rom.symbol,
                img_size=rom.img_size,
                img_entry=rom.img_symbol if rom.img_size else "NULL",
                save_entry=(save_prefix + str(i)) if rom.enable_save else "NULL",
                save_size=("sizeof(" + save_prefix + str(i) + ")") if rom.enable_save else "0",
                region=region,
                extension=rom.ext,
                system=system,
                cheat_codes=gg_code_array_name if cheat_codes_prefix else "NULL",
                cheat_descs=gg_desc_array_name if cheat_codes_prefix else 0,
                cheat_count=gg_count_name if cheat_codes_prefix else 0,
                mapper=rom.mapper,
                game_config=rom.game_config,
            )
            body += "\n"
            pubcount += 1

        return ROM_ENTRIES_TEMPLATE.format(name=name, body=body, rom_count=pubcount)

    def generate_object_file(self, rom: ROM,system_name) -> str:
        # convert rom to an .o file and place the data in the .extflash_game_rom section
        prefix = ""
        if "GCC_PATH" in os.environ:
            prefix = os.environ["GCC_PATH"]
        prefix = Path(prefix)
        if system_name == "Sega Genesis":
            subprocess.check_output(
                [
                    prefix / "arm-none-eabi-objcopy",
                    "--rename-section",
                    ".data=.extflash_game_rom,alloc,load,readonly,data,contents",
                    "-I",
                    "binary",
                    "-O",
                    "elf32-littlearm",
                    "-B",
                    "armv7e-m",
                    "--reverse-bytes=2",
                    rom.path,
                    rom.obj_path,
                ]
            )
        else:
            subprocess.check_output(
                [
                    prefix / "arm-none-eabi-objcopy",
                    "--rename-section",
                    ".data=.extflash_game_rom,alloc,load,readonly,data,contents",
                    "-I",
                    "binary",
                    "-O",
                    "elf32-littlearm",
                    "-B",
                    "armv7e-m",
                    rom.path,
                    rom.obj_path,
                ]
            )
        subprocess.check_output(
            [
                prefix / "arm-none-eabi-ar",
                "-cr",
                "build/roms.a",
                rom.obj_path,
            ]
        )
        template = "extern const uint8_t {name}[];\n"
        return template.format(name=rom.symbol)

    def generate_img_object_file(self, rom: ROM, w, h) -> str:
        # convert rom_img to an .o file and place the data in the .extflash_game_rom section
        prefix = ""
        if "GCC_PATH" in os.environ:
            prefix = os.environ["GCC_PATH"]

        prefix = Path(prefix)

        imgs = []
        imgs.append(str(rom.img_path.with_suffix(".png")))
        imgs.append(str(rom.img_path.with_suffix(".PNG")))
        imgs.append(str(rom.img_path.with_suffix(".Png")))
        imgs.append(str(rom.img_path.with_suffix(".jpg")))
        imgs.append(str(rom.img_path.with_suffix(".JPG")))
        imgs.append(str(rom.img_path.with_suffix(".Jpg")))
        imgs.append(str(rom.img_path.with_suffix(".jpeg")))
        imgs.append(str(rom.img_path.with_suffix(".JPEG")))
        imgs.append(str(rom.img_path.with_suffix(".Jpeg")))
        imgs.append(str(rom.img_path.with_suffix(".bmp")))
        imgs.append(str(rom.img_path.with_suffix(".BMP")))
        imgs.append(str(rom.img_path.with_suffix(".Bmp")))

        for img in imgs:
            if Path(img).exists():
                write_covart(Path(img), rom.img_path, w, h, args.jpg_quality)
                break

        if not rom.img_path.exists():
            raise NoArtworkError

        print(f"INFO: Packing {rom.name} Cover> {rom.img_path} ...")
        subprocess.check_output(
            [
                prefix / "arm-none-eabi-objcopy",
                "--rename-section",
                ".data=.extflash_game_rom,alloc,load,readonly,data,contents",
                "-I",
                "binary",
                "-O",
                "elf32-littlearm",
                "-B",
                "armv7e-m",
                rom.img_path,
                rom.obj_img,
            ]
        )
        subprocess.check_output(
            [
                prefix / "arm-none-eabi-ar",
                "-cru",
                "build/roms.a",
                rom.obj_img,
            ]
        )
        template = "extern const uint8_t {name}[];\n"
        return template.format(name=rom.img_symbol)

    def generate_save_entry(self, name: str, save_size: int) -> str:
        return f'uint8_t {name}[{save_size}]  __attribute__((section (".saveflash"))) __attribute__((aligned(4096)));\n'

    def generate_cheat_entry(self, name: str, num: int, cheat_codes_and_descs: []) -> str:
        str = ""

        codes = "{%s}" % ",".join(f'"{c}"' for (c,d) in cheat_codes_and_descs)
        descs = "{%s}" % ",".join(f'NULL' if d is None else f'"{d}"' for (c,d) in cheat_codes_and_descs)
        number_of_codes = len(cheat_codes_and_descs)

        count_name = "%s%s_COUNT" % (name, num)
        code_array_name = "%sCODE_%s" % (name, num)
        desc_array_name = "%sDESC_%s" % (name, num)
        str += f'#if CHEAT_CODES == 1\n'
        str += f'const char* {code_array_name}[{number_of_codes}] = {codes};\n'
        str += f'const char* {desc_array_name}[{number_of_codes}] = {descs};\n'
        str += f'const int {count_name} = {number_of_codes};\n'
        str += f'#endif\n'

        return str

    def get_gameboy_save_size(self, file: Path):
        total_size = 4096
        file = Path(file)

        if file.suffix in COMPRESSIONS:
            file = file.with_suffix("")  # Remove compression suffix

        with open(file, "rb") as f:
            # cgb
            f.seek(0x143)
            cgb = ord(f.read(1))

            # 0x80 = Gameboy color but supports old gameboy
            # 0xc0 = Gameboy color only
            if cgb & 0x80 or cgb == 0xC0:
                # Bank 0 + 1-7 for gameboy color work ram
                total_size += 8 * 4096  # irl

                # Bank 0 + 1 for gameboy color video ram, 2*8KiB
                total_size += 4 * 4096  # vrl
            else:
                # Bank 0 + 1 for gameboy classic work ram
                total_size += 2 * 4096  # irl

                # Bank 0 only for gameboy classic video ram, 1*8KiB
                total_size += 2 * 4096  # vrl

            # Cartridge ram size
            f.seek(0x149)
            total_size += [1, 1, 1, 4, 16, 8][ord(f.read(1))] * 8 * 1024
            return total_size

        return 0

    def get_nes_save_size(self, file: Path):
        file = Path(file)

        if file.suffix in COMPRESSIONS:
            file = file.with_suffix("")  # Remove compression suffix

        total_size = int(subprocess.check_output([sys.executable, "./fceumm-go/nesmapper.py", "savesize", file]))
        return total_size

        return 0
    def _compress_rom(self, variable_name, rom, compress_gb_speed=False, compress=None):
        """This will create a compressed rom file next to the original rom."""
        global sms_reserved_flash_size
        if not (rom.publish):
            return
        if compress is None:
            compress = "lz4"

        if compress not in COMPRESSIONS:
            raise ValueError(f'Unknown compression method: "{compress}"')

        if compress[0] != ".":
            compress = "." + compress
        output_file = Path(str(rom.path) + compress)
        compress = COMPRESSIONS[compress]
        data = rom.read()

        if "nes_system" in variable_name:  # NES
            if rom.path.stat().st_size > MAX_COMPRESSED_NES_SIZE:
                print(
                    f"INFO: {rom.name} is too large to compress, skipping compression!"
                )
                return
            compressed_data = compress(data)
            output_file.write_bytes(compressed_data)
        elif "pce_system" in variable_name:  # PCE
            if rom.path.stat().st_size > MAX_COMPRESSED_PCE_SIZE:
                print(
                    f"INFO: {rom.name} is too large to compress, skipping compression!"
                )
                return
            compressed_data = compress(data)
            output_file.write_bytes(compressed_data)
        elif "msx_system" in variable_name:  # MSX
            if rom.path.stat().st_size > MAX_COMPRESSED_MSX_SIZE:
                print(
                    f"INFO: {rom.name} is too large to compress, skipping compression!"
                )
                return
            compressed_data = compress(data)
            output_file.write_bytes(compressed_data)
        elif "wsv_system" in variable_name:  # WSV
            if rom.path.stat().st_size > MAX_COMPRESSED_WSV_SIZE:
                print(
                    f"INFO: {rom.name} is too large to compress, skipping compression!"
                )
                return
            compressed_data = compress(data)
            output_file.write_bytes(compressed_data)
        elif "a7800_system" in variable_name:  # Atari 7800
            if rom.path.stat().st_size > MAX_COMPRESSED_A7800_SIZE:
                print(
                    f"INFO: {rom.name} is too large to compress, skipping compression!"
                )
                return
            compressed_data = compress(data)
            output_file.write_bytes(compressed_data)
        elif variable_name in ["col_system","sg1000_system"] :  # COL or SG
            if rom.path.stat().st_size > MAX_COMPRESSED_SG_COL_SIZE:
                print(
                    f"INFO: {rom.name} is too large to compress, skipping compression!"
                )
                return
            compressed_data = compress(data)
            output_file.write_bytes(compressed_data)

        elif variable_name in ["sms_system","gg_system","md_system"]:  # GG or SMS or MD

            BANK_SIZE = 128*1024
            banks = [data[i : i + BANK_SIZE] for i in range(0, len(data), BANK_SIZE)]
            compressed_banks = [compress(bank) for bank in banks]

            # add header + number of banks + banks(offset)
            output_data=[]
            output_data.append( b'SMS+')
            output_data.append(pack("<l", len(compressed_banks)))

            for compressed_bank in compressed_banks:
                output_data.append(pack("<l", len(compressed_bank)))

            # Reassemble all banks back into one file
            for compressed_bank in compressed_banks:
                output_data.append(compressed_bank)

            output_data = b"".join(output_data)

            output_file.write_bytes(output_data)
        elif "gb_system" in variable_name:  # GB/GBC
            BANK_SIZE = 16384
            banks = [data[i : i + BANK_SIZE] for i in range(0, len(data), BANK_SIZE)]
            compressed_banks = [compress(bank) for bank in banks]

            # For ROM having continous bank switching we can use 'partial' compression
            # a mix of comcompressed and uncompress
            # compress empty banks and the bigger compress ratio
            compress_its = [True] * len(banks)
            compress_its[0] = False  # keep bank0 uncompressed

            # START : ALTERNATIVE COMPRESSION STRATEGY
            if compress_gb_speed:
                # the larger banks only are compressed.
                # It shoul fit exactly in the cache reducing the SWAP cache feequency to 0.
                # any empty bank is compressed (=98bytes). considered never used by MBC.

                # Ths is the cache size used as a compression credit
                # TODO : can we the value from the linker ?
                compression_credit = 26
                compress_size = [len(bank) for bank in compressed_banks[1:]]

                # to keep empty banks compressed (size=98)
                compress_size = [i for i in compress_size if i > 98]

                ordered_size = sorted(compress_size)

                if compression_credit > len(ordered_size):
                    compression_credit = len(ordered_size) - 1

                compress_threshold = ordered_size[int(compression_credit)]

                for i, bank in enumerate(compressed_banks):
                    if len(bank) >= compress_threshold:
                        # Don't compress banks with poor compression
                        compress_its[i] = False
            # END : ALTERNATIVE COMPRESSION STRATEGY

            # Reassemble all banks back into one file
            output_banks = []
            for bank, compressed_bank, compress_it in zip(
                banks, compressed_banks, compress_its
            ):
                if compress_it:
                    output_banks.append(compressed_bank)
                else:
                    output_banks.append(compress(bank, level=DONT_COMPRESS))
            output_data = b"".join(output_banks)

            output_file.write_bytes(output_data)

    def _convert_dsk(self, variable_name, dsk, compress):
        """This will convert dsk image to cdk."""
        if not (dsk.publish):
            return
        if compress is None:
            compress="none"

        if "msx_system" in variable_name:  # MSX disk compression
            subprocess.check_output("python3 tools/dsk2lzma.py \""+str(dsk.path)+"\" "+compress, shell=True)

        if "amstrad_system" in variable_name:  # Amstrad disk compression
            subprocess.check_output("python3 tools/amdsk2lzma.py \""+str(dsk.path)+"\" "+compress, shell=True)

    def generate_system(
        self,
        file: str,
        system_name: str,
        variable_name: str,
        folder: str,
        extensions: List[str],
        save_prefix: str,
        romdefs: dict,
        cheat_codes_prefix: str,
        current_id: int,
        compress: str = None,
        compress_gb_speed: bool = False,
    ) -> int:
        import json;
        script_path = Path(__file__).parent
        json_file = script_path / "roms" / str(folder + ".json")
        print(json_file)
        if Path(json_file).exists():
            with open(json_file,'r') as load_f:
                try:
                    romdef = json.load(load_f)
                    load_f.close()
                    romdefs = romdef
                except: 
                    load_f.close()
 
        roms_raw = []
        for e in extensions:
            roms_raw += self.find_roms(system_name, folder, e, romdefs)

        roms_uncompressed = roms_raw

        def find_compressed_roms():
            if not compress:
                return []

            roms = []
            for e in extensions:
                roms += self.find_roms(system_name, folder, e + "." + compress, romdefs)
            return roms

        def find_disks():
            disks = self.find_roms(system_name, folder, "dsk", romdefs)
            # If a disk name ends with _no_save then it means that we shouldn't
            # allocate save space for this disk (to use with multi disks games
            # as they only need to get a save for the first disk)
            for disk in disks:
                suffix = "_no_save"
                if disk.name.endswith(suffix) :
                    disk.name = disk.name[:-len(suffix)]
                    disk.enable_save = False
            return disks

        def find_cdk_disks():
            disks = self.find_roms(system_name, folder, "cdk", romdefs)
            for disk in disks:
                suffix = "_no_save"
                if disk.name.endswith(suffix) :
                    disk.name = disk.name[:-len(suffix)]
                    disk.enable_save = False
            return disks

        def contains_rom_by_name(rom, roms):
            for r in roms:
                if r.name == rom.name:
                    return True
            return False

        cdk_disks = find_cdk_disks()

        disks_raw = [r for r in roms_raw if not contains_rom_by_name(r, cdk_disks)]
        disks_raw = [r for r in disks_raw if r.ext == "dsk"]

        if disks_raw:
            pbar = tqdm(disks_raw) if tqdm else disks_raw
            for r in pbar:
                if tqdm:
                    pbar.set_description(f"Converting: {system_name} / {r.name}")
                self._convert_dsk(
                    variable_name,
                    r,
                    compress)
            # Re-generate the cdk disks list
            cdk_disks = find_cdk_disks()
        #remove .dsk from list
        roms_raw = [r for r in roms_raw if not contains_rom_by_name(r, cdk_disks)]
        #add .cdk disks to list
        roms_raw.extend(cdk_disks)

        roms_compressed = find_compressed_roms()

        roms_raw = [r for r in roms_raw if not contains_rom_by_name(r, roms_compressed)]
        if roms_raw and compress is not None:
            pbar = tqdm(roms_raw) if tqdm else roms_raw
            for r in pbar:
                if tqdm:
                    pbar.set_description(f"Compressing: {system_name} / {r.name}")
                self._compress_rom(
                    variable_name,
                    r,
                    compress_gb_speed=compress_gb_speed,
                    compress=compress,
                )
            # Re-generate the compressed rom list
            roms_compressed = find_compressed_roms()

        # Create a list with all compressed roms and roms that
        # don't have a compressed counterpart.
        roms = roms_compressed[:]
        for r in roms_raw:
            if not contains_rom_by_name(r, roms_compressed):
                roms.append(r)

        for rom in roms:
            rom.rom_id = current_id
            current_id += 1

        system_save_size = 0
        total_save_size = 0
        total_rom_size = 0
        total_img_size = 0
        pubcount = 0
        for i, rom in enumerate(roms):
            if not (rom.publish):
                continue
            else:
               pubcount += 1

        save_size = SAVE_SIZES.get(folder, 0)
        romdefs.setdefault("_cover_width", 128)
        romdefs.setdefault("_cover_height", 96)
        cover_width = romdefs["_cover_width"]
        cover_height = romdefs["_cover_height"]
        cover_width = 180 if cover_width > 180 else 64 if cover_width < 64 else cover_width
        cover_height = 136 if cover_height > 136 else 64 if cover_height < 64 else cover_height

        img_max = cover_width * cover_height

        if img_max > 18600:
            print(f"Error: {system_name} Cover art image [width:{cover_width} height: {cover_height}] will overflow!")
            exit(-1)        

        with open(file, "w", encoding = args.codepage) as f:
            f.write(SYSTEM_PROTO_TEMPLATE.format(name=variable_name))

            for i, rom in enumerate(roms):
                if not (rom.publish):
                    continue
                if folder == "gb":
                    save_size = self.get_gameboy_save_size(rom.path)
                elif folder == "nes" and args.nofrendo == 0:
                    save_size = self.get_nes_save_size(rom.path)

                # Aligned
                aligned_size = 4 * 1024
                if rom.enable_save:
                    rom_save_size = (
                        (save_size + aligned_size - 1) // (aligned_size)
                    ) * aligned_size
                    total_save_size += rom_save_size
                    if system_save_size < rom_save_size:
                        system_save_size = rom_save_size
                total_rom_size += rom.size
                if (args.coverflow != 0) :
                    total_img_size += rom.img_size

                if (rom.embed):
                    f.write(self.generate_object_file((rom),system_name))
                if (args.coverflow != 0) :
                    try:
                        f.write(self.generate_img_object_file(rom, cover_width, cover_height))
                    except NoArtworkError:
                        pass
                if rom.enable_save:
                    f.write(self.generate_save_entry(save_prefix + str(i), save_size))

                cheat_codes_and_descs = rom.get_cheat_codes();
                if cheat_codes_prefix:
                    f.write(self.generate_cheat_entry(cheat_codes_prefix, i, cheat_codes_and_descs))

            rom_entries = self.generate_rom_entries(
                folder + "_roms", roms, save_prefix, variable_name, cheat_codes_prefix
            )
            f.write(rom_entries)

            f.write(
                SYSTEM_TEMPLATE.format(
                    name=variable_name,
                    system_name=system_name,
                    variable_name=folder + "_roms",
                    extension=folder,
                    cover_width=cover_width,
                    cover_height=cover_height,
                    roms_count=pubcount,
                )
            )

        larger_rom_size = 0
        for r in roms_uncompressed:
            if r.ext in ["gg","sms","md","gen","bin"]:
                if larger_rom_size < r.size: larger_rom_size = r.size
        return system_save_size, total_save_size, total_rom_size, total_img_size, current_id, larger_rom_size

    def write_if_changed(self, path: str, data: str):
        path = Path(path)
        old_data = None
        if path.exists():
            old_data = path.read_text()
        if data != old_data:
            path.write_text(data)

    def parse(self, args):
        larger_save_size = 0
        total_save_size = 0
        total_rom_size = 0
        sega_larger_rom_size = 0
        total_img_size = 0
        build_config = ""
        current_id = 0

        import json;
        script_path = Path(__file__).parent
        json_file = script_path / "roms" / "roms.json"
        if Path(json_file).exists():
            with open(json_file,'r') as load_f:
                try:
                    romdef = json.load(load_f)
                    load_f.close()
                except: 
                    romdef = {}
                    load_f.close()
        else :
            romdef = {}

        romdef.setdefault('gb', {})
        romdef.setdefault('nes', {})
        romdef.setdefault('nes_bios', {})
        romdef.setdefault('sms', {})
        romdef.setdefault('gg', {})
        romdef.setdefault('col', {})
        romdef.setdefault('sg', {})
        romdef.setdefault('pce', {})
        romdef.setdefault('gw', {})
        romdef.setdefault('md', {})
        romdef.setdefault('msx', {})
        romdef.setdefault('msx_bios', {})
        romdef.setdefault('wsv', {})
        romdef.setdefault('a7800', {})
        romdef.setdefault('amstrad', {})
        romdef.setdefault('zelda3', {
            'zelda3': {'embed': '0'},
            'zelda3_de': {'publish': '0'},
            'zelda3_fr': {'publish': '0'},
            'zelda3_fr-c': {'publish': '0'},
            'zelda3_en': {'publish': '0'},
            'zelda3_es': {'publish': '0'},
            'zelda3_pl': {'publish': '0'},
            'zelda3_pt': {'publish': '0'},
            'zelda3_nl': {'publish': '0'},
            'zelda3_sv': {'publish': '0'},
        })
        romdef.setdefault('smw', {'smw': {'embed': '0'}})
        romdef.setdefault('tama', {})

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/gb_roms.c",
            "Nintendo Gameboy",
            "gb_system",
            "gb",
            ["gb", "gbc"],
            "SAVE_GB_",
            romdef["gb"],
            None,
            current_id,
            args.compress,
            args.compress_gb_speed,
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_GB\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        # Delete NES bios/mappers.h file to recreate it
        mappers_file = "build/mappers.h"
        if os.path.isfile(mappers_file):
            os.remove(mappers_file)
        # Create empty file to prevent compilation crash
        mappers = open(mappers_file, 'w')
        mappers.close

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/nes_roms.c",
            "Nintendo Entertainment System",
            "nes_system",
            "nes",
            ["nes","fds","nsf"],
            "SAVE_NES_",
            romdef["nes"],
            "GG_NES_",
            current_id,
            args.compress,
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_NES\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        # NES FDS bios (only parse if there are some NES games)
        if rom_size > 0:
            system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
                "Core/Src/retro-go/nes_bios.c",
                "NES_BIOS",
                "nes_bios",
                "nes_bios",
                ["rom","nes"],
                "SAVE_NESB_",
                romdef["nes_bios"],
                None,
                current_id
            )
            total_save_size += save_size
            total_rom_size += rom_size
            total_img_size += img_size
        else:
            system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
                "Core/Src/retro-go/nes_bios.c",
                "NES_BIOS",
                "nes_bios",
                "nes_bios",
                ["fakeToGenerateEmtyC"],
                "SAVE_NESB_",
                romdef["nes_bios"],
                None,
                current_id
            )

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/sms_roms.c",
            "Sega Master System",
            "sms_system",
            "sms",
            ["sms"],
            "SAVE_SMS_",
            romdef["sms"],
            None,
            current_id,
        )
        if sega_larger_rom_size < larger_rom_size : sega_larger_rom_size = larger_rom_size

        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_SMS\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/gg_roms.c",
            "Sega Game Gear",
            "gg_system",
            "gg",
            ["gg"],
            "SAVE_GG_",
            romdef["gg"],
            None,
            current_id,
        )
        if sega_larger_rom_size < larger_rom_size : sega_larger_rom_size = larger_rom_size
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_GG\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/md_roms.c",
            "Sega Genesis",
            "md_system",
            "md",
            ["md","gen","bin"],
            "SAVE_MD_",
            romdef["md"],
            None,
            current_id,
        )
        if sega_larger_rom_size < larger_rom_size : sega_larger_rom_size = larger_rom_size
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_MD\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/col_roms.c",
            "Colecovision",
            "col_system",
            "col",
            ["col"],
            "SAVE_COL_",
            romdef["col"],
            None,
            current_id,
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_COL\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/sg1000_roms.c",
            "Sega SG-1000",
            "sg1000_system",
            "sg",
            ["sg"],
            "SAVE_SG1000_",
            romdef["sg"],
            None,
            current_id,
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_SG1000\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/pce_roms.c",
            "PC Engine",
            "pce_system",
            "pce",
            ["pce"],
            "SAVE_PCE_",
            romdef["pce"],
            "GG_PCE_",
            current_id,
            args.compress,
        )

        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_PCE\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/gw_roms.c",
            "Game & Watch",
            "gw_system",
            "gw",
            ["gw"],
            "SAVE_GW_",
            romdef["gw"],
            None,
            current_id,
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_GW\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/msx_roms.c",
            "MSX",
            "msx_system",
            "msx",
            ["rom","mx1","mx2","dsk"],
            "SAVE_MSX_",
            romdef["msx"],
            "MCF_MSX_",
            current_id,
            args.compress
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_MSX\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        #MSX bios (only parse if there are some MSX games)
        if rom_size > 0:
            #Check that required bios files are here and patch files if needed
            if parse_msx_bios_files():
                system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
                    "Core/Src/retro-go/msx_bios.c",
                    "MSX_BIOS",
                    "msx_bios",
                    "msx_bios",
                    ["rom"],
                    "SAVE_MSXB_",
                    romdef["msx_bios"],
                    None,
                    current_id
                )
                total_save_size += save_size
                total_rom_size += rom_size
                total_img_size += img_size
            else:
                exit(-1)
        else:
            system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
                "Core/Src/retro-go/msx_bios.c",
                "MSX_BIOS",
                "msx_bios",
                "msx_bios",
                ["fakeToGenerateEmtyC"],
                "SAVE_MSXB_",
                romdef["msx_bios"],
                None,
                current_id
            )

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/wsv_roms.c",
            "Watara Supervision",
            "wsv_system",
            "wsv",
            ["bin","sv"],
            "SAVE_WSV_",
            romdef["wsv"],
            None,
            current_id,
            args.compress
        )
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_WSV\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/a7800_roms.c",
            "Atari 7800",
            "a7800_system",
            "a7800",
            ["a78","bin"],
            "SAVE_A7800_",
            romdef["a7800"],
            None,
            current_id,
            args.compress
        )
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_A7800\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/amstrad_roms.c",
            "Amstrad CPC",
            "amstrad_system",
            "amstrad",
            ["dsk"],
            "SAVE_AMSTRAD_",
            romdef["amstrad"],
            None,
            current_id,
            args.compress
        )
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_AMSTRAD\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/zelda3_roms.c",
            "Zelda3",
            "zelda3_system",
            "zelda3",
            ["sfc"],
            "SAVE_ZELDA3_",
            romdef["zelda3"],
            None,
            current_id
        )
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_HOMEBREW_ZELDA3\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size
        if rom_size > 0:
            self.write_if_changed(
                "build/zelda3_extflash.ld",
                """
                . = ALIGN(4K);
                __extflash_zelda3_start__ = .;
                build/zelda3/ancilla.o (.text .text* .rodata .rodata*)
                build/zelda3/attract.o (.text .text* .rodata .rodata*)
                build/zelda3/audio.o (.text .text* .rodata .rodata*)
                build/zelda3/dungeon.o (.text .text* .rodata .rodata*)
                build/zelda3/ending.o (.text .text* .rodata .rodata*)
                build/zelda3/misc.o (.text .text* .rodata .rodata*)
                build/zelda3/overlord.o (.text .text* .rodata .rodata*)
                build/zelda3/opus_decoder_amalgam.o (.text .text* .rodata .rodata*)
                build/zelda3/util.o (.text .text* .rodata .rodata*)
                build/zelda3/select_file.o (.text .text* .rodata .rodata*)
                build/zelda3/tagalong.o (.text .text* .rodata .rodata*)
                __extflash_zelda3_end__ = .;
                """
            )
        else:
            self.write_if_changed(
                "build/zelda3_extflash.ld",""
            )

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/smw_roms.c",
            "SMW",
            "smw_system",
            "smw",
            ["sfc"],
            "SAVE_SMW_",
            romdef["smw"],
            None,
            current_id
        )
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_HOMEBREW_SMW\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size
        if rom_size > 0:
            self.write_if_changed(
                "build/smw_extflash.ld",
                """
                . = ALIGN(4K);
                __extflash_smw_start__ = .;
                build/smw/apu.o (.text .text* .rodata .rodata*)
                build/smw/cart.o (.text .text* .rodata .rodata*)
                build/smw/common_cpu_infra.o (.text .text* .rodata .rodata*)
                build/smw/common_rtl.o (.text .text* .rodata .rodata*)
                build/smw/config.o (.text .text* .rodata .rodata*)
                build/smw/cpu.o (.text .text* .rodata .rodata*)
                build/smw/lm.o (.text .text* .rodata .rodata*)
                build/smw/smw_cpu_infra.o (.text .text* .rodata .rodata*)
                build/smw/snes.o (.text .text* .rodata .rodata*)
                build/smw/spc.o (.text .text* .rodata .rodata*)
                build/smw/tracing.o (.text .text* .rodata .rodata*)
                build/smw/util.o (.text .text* .rodata .rodata*)
                __extflash_smw_end__ = .;
                """
            )
        else:
            self.write_if_changed(
                "build/smw_extflash.ld",""
            )

        system_save_size, save_size, rom_size, img_size, current_id, larger_rom_size = self.generate_system(
            "Core/Src/retro-go/tama_roms.c",
            "Tamagotchi",
            "tama_system",
            "tama",
            ["b"],
            "SAVE_TAMA_",
            romdef["tama"],
            None,
            current_id,
        )
        total_save_size += save_size
        total_rom_size += rom_size
        total_img_size += img_size
        build_config += "#define ENABLE_EMULATOR_TAMA\n" if rom_size > 0 else ""
        if system_save_size > larger_save_size : larger_save_size = system_save_size

        total_size = total_save_size + total_rom_size + total_img_size
        #total_size +=sega_larger_rom_size
        sega_larger_rom_size = 0

        if total_size == 0:
            print(
                "No roms found! Please add at least one rom to one of the the directories in roms/"
            )
            exit(-1)

        if args.verbose:
            print(
                f"Save data:\t{total_save_size} bytes\nROM data:\t{total_rom_size} bytes\nROMs Cache:\t{sega_larger_rom_size} bytes\n"
                f"Cover images:\t{total_img_size} bytes\n"
                f"Total:\t\t{total_size} / {args.flash_size} bytes (plus some metadata)."
            )
        if total_size > args.flash_size:
            print(f"Error: External flash will overflow! Need at least {total_size / 1024 / 1024 :.2f} MB")
            # Delete build/roms.a - the makefile will run parse_roms.py if this file is outdated or missing.
            try:
                Path("build/roms.a").unlink()
            except FileNotFoundError as e:
                pass
            exit(-1)

        build_config += "#define ROM_COUNT %d\n" % current_id
        build_config += "#define MAX_CHEAT_CODES %d\n" % MAX_CHEAT_CODES

        self.write_if_changed(
            "build/saveflash.ld", f"__SAVEFLASH_LENGTH__ = {total_save_size};\n"
        )
        if (args.off_saveflash == 1):
            self.write_if_changed(
                "build/offsaveflash.ld", f"__OFFSAVEFLASH_LENGTH__ = {larger_save_size};\n"
            )
        else:
            self.write_if_changed(
                "build/offsaveflash.ld", f"__OFFSAVEFLASH_LENGTH__ = 0;\n"
            )
        self.write_if_changed(
             "build/cacheflash.ld", f"__CACHEFLASH_LENGTH__ = {sega_larger_rom_size};\n")
        self.write_if_changed("build/config.h", build_config)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Import ROMs to the build environment")
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--flash-size",
        "-s",
        type=int,
        default=1024 * 1024,
        help="Size of external SPI flash in bytes.",
    )
    parser.add_argument(
        "--codepage",
        type=str,
        default="ansi",
        help="save file's code page.",
    )
    parser.add_argument(
        "--coverflow",
        type=int,
        default=0,
        help="set coverflow image file pack",
    )
    parser.add_argument(
        "--jpg_quality",
        type=int,
        default=90,
        help="skip convert cover art image jpg quality",
    )
    parser.add_argument(
        "--off_saveflash",
        type=int,
        default=0,
        help="set separate flash zone for off/on savestate",
    )
    compression_choices = [t for t in COMPRESSIONS if not t[0] == "."]
    parser.add_argument(
        "--compress",
        choices=compression_choices,
        type=str,
        default=None,
        help="Compression method. Defaults to no compression.",
    )
    parser.add_argument(
        "--compress_gb_speed",
        dest="compress_gb_speed",
        action="store_true",
        help="Apply only selective compression to gameboy banks. Only apply "
        "if bank decompression during switching is too slow.",
    )
    parser.add_argument(
        "--nofrendo",
        type=int,
        default=0,
        help="force nofrendo nes emulator instead of fceumm",
    )
    parser.add_argument(
        "--no-compress_gb_speed", dest="compress_gb_speed", action="store_false"
    )
    parser.set_defaults(compress_gb_speed=False)
    parser.add_argument("--no-save", dest="save", action="store_false")
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose prints",
    )
    args = parser.parse_args()
    
    if args.compress and "." + args.compress not in COMPRESSIONS:
        raise ValueError(f"Unknown compression method specified: {args.compress}")

    roms_path = Path("build/roms")
    roms_path.mkdir(mode=0o755, parents=True, exist_ok=True)

    # Check for zip and 7z files. If found, tell user to extract and delete them.
    zip_files = []
    zip_files.extend(Path("roms").glob("*/*.zip"))
    zip_files.extend(Path("roms").glob("*/*.7z"))
    if zip_files:
        print("\nzip and/or 7z rom files found. Please extract and delete them:")
        for f in zip_files:
            print(f"    {f}")
        print("")
        exit(-1)

    try:
        ROMParser().parse(args)
    except ImportError as e:
        print(e)
        print("Missing dependencies. Run:")
        print("    python -m pip install -r requirements.txt")
        exit(-1)
