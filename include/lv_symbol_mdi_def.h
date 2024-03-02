/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef LV_SYMBOL_MDI_DEF_H
#define LV_SYMBOL_MDI_DEF_H
/* clang-format off */

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------
 * Symbols from Material Design Icons font
 *--------------------------------------*/

#define LV_SYMBOL_DOWN              "\xEE\x85\x80"  /* 0xE140, chevron-down */
#define LV_SYMBOL_LEFT              "\xEE\x85\x81"  /* 0xE141, chevron-left */
#define LV_SYMBOL_RIGHT             "\xEE\x85\x82"  /* 0xE142, chevron-right */
#define LV_SYMBOL_OK                "\xEE\x84\xAC"  /* 0xE12C, check */
#define LV_SYMBOL_EYE_OPEN          "\xEE\xBF\x86"  /* 0xEFC6, lock-open-variant */
#define LV_SYMBOL_EYE_CLOSE         "\xEE\x8C\xBE"  /* 0xE33E, lock*/
#define LV_SYMBOL_WIFI              "\xEE\x96\xA9"  /* 0xE5A9, wifi */
#define LV_SYMBOL_KEY               "\xEE\x8C\x8B"  /* 0xE30B, key-variant */
#define LV_SYMBOL_CLOSE             "\xEE\x85\x96"  /* 0xE156, close */
#define LV_SYMBOL_NEW_LINE          "\xEE\x98\x8C"  /* 0xE60C, subdirectory-arrow-left */
#define LV_SYMBOL_BACKSPACE         "\xEE\x81\x8D"  /* 0xE04D, arrow-left */

/** Invalid symbol at (U+F8FF). If written before a string then `lv_img` will show it as a label*/
#define LV_SYMBOL_DUMMY             "\xEF\xA3\xBF"

/*-------------------------------
 * Symbols from "normal" font
 *-----------------------------*/
#define LV_SYMBOL_BULLET            "\xE2\x80\xA2" /* 20042, 0x2022 */



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_SYMBOL_MDI_DEF_H*/
