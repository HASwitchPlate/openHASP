/**
 * @file lv_zifont.h
 *
 */

#ifndef LV_ZIFONT_H
#define LV_ZIFONT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <Arduino.h>
#include <stdint.h>
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef uint8_t lv_zifont_char_offset_t[3];

typedef struct
{
    uint16_t character;
    uint8_t width;
    uint8_t kerningL;
    uint8_t kerningR;
    lv_zifont_char_offset_t pos;
    uint16_t length;
} lv_zifont_char_t;

typedef struct
{
    uint8_t Password;
    uint8_t SkipL0;
    uint8_t SkipLH;
    uint8_t Orientation;
    uint8_t Codepageid;
    uint8_t State;
    uint8_t CharWidth;
    uint8_t CharHeight;
    uint8_t SecondByteStart;
    uint8_t SecondByteEnd;
    uint8_t FirstByteStart;
    uint8_t FirstByteEnd;
    uint32_t Maximumnumchars;
    uint8_t Version;
    uint8_t Descriptionlength;
    uint16_t Zimobinbeg;
    uint32_t Totaldatalength;
    uint32_t Startdataaddress;
    uint8_t CodeT0;
    uint8_t CodeDec;
    uint8_t Antialias;
    uint8_t Variablewidth;
    uint8_t Namelength;
    uint8_t Fontdataadd8byte;
    uint16_t last_glyph_id; // Reserved 1
    uint32_t Actualnumchars;
    lv_zifont_char_t * last_glyph_dsc; // Reserved 3
} lv_font_fmt_zifont_dsc_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
int lv_zifont_init(void);
int lv_zifont_font_init(lv_font_t * font, const char * font_path, uint16_t size);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_ZIFONTs_H*/
