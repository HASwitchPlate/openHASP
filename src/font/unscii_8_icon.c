#include "lvgl.h"

/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts:
 ******************************************************************************/

#ifndef UNSCII_8_ICON
#define UNSCII_8_ICON 1
#endif

#if UNSCII_8_ICON

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+21 "!" */
    0xf2,

    /* U+22 "\"" */
    0x99, 0x90,

    /* U+23 "#" */
    0x49, 0x2f, 0xd2, 0xfd, 0x24, 0x80,

    /* U+24 "$" */
    0x23, 0xe8, 0xe2, 0xf8, 0x80,

    /* U+25 "%" */
    0xc7, 0x21, 0x8, 0x4e, 0x30,

    /* U+26 "&" */
    0x62, 0x49, 0x18, 0x96, 0x27, 0x40,

    /* U+27 "'" */
    0x2a, 0x0,

    /* U+28 "(" */
    0x2a, 0x48, 0x88,

    /* U+29 ")" */
    0x88, 0x92, 0xa0,

    /* U+2A "*" */
    0x25, 0x5c, 0x47, 0x54, 0x80,

    /* U+2B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+2C "," */
    0x58,

    /* U+2D "-" */
    0xf8,

    /* U+2E "." */
    0x80,

    /* U+2F "/" */
    0x2, 0x8, 0x20, 0x82, 0x8, 0x20, 0x0,

    /* U+30 "0" */
    0x74, 0x67, 0x5c, 0xc5, 0xc0,

    /* U+31 "1" */
    0x23, 0x28, 0x42, 0x13, 0xe0,

    /* U+32 "2" */
    0x74, 0x42, 0x26, 0x43, 0xe0,

    /* U+33 "3" */
    0x74, 0x42, 0x60, 0xc5, 0xc0,

    /* U+34 "4" */
    0x11, 0x95, 0x2f, 0x88, 0x40,

    /* U+35 "5" */
    0xfc, 0x3c, 0x10, 0xc5, 0xc0,

    /* U+36 "6" */
    0x3a, 0x21, 0xe8, 0xc5, 0xc0,

    /* U+37 "7" */
    0xf8, 0x44, 0x44, 0x21, 0x0,

    /* U+38 "8" */
    0x74, 0x62, 0xe8, 0xc5, 0xc0,

    /* U+39 "9" */
    0x74, 0x62, 0xf0, 0x8b, 0x80,

    /* U+3A ":" */
    0x90,

    /* U+3B ";" */
    0x41, 0x60,

    /* U+3C "<" */
    0x12, 0x48, 0x42, 0x10,

    /* U+3D "=" */
    0xf8, 0x3e,

    /* U+3E ">" */
    0x84, 0x21, 0x24, 0x80,

    /* U+3F "?" */
    0x7a, 0x10, 0x84, 0x10, 0x1, 0x0,

    /* U+40 "@" */
    0x7a, 0x19, 0x6b, 0x9a, 0x7, 0x80,

    /* U+41 "A" */
    0x31, 0x28, 0x7f, 0x86, 0x18, 0x40,

    /* U+42 "B" */
    0xfa, 0x18, 0x7e, 0x86, 0x1f, 0x80,

    /* U+43 "C" */
    0x7a, 0x18, 0x20, 0x82, 0x17, 0x80,

    /* U+44 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x2f, 0x0,

    /* U+45 "E" */
    0xfe, 0x8, 0x3c, 0x82, 0xf, 0xc0,

    /* U+46 "F" */
    0xfe, 0x8, 0x3c, 0x82, 0x8, 0x0,

    /* U+47 "G" */
    0x7a, 0x18, 0x27, 0x86, 0x17, 0x80,

    /* U+48 "H" */
    0x86, 0x18, 0x7f, 0x86, 0x18, 0x40,

    /* U+49 "I" */
    0xe9, 0x24, 0xb8,

    /* U+4A "J" */
    0x8, 0x42, 0x10, 0xc5, 0xc0,

    /* U+4B "K" */
    0x86, 0x29, 0x38, 0x92, 0x28, 0x40,

    /* U+4C "L" */
    0x82, 0x8, 0x20, 0x82, 0xf, 0xc0,

    /* U+4D "M" */
    0x87, 0x3b, 0x61, 0x86, 0x18, 0x40,

    /* U+4E "N" */
    0x87, 0x1a, 0x65, 0x8e, 0x18, 0x40,

    /* U+4F "O" */
    0x7a, 0x18, 0x61, 0x86, 0x17, 0x80,

    /* U+50 "P" */
    0xfa, 0x18, 0x7e, 0x82, 0x8, 0x0,

    /* U+51 "Q" */
    0x7a, 0x18, 0x61, 0x96, 0x27, 0x40,

    /* U+52 "R" */
    0xfa, 0x18, 0x7e, 0x92, 0x28, 0x40,

    /* U+53 "S" */
    0x7a, 0x18, 0x1e, 0x6, 0x17, 0x80,

    /* U+54 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+55 "U" */
    0x86, 0x18, 0x61, 0x86, 0x17, 0x80,

    /* U+56 "V" */
    0x86, 0x18, 0x61, 0x85, 0x23, 0x0,

    /* U+57 "W" */
    0x86, 0x18, 0x61, 0xb7, 0x38, 0x40,

    /* U+58 "X" */
    0x86, 0x14, 0x8c, 0x4a, 0x18, 0x40,

    /* U+59 "Y" */
    0x8c, 0x62, 0xe2, 0x10, 0x80,

    /* U+5A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+5B "[" */
    0xf2, 0x49, 0x38,

    /* U+5C "\\" */
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,

    /* U+5D "]" */
    0xe4, 0x92, 0x78,

    /* U+5E "^" */
    0x22, 0xa2,

    /* U+5F "_" */
    0xf8,

    /* U+60 "`" */
    0x88, 0x80,

    /* U+61 "a" */
    0x70, 0x5f, 0x17, 0x80,

    /* U+62 "b" */
    0x84, 0x3d, 0x18, 0xc7, 0xc0,

    /* U+63 "c" */
    0x74, 0x61, 0x17, 0x0,

    /* U+64 "d" */
    0x8, 0x5f, 0x18, 0xc5, 0xe0,

    /* U+65 "e" */
    0x74, 0x7f, 0x7, 0x0,

    /* U+66 "f" */
    0x18, 0x92, 0x3e, 0x20, 0x82, 0x0,

    /* U+67 "g" */
    0x7c, 0x62, 0xf0, 0xb8,

    /* U+68 "h" */
    0x84, 0x3d, 0x18, 0xc6, 0x20,

    /* U+69 "i" */
    0x43, 0x24, 0xb8,

    /* U+6A "j" */
    0x10, 0x31, 0x11, 0x96,

    /* U+6B "k" */
    0x84, 0x23, 0x2e, 0x4a, 0x20,

    /* U+6C "l" */
    0xc9, 0x24, 0xb8,

    /* U+6D "m" */
    0xd5, 0x6b, 0x5a, 0x80,

    /* U+6E "n" */
    0xf4, 0x63, 0x18, 0x80,

    /* U+6F "o" */
    0x74, 0x63, 0x17, 0x0,

    /* U+70 "p" */
    0xf4, 0x63, 0xe8, 0x40,

    /* U+71 "q" */
    0x7c, 0x62, 0xf0, 0x84,

    /* U+72 "r" */
    0xbe, 0x21, 0x8, 0x0,

    /* U+73 "s" */
    0x7c, 0x1c, 0x1f, 0x0,

    /* U+74 "t" */
    0x42, 0x3c, 0x84, 0x24, 0xc0,

    /* U+75 "u" */
    0x8c, 0x63, 0x17, 0x0,

    /* U+76 "v" */
    0x8c, 0x62, 0xa2, 0x0,

    /* U+77 "w" */
    0x8d, 0x6b, 0x55, 0x0,

    /* U+78 "x" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+79 "y" */
    0x8c, 0x62, 0xf0, 0xb8,

    /* U+7A "z" */
    0xf8, 0x88, 0x8f, 0x80,

    /* U+7B "{" */
    0x34, 0x48, 0x44, 0x30,

    /* U+7C "|" */
    0xff,

    /* U+7D "}" */
    0xc2, 0x21, 0x22, 0xc0,

    /* U+7E "~" */
    0x45, 0x44,

    /* U+7F "" */
    0xc1, 0x42, 0xbd, 0x2c, 0x40, 0x81, 0x0,

    /* U+F002 "" */
    0xf, 0x80, 0x3f, 0xe0, 0x78, 0xf0, 0x70, 0x70, 0xe0, 0x38, 0xe0, 0x38, 0xe0, 0x38, 0xe0, 0x38, 0xe0, 0x38, 0x70,
    0x70, 0x78, 0xf0, 0x3f, 0xf8, 0xf, 0xbc, 0x0, 0x1e, 0x0, 0xf, 0x0, 0x6,

    /* U+F00C "" */
    0x0, 0x6, 0x0, 0xf, 0x0, 0x1f, 0x0, 0x3e, 0x60, 0x7c, 0xf0, 0xf8, 0xf9, 0xf0, 0x7f, 0xe0, 0x3f, 0xc0, 0x1f, 0x80,
    0xf, 0x0, 0x6, 0x0,

    /* U+F00D "" */
    0x60, 0xde, 0x3f, 0xef, 0xbf, 0xe3, 0xf8, 0x3e, 0xf, 0xe3, 0xfe, 0xfb, 0xfe, 0x3d, 0x83, 0x0,

    /* U+F023 "" */
    0x7, 0x80, 0x3f, 0x1, 0xce, 0xe, 0x1c, 0x38, 0x70, 0xe1, 0xc3, 0x87, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    /* U+F053 "" */
    0x3, 0x7, 0xe, 0x1c, 0x38, 0x70, 0xe0, 0x70, 0x38, 0x1c, 0xe, 0x7, 0x3,

    /* U+F054 "" */
    0xc0, 0xe0, 0x70, 0x38, 0x1c, 0xe, 0x7, 0xe, 0x1c, 0x38, 0x70, 0xe0, 0xc0,

    /* U+F0C1 "" */
    0x0, 0x78, 0x1, 0xfe, 0x3, 0xce, 0x7, 0x87, 0xf, 0x7, 0xe, 0x7, 0xe, 0xef, 0x2f, 0xee, 0x77, 0xfc, 0xf7, 0x70, 0xe0,
    0x70, 0xe0, 0xf0, 0xe1, 0xe0, 0x73, 0xc0, 0x7f, 0x80, 0x1e, 0x0,

    /* U+F1EB "" */
    0x3, 0xfc, 0x0, 0xff, 0xf0, 0x3f, 0xff, 0xc7, 0x80, 0x1e, 0xe0, 0x0, 0x74, 0x1f, 0x82, 0xf, 0xff, 0x0, 0xe0, 0x70,
    0x4, 0x2, 0x0, 0x0, 0x0, 0x0, 0x60, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0x6, 0x0,

    /* U+F55A "" */
    0x7, 0xff, 0xe0, 0xff, 0xff, 0x1f, 0xff, 0xf3, 0xf9, 0x9f, 0x7f, 0x81, 0xff, 0xfc, 0x3f, 0xff, 0xc3, 0xf7, 0xf8,
    0x1f, 0x3f, 0x99, 0xf1, 0xff, 0xff, 0xf, 0xff, 0xf0, 0x7f, 0xfe,

    /* U+F8A2 "" */
    0x0, 0x0, 0x80, 0x0, 0xc2, 0x0, 0xe3, 0x80, 0x73, 0xc0, 0x3b, 0xff, 0xfd, 0xff, 0xfe, 0x78, 0x0, 0x1c, 0x0, 0x4,
    0x0, 0x0};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 13},
    {.bitmap_index = 1, .adv_w = 128, .box_w = 1, .box_h = 7, .ofs_x = 3, .ofs_y = 6},
    {.bitmap_index = 2, .adv_w = 128, .box_w = 4, .box_h = 3, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 4, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 10, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 15, .adv_w = 128, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 20, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 26, .adv_w = 128, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 28, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 31, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 34, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 39, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 43, .adv_w = 128, .box_w = 2, .box_h = 3, .ofs_x = 3, .ofs_y = 5},
    {.bitmap_index = 44, .adv_w = 128, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 45, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 3, .ofs_y = 6},
    {.bitmap_index = 46, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 53, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 58, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 63, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 68, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 73, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 78, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 83, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 88, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 93, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 98, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 103, .adv_w = 128, .box_w = 1, .box_h = 4, .ofs_x = 3, .ofs_y = 7},
    {.bitmap_index = 104, .adv_w = 128, .box_w = 2, .box_h = 6, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 106, .adv_w = 128, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 110, .adv_w = 128, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 112, .adv_w = 128, .box_w = 4, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 116, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 122, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 128, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 134, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 140, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 146, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 152, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 158, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 164, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 170, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 176, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 179, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 184, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 190, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 196, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 202, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 208, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 214, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 220, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 226, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 232, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 238, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 243, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 249, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 255, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 261, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 267, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 272, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 277, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 280, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 287, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 290, .adv_w = 128, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 292, .adv_w = 128, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 293, .adv_w = 128, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 295, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 299, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 304, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 308, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 313, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 317, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 323, .adv_w = 128, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 327, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 332, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 335, .adv_w = 128, .box_w = 4, .box_h = 8, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 339, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 344, .adv_w = 128, .box_w = 3, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 347, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 351, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 355, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 359, .adv_w = 128, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 363, .adv_w = 128, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 367, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 371, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 375, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 380, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 384, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 388, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 392, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 396, .adv_w = 128, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 400, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 404, .adv_w = 128, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 408, .adv_w = 128, .box_w = 1, .box_h = 8, .ofs_x = 3, .ofs_y = 5},
    {.bitmap_index = 409, .adv_w = 128, .box_w = 4, .box_h = 7, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 413, .adv_w = 128, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 415, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 422, .adv_w = 256, .box_w = 16, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 454, .adv_w = 256, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 176, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 224, .box_w = 14, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 522, .adv_w = 160, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 535, .adv_w = 160, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 548, .adv_w = 256, .box_w = 16, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 580, .adv_w = 320, .box_w = 20, .box_h = 14, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 615, .adv_w = 320, .box_w = 20, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 258, .box_w = 17, .box_h = 10, .ofs_x = -1, .ofs_y = 1}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {0x0, 0xa, 0xb, 0x21, 0x51, 0x52, 0xbf, 0x1e9, 0x558, 0x8a0};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = {{.range_start       = 32,
                                                .range_length      = 96,
                                                .glyph_id_start    = 1,
                                                .unicode_list      = NULL,
                                                .glyph_id_ofs_list = NULL,
                                                .list_length       = 0,
                                                .type              = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY},
                                               {.range_start       = 61442,
                                                .range_length      = 2209,
                                                .glyph_id_start    = 97,
                                                .unicode_list      = unicode_list_1,
                                                .glyph_id_ofs_list = NULL,
                                                .list_length       = 10,
                                                .type              = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {.glyph_bitmap  = gylph_bitmap,
                                         .glyph_dsc     = glyph_dsc,
                                         .cmaps         = cmaps,
                                         .kern_dsc      = NULL,
                                         .kern_scale    = 0,
                                         .cmap_num      = 2,
                                         .bpp           = 1,
                                         .kern_classes  = 0,
                                         .bitmap_format = 0};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t unscii_8_icon = {
    .get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height      = 16,                            /*The maximum line height required by the font*/
    .base_line        = 0,                             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
    .dsc = &font_dsc /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};

#endif /*#if UNSCII_8_ICON*/
