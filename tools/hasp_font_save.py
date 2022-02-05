#!/usr/bin/env python3.6

import argparse
from argparse import RawTextHelpFormatter
import os
import sys
import json
import jsmin
from jsmin import jsmin

# with open("src/custom/fonts.json") as f:
#     fonts = json.load(f)
#     print(fonts)

with open("src/font/encodings.json") as js_file:
    minified = jsmin(js_file.read())
    fonts = json.loads(minified)
#print(fonts)

with open("src/font/md-icons.json") as js_file:
    minified = jsmin(js_file.read())
    icons = json.loads(minified)
#print(icons)

symbol_list = []
symbol_names = []
for (obj,list) in icons["include"].items():
    for name in list:
        if name != "0":
            # print(name)
            code = icons["icons"][name]
            symbol_list.append('"{}"'.format(str(code)))
            symbol_names.append('"{}"'.format(str(name)))

symbol_list.sort()
symbol_names.sort()
symbols = ",".join(symbol_list)

print(",".join(symbol_names))
print("{} icons selected".format(len(symbol_list)))

compr = "--no-kerning"
bpp = 3
size = str(sys.argv[1])
shift = int("0xE2000", base=0)

for (item, data) in fonts.items():
        output = data["bin"].format(size)
        chars = []
        for (desc, char) in data["chars"].items():
            chars.append(char)

        cmd = "lv_font_conv {} --bpp {} --size {} --font {} -r {} --font {} -r {} --format bin -o {} --format bin".format(
            compr,
            bpp,
            size,
            data["textfont"],
            ",".join(chars),
            icons["iconfont"],
            symbols,
            output,
        )
        os.system(cmd)

