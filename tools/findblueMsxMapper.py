#!/usr/bin/env python3
import os
import sys
import hashlib
import xml.dom.minidom

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
    if (name == "ASCII16"):          return ROM_ASCII16
    if (name == "ASCII16SRAM2"):     return ROM_ASCII16SRAM
    if (name == "ASCII8"):           return ROM_ASCII8
    if (name == "ASCII8SRAM8"):      return ROM_ASCII8SRAM
    if (name == "KoeiSRAM8"):        return ROM_KOEI
    if (name == "KoeiSRAM32"):       return ROM_KOEI
    if (name == "Konami"):           return ROM_KONAMI4
    if (name == "KonamiSCC"):        return ROM_KONAMI5
    if (name == "MuPack"):           return ROM_MUPACK
    if (name == "Manbow2"):          return ROM_MANBOW2
    if (name == "Manbow2v2"):        return ROM_MANBOW2_V2
    if (name == "HamarajaNight"):    return ROM_HAMARAJANIGHT
    if (name == "MegaFlashRomScc"):  return ROM_MEGAFLSHSCC
    if (name == "MegaFlashRomSccPlus"): return ROM_MEGAFLSHSCCPLUS
    if (name == "Halnote"):          return ROM_HALNOTE
    if (name == "HarryFox"):         return ROM_HARRYFOX
    if (name == "Playball"):         return ROM_PLAYBALL
    if (name == "Dooly"):            return ROM_DOOLY
    if (name == "HolyQuran"):        return ROM_HOLYQURAN
    if (name == "CrossBlaim"):       return ROM_CROSSBLAIM
    if (name == "Zemina80in1"):      return ROM_KOREAN80
    if (name == "Zemina90in1"):      return ROM_KOREAN90
    if (name == "Zemina126in1"):     return ROM_KOREAN126
    if (name == "Wizardry"):         return ROM_ASCII8SRAM
    if (name == "GameMaster2"):      return ROM_GAMEMASTER2
    if (name == "SuperLodeRunner"):  return ROM_LODERUNNER
    if (name == "R-Type"):           return ROM_RTYPE
    if (name == "Majutsushi"):       return ROM_MAJUTSUSHI
    if (name == "Synthesizer"):      return ROM_KONAMISYNTH
    if (name == "KeyboardMaster"):   return ROM_KONAMKBDMAS
    if (name == "GenericKonami"):    return ROM_KONAMI4NF
    if (name == "SuperPierrot"):     return ROM_ASCII16NF
    if (name == "WordPro"):          return ROM_KONWORDPRO
    if (name == "Normal"):           return ROM_STANDARD
    if (name == "MatraInk"):         return ROM_MATRAINK
    if (name == "NettouYakyuu"):     return ROM_NETTOUYAKYUU
    if (name == "0x4000"):       return ROM_0x4000
    if (name == "0xC000"):       return ROM_0xC000
    if (name == "auto"):         return ROM_PLAIN
    if (name == "basic"):        return ROM_BASIC
    if (name == "mirrored"):     return ROM_PLAIN
    if (name == "forteII"):      return ROM_FORTEII
    if (name == "msxdos2"):      return ROM_MSXDOS2
    if (name == "konami5"):      return ROM_KONAMI5
    if (name == "MuPack"):       return ROM_MUPACK
    if (name == "konami4"):      return ROM_KONAMI4
    if (name == "ascii8"):       return ROM_ASCII8
    if (name == "halnote"):      return ROM_HALNOTE
    if (name == "konamisynth"):  return ROM_KONAMISYNTH
    if (name == "kbdmaster"):    return ROM_KONAMKBDMAS
    if (name == "majutsushi"):   return ROM_MAJUTSUSHI
    if (name == "ascii16"):      return ROM_ASCII16
    if (name == "gamemaster2"):  return ROM_GAMEMASTER2
    if (name == "ascii8sram"):   return ROM_ASCII8SRAM
    if (name == "koei"):         return ROM_KOEI
    if (name == "ascii16sram"):  return ROM_ASCII16SRAM
    if (name == "konami4nf"):    return ROM_KONAMI4NF
    if (name == "ascii16nf"):    return ROM_ASCII16NF
    if (name == "snatcher"):     return ROM_SNATCHER
    if (name == "sdsnatcher"):   return ROM_SDSNATCHER
    if (name == "sccmirrored"):  return ROM_SCCMIRRORED
    if (name == "sccexpanded"):  return ROM_SCCEXTENDED
    if (name == "scc"):          return ROM_SCC
    if (name == "sccplus"):      return ROM_SCCPLUS
    if (name == "scc-i"):        return ROM_SCCPLUS
    if (name == "scc+"):         return ROM_SCCPLUS
    if (name == "pac"):          return ROM_PAC
    if (name == "fmpac"):        return ROM_FMPAC
    if (name == "fmpak"):        return ROM_FMPAK
    if (name == "rtype"):        return ROM_RTYPE
    if (name == "crossblaim"):   return ROM_CROSSBLAIM
    if (name == "harryfox"):     return ROM_HARRYFOX
    if (name == "loderunner"):   return ROM_LODERUNNER
    if (name == "korean80"):     return ROM_KOREAN80
    if (name == "korean90"):     return ROM_KOREAN90
    if (name == "korean126"):    return ROM_KOREAN126
    if (name == "holyquran"):    return ROM_HOLYQURAN  
    if (name == "opcodesave"):   return ROM_OPCODESAVE
    if (name == "opcodebios"):   return ROM_OPCODEBIOS
    if (name == "opcodeslot"):   return ROM_OPCODESLOT
    if (name == "opcodeega"):    return ROM_OPCODEMEGA
    if (name == "coleco"):       return ROM_COLECO
    return ROM_UNKNOWN

def getRomMapper(collection,sha1):
    for software in collection.getElementsByTagName("software"):
        system = software.getElementsByTagName('system')[0]
        for dump in software.getElementsByTagName('dump'):
            for rom in dump.getElementsByTagName('rom'):
                hash = rom.getElementsByTagName('hash')[0].childNodes[0].data
                mapper = ROM_PLAIN
                if (hash == sha1string):
                    if (rom.getElementsByTagName('type')):
                        mapper = getMapperValue(rom.getElementsByTagName('type')[0].childNodes[0].data)
                        if (mapper == ROM_STANDARD):
                            if rom.getElementsByTagName('start') :
                                start = rom.getElementsByTagName('start')[0].childNodes[0].data
                                if (start == "0x4000"):
                                    mapper = ROM_0x4000
                                elif (start == "0x8000"):
                                    mapper = ROM_BASIC
                                elif (start == "0xC000"):
                                    mapper = ROM_0xC000
                    return mapper
            for megarom in dump.getElementsByTagName('megarom'):
                hash = megarom.getElementsByTagName('hash')[0].childNodes[0].data
                if sha1string == hash:
                    type = megarom.getElementsByTagName('type')[0].childNodes[0].data
                    return getMapperValue(type)
            for disk in dump.getElementsByTagName('disk'):
                hash = disk.getElementsByTagName('hash')[0].childNodes[0].data
                if sha1string == hash:
                    if disk.getElementsByTagName('type') :
                        type = disk.getElementsByTagName('type')[0].childNodes[0].data
                        return getMapperValue(type)
    return ROM_UNKNOWN

BUF_SIZE = 65536  # lets read stuff in 64kb chunks!

n = len(sys.argv)

if n < 2:
    print("Usage :\nrfindBlueMsxMapper.py database.xml file.rom \n")
    sys.exit(0)

# Open file
#print("Opening "+sys.argv[2])
sha1 = hashlib.sha1()

with open(sys.argv[2], 'rb') as f:
    while True:
        data = f.read(BUF_SIZE)
        if not data:
            break
        sha1.update(data)

sha1string = sha1.hexdigest()
#print("Rom SHA1: %s" % sha1string)

DOMTree = xml.dom.minidom.parse(sys.argv[1])
collection = DOMTree.documentElement

print(getRomMapper(collection,sha1string))
sys.exit(0)

#tree = ElementTree.parse(sys.argv[1])
#root = tree.getroot()

#def print_subtree(subtree):
#    for y in subtree:
#        print ("\t", y.tag, ":", y.text)

#for x in root.findall('.//software'):
#    print (x.tag)
#    for dump in x.getchildren().findall('.//dump'):
#        print (dump.tag)    
#    print_subtree(x.getchildren())

#for p in root.findall('.//software'):
#    print("%s | %s" % (p.find('Name').text, p.find('Value').text))