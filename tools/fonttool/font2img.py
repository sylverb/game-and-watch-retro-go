#!/usr/bin/env python3

import argparse
import struct
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import sys


def Paint_Fontogn(font_name, font_size:int, codepage:str, xoffset:int, yoffset:int):
    print("Process:" + font_name)
    out_size = 12
    h_s = out_size // 2
    s_w = out_size * 4
    s_h = out_size * 4
    #new image
    img = Image.new('RGB', (s_w * 16, s_h * 16) , 0)
    tipfnt = ImageFont.truetype("cour.ttf", out_size)
    outfnt = ImageFont.truetype(font_name, font_size)
    draw = ImageDraw.Draw(img)
    for x in range(16):
        for y in range(16):
            #paint grid ---
            draw.line((0, y * s_h, s_w * 16, y * s_h), width=1, fill="#404040")
            draw.line((0, y * s_h + out_size * 2 - 1, s_w * 16, y * s_h + out_size * 2 - 1), width=2, fill="#404040")
            draw.line((0, y * s_h + s_h - 1, s_w * 16, y * s_h + s_h - 1), width=1, fill="#404040")
            #paint grid |||
            draw.line((x * s_w, y * s_h, x * s_w, y * s_h + s_h - 1), width=1, fill="#404040")
            draw.line((x * s_w + out_size * 2 - 1, y * s_h + out_size * 2, x * s_w + out_size * 2 - 1, y * s_h + s_h - 1), width=2, fill="#404040")
            draw.line((x * s_w + s_w - 1, y * s_h, x * s_w + s_w - 1, y * s_h + s_h - 1), width=1, fill="#404040")
            #paint a fixed grid;
            draw.rectangle((x * s_w + out_size * 2 + h_s, y * s_h + out_size * 2 + h_s, x * s_w + out_size * 2 + h_s + out_size - 1, y * s_h + out_size * 2 + h_s + out_size - 1), outline="#101010")

            draw.text((x * s_w + h_s, y * s_h + h_s), "%03d"%(y*16+x), font=tipfnt, fill="#A0A0A0")
            #skip 0-31,128-159
            chrno = bytes([y*16+x])
            try:
                character = chrno.decode(codepage)
                draw.fontmode = "L"
                draw.text((x * s_w + h_s, y * s_h + out_size * 2 + h_s), character, font=tipfnt, fill=(120,120,120))
                draw.fontmode = "1"
                draw.text((x * s_w + h_s + out_size * 2 + xoffset, y * s_h + out_size * 2 + h_s + yoffset), character, font=outfnt, fill=(255,255,255))
            except:
                character = ""
                #print(y*16+x)
    png_file = Path(font_name).parent / ((Path(font_name)).stem + ".png")
    img.save(png_file, "png")


def main():
    if (len(sys.argv) > 1):
        #Paint_Fontogn(font_name, font_size, xoffset, yoffset):
        Paint_Fontogn(sys.argv[1], 
            int(sys.argv[2]) if (len(sys.argv) > 2) else 12, 
            str(sys.argv[3]) if (len(sys.argv) > 3) else "cp1252", 
            int(sys.argv[4]) if (len(sys.argv) > 4) else 0, 
            int(sys.argv[5]) if (len(sys.argv) > 5) else 0)
    else:
        print("Usage: " + sys.argv[0] + " fontfile [fontsize] [codepage] [xoffset] [yoffset]")
        print("put cour.ttf to this folder to use")
        print("python support .ttf .bdf .pcf or more fontfile")

if __name__ == "__main__":
    main()