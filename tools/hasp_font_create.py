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
size = 16
shift = int("0xE2000", base=0)

for (item, data) in fonts.items():
    for size in data["size"]:
        output = data["cpp"].format(size)
        chars = []
        for (desc, char) in data["chars"].items():
            chars.append(char)

        cmd = "lv_font_conv {} --bpp {} --size {} --font {} -r {} --font {} -r {} --format bin -o {} --format lvgl".format(
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

        with open(output, "r", encoding="utf8") as f:
            contents = f.readlines()

        contents[13] = contents[13].replace(" 1", " 0     // default to off")
        contents.insert(0, "/* clang-format off */\n")  # Add c-lang directive
              
        for idx,line in enumerate(contents):
            if "#if LV_VERSION_CHECK(7, 4, 0)" in line:
                contents[idx] = "#if LV_VERSION_CHECK(7, 4, 0) || LV_VERSION_CHECK(8, 0, 0)\r"
                print(line)

        with open(output, "w", encoding="utf8") as f:
            contents = "".join(contents)
            f.write(contents)
