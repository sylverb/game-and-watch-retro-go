import argparse
import hashlib
import logging
import lzma
import readline
from pathlib import Path
from time import sleep, time
from typing import Union, List
from concurrent.futures import ThreadPoolExecutor
from tqdm import tqdm
from datetime import datetime, timezone

from collections import namedtuple

from pyocd.core.helpers import ConnectHelper
from littlefs import LittleFS, LittleFSError


logging.getLogger('pyocd').setLevel(logging.WARNING)


# Variable to aid in profiling
t_wait = 0
sleep_duration = 0.05


def sha256(data):
    return hashlib.sha256(data).digest()


_EMPTY_HASH_DIGEST = sha256(b"")
Variable = namedtuple("Variable", ['address', 'size'])

# fmt: off
# These addresses are fixed via a carefully crafted linker script.
comm = {
    "framebuffer1":            Variable(0x2400_0000, 320 * 240 * 2),
    "framebuffer2":            Variable(0x2402_5800, 320 * 240 * 2),
    "boot_magic":              Variable(0x2000_0000, 4),
    "log_idx":                 Variable(0x2000_0008, 4),
    "logbuf":                  Variable(0x2000_000c, 4096),
    "lfs_cfg":                 Variable(0x2000_1010, 4),
}

# Communication Variables
comm["flashapp_comm"] = comm["framebuffer2"]

comm["flashapp_state"]           = last_variable = Variable(comm["flashapp_comm"].address, 4)
comm["program_status"]           = last_variable = Variable(last_variable.address + last_variable.size, 4)
comm["utc_timestamp"]            = last_variable = Variable(last_variable.address + last_variable.size, 4)
comm["program_chunk_idx"]        = last_variable = Variable(last_variable.address + last_variable.size, 4)
comm["program_chunk_count"]      = last_variable = Variable(last_variable.address + last_variable.size, 4)

contexts = [{} for i in range(2)]
for i in range(2):
    contexts[i]["size"]              = last_variable = Variable(last_variable.address + last_variable.size, 4)
    contexts[i]["address"]           = last_variable = Variable(last_variable.address + last_variable.size, 4)
    contexts[i]["erase"]             = last_variable = Variable(last_variable.address + last_variable.size, 4)
    contexts[i]["erase_bytes"]       = last_variable = Variable(last_variable.address + last_variable.size, 4)
    contexts[i]["decompressed_size"] = last_variable = Variable(last_variable.address + last_variable.size, 4)
    contexts[i]["expected_sha256"]   = last_variable = Variable(last_variable.address + last_variable.size, 32)
    contexts[i]["expected_sha256_decompressed"]   = last_variable = Variable(last_variable.address + last_variable.size, 32)

    # Don't ever directly use this, just here for alignment purposes
    contexts[i]["__buffer_ptr"]        = last_variable = Variable(last_variable.address + last_variable.size, 4)

    contexts[i]["ready"]             = last_variable = Variable(last_variable.address + last_variable.size, 4)

for i in range(2):
    contexts[i]["buffer"]            = last_variable = Variable(last_variable.address + last_variable.size, 256 << 10)

comm["active_context_index"] = last_variable = Variable(last_variable.address + last_variable.size, 4)
context_size = sum(x.size for x in contexts[i].values())
comm["active_context"] = last_variable = Variable(last_variable.address + last_variable.size, context_size)
comm["decompress_buffer"] = last_variable = Variable(last_variable.address + last_variable.size, 256 << 10)

# littlefs config struct elements
comm["lfs_cfg_context"]      = Variable(comm["lfs_cfg"].address + 0,  4)
comm["lfs_cfg_read"]         = Variable(comm["lfs_cfg"].address + 4,  4)
comm["lfs_cfg_prog"]         = Variable(comm["lfs_cfg"].address + 8,  4)
comm["lfs_cfg_erase"]        = Variable(comm["lfs_cfg"].address + 12, 4)
comm["lfs_cfg_sync"]         = Variable(comm["lfs_cfg"].address + 16, 4)
comm["lfs_cfg_read_size"]    = Variable(comm["lfs_cfg"].address + 20, 4)
comm["lfs_cfg_prog_size"]    = Variable(comm["lfs_cfg"].address + 24, 4)
comm["lfs_cfg_block_size"]   = Variable(comm["lfs_cfg"].address + 28, 4)
comm["lfs_cfg_block_count"]  = Variable(comm["lfs_cfg"].address + 32, 4)
# TODO: too lazy to add the other lfs_config attributes


_flashapp_state_enum_to_str = {
    0x00000000: "INIT",
    0x00000001: "IDLE",
    0x00000002: "START",
    0x00000003: "CHECK_HASH_RAM_NEXT",
    0x00000004: "CHECK_HASH_RAM",
    0x00000005: "DECOMPRESSING",
    0x00000006: "ERASE_NEXT",
    0x00000007: "ERASE",
    0x00000008: "PROGRAM_NEXT",
    0x00000009: "PROGRAM",
    0x0000000a: "CHECK_HASH_FLASH_NEXT",
    0x0000000b: "CHECK_HASH_FLASH",
    0x0000000c: "FINAL",
    0x0000000d: "ERROR",
}
_flashapp_state_str_to_enum = {v: k for k, v in _flashapp_state_enum_to_str.items()}

_flashapp_status_enum_to_str  = {
    0         : "BOOTING",
    0xbad00001: "BAD_HASH_RAM",
    0xbad00002: "BAD_HAS_FLASH",
    0xbad00003: "NOT_ALIGNED",
    0xcafe0000: "IDLE",
    0xcafe0001: "DONE",
    0xcafe0002: "BUSY",
}
_flashapp_status_str_to_enum = {v: k for k, v in _flashapp_status_enum_to_str.items()}

# fmt: on


##############
# Exceptions #
##############

class TimeoutError(Exception):
    """Some operation timed out."""


class DataError(Exception):
    """Some data was not as expected."""


class StateError(Exception):
    """On-device flashapp is in the ERROR state."""


###############
# Compression #
###############
def compress_lzma(data):
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

    return compressed_data[13:]


def compress_chunks(chunks: List[bytes], max_workers=2):
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(compress_lzma, chunk) for chunk in chunks]
        for future in futures:
            yield future.result()


############
# LittleFS #
############
class LfsDriverContext:
    def __init__(self, offset) -> None:
        validate_extflash_offset(offset)
        self.offset = offset
        self.cache = {}

    def read(self, cfg: 'LFSConfig', block: int, off: int, size: int) -> bytes:
        logging.getLogger(__name__).debug('LFS Read : Block: %d, Offset: %d, Size=%d' % (block, off, size))
        try:
            return bytes(self.cache[block][off:off+size])
        except KeyError:
            pass
        self.cache[block] = bytearray(extflash_read(self.offset + (block * cfg.block_size), size))
        return bytes(self.cache[block][off:off+size])

    def prog(self, cfg: 'LFSConfig', block: int, off: int, data: bytes) -> int:
        logging.getLogger(__name__).debug('LFS Prog : Block: %d, Offset: %d, Data=%r' % (block, off, data))

        # Update the local block if it has previosly been read
        try:
            barray = self.cache[block]
            barray[off:off + len(data)] = data
        except KeyError:
            pass

        extflash_write(self.offset + (block * cfg.block_size) + off, data, erase=False, blocking=True)
        return 0

    def erase(self, cfg: 'LFSConfig', block: int) -> int:
        logging.getLogger(__name__).debug('LFS Erase: Block: %d' % block)
        self.cache.pop(block, None)  # Remove the block from the cache
        extflash_erase(self.offset + (block * cfg.block_size), cfg.block_size)
        return 0

    def sync(self, cfg: 'LFSConfig') -> int:
        return 0


def timestamp_now() -> int:
    return int(round(datetime.now().replace(tzinfo=timezone.utc).timestamp()))


def timestamp_now_bytes() -> bytes:
    return timestamp_now().to_bytes(4, "little")


###########
# OpenOCD #
###########
def chunk_bytes(data, chunk_size):
    return [data[i:i+chunk_size] for i in range(0, len(data), chunk_size)]


def read_int(key: Union[str, Variable], signed: bool = False)-> int:
    if isinstance(key, str):
        args = comm[key]
    elif isinstance(key, Variable):
        args = key
    else:
        raise ValueError
    return int.from_bytes(target.read_memory_block8(*args), byteorder='little', signed=signed)


def disable_debug():
    """Disables the Debug block, reducing battery consumption."""
    target.write32(0x5C001004, 0x00000000)


def write_chunk_idx(idx: int) -> None:
    target.write32(comm["program_chunk_idx"].address, idx)


def write_chunk_count(count: int) -> None:
    target.write32(comm["program_chunk_count"].address, count)

def write_state(state: str) -> None:
    target.write32(comm["flashapp_state"].address, _flashapp_state_str_to_enum[state])


def extflash_erase(offset: int, size: int, whole_chip:bool = False, **kwargs) -> None:
    """Erase a range of data on extflash.

    On-device flashapp will round up to nearest minimum erase size.
    ``program_chunk_idx`` must externally be set.

    Parameters
    ----------
    offset: int
        Offset into extflash to erase.
    size: int
        Number of bytes to erase.
    whole_chip: bool
        If ``True``, ``size`` is ignored and the entire chip is erased.
        Defaults to ``False``.
    """
    validate_extflash_offset(offset)
    if size <= 0 and not whole_chip:
        raise ValueError(f"Size must be >0; 0 erases the entire chip.")

    context = get_context()
    wait_for("IDLE")
    target.halt()

    target.write32(context["address"].address, offset)
    target.write32(context["erase"].address, 1)       # Perform an erase at `program_address`
    target.write32(context["size"].address, 0)

    if whole_chip:
        target.write32(context["erase_bytes"].address, 0)    # Note: a 0 value erases the whole chip
    else:
        target.write32(context["erase_bytes"].address, size)

    target.write_memory_block8(context["expected_sha256"].address, _EMPTY_HASH_DIGEST)

    target.write32(context["ready"].address, 1)

    target.resume()
    wait_for_all_contexts_complete(**kwargs)


def extflash_read(offset: int, size: int) -> bytes:
    """Read data from extflash.

    Parameters
    ----------
    offset: int
        Offset into extflash to read.
    size: int
        Number of bytes to read.
    """
    validate_extflash_offset(offset)
    return bytes(target.read_memory_block8(0x9000_0000 + offset, size))


def extflash_write(offset:int, data: bytes, erase=True, blocking=False, decompressed_size=0, decompressed_hash=None) -> None:
    """Write data to extflash.

    Limited to RAM constraints (i.e. <256KB writes).

    ``program_chunk_idx`` must externally be set.

    Parameters
    ----------
    offset: int
        Offset into extflash to write.
    size: int
        Number of bytes to write.
    erase: bool
        Erases flash prior to write.
        Defaults to ``True``.
    compressed: int
        Size of decompressed data.
        0 if data has not been previously LZMA compressed.
    """
    validate_extflash_offset(offset)
    if not data:
        return
    if len(data) > (256 << 10):
        raise ValueError(f"Too large of data for a single write.")

    context = get_context()

    if blocking:
        wait_for("IDLE")
        target.halt()

    target.write32(context["address"].address, offset)
    target.write32(context["size"].address, len(data))

    if erase:
        target.write32(context["erase"].address, 1)       # Perform an erase at `program_address`

        if decompressed_size:
            target.write32(context["erase_bytes"].address, decompressed_size)
        else:
            target.write32(context["erase_bytes"].address, len(data))

    target.write32(context["decompressed_size"].address, decompressed_size)
    target.write_memory_block8(context["expected_sha256"].address, sha256(data))
    if decompressed_hash:
        target.write_memory_block8(context["expected_sha256_decompressed"].address, decompressed_hash)
    target.write_memory_block8(context["buffer"].address, data)

    target.write32(context["ready"].address, 1)

    if blocking:
        target.resume()
        wait_for_all_contexts_complete()


def read_logbuf():
    return bytes(target.read_memory_block8(*comm["logbuf"])[:read_int("log_idx")]).decode()


def start_flashapp():
    target.reset_and_halt()
    target.write32(comm["flashapp_state"].address, _flashapp_state_str_to_enum["INIT"])
    target.write32(comm["boot_magic"].address, 0xf1a5f1a5)  # Tell bootloader to boot into flashapp
    target.write32(comm["program_status"].address, 0)
    target.write32(comm["program_chunk_idx"].address, 1)  # Can be overwritten later
    target.write32(comm["program_chunk_count"].address, 100)  # Can be overwritten later
    target.resume()
    wait_for("IDLE")

    # Set game-and-watch RTC
    target.write32(comm["utc_timestamp"].address, timestamp_now())


def get_context(timeout=10):
    global t_wait
    t_start = time()
    t_deadline = time() + timeout
    while True:
        for context in contexts:
            if not read_int(context["ready"]):
                t_wait += (time() - t_start)
                return context
            if time() > t_deadline:
                raise TimeoutError

        sleep(sleep_duration)


def wait_for_all_contexts_complete(timeout=10):
    global t_wait
    t_start = time()
    t_deadline = time() + timeout
    for context in contexts:
        while read_int(context["ready"]):
            if time() > t_deadline:
                raise TimeoutError
            sleep(sleep_duration)
    t_wait += (time() - t_start)
    wait_for("IDLE", timeout = t_deadline - time())



def wait_for(status: str, timeout=10):
    """Block until the on-device status is matched."""
    global t_wait
    t_start = time()
    t_deadline = time() + timeout
    error_mask = 0xFFFF_0000

    while True:
        status_enum = read_int("program_status")
        status_str = _flashapp_status_enum_to_str.get(status_enum, "UNKNOWN")
        if status_str == status:
            break
        elif (status_enum & error_mask) == 0xbad0_0000:
            raise DataError(status_str)
        if time() > t_deadline:
            raise TimeoutError
        sleep(sleep_duration)

    t_wait += (time() - t_start)


def validate_extflash_offset(val):
    if val >= 0x9000_0000:
        raise ValueError(f"Provided extflash offset 0x{val:08X}, did you mean 0x{(val - 0x9000_0000):08X} ?")
    if val % 4096 != 0:
        raise ValueError(f"Extflash offset must be a multiple of 4096.")


################
# CLI Commands #
################

def flash(args, fs, block_size, block_count):
    """Flash a binary to the external flash."""
    validate_extflash_offset(args.address)
    data = args.file.read_bytes()
    chunk_size = contexts[0]["buffer"].size  # Assumes all contexts have same size buffer
    chunks = chunk_bytes(data, chunk_size)

    write_chunk_count(len(chunks));

    for i, (chunk, compressed_chunk) in tqdm(enumerate(zip(chunks, compress_chunks(chunks))), total=len(chunks)):
        if len(compressed_chunk) < len(chunk):
            decompressed_size = len(chunk)
            decompressed_hash = sha256(chunk)
            chunk = compressed_chunk
        else:
            decompressed_size = 0
            decompressed_hash = None
        write_chunk_idx(i + 1)
        extflash_write(args.address + (i * chunk_size), chunk,
                       decompressed_size=decompressed_size,
                       decompressed_hash=decompressed_hash,
                       )
    wait_for_all_contexts_complete()
    wait_for("IDLE")


def erase(args, fs, block_size, block_count):
    """Erase the entire external flash."""
    # Just setting an artibrarily long timeout
    extflash_erase(0, 0, whole_chip=True, timeout=10_000)


def _ls(fs, path):
    try:
        for element in fs.scandir(path):
            if element.type == 1:
                typ = "FILE"
            elif element.type == 2:
                typ = "DIR "
            else:
                typ = "UKWN"

            fullpath = f"{path}/{element.name}"
            try:
                time_val = int.from_bytes(fs.getattr(fullpath, "t"), byteorder='little')
                time_str = datetime.fromtimestamp(time_val, timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
            except LittleFSError:
                time_str = " " * 19

            print(f"{element.size:7}B {typ} {time_str} {element.name}")
    except LittleFSError as e:
        if e.code != -2:
            raise
        print(f"ls {path}: No such directory")


def ls(args, fs, block_size, block_count):
    if not args.interactive:
        _ls(fs, args.path)
        return

    print("Interactive mode. Press Ctrl-D to exit.")
    while True:
        try:
            path = input(">>> ")
        except EOFError:
            return
        _ls(fs, path)


def pull(args, fs, block_size, block_count):
    try:
        stat = fs.stat(args.gnw_path.as_posix())
    except LittleFSError as e:
        if e.code != -2:
            raise
        print(f"{args.gnw_path.as_posix()}: No such file or directory")
        return

    if stat.type == 1:  # file
        with fs.open(args.gnw_path.as_posix(), 'rb') as f:
            data = f.read()
        if args.local_path.is_dir():
            args.local_path = args.local_path / args.gnw_path.name
        args.local_path.write_bytes(data)
    elif stat.type == 2:  # dir
        if args.local_path.is_file():
            raise ValueError(f"Cannot backup directory \"{args.gnw_path.as_posix()}\" to file \"{args.local_path}\"")

        strip_root = not args.local_path.exists()
        for root, _, files in fs.walk(args.gnw_path.as_posix()):
            root = Path(root.lstrip("/"))
            for file in files:
                full_src_path = root / file

                if strip_root:
                    full_dst_path = args.local_path / Path(*full_src_path.parts[1:])
                else:
                    full_dst_path = args.local_path / full_src_path

                full_dst_path.parent.mkdir(exist_ok=True, parents=True)

                if args.verbose:
                    print(f"{full_src_path}  ->  {full_dst_path}")

                with fs.open(full_src_path.as_posix(), 'rb') as f:
                    data = f.read()

                full_dst_path.write_bytes(data)
    else:
        raise NotImplementedError(f"Unknown type: {stat.type}")


def push(args, fs, block_size, block_count):
    if not args.local_path.exists():
        raise ValueError(f"Local \"{args.local_path}\" does not exist.")

    if args.local_path.is_file():
        data = args.local_path.read_bytes()

        if args.verbose:
            print(f"{args.local_path}  ->  {args.gnw_path.as_posix()}")

        fs.makedirs(args.gnw_path.parent.as_posix(), exist_ok=True)
        with fs.open(args.gnw_path.as_posix(), "wb") as f:
            f.write(data)

        fs.setattr(args.gnw_path.as_posix(), 't', timestamp_now_bytes())
    else:
        # TODO: timestamp
        raise NotImplementedError


def main():
    commands = {}

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command")
    parser.add_argument("--no-disable-debug", action="store_true",
                        help="Don't disable the debug hw block after flashing.")
    parser.add_argument("--verbose", action="store_true")

    def add_command(handler):
        """Add a subcommand, like "flash"."""
        subparser = subparsers.add_parser(handler.__name__)
        commands[handler.__name__] = handler
        return subparser

    subparser = add_command(flash)
    subparser.add_argument("file", type=Path,
           help="binary file to flash")
    subparser.add_argument("address", type=lambda x: int(x,0),
           help="Offset into external flash")

    subparser = add_command(erase)

    subparser = add_command(ls)
    subparser.add_argument('path', nargs='?', type=str, default='')
    subparser.add_argument('--interactive', '-i', action="store_true")

    subparser = add_command(pull)
    subparser.add_argument("gnw_path", type=Path,
            help="Game-and-watch file or folder to copy to computer.")
    subparser.add_argument("local_path", type=Path,
            help="Local file or folder to copy data to.")

    subparser = add_command(push)
    subparser.add_argument("gnw_path", type=Path,
            help="Game-and-watch file or folder to write to.")
    subparser.add_argument("local_path", type=Path,
            help="Local file or folder to copy data from.")

    args = parser.parse_args()

    options = {
        "frequency": 5_000_000,
        "target_override": "STM32H7B0VBTx",
    }

    with ConnectHelper.session_with_chosen_probe(options=options) as session:
        global target
        board = session.board
        assert board is not None
        target = board.target

        start_flashapp()

        filesystem_offset = read_int("lfs_cfg_context") - 0x9000_0000
        block_size = read_int("lfs_cfg_block_size")
        block_count = read_int("lfs_cfg_block_count")

        if block_size==0 or block_count==0:
            raise DataError

        lfs_context = LfsDriverContext(filesystem_offset)
        fs = LittleFS(lfs_context, block_size=block_size, block_count=block_count)

        try:
            f = commands[args.command]
        except KeyError:
            print(f"Unknown command \"{args.command}\"")
            parser.print_help()
            exit(1)

        try:
            f(args, fs, block_size, block_count)
        finally:
            if not args.no_disable_debug:
                disable_debug()

            target.reset()


if __name__ == "__main__":
    main()
