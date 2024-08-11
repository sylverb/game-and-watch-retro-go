#!/usr/bin/env python3
import os
import sys
import hashlib
import xml.dom.minidom
import xml.etree.ElementTree as ET

ROM_UNKNOWN     = 0
ROM_STANDARD    = 1
ROM_MSXDOS2     = 2
ROM_KONAMI5     = 3
ROM_KONAMI4     = 4
ROM_ASCII8      = 5
ROM_ASCII16     = 6
ROM_GAMEMASTER2 = 7
ROM_ASCII8SRAM  = 8
ROM_ASCII16SRAM = 9
ROM_RTYPE       = 10
ROM_CROSSBLAIM  = 11
ROM_HARRYFOX    = 12
ROM_KOREAN80    = 13
ROM_KOREAN126   = 14
ROM_SCCEXTENDED = 15
ROM_FMPAC       = 16
ROM_KONAMI4NF   = 17
ROM_ASCII16NF   = 18
ROM_PLAIN       = 19
ROM_NORMAL      = 20
ROM_DISKPATCH   = 21
RAM_MAPPER      = 22
RAM_NORMAL      = 23
ROM_KANJI       = 24
ROM_HOLYQURAN   = 25
SRAM_MATSUCHITA = 26
ROM_PANASONIC16 = 27
ROM_BUNSETU     = 28
ROM_JISYO       = 29
ROM_KANJI12     = 30
ROM_NATIONAL    = 31
SRAM_S1985      = 32
ROM_F4DEVICE    = 33
ROM_F4INVERTED  = 34
ROM_KOEI        = 38
ROM_BASIC       = 39
ROM_HALNOTE     = 40
ROM_LODERUNNER  = 41
ROM_0x4000      = 42
ROM_PAC         = 43
ROM_MEGARAM     = 44
ROM_MEGARAM128  = 45
ROM_MEGARAM256  = 46
ROM_MEGARAM512  = 47
ROM_MEGARAM768  = 48
ROM_MEGARAM2M   = 49
ROM_MSXAUDIO    = 50
ROM_KOREAN90    = 51
ROM_SNATCHER    = 52
ROM_SDSNATCHER  = 53
ROM_SCCMIRRORED = 54
ROM_SCC         = 55
ROM_SCCPLUS     = 56
ROM_TC8566AF    = 57
ROM_S1990       = 58
ROM_TURBORTIMER = 59
ROM_TURBORPCM   = 60
ROM_KONAMISYNTH = 61
ROM_MAJUTSUSHI  = 62
ROM_MICROSOL    = 63
ROM_NATIONALFDC = 64
ROM_PHILIPSFDC  = 65
ROM_CASPATCH    = 66
ROM_SVI738FDC   = 67
ROM_PANASONIC32 = 68
ROM_EXTRAM      = 69
ROM_EXTRAM512KB = 70
ROM_EXTRAM1MB   = 71
ROM_EXTRAM2MB   = 72
ROM_EXTRAM4MB   = 73
ROM_SVI328CART  = 74
ROM_SVI328FDC   = 75
ROM_COLECO      = 76
ROM_SONYHBI55   = 77
ROM_MSXMUSIC    = 78
ROM_MOONSOUND   = 79
ROM_MSXAUDIODEV = 80
ROM_V9958       = 81
ROM_SVI328COL80 = 82
ROM_SVI328PRN   = 83
ROM_MSXPRN      = 84
ROM_SVI328RS232 = 85
ROM_0xC000      = 86
ROM_FMPAK       = 87
ROM_MSXMIDI     = 88
ROM_MSXRS232    = 89
ROM_TURBORIO    = 90
ROM_KONAMKBDMAS = 91
ROM_GAMEREADER  = 92
RAM_1KB_MIRRORED= 93
ROM_SG1000      = 94
ROM_SG1000CASTLE= 95
ROM_SUNRISEIDE  = 96
ROM_GIDE        = 97
ROM_BEERIDE     = 98
ROM_KONWORDPRO  = 99
ROM_MICROSOL80  = 100
ROM_NMS8280DIGI = 101
ROM_SONYHBIV1   = 102
ROM_SVI727COL80 = 103
ROM_FMDAS       = 104
ROM_YAMAHASFG05 = 105
ROM_YAMAHASFG01 = 106
ROM_SF7000IPL   = 107
ROM_SC3000      = 108
ROM_PLAYBALL    = 109
ROM_OBSONET     = 110
RAM_2KB_MIRRORED= 111
ROM_SEGABASIC   = 112
ROM_CVMEGACART  = 113
ROM_DUMAS       = 114
SRAM_MEGASCSI   = 115
SRAM_MEGASCSI128= 116
SRAM_MEGASCSI256= 117
SRAM_MEGASCSI512= 118
SRAM_MEGASCSI1MB= 119
SRAM_ESERAM     = 120
SRAM_ESERAM128  = 121
SRAM_ESERAM256  = 122
SRAM_ESERAM512  = 123
SRAM_ESERAM1MB  = 124
SRAM_ESESCC     = 125
SRAM_ESESCC128  = 126
SRAM_ESESCC256  = 127
SRAM_ESESCC512  = 128
SRAM_WAVESCSI   = 129
SRAM_WAVESCSI128= 130
SRAM_WAVESCSI256= 131
SRAM_WAVESCSI512= 132
SRAM_WAVESCSI1MB= 133
ROM_NOWIND      = 134
ROM_GOUDASCSI   = 135
ROM_MANBOW2     = 136
ROM_MEGAFLSHSCC = 137
ROM_FORTEII     = 138
ROM_PANASONIC8  = 139
ROM_FSA1FMMODEM = 140
ROM_DRAM        = 141
ROM_PANASONICWX16=142
ROM_TC8566AF_TR = 143
ROM_MATRAINK    = 144
ROM_NETTOUYAKYUU= 145
ROM_YAMAHANET   = 146
ROM_JOYREXPSG   = 147
ROM_OPCODEPSG   = 148
ROM_EXTRAM16KB  = 149
ROM_EXTRAM32KB  = 150
ROM_EXTRAM48KB  = 151
ROM_EXTRAM64KB  = 152
ROM_NMS1210     = 153
ROM_ARC         = 154
ROM_OPCODEBIOS  = 155
ROM_OPCODESLOT  = 156
ROM_OPCODESAVE  = 157
ROM_OPCODEMEGA  = 158
SRAM_MATSUCHITA_INV = 159
ROM_SVI328RSIDE = 160
ROM_ACTIVISIONPCB_2K = 161
ROM_SVI707FDC   = 162
ROM_MANBOW2_V2  = 163
ROM_HAMARAJANIGHT = 164
ROM_MEGAFLSHSCCPLUS = 165
ROM_DOOLY       = 166
ROM_SG1000_RAMEXPANDER_A = 167
ROM_SG1000_RAMEXPANDER_B = 168
ROM_MSXMIDI_EXTERNAL     = 169
ROM_MUPACK               = 170
ROM_ACTIVISIONPCB_256K = 171
ROM_ACTIVISIONPCB = 172
ROM_ACTIVISIONPCB_16K = 173
ROM_MAXROMID    = 173

def getMapperValue(name):
    mapper_dict = {
        "ASCII16": ROM_ASCII16,
        "ASCII16SRAM2": ROM_ASCII16SRAM,
        "ASCII8": ROM_ASCII8,
        "ASCII8SRAM8": ROM_ASCII8SRAM,
        "KoeiSRAM8": ROM_KOEI,
        "KoeiSRAM32": ROM_KOEI,
        "Konami": ROM_KONAMI4,
        "KonamiSCC": ROM_KONAMI5,
        "MuPack": ROM_MUPACK,
        "Manbow2": ROM_MANBOW2,
        "Manbow2v2": ROM_MANBOW2_V2,
        "HamarajaNight": ROM_HAMARAJANIGHT,
        "MegaFlashRomScc": ROM_MEGAFLSHSCC,
        "MegaFlashRomSccPlus": ROM_MEGAFLSHSCCPLUS,
        "Halnote": ROM_HALNOTE,
        "HarryFox": ROM_HARRYFOX,
        "Playball": ROM_PLAYBALL,
        "Dooly": ROM_DOOLY,
        "HolyQuran": ROM_HOLYQURAN,
        "CrossBlaim": ROM_CROSSBLAIM,
        "Zemina80in1": ROM_KOREAN80,
        "Zemina90in1": ROM_KOREAN90,
        "Zemina126in1": ROM_KOREAN126,
        "Wizardry": ROM_ASCII8SRAM,
        "GameMaster2": ROM_GAMEMASTER2,
        "SuperLodeRunner": ROM_LODERUNNER,
        "R-Type": ROM_RTYPE,
        "Majutsushi": ROM_MAJUTSUSHI,
        "Synthesizer": ROM_KONAMISYNTH,
        "KeyboardMaster": ROM_KONAMKBDMAS,
        "GenericKonami": ROM_KONAMI4NF,
        "SuperPierrot": ROM_ASCII16NF,
        "WordPro": ROM_KONWORDPRO,
        "Normal": ROM_STANDARD,
        "MatraInk": ROM_MATRAINK,
        "NettouYakyuu": ROM_NETTOUYAKYUU,
        "0x4000": ROM_0x4000,
        "0xC000": ROM_0xC000,
        "auto": ROM_PLAIN,
        "basic": ROM_BASIC,
        "mirrored": ROM_PLAIN,
        "forteII": ROM_FORTEII,
        "msxdos2": ROM_MSXDOS2,
        "konami5": ROM_KONAMI5,
        "konami4": ROM_KONAMI4,
        "ascii8": ROM_ASCII8,
        "halnote": ROM_HALNOTE,
        "konamisynth": ROM_KONAMISYNTH,
        "kbdmaster": ROM_KONAMKBDMAS,
        "majutsushi": ROM_MAJUTSUSHI,
        "ascii16": ROM_ASCII16,
        "gamemaster2": ROM_GAMEMASTER2,
        "ascii8sram": ROM_ASCII8SRAM,
        "koei": ROM_KOEI,
        "ascii16sram": ROM_ASCII16SRAM,
        "konami4nf": ROM_KONAMI4NF,
        "ascii16nf": ROM_ASCII16NF,
        "snatcher": ROM_SNATCHER,
        "sdsnatcher": ROM_SDSNATCHER,
        "sccmirrored": ROM_SCCMIRRORED,
        "sccexpanded": ROM_SCCEXTENDED,
        "scc": ROM_SCC,
        "sccplus": ROM_SCCPLUS,
        "scc-i": ROM_SCCPLUS,
        "scc+": ROM_SCCPLUS,
        "pac": ROM_PAC,
        "fmpac": ROM_FMPAC,
        "fmpak": ROM_FMPAK,
        "rtype": ROM_RTYPE,
        "crossblaim": ROM_CROSSBLAIM,
        "harryfox": ROM_HARRYFOX,
        "loderunner": ROM_LODERUNNER,
        "korean80": ROM_KOREAN80,
        "korean90": ROM_KOREAN90,
        "korean126": ROM_KOREAN126,
        "holyquran": ROM_HOLYQURAN,
        "opcodesave": ROM_OPCODESAVE,
        "opcodebios": ROM_OPCODEBIOS,
        "opcodeslot": ROM_OPCODESLOT,
        "opcodeega": ROM_OPCODEMEGA,
        "coleco": ROM_COLECO
    }
    return mapper_dict.get(name, ROM_UNKNOWN)

def getRomInfo(xml_file,sha1):
    mapper = ROM_UNKNOWN
    controls = "127"
    ctrl = "0" # Does the game require to press ctrl at boot ?

    context = ET.iterparse(xml_file, events=("end",))
    for event, elem in context:
        if elem.tag == "software":
            for dump in elem.findall('dump'):
                for rom in dump.findall('rom'):
                    hash = rom.find('hash').text
                    if hash == sha1string:
                        mapper = ROM_PLAIN
                        rom_type = rom.find('type')
                        if rom_type is not None:
                            mapper = getMapperValue(rom_type.text)
                            if mapper == ROM_STANDARD:
                                start = rom.find('start')
                                if start is not None:
                                    if start.text == "0x4000":
                                        mapper = ROM_0x4000
                                    elif start.text == "0x8000":
                                        mapper = ROM_BASIC
                                    elif start.text == "0xC000":
                                        mapper = ROM_0xC000
                        controls_elem = rom.find('controls')
                        if controls_elem is not None:
                            controls = controls_elem.text
                        else:
                            # Find controls in parent <software> element
                            controls_elem = elem.find('controls')
                            if controls_elem is not None:
                                controls = controls_elem.text
                for megarom in dump.findall('megarom'):
                    hash = megarom.find('hash').text
                    if sha1string == hash:
                        mapper = getMapperValue(megarom.find('type').text)
                        controls_elem = megarom.find('controls')
                        if controls_elem is not None:
                            controls = controls_elem.text
                        else:
                            # Find controls in parent <software> element
                            controls_elem = elem.find('controls')
                            if controls_elem is not None:
                                controls = controls_elem.text
                for disk in dump.findall('disk'):
                    hash = disk.find('hash').text
                    if sha1string == hash:
                        controls_elem = disk.find('controls')
                        if controls_elem is not None:
                            controls = controls_elem.text
                        else:
                            # Find controls in parent <software> element
                            controls_elem = elem.find('controls')
                            if controls_elem is not None:
                                controls = controls_elem.text

                        ctrl_elem = disk.find('ctrl')
                        if ctrl_elem is not None:
                            ctrl = ctrl_elem.text
                        else:
                            # Find ctrl in parent <software> element
                            ctrl_elem = elem.find('ctrl')
                            if ctrl_elem is not None:
                                ctrl = ctrl_elem.text

    return str(mapper) + "\n" + controls + "\n" + ctrl

BUF_SIZE = 65536  # lets read stuff in 64kb chunks!

n = len(sys.argv)

if n < 2:
    print("Usage :\nfindBlueMsxMapper.py database.xml file.rom \n")
    sys.exit(0)

sha1 = hashlib.sha1()

with open(sys.argv[2], 'rb') as f:
    while True:
        data = f.read(BUF_SIZE)
        if not data:
            break
        sha1.update(data)

sha1string = sha1.hexdigest()

print(getRomInfo(sys.argv[1],sha1string))
sys.exit(0)
