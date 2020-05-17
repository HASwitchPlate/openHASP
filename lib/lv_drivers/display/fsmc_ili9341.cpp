/**
 * @file fsmc_ili9341.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "fsmc_ili9341.h"

#if USE_FSMC_ILI9341 != 0

#include <stdbool.h>
#include <Arduino.h>
#include <SPI.h>
//#include <XPT2046_Touchscreen.h>
#include <Wire.h>

#include "GxTFT_GFX.h" // Hardware-specific library
#include "GxTFT.h" // Hardware-specific library
#define TFT_Class GxTFT

#include "GxIO/GxIO.h"

// select one GxIO class, 
// note: "error: 'GxIO_Class' does not name a type": indicates target board selection mismatch
// this version is for use with Arduino package STM32GENERIC, board "BLACK F407VE/ZE/ZG boards".
// Specific Board "BLACK F407ZG (M4 DEMO)"
// I use it with ST-LINK-V2, Upload method "STLink[Automatic serial = SerialUSB]", USB disabled.
// For Serial I use a Serial to USB converter on PA9, PA10, "SerialUART1".
// https://github.com/danieleff/STM32GENERIC
#include "GxIO/STM32DUINO/GxIO_STM32F4_FSMC/GxIO_STM32F4_FSMC.h"
#include "myTFTs/my_3.2_TFT_320x240_ILI9341_STM32F407ZGM4_FSMC.h"

#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE

// For 3.2" TFT of bundle 1 of:
// https://www.aliexpress.com/item/STM32F407ZGT6-Development-Board-ARM-M4-STM32F4-cortex-M4-core-Board-Compatibility-Multiple-Extension/32795142050.html
// select one GxCTRL class
#include <GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h> // 240x320
#include "GxReadRegisters.h"

/*********************
 *      DEFINES
 *********************/
#if !defined(ESP8266)
#define yield()
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the R61581 display controller
 * @return HW_RES_OK or any error from hw_res_t enum
 */
void fsmc_ili9341_init(uint8_t rotation)
{
    tft.init();
    tft.setRotation(rotation);
}

void fsmc_ili9341_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    size_t len = (x2 - x1 + 1) * (y2 - y1 + 1); /* Number of pixels */

    /* Update TFT */
    // tft.startWrite();                      /* Start new TFT transaction */
    tft.setWindow(x1, y1, x2, y2);            /* set the working window */
    tft.pushColors((uint16_t *)color_p, len); /* Write words at once */
    // tft.endWrite();                        /* terminate TFT transaction */

    /* Tell lvgl that flushing is done */
    // lv_disp_flush_ready();
}

void fsmc_ili9341_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
    tft.fillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, color.full);
}

void fsmc_ili9341_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    fsmc_ili9341_flush(x1, y1, x2, y2, color_p);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
