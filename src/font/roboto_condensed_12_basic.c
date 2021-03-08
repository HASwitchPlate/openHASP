#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*******************************************************************************
 * Size: 12 px
 * Bpp: 2
 * Opts: 0x020,0x021,0x025,0x028-0x03A,0x03C-0x05B,0x05D,0x05F,0x061-0x07A,0x07C,0x0B0,0x0B1,0x0B5,0x0B7,0x0BC-0x0BE
 ******************************************************************************/

#ifndef ROBOTO_CONDENSED_12_BASIC
#define ROBOTO_CONDENSED_12_BASIC 1
#endif

#if ROBOTO_CONDENSED_12_BASIC

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */

    /* U+21 "!" */
    0x30, 0xc3, 0xc, 0x30, 0xc2, 0x0, 0x30,

    /* U+25 "%" */
    0x2c, 0x0, 0x52, 0x20, 0x66, 0x90, 0x28, 0x80,
    0x2, 0x0, 0x2, 0x74, 0x8, 0xcc, 0x8, 0xcc,
    0x0, 0x78,

    /* U+28 "(" */
    0x0, 0x9, 0x8, 0x24, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x14, 0xc, 0x5,

    /* U+29 ")" */
    0x1, 0x43, 0x9, 0x18, 0x30, 0xc3, 0xc, 0x31,
    0x88, 0x21, 0x0,

    /* U+2A "*" */
    0xc, 0x13, 0x17, 0xf8, 0x78, 0x33, 0x0, 0x0,

    /* U+2B "+" */
    0x5, 0x0, 0xa0, 0xa, 0x7, 0xfd, 0xa, 0x0,
    0xa0, 0xa, 0x0,

    /* U+2C "," */
    0x76, 0x90,

    /* U+2D "-" */
    0xb8,

    /* U+2E "." */
    0x10, 0xc0,

    /* U+2F "/" */
    0x3, 0x1, 0x80, 0x90, 0x30, 0x8, 0x5, 0x3,
    0x0, 0xc0, 0x60, 0x20, 0x0,

    /* U+30 "0" */
    0x1f, 0x43, 0xc, 0x30, 0xc6, 0xc, 0x60, 0xd6,
    0xc, 0x30, 0xc3, 0xc, 0x1f, 0x40,

    /* U+31 "1" */
    0x6, 0x3b, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
    0x7,

    /* U+32 "2" */
    0x1f, 0x43, 0xc, 0x50, 0xc0, 0xc, 0x2, 0x40,
    0x70, 0xc, 0x3, 0x40, 0x7f, 0xd0,

    /* U+33 "3" */
    0x1f, 0x43, 0x1c, 0x10, 0xc0, 0x1c, 0xf, 0x40,
    0x1c, 0x10, 0xc3, 0xc, 0x2f, 0x40,

    /* U+34 "4" */
    0x2, 0x80, 0x38, 0xa, 0x80, 0x98, 0x21, 0x86,
    0x18, 0xbf, 0xe0, 0x18, 0x1, 0x80,

    /* U+35 "5" */
    0x2f, 0xc2, 0x40, 0x20, 0x3, 0xf4, 0x25, 0xc0,
    0x9, 0x10, 0x93, 0xc, 0x1f, 0x40,

    /* U+36 "6" */
    0xb, 0x42, 0x80, 0x30, 0x3, 0xb4, 0x34, 0xc3,
    0x9, 0x30, 0x93, 0x4c, 0xf, 0x40,

    /* U+37 "7" */
    0x7f, 0xd0, 0xc, 0x1, 0x80, 0x24, 0x3, 0x0,
    0x60, 0x9, 0x0, 0xc0, 0x18, 0x0,

    /* U+38 "8" */
    0x1f, 0x43, 0x1c, 0x30, 0xc3, 0x1c, 0x1f, 0x43,
    0xc, 0x70, 0xc3, 0xc, 0x1f, 0x40,

    /* U+39 "9" */
    0x1f, 0x3, 0x18, 0x60, 0xc6, 0xc, 0x31, 0xc1,
    0xec, 0x0, 0xc0, 0x24, 0x1e, 0x0,

    /* U+3A ":" */
    0x30, 0x40, 0x0, 0x0, 0x43, 0x0,

    /* U+3C "<" */
    0x1, 0x43, 0xc7, 0x41, 0xe0, 0xb, 0x40, 0x10,

    /* U+3D "=" */
    0x3f, 0xc0, 0x0, 0x0, 0xff, 0x0, 0x0,

    /* U+3E ">" */
    0x20, 0x2, 0xd0, 0x2, 0xc0, 0x78, 0x3c, 0x1,
    0x0,

    /* U+3F "?" */
    0x2e, 0x18, 0x90, 0x28, 0xc, 0xa, 0x3, 0x0,
    0x40, 0x0, 0xc, 0x0,

    /* U+40 "@" */
    0x2, 0xf8, 0x3, 0x41, 0x82, 0x40, 0x20, 0xc3,
    0x85, 0x22, 0x14, 0x98, 0xc8, 0x25, 0x22, 0x9,
    0x8c, 0xc5, 0x22, 0x9e, 0xc, 0x0, 0x1, 0x80,
    0x0, 0x1f, 0xc0,

    /* U+41 "A" */
    0x3, 0x0, 0x1d, 0x0, 0xe8, 0x3, 0x30, 0x18,
    0xd0, 0x92, 0x83, 0xff, 0x1c, 0xc, 0xa0, 0x28,

    /* U+42 "B" */
    0x3f, 0x83, 0xa, 0x30, 0x63, 0x9, 0x3f, 0xc3,
    0xa, 0x30, 0x73, 0xa, 0x3f, 0xc0,

    /* U+43 "C" */
    0xb, 0xc0, 0xa1, 0xc3, 0x3, 0xc, 0x0, 0x70,
    0x0, 0xc0, 0x3, 0x3, 0xd, 0x1c, 0xb, 0xc0,

    /* U+44 "D" */
    0x3f, 0x80, 0xc2, 0x83, 0x3, 0xc, 0xc, 0x30,
    0x30, 0xc0, 0xc3, 0x3, 0xc, 0x28, 0x3f, 0x80,

    /* U+45 "E" */
    0x3f, 0xd3, 0x0, 0x30, 0x3, 0x0, 0x3f, 0xc3,
    0x0, 0x30, 0x3, 0x0, 0x3f, 0xe0,

    /* U+46 "F" */
    0x3f, 0xd3, 0x0, 0x30, 0x3, 0x0, 0x3f, 0xc3,
    0x0, 0x30, 0x3, 0x0, 0x30, 0x0,

    /* U+47 "G" */
    0xf, 0xd0, 0xd1, 0xc3, 0x1, 0xc, 0x0, 0x71,
    0xf4, 0xc0, 0xd3, 0x3, 0x4e, 0xc, 0xb, 0xd0,

    /* U+48 "H" */
    0x30, 0x28, 0xc0, 0xa3, 0x2, 0x8c, 0xa, 0x3f,
    0xf8, 0xc0, 0xa3, 0x2, 0x8c, 0xa, 0x30, 0x28,

    /* U+49 "I" */
    0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+4A "J" */
    0x0, 0xc0, 0x30, 0xc, 0x3, 0x0, 0xc0, 0x35,
    0xd, 0x8a, 0x2f, 0x0,

    /* U+4B "K" */
    0x30, 0x70, 0xc3, 0x3, 0x28, 0xd, 0xc0, 0x3f,
    0x0, 0xdd, 0x3, 0xc, 0xc, 0x28, 0x30, 0x30,

    /* U+4C "L" */
    0x30, 0x3, 0x0, 0x30, 0x3, 0x0, 0x30, 0x3,
    0x0, 0x30, 0x3, 0x0, 0x3f, 0xd0,

    /* U+4D "M" */
    0x34, 0x7, 0xf, 0x2, 0xc3, 0xc0, 0xf0, 0xe4,
    0x6c, 0x36, 0x27, 0xc, 0xcc, 0xc3, 0x26, 0x30,
    0xc7, 0x4c, 0x30, 0xc3, 0x0,

    /* U+4E "N" */
    0x30, 0x28, 0xe0, 0xa3, 0xc2, 0x8d, 0x8a, 0x33,
    0x28, 0xc6, 0xa3, 0xf, 0x8c, 0x1e, 0x30, 0x38,

    /* U+4F "O" */
    0xb, 0xc0, 0xe1, 0xc3, 0x3, 0x4c, 0x9, 0x70,
    0x24, 0xc0, 0x93, 0x3, 0x4e, 0x1c, 0xb, 0xc0,

    /* U+50 "P" */
    0x3f, 0xd0, 0xc1, 0xc3, 0x3, 0xc, 0x1c, 0x3f,
    0xd0, 0xc0, 0x3, 0x0, 0xc, 0x0, 0x30, 0x0,

    /* U+51 "Q" */
    0xb, 0xc0, 0xe1, 0xc3, 0x3, 0x5c, 0x9, 0x70,
    0x25, 0xc0, 0x93, 0x3, 0x4d, 0x1c, 0xb, 0xe0,
    0x0, 0xd0, 0x0, 0x0,

    /* U+52 "R" */
    0x3f, 0x80, 0xc2, 0x83, 0x7, 0xc, 0x28, 0x3f,
    0xc0, 0xc7, 0x3, 0xc, 0xc, 0x28, 0x30, 0x30,

    /* U+53 "S" */
    0x1f, 0x83, 0x9, 0x30, 0x53, 0x40, 0xb, 0x40,
    0xd, 0x50, 0x63, 0xa, 0x1f, 0x80,

    /* U+54 "T" */
    0xbf, 0xf0, 0x60, 0x6, 0x0, 0x60, 0x6, 0x0,
    0x60, 0x6, 0x0, 0x60, 0x6, 0x0,

    /* U+55 "U" */
    0x30, 0x33, 0x3, 0x30, 0x33, 0x3, 0x30, 0x33,
    0x3, 0x30, 0x33, 0x46, 0x1f, 0xc0,

    /* U+56 "V" */
    0xa0, 0x35, 0xc0, 0xc3, 0x6, 0x9, 0x24, 0x18,
    0xc0, 0x33, 0x0, 0xd8, 0x2, 0xd0, 0x7, 0x0,

    /* U+57 "W" */
    0x60, 0xc1, 0x98, 0x34, 0xa3, 0x1e, 0x24, 0xc9,
    0xcc, 0x27, 0x33, 0x9, 0xc9, 0xc1, 0xe1, 0xe0,
    0x34, 0x34, 0xc, 0xc, 0x0,

    /* U+58 "X" */
    0x70, 0x70, 0xd3, 0x41, 0xcc, 0x3, 0xd0, 0x7,
    0x0, 0x3d, 0x1, 0xcc, 0xd, 0x24, 0x70, 0x70,

    /* U+59 "Y" */
    0xa0, 0x70, 0xc2, 0x43, 0x4c, 0x6, 0x60, 0xf,
    0x0, 0x1c, 0x0, 0x60, 0x1, 0x80, 0x6, 0x0,

    /* U+5A "Z" */
    0x7f, 0xe0, 0xd, 0x1, 0x80, 0x30, 0xa, 0x0,
    0xc0, 0x28, 0x3, 0x0, 0x7f, 0xf0,

    /* U+5B "[" */
    0x38, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0x38,

    /* U+5D "]" */
    0xf0, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0xf0,

    /* U+5F "_" */
    0xff, 0x80,

    /* U+61 "a" */
    0x1f, 0x43, 0x1c, 0x0, 0xc1, 0xfc, 0x30, 0xc7,
    0x1c, 0x2e, 0xc0,

    /* U+62 "b" */
    0x30, 0x3, 0x0, 0x30, 0x3, 0xb4, 0x30, 0xc3,
    0x9, 0x30, 0x93, 0x9, 0x30, 0xc3, 0xb4,

    /* U+63 "c" */
    0x1f, 0x43, 0xc, 0x60, 0x46, 0x0, 0x60, 0x3,
    0xc, 0x1f, 0x40,

    /* U+64 "d" */
    0x0, 0xc0, 0xc, 0x0, 0xc2, 0xec, 0x30, 0xc6,
    0xc, 0x60, 0xc6, 0xc, 0x30, 0xc2, 0xec,

    /* U+65 "e" */
    0x1f, 0x43, 0x1c, 0x60, 0xc7, 0xfc, 0x60, 0x3,
    0x4, 0x1f, 0x80,

    /* U+66 "f" */
    0xe, 0x24, 0x34, 0xbd, 0x34, 0x34, 0x34, 0x34,
    0x34, 0x34,

    /* U+67 "g" */
    0x2e, 0xc3, 0xc, 0x60, 0xc6, 0xc, 0x60, 0xc3,
    0xc, 0x2e, 0xc0, 0xc, 0x11, 0xc2, 0xf4,

    /* U+68 "h" */
    0x30, 0x3, 0x0, 0x30, 0x3, 0xb4, 0x31, 0xc3,
    0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc,

    /* U+69 "i" */
    0x30, 0x33, 0x33, 0x33, 0x30,

    /* U+6A "j" */
    0x8, 0x0, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x31,
    0xcd,

    /* U+6B "k" */
    0x30, 0x3, 0x0, 0x30, 0x3, 0x28, 0x33, 0x3,
    0xd0, 0x3d, 0x3, 0x70, 0x32, 0x43, 0xc,

    /* U+6C "l" */
    0x33, 0x33, 0x33, 0x33, 0x33,

    /* U+6D "m" */
    0x3f, 0x6e, 0xc, 0x74, 0xc3, 0xc, 0x30, 0xc3,
    0xd, 0x30, 0xc3, 0x4c, 0x30, 0xd3, 0xc, 0x34,

    /* U+6E "n" */
    0x3b, 0x43, 0x1c, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0x30, 0xc0,

    /* U+6F "o" */
    0x1f, 0x43, 0xc, 0x60, 0x96, 0x9, 0x60, 0x93,
    0xc, 0x1f, 0x40,

    /* U+70 "p" */
    0x3b, 0x43, 0xc, 0x30, 0x93, 0x9, 0x30, 0x93,
    0xc, 0x3b, 0x43, 0x0, 0x30, 0x3, 0x0,

    /* U+71 "q" */
    0x2e, 0xc3, 0xc, 0x60, 0xc6, 0xc, 0x60, 0xc3,
    0xc, 0x2e, 0xc0, 0xc, 0x0, 0xc0, 0xc,

    /* U+72 "r" */
    0x0, 0x3d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,

    /* U+73 "s" */
    0x2f, 0xc, 0x63, 0x0, 0x78, 0x2, 0xdc, 0x72,
    0xf4,

    /* U+74 "t" */
    0x10, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x2c,

    /* U+75 "u" */
    0x70, 0xc7, 0xc, 0x70, 0xc7, 0xc, 0x30, 0xc3,
    0x1c, 0x2e, 0xc0,

    /* U+76 "v" */
    0x91, 0x98, 0x93, 0x30, 0xcc, 0x2a, 0x3, 0x40,
    0xc0,

    /* U+77 "w" */
    0x92, 0x49, 0x63, 0x8c, 0x33, 0xcc, 0x36, 0xd8,
    0x2d, 0xb4, 0x2c, 0x74, 0x1c, 0x30,

    /* U+78 "x" */
    0x72, 0x8c, 0xc1, 0xe0, 0x34, 0x1e, 0xc, 0xc6,
    0x18,

    /* U+79 "y" */
    0x91, 0x98, 0x93, 0x30, 0xdc, 0x2e, 0x7, 0x40,
    0xc0, 0x30, 0x18, 0x1c, 0x0,

    /* U+7A "z" */
    0x7f, 0x80, 0xd0, 0x60, 0x30, 0x28, 0xc, 0x7,
    0xfc,

    /* U+7C "|" */
    0xff, 0xff, 0xfc,

    /* U+B0 "°" */
    0x2c, 0x22, 0x2d,

    /* U+B1 "±" */
    0x9, 0x0, 0x90, 0x9, 0x7, 0xfc, 0x9, 0x0,
    0x90, 0x0, 0x3, 0xfc,

    /* U+B5 "µ" */
    0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0x3f, 0xc3, 0x0, 0x30, 0x3, 0x0,

    /* U+B7 "·" */
    0x0, 0xc0, 0x0,

    /* U+BC "¼" */
    0x0, 0x0, 0x74, 0x0, 0x24, 0x20, 0x14, 0x80,
    0x14, 0x80, 0x16, 0x18, 0x2, 0x38, 0x8, 0x98,
    0x8, 0xfc, 0x0, 0x8,

    /* U+BD "½" */
    0x0, 0x0, 0x34, 0x0, 0x24, 0x50, 0x24, 0x80,
    0x25, 0x40, 0x16, 0x3c, 0x5, 0x59, 0x8, 0xc,
    0x4, 0x30, 0x0, 0x7d,

    /* U+BE "¾" */
    0x2c, 0x0, 0x16, 0x10, 0xd, 0x20, 0x16, 0x50,
    0x3d, 0x8c, 0x1, 0x6c, 0x2, 0x2c, 0x5, 0xbe,
    0x0, 0xc
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 44, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 47, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 122, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 25, .adv_w = 60, .box_w = 4, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 39, .adv_w = 61, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 50, .adv_w = 83, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 58, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 69, .adv_w = 38, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 71, .adv_w = 48, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 72, .adv_w = 51, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 71, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 87, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 95, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 45, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 86, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 236, .adv_w = 92, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 243, .adv_w = 88, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 252, .adv_w = 81, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 148, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 291, .adv_w = 111, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 105, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 109, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 110, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 95, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 92, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 48, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 93, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 432, .adv_w = 105, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 91, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 145, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 515, .adv_w = 106, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 531, .adv_w = 115, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 551, .adv_w = 102, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 99, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 100, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 595, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 609, .adv_w = 108, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 146, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 106, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 101, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 678, .adv_w = 100, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 692, .adv_w = 48, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 702, .adv_w = 48, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 712, .adv_w = 78, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 714, .adv_w = 92, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 725, .adv_w = 95, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 740, .adv_w = 89, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 751, .adv_w = 95, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 766, .adv_w = 90, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 777, .adv_w = 61, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 787, .adv_w = 95, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 802, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 817, .adv_w = 44, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 822, .adv_w = 43, .box_w = 3, .box_h = 12, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 831, .adv_w = 87, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 846, .adv_w = 44, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 851, .adv_w = 144, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 867, .adv_w = 93, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 878, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 95, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 904, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 919, .adv_w = 58, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 927, .adv_w = 87, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 936, .adv_w = 57, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 945, .adv_w = 93, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 956, .adv_w = 82, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 965, .adv_w = 125, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 979, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 988, .adv_w = 80, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1001, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1010, .adv_w = 47, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1013, .adv_w = 72, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 1016, .adv_w = 90, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1028, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1043, .adv_w = 47, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 1046, .adv_w = 122, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1066, .adv_w = 130, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1086, .adv_w = 129, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 1, 0, 0, 0, 2
};

static const uint8_t glyph_id_ofs_list_3[] = {
    0, 0, 1
};

static const uint16_t unicode_list_5[] = {
    0x0, 0x34, 0x35, 0x39, 0x3b, 0x40, 0x41, 0x42
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 6, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 6, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 40, .range_length = 19, .glyph_id_start = 4,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 60, .range_length = 32, .glyph_id_start = 23,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 93, .range_length = 3, .glyph_id_start = 55,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_3, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 57,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 124, .range_length = 67, .glyph_id_start = 83,
        .unicode_list = unicode_list_5, .glyph_id_ofs_list = NULL, .list_length = 8, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    1, 47,
    4, 49,
    4, 50,
    4, 52,
    11, 11,
    28, 26,
    28, 30,
    28, 34,
    28, 42,
    28, 44,
    28, 47,
    28, 48,
    28, 49,
    28, 50,
    28, 52,
    28, 71,
    28, 76,
    28, 77,
    28, 78,
    28, 79,
    28, 81,
    28, 82,
    29, 47,
    29, 49,
    29, 52,
    30, 5,
    30, 47,
    30, 55,
    31, 8,
    31, 10,
    31, 28,
    31, 47,
    31, 49,
    31, 51,
    31, 52,
    31, 53,
    32, 47,
    32, 59,
    32, 60,
    32, 61,
    32, 62,
    32, 63,
    32, 71,
    32, 73,
    32, 77,
    32, 78,
    32, 79,
    32, 81,
    33, 8,
    33, 10,
    33, 28,
    33, 37,
    33, 47,
    33, 57,
    33, 59,
    33, 60,
    33, 61,
    33, 63,
    33, 71,
    33, 73,
    33, 74,
    33, 77,
    33, 78,
    33, 81,
    35, 28,
    35, 47,
    35, 51,
    35, 52,
    36, 28,
    36, 47,
    36, 51,
    36, 52,
    37, 28,
    38, 9,
    38, 30,
    38, 34,
    38, 42,
    38, 44,
    38, 59,
    38, 60,
    38, 61,
    38, 63,
    38, 69,
    38, 70,
    38, 71,
    38, 72,
    38, 73,
    38, 77,
    38, 78,
    38, 79,
    38, 81,
    39, 28,
    39, 30,
    39, 34,
    39, 42,
    39, 44,
    39, 47,
    39, 48,
    39, 49,
    39, 50,
    39, 52,
    39, 77,
    39, 78,
    39, 79,
    39, 81,
    40, 28,
    40, 47,
    40, 51,
    40, 52,
    41, 28,
    41, 47,
    41, 51,
    41, 52,
    42, 8,
    42, 10,
    42, 28,
    42, 47,
    42, 49,
    42, 51,
    42, 52,
    42, 53,
    43, 8,
    43, 10,
    43, 28,
    43, 37,
    43, 51,
    43, 53,
    43, 57,
    43, 59,
    43, 60,
    43, 61,
    43, 63,
    43, 71,
    43, 73,
    43, 76,
    43, 78,
    43, 81,
    44, 47,
    44, 49,
    44, 50,
    44, 52,
    45, 47,
    45, 49,
    45, 52,
    47, 1,
    47, 8,
    47, 9,
    47, 10,
    47, 28,
    47, 30,
    47, 34,
    47, 37,
    47, 42,
    47, 44,
    47, 46,
    47, 47,
    47, 49,
    47, 50,
    47, 52,
    47, 57,
    47, 59,
    47, 60,
    47, 61,
    47, 63,
    47, 69,
    47, 70,
    47, 71,
    47, 72,
    47, 73,
    47, 74,
    47, 75,
    47, 77,
    47, 78,
    47, 79,
    47, 80,
    47, 81,
    47, 82,
    48, 28,
    49, 5,
    49, 8,
    49, 9,
    49, 10,
    49, 28,
    49, 30,
    49, 34,
    49, 42,
    49, 44,
    49, 55,
    49, 57,
    49, 59,
    49, 60,
    49, 61,
    49, 63,
    49, 71,
    49, 73,
    49, 74,
    49, 77,
    49, 78,
    49, 81,
    50, 5,
    50, 8,
    50, 9,
    50, 10,
    50, 28,
    50, 47,
    50, 55,
    50, 57,
    50, 59,
    50, 60,
    50, 61,
    50, 63,
    50, 71,
    50, 73,
    50, 74,
    50, 77,
    51, 9,
    51, 30,
    51, 34,
    51, 42,
    51, 44,
    51, 49,
    51, 59,
    51, 60,
    51, 61,
    51, 63,
    51, 71,
    51, 73,
    51, 77,
    51, 78,
    51, 81,
    52, 5,
    52, 6,
    52, 8,
    52, 9,
    52, 10,
    52, 28,
    52, 30,
    52, 34,
    52, 37,
    52, 42,
    52, 44,
    52, 46,
    52, 47,
    52, 48,
    52, 49,
    52, 50,
    52, 51,
    52, 52,
    52, 55,
    52, 57,
    52, 59,
    52, 60,
    52, 61,
    52, 62,
    52, 63,
    52, 69,
    52, 70,
    52, 71,
    52, 72,
    52, 73,
    52, 74,
    52, 75,
    52, 76,
    52, 77,
    52, 78,
    52, 80,
    52, 81,
    52, 82,
    53, 28,
    53, 30,
    53, 34,
    53, 42,
    53, 44,
    53, 59,
    53, 60,
    53, 61,
    53, 63,
    53, 71,
    53, 73,
    53, 77,
    53, 78,
    53, 79,
    53, 81,
    54, 37,
    54, 48,
    57, 78,
    57, 81,
    58, 78,
    58, 80,
    58, 81,
    58, 82,
    61, 78,
    61, 81,
    62, 5,
    62, 55,
    62, 59,
    62, 60,
    62, 61,
    62, 63,
    62, 73,
    67, 59,
    67, 60,
    67, 61,
    67, 63,
    67, 73,
    71, 78,
    71, 80,
    71, 81,
    71, 82,
    72, 78,
    72, 80,
    72, 81,
    72, 82,
    74, 8,
    74, 10,
    74, 57,
    74, 59,
    74, 60,
    74, 61,
    74, 62,
    74, 63,
    74, 71,
    74, 73,
    74, 76,
    74, 78,
    74, 79,
    74, 81,
    76, 71,
    78, 8,
    78, 10,
    78, 57,
    78, 59,
    78, 60,
    78, 61,
    78, 62,
    78, 63,
    78, 71,
    78, 73,
    79, 8,
    79, 10,
    80, 59,
    80, 60,
    80, 61,
    80, 63,
    80, 71,
    80, 73,
    81, 8,
    81, 10,
    81, 57,
    81, 59,
    81, 60,
    81, 61,
    81, 62,
    81, 63,
    81, 71,
    81, 73,
    82, 59,
    82, 60,
    82, 61,
    82, 63,
    82, 71,
    82, 73
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -4, 2, 2, 2, -21, -6, -1, -1,
    -1, -1, -12, -2, -8, -6, -9, -1,
    -2, -1, -5, -3, -5, 1, -3, -2,
    -5, -2, -3, -1, -10, -10, -2, -3,
    -2, -2, -4, -2, 2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2,
    -22, -22, -16, -25, 2, -3, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2,
    2, -3, 2, -3, 2, -3, 2, -3,
    -2, -6, -3, -3, -3, -3, -2, -2,
    -2, -2, -2, -2, -3, -2, -2, -2,
    -4, -6, -4, 2, -6, -6, -6, -6,
    -26, -5, -16, -13, -22, -4, -12, -9,
    -12, 2, -3, 2, -3, 2, -3, 2,
    -3, -10, -10, -2, -3, -2, -2, -4,
    -2, -30, -30, -13, -19, -3, -2, -1,
    -1, -1, -1, -1, -1, -1, 1, 1,
    1, -4, -3, -2, -3, -7, -2, -4,
    -4, -20, -22, -20, -7, -3, -3, -22,
    -3, -3, -1, 2, 2, 1, 2, -11,
    -9, -9, -9, -9, -10, -10, -9, -10,
    -9, -7, -11, -9, -7, -5, -7, -7,
    -6, -2, 2, -21, -3, -21, -7, -1,
    -1, -1, -1, 2, -4, -4, -4, -4,
    -4, -4, -4, -3, -3, -1, -1, 1,
    -12, -6, -12, -4, 1, 1, -3, -3,
    -3, -3, -3, -3, -3, -2, -2, -4,
    -2, -2, -2, -2, 1, -2, -2, -2,
    -2, -2, -2, -2, -3, -3, 2, -5,
    -20, -5, -20, -9, -3, -3, -9, -3,
    -3, -1, 2, -9, 2, 2, 1, 2,
    2, -7, -6, -6, -6, -2, -6, -4,
    -4, -6, -4, -6, -4, -5, -2, -4,
    -2, -2, -2, -3, 1, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2,
    -3, -3, -3, -2, -2, -1, -1, -1,
    -1, -1, -1, -1, -1, 2, 2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -1, -2, -1, -1, -1, -1, -1,
    -1, -12, -12, -4, -2, -2, -2, 1,
    -2, -2, -2, 5, 2, 2, 2, -2,
    -10, -10, -1, -1, -1, -1, 1, -1,
    -1, -1, -12, -12, -2, -2, -2, -2,
    -2, -2, -10, -10, -1, -1, -1, -1,
    1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 362,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 6,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t roboto_condensed_12_basic = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if ROBOTO_CONDENSED_12_BASIC*/

