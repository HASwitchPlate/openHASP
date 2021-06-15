/**
 * @file fsmc_ili9341.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_drv_conf.h"

#if USE_FSMC_ILI9341 > 0

    #include "fsmc_ili9341.h"

    #include <stdbool.h>
    #include <Arduino.h>
    #include <SPI.h>
    //#include <XPT2046_Touchscreen.h>
    #include <Wire.h>

    #include "GxTFT_GFX.h" // Hardware-specific library
    #include "GxTFT.h"     // Hardware-specific library
    #define TFT_Class GxTFT

    #include "GxIO/GxIO.h"

    // select one GxIO class,
    // note: "error: 'GxIO_Class' does not name a type": indicates target board selection mismatch
    // this version is for use with Arduino package STM32GENERIC, board "BLACK F407VE/ZE/ZG boards".
    // Specific Board "BLACK F407ZG (M4 DEMO)"
    // I use it with ST-LINK-V2, Upload method "STLink[Automatic serial = SerialUSB]", USB disabled.
    // For Serial I use a Serial to USB converter on PA9, PA10, "SerialUART1".
    // https://github.com/danieleff/STM32GENERIC
    #if defined(STM32F407ZG)
        #include "GxIO/STM32GENERIC/GxIO_STM32F407ZGx_FSMC/GxIO_STM32F407ZGx_FSMC.h"
    #else
        #include "GxIO/STM32DUINO/GxIO_STM32F4_FSMC/GxIO_STM32F4_FSMC.h"
    //#include "myTFTs/my_3.2_TFT_320x240_ILI9341_STM32F407ZGM4_FSMC.h"
    #endif
    // #include "../GxIO/STM32GENERIC/GxIO_STM32F407ZGM4_FSMC/GxIO_STM32F407ZGM4_FSMC.h"
    //#include "../GxIO/STM32DUINO/GxIO_STM32F4_FSMC/GxIO_STM32F4_FSMC.h"
    #include "GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h" // 240x320
GxIO_Class io;                                        // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io);                          // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, TFT_WIDTH, TFT_HEIGHT);

    #include LV_DRV_DISP_INCLUDE
    #include LV_DRV_DELAY_INCLUDE

    // For 3.2" TFT of bundle 1 of:
    // https://www.aliexpress.com/item/STM32F407ZGT6-Development-Board-ARM-M4-STM32F4-cortex-M4-core-Board-Compatibility-Multiple-Extension/32795142050.html
    // select one GxCTRL class
    #include <GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h> // 240x320
    #include "GxReadRegisters.h"

    #include "custom/bootlogo_template.h" // Sketch tab header for xbm images

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
 * Initialize the ILI9341 display controller
 */
void fsmc_ili9341_init(uint8_t rotation, bool invert_display)
{
    tft.init();
    tft.setRotation(rotation);
    tft.invertDisplay(invert_display);
    tft.fillScreen(TFT_DARKCYAN);
    int x = (tft.width() - logoWidth) / 2;
    int y = (tft.height() - logoHeight) / 2;
    // tft.drawBitmap(x, y, bootscreen, logoWidth, logoHeight, TFT_WHITE);

    io.startTransaction();
    int32_t i, j, byteWidth = (logoWidth + 7) / 8;

    for(j = 0; j < logoHeight; j++) {
        for(i = 0; i < logoWidth; i++) {
            if(pgm_read_byte(bootscreen + j * byteWidth + i / 8) & (1 << (i & 7))) {
                tft.drawPixel(x + i, y + j, TFT_WHITE);
            }
        }
    }

    io.endTransaction();
    delay(800);
}

/* GxTFT::pushColors only supports writing up to 255 at a time */
/* This *local* function circumvents this artificial limititaion */
static inline void pushColors(uint16_t * data, uint32_t len)
{
    io.startTransaction();
    while(len--) {
        uint16_t color = *data++;
        io.writeData16(color);
    }
    io.endTransaction();
}

void fsmc_ili9341_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    size_t len = lv_area_get_size(area);
    // size_t len = (x2 - x1 + 1) * (y2 - y1 + 1); /* Number of pixels */

    tft.setWindow(area->x1, area->y1, area->x2, area->y2);
    pushColors((uint16_t *)color_p, len);

    /* Update TFT */
    // while(len > 255) {
    //     tft.pushColors((uint16_t *)color_p, 255);
    //     len -= 255;
    //     color_p += 255;
    // }
    // tft.pushColors((uint16_t *)color_p, len); // remainder

    /* Tell lvgl that flushing is done */
    lv_disp_flush_ready(disp);
}

// void fsmc_ili9341_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
// {
//     tft.fillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, color.full);
// }

// void fsmc_ili9341_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t * color_p)
// {
//     fsmc_ili9341_flush(x1, y1, x2, y2, color_p);
// }

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
