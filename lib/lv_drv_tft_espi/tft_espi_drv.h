/**
 * @file tft_espi_drv.h
 *
 */

#ifndef TFT_ESPI_DRV_H
#define TFT_ESPI_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_TFT_ESPI

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tft_espi_init(uint8_t rotation);
void tft_espi_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
// void tft_espi_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
void tft_espi_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
void tft_espi_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);

#if defined(TOUCH_CS)
void tft_espi_calibrate(uint16_t * calData);
void tft_espi_set_touch(uint16_t * calData);
bool tft_espi_get_touch(uint16_t * touchX, uint16_t * touchY, uint16_t threshold);
#endif

/**********************
 *      MACROS
 **********************/

#endif /* USE_TFT_ESPI */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TFT_ESPI_DRV_H */
