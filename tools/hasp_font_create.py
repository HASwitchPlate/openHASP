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

icon_list = []
icon_names = []
for (obj,list) in icons["include"].items():
    for name in list:
        if name != "0":
            # print(name)
            code = icons["icons"][name]
            icon_list.append('"{}"'.format(str(code)))
            icon_names.append('"{}"'.format(str(name)))

icon_list.sort()
icon_names.sort()
icon_symbols = ",".join(icon_list)

print(",".join(icon_names))
print("{} icons selected".format(len(icon_list)))

compr = "--no-kerning"
bpp = 3
shift = int("0xE2000", base=0)

for (item, data) in fonts.items():
    for size in data["size"]:
        output = data["cpp"].format(size)
        chars = []
        for (desc, char) in data["chars"].items():
            chars.append(char)
        symbols = []
        for (desc, symbol) in data["symbols"].items():
            symbols.append(symbol)

        if len(symbols)>0:
            symbol_flag = "--symbols"
        else:
            symbol_flag = ""

        cmd = "lv_font_conv {} --bpp {} --size {} --font {} -r {} {} {} --font {} -r {} -o {} --format lvgl".format(
            compr,
            bpp,
            size,
            data["textfont"],
            ",".join(chars),
            symbol_flag,
            "".join(symbols),
            icons["iconfont"],
            icon_symbols,
            output,
        )
        os.system(cmd)

        with open(output, "r", encoding="utf8") as f:
            contents = f.readlines()

        contents[13] = contents[13].replace(" 1", " 0     // default to off")
        contents.insert(0, "/* clang-format off */\n")  # Add c-lang directive

        with open(output, "w", encoding="utf8") as f:
            contents = "".join(contents)
            f.write(contents)
