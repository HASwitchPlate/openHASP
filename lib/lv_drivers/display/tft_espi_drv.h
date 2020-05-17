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
#define ILI9341_DRIVER 1
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_ROTATION 2 // 0=0, 1=90, 2=180 or 3=270 degree
#define SPI_FREQUENCY 40000000
#define SPI_TOUCH_FREQUENCY 2500000
#define SPI_READ_FREQUENCY 20000000
#define USER_SETUP_LOADED 1
#define TOUCH_DRIVER 0 // XPT2606 Resistive touch panel driver
#define SUPPORT_TRANSACTIONS

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tft_espi_init(uint8_t rotation);
void tft_espi_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
void tft_espi_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
void tft_espi_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
/**********************
 *      MACROS
 **********************/

#endif /* USE_TFT_ESPI */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TFT_ESPI_DRV_H */
