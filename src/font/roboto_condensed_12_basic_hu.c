#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*******************************************************************************
 * Size: 12 px
 * Bpp: 2
 * Opts: 0x020,0x021,0x025,0x028-0x03A,0x03C-0x05B,0x05D,0x05F,0x061-0x07A,0x07C,0x0B0,0x0B1,0x0B5,0x0B7,0x0BC-0x0BE,0x0C1,0x0C9,0x0CD,0x0D3,0x0D6,0x0DA,0x0DC,0x0E1,0x0E9,0x0ED,0x0F3,0x0F6,0x0FA,0x0FC,0x150,0x151,0x170,0x171
 ******************************************************************************/

#ifndef ROBOTO_CONDENSED_12_BASIC_HU
#define ROBOTO_CONDENSED_12_BASIC_HU 1
#endif

#if ROBOTO_CONDENSED_12_BASIC_HU

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
    0x0, 0xc,

    /* U+C1 "Á" */
    0x0, 0x40, 0x9, 0x0, 0x0, 0x0, 0xc0, 0x7,
    0x40, 0x3a, 0x0, 0xcc, 0x6, 0x34, 0x24, 0xa0,
    0xff, 0xc7, 0x3, 0x28, 0xa,

    /* U+C9 "É" */
    0x2, 0x80, 0x30, 0x0, 0x3, 0xfd, 0x30, 0x3,
    0x0, 0x30, 0x3, 0xfc, 0x30, 0x3, 0x0, 0x30,
    0x3, 0xfe,

    /* U+CD "Í" */
    0x1c, 0x30, 0x0, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30,

    /* U+D3 "Ó" */
    0x0, 0xc0, 0x9, 0x0, 0x0, 0x2, 0xf0, 0x38,
    0x70, 0xc0, 0xd3, 0x2, 0x5c, 0x9, 0x30, 0x24,
    0xc0, 0xd3, 0x87, 0x2, 0xf0,

    /* U+D6 "Ö" */
    0x0, 0x0, 0x32, 0x40, 0x0, 0x2, 0xf0, 0x38,
    0x70, 0xc0, 0xd3, 0x2, 0x5c, 0x9, 0x30, 0x24,
    0xc0, 0xd3, 0x87, 0x2, 0xf0,

    /* U+DA "Ú" */
    0x1, 0xc0, 0x30, 0x0, 0x3, 0x3, 0x30, 0x33,
    0x3, 0x30, 0x33, 0x3, 0x30, 0x33, 0x3, 0x34,
    0x61, 0xfc,

    /* U+DC "Ü" */
    0x0, 0x1, 0x8c, 0x0, 0x3, 0x3, 0x30, 0x33,
    0x3, 0x30, 0x33, 0x3, 0x30, 0x33, 0x3, 0x34,
    0x61, 0xfc,

    /* U+E1 "á" */
    0x3, 0x40, 0x60, 0x0, 0x1, 0xf4, 0x31, 0xc0,
    0xc, 0x1f, 0xc3, 0xc, 0x71, 0xc2, 0xec,

    /* U+E9 "é" */
    0x3, 0x40, 0x60, 0x0, 0x1, 0xf4, 0x31, 0xc6,
    0xc, 0x7f, 0xc6, 0x0, 0x30, 0x41, 0xf8,

    /* U+ED "í" */
    0x1c, 0x30, 0x0, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30,

    /* U+F3 "ó" */
    0x3, 0x40, 0x60, 0x0, 0x1, 0xf4, 0x30, 0xc6,
    0x9, 0x60, 0x96, 0x9, 0x30, 0xc1, 0xf4,

    /* U+F6 "ö" */
    0x31, 0x80, 0x0, 0x1f, 0x43, 0xc, 0x60, 0x96,
    0x9, 0x60, 0x93, 0xc, 0x1f, 0x40,

    /* U+FA "ú" */
    0x3, 0x40, 0x60, 0x0, 0x7, 0xc, 0x70, 0xc7,
    0xc, 0x70, 0xc3, 0xc, 0x31, 0xc2, 0xec,

    /* U+FC "ü" */
    0x31, 0x80, 0x0, 0x70, 0xc7, 0xc, 0x70, 0xc7,
    0xc, 0x30, 0xc3, 0x1c, 0x2e, 0xc0,

    /* U+150 "Ő" */
    0x3, 0x60, 0x17, 0x0, 0x0, 0x2, 0xf0, 0x38,
    0x70, 0xc0, 0xd3, 0x2, 0x5c, 0x9, 0x30, 0x24,
    0xc0, 0xd3, 0x87, 0x2, 0xf0,

    /* U+151 "ő" */
    0xd, 0xc0, 0xa4, 0x0, 0x1, 0xf4, 0x30, 0xc6,
    0x9, 0x60, 0x96, 0x9, 0x30, 0xc1, 0xf4,

    /* U+170 "Ű" */
    0x6, 0x90, 0x98, 0x0, 0x3, 0x3, 0x30, 0x33,
    0x3, 0x30, 0x33, 0x3, 0x30, 0x33, 0x3, 0x34,
    0x61, 0xfc,

    /* U+171 "ű" */
    0xd, 0xc0, 0xa0, 0x0, 0x7, 0xc, 0x70, 0xc7,
    0xc, 0x70, 0xc3, 0xc, 0x31, 0xc2, 0xec
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
    {.bitmap_index = 1086, .adv_w = 129, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1104, .adv_w = 111, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1125, .adv_w = 95, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1143, .adv_w = 48, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1155, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1176, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1197, .adv_w = 108, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1215, .adv_w = 108, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1233, .adv_w = 92, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1248, .adv_w = 90, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1263, .adv_w = 45, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1273, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1288, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1302, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1317, .adv_w = 93, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1331, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1352, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1367, .adv_w = 108, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1385, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0}
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
    0x0, 0x34, 0x35, 0x39, 0x3b, 0x40, 0x41, 0x42,
    0x45, 0x4d, 0x51, 0x57, 0x5a, 0x5e, 0x60, 0x65,
    0x6d, 0x71, 0x77, 0x7a, 0x7e, 0x80, 0xd4, 0xd5,
    0xf4, 0xf5
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
        .range_start = 124, .range_length = 246, .glyph_id_start = 83,
        .unicode_list = unicode_list_5, .glyph_id_ofs_list = NULL, .list_length = 26, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 1, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 5, 6, 7,
    8, 9, 0, 10, 10, 11, 12, 13,
    10, 10, 7, 14, 15, 16, 0, 17,
    11, 18, 19, 20, 21, 22, 23, 0,
    0, 24, 25, 0, 0, 26, 27, 0,
    0, 0, 0, 28, 0, 0, 0, 29,
    25, 0, 30, 0, 31, 0, 32, 33,
    34, 32, 35, 0, 0, 0, 0, 0,
    0, 0, 0, 4, 8, 10, 7, 7,
    11, 11, 24, 26, 0, 29, 29, 0,
    0, 7, 29, 11, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 1, 0, 0, 0, 2, 3, 0,
    4, 5, 4, 6, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 0, 8, 0, 9, 0,
    0, 0, 9, 0, 0, 10, 0, 0,
    0, 0, 9, 0, 9, 0, 11, 12,
    13, 14, 15, 16, 17, 18, 0, 19,
    0, 20, 0, 21, 21, 21, 22, 21,
    0, 0, 0, 0, 0, 23, 23, 24,
    23, 21, 25, 26, 27, 28, 29, 30,
    31, 29, 32, 0, 0, 0, 0, 0,
    0, 0, 0, 8, 0, 0, 9, 9,
    13, 13, 20, 21, 0, 24, 24, 28,
    28, 9, 24, 13, 28
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 2, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -21, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -6, 0,
    -1, 0, 0, -12, -2, -8, -6, 0,
    -9, 0, 0, 0, 0, 0, 0, -1,
    0, 0, -2, -1, -5, -3, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -3, 0, -2, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -3, 0, 0, 0, 0,
    0, 0, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -10, 0, 0, 0, -2,
    0, 0, 0, -3, 0, -2, 0, -2,
    -4, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0, 0,
    0, 0, 0, 0, -2, -2, 0, -2,
    0, 0, 0, -2, -2, -2, 0, 0,
    0, 0, 0, -22, 0, 0, 0, -16,
    0, -25, 0, 2, 0, 0, 0, 0,
    0, 0, 0, -3, -2, 0, 0, -2,
    -2, 0, 0, -2, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, -3, 0, 0, 0, 2,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -6, 0, 0, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, -2, -3,
    0, 0, 0, -2, -4, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2,
    -6, 0, 0, -26, -5, -16, -13, 0,
    -22, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, -12, -9, 0, 0,
    0, 0, 0, -30, 0, 0, 0, -13,
    0, -19, 0, 0, 0, 0, 0, -3,
    0, -2, 0, -1, -1, 0, 0, -1,
    0, 0, 1, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, 0, -3, -2, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -7, 0, -2, 0, 0,
    -4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -4, 0, 0, -20, -22, 0, 0, -7,
    -3, -22, -1, 2, 0, 2, 1, 0,
    2, 0, 0, -11, -9, 0, -10, -9,
    -7, -11, 0, -9, -7, -5, -7, -6,
    0, 2, 0, -21, -3, 0, 0, -7,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, -4, -4, 0, 0, -4,
    -3, 0, 0, -3, -1, 0, 0, 0,
    0, 1, 0, -12, -6, 0, 0, -4,
    0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 1, -3, -3, 0, 0, -3,
    -2, 0, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    -2, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, -2, 0, 0, -2,
    0, 0, 0, -2, -3, 0, 0, 0,
    0, 2, -5, -20, -5, 0, 0, -9,
    -3, -9, -1, 2, -9, 2, 2, 1,
    2, 0, 2, -7, -6, -2, -4, -6,
    -4, -5, -2, -4, -2, 0, -2, -3,
    0, 0, 0, 0, 0, 0, 0, 1,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, -2,
    0, 0, 0, -2, -3, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -2, 0, 0, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, -1, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, -2, -1,
    0, 0, 0, -12, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, -2, 1, 0, -2,
    0, 0, 5, 0, 2, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -10, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -1, -1, 1, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -12, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 35,
    .right_class_cnt     = 32,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 6,
    .bpp = 2,
    .kern_classes = 1,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t roboto_condensed_12_basic_hu = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
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



#endif /*#if ROBOTO_CONDENSED_12_BASIC_HU*/

