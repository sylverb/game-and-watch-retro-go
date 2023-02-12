#!/usr/bin/env python3

import argparse
import struct
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import sys



def Paint_One(img, draw, fnt, character, xoffset, yoffset, half=False)->str:
    ret = ""
    draw.rectangle((0,0,16,16), fill='#000000')
    draw.fontmode = "1"
    draw.text((xoffset, yoffset), character, font=fnt, fill=(255,255,255))
    pixels = img.load()
    for y in range(12):
        b1 = 0
        b2 = 0
        s1 = ''
        s2 = ''
        for x in range(8):
            #print(x)
            pt = pixels[x, y]
            pt1 = pixels[x + 8, y]
            if ((pt[0] + pt[1] + pt[2]) >= 100):
                b1 = b1 | (0x80 >> x)
                s1 += 'O'
            else:
                s1 += '.' 
            if (x < 4):
                if ((pt1[0] + pt1[1] + pt1[2]) >= 100):
                    b2 = b2 | (0x80 >> x)
                    s2 += 'O'
                else:
                    s2 += '.'
        if half:
            ret += '  0x%02x'%(b1)  + ', //  ' + s1[:6] +'\n'
        else:
            ret += '  0x%02x'%(b1) + ',0x%02x'%(b2) + ', //  ' + s1 + s2 +'\n'
            #print(ret)
    return ret

def Paint_All(font_name, font_size:int, codepage:str, xoffset, yoffset, index):
    print("Process:" + font_name)
    #new image
    img = Image.new('RGB', (16, 16) , 0)
    outfnt = ImageFont.truetype(font_name, font_size, index)
    draw = ImageDraw.Draw(img)
    fn = Path(font_name).parent / ((Path(font_name)).stem + '_' + codepage + ".h")
    with open(fn, "w") as f:
        f.write("#pragma once\n\nconst char font_")
        f.write(codepage)
        f.write("[] __attribute__((section(\".extflash_font\"))) = {\n")
        cjklist = ['cp936', 'cp949']

        if (codepage in cjklist):
            for b1 in range(0xFE - 0xA1 + 1):
                for b2 in range (0xFE - 0xA1 + 1):
                    bb1 = b1 + 0xA1
                    bb2 = b2 + 0xA1
                    s = ''
                    chno = bytes([bb1, bb2])
                    s1 = ''
                    try:
                        s1 = chno.decode(codepage)
                        s = chno
                    except:
                        s1 = ''
                        s = 'NULL'
                    f.write('// ' + str(s) + ' 0x%02x'%(bb1) + '%02x'%(bb2) + '\n')
                    oneline = Paint_One(img, draw, outfnt, s1, xoffset, yoffset)
                    f.write(oneline)

        if (codepage == 'cp932'): #ja_jp
            for b1 in range(0xDF - 0xA1 + 1):
                bb1 = b1 + 0xA1
                s = ''
                chno = bytes([bb1])
                s1 = ''
                try:
                    s1 = chno.decode(codepage)
                    s = chno
                except:
                    s1 = ''
                    s = 'NULL'
                f.write('// ' + str(s) + ' 0x%02x'%(bb1) + '\n')
                oneline = Paint_One(img, draw, outfnt, s1, xoffset, yoffset, True)
                f.write(oneline)

            for b1 in range(0xEF - 0x81 + 1):
                for b2 in range (0xFC - 0x40 + 1):
                    bb1 = b1 + 0x81
                    bb2 = b2 + 0x40
                    if ((bb1 <= 0x9F) or (bb1 >= 0xe0)) and ((bb2 <= 0x7E) or (bb2 >= 0x80)):
                        s = ''
                        chno = bytes([bb1, bb2])
                        s1 = ''
                        try:
                            s1 = chno.decode(codepage)
                            s = chno
                        except:
                            s1 = ''
                            s = 'NULL'
                        f.write('// ' + str(s) + ' 0x%02x'%(bb1) + '%02x'%(bb2) + '\n')
                        oneline = Paint_One(img, draw, outfnt, s1, xoffset, yoffset)
                        f.write(oneline)

        if (codepage == 'cp950'): #zh_tw
            for b1 in range(0xF9 - 0xA1 + 1):
                for b2 in range (0xFE - 0x40 + 1):
                    bb1 = b1 + 0xA1
                    bb2 = b2 + 0x40
                    if (bb2 <= 0x7E) or (bb2 >= 0xA1):
                        s = ''
                        chno = bytes([bb1, bb2])
                        s1 = ''
                        try:
                            s1 = chno.decode(codepage)
                            s = chno
                        except:
                            s1 = ''
                            s = 'NULL'
                        f.write('// ' + str(s) + ' 0x%02x'%(bb1) + '%02x'%(bb2) + '\n')
                        oneline = Paint_One(img, draw, outfnt, s1, xoffset, yoffset)
                        f.write(oneline)
        f.write('};\n')

def main():
    if (len(sys.argv) > 1):
        #Paint_Fontogn(font_name, font_size, xoffset, yoffset):
        Paint_All(sys.argv[1], 
            int(sys.argv[2]) if (len(sys.argv) > 2) else 12, 
            str(sys.argv[3]) if (len(sys.argv) > 3) else "cp936",
            int(sys.argv[4]) if (len(sys.argv) > 4) else 0, 
            int(sys.argv[5]) if (len(sys.argv) > 5) else 0, 
            int(sys.argv[6]) if (len(sys.argv) > 6) else 0)
    else:
        print("Usage: " + sys.argv[0] + " fontfile [fontsize] [codepage] [xoffset] [yoffset] [index]")
        print("python support .ttf .bdf .pcf or more fontfile")

if __name__ == "__main__":
    main()