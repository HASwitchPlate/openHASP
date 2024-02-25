/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef TFT_ESPI_DRV_H
#define TFT_ESPI_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
//#if USE_TFT_ESPI

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#if defined(TOUCH_CS)
void tft_espi_calibrate(uint16_t* calData);
void tft_espi_set_touch(uint16_t* calData);
bool tft_espi_get_touch(int16_t* touchX, int16_t* touchY, uint16_t threshold);
#endif

//#endif /* USE_TFT_ESPI */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TFT_ESPI_DRV_H */
