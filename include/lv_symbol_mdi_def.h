#ifndef LV_SYMBOL_MDI_DEF_H
#define LV_SYMBOL_MDI_DEF_H
/* clang-format off */

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_conf_internal.h"

/*----------------------------------------
 * Symbols from Material Design Icons font
 *--------------------------------------*/

/* In the font converter use this list as range:
    0XF0140, 0XF0141, 0XF0142, 0XF0143, 0XF012C, 0XF0208, 0XF0209, 0XF05A9, 0XF0156, 0XF060C, 0XF030D
*/

#define LV_SYMBOL_DOWN              "\xf3\xb0\x85\x80" /*983360, 0XF0140*/
#define LV_SYMBOL_LEFT              "\xf3\xb0\x85\x81" /*983361, 0XF0141*/
#define LV_SYMBOL_RIGHT             "\xf3\xb0\x85\x82" /*983362, 0XF0142*/
#define LV_SYMBOL_OK                "\xf3\xb0\x84\xac" /*984545, 0XF012C*/
#define LV_SYMBOL_EYE_OPEN          "\xf3\xb0\x88\x88" /*983560, 0XF0208*/
#define LV_SYMBOL_EYE_CLOSE         "\xf3\xb0\x88\x89" /*983561, 0XF0209*/
#define LV_SYMBOL_WIFI              "\xf3\xb0\x96\xa9" /*984489, 0XF05A9*/
#define LV_SYMBOL_CLOSE             "\xf3\xb0\x85\x96" /*983382, 0XF0156*/
#define LV_SYMBOL_NEW_LINE          "\xf3\xb0\x98\x8c" /*984588, 0XF060C*/
#define LV_SYMBOL_BACKSPACE         "\xf3\xb0\x8c\x8d" /*983821, 0XF030D*/

/** Invalid symbol at (U+F8FF). If written before a string then `lv_img` will show it as a label*/
#define LV_SYMBOL_DUMMY             "\xEF\xA3\xBF"

/*-------------------------------
 * Symbols from "normal" font
 *-----------------------------*/
#define LV_SYMBOL_BULLET            "\xE2\x80\xA2" /*20042, 0x2022*/



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_SYMBOL_MDI_DEF_H*/
