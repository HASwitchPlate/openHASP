/**
 * @file tft_espi_drv.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "tft_espi_drv.h"

#if USE_TFT_ESPI != 0

#include <stdbool.h>
#include "TFT_eSPI.h"

#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE
#include "../../../src/hasp_tft.h"
#include "bootscreen.h" // Sketch tab header for xbm images

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static TFT_eSPI tft;

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
void tft_espi_init(uint8_t rotation)
{
    /* TFT init */
    tft.begin();
    tft.setSwapBytes(true); /* set endianess */
    tft.setRotation(rotation);
    tft.fillScreen(TFT_DARKCYAN);
    int x = (tft.width() - logoWidth) / 2;
    int y = (tft.height() - logoHeight) / 2;
    tft.drawXBitmap(x, y, bootscreen, logoWidth, logoHeight, TFT_WHITE);

#ifdef USE_DMA_TO_TFT
    // DMA - should work with STM32F2xx/F4xx/F7xx processors
    // NOTE: >>>>>> DMA IS FOR SPI DISPLAYS ONLY <<<<<<
    tft.initDMA(); // Initialise the DMA engine (tested with STM32F446 and STM32F767)
#endif

    tftSetup(tft);
}

void tft_espi_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    size_t len = (x2 - x1 + 1) * (y2 - y1 + 1); /* Number of pixels */

    /* Update TFT */
    tft.startWrite();              /* Start new TFT transaction */
    tft.setWindow(x1, y1, x2, y2); /* set the working window */
#ifdef USE_DMA_TO_TFT
    tft.pushPixelsDMA((uint16_t *)color_p, len); /* Write words at once */
#else
    tft.pushPixels((uint16_t *)color_p, len); /* Write words at once */
#endif
    tft.endWrite(); /* terminate TFT transaction */

    /* Tell lvgl that flushing is done */
    // lv_disp_flush_ready();
}

void tft_espi_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
    tft.fillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, color.full);
}

void tft_espi_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    tft_espi_flush(x1, y1, x2, y2, color_p);
}

#if defined(TOUCH_CS)

void tft_espi_calibrate(uint16_t * calData)
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println(PSTR("Touch corners as indicated"));

    tft.setTextFont(1);
    delay(500);
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    tft.setTouch(calData);
}

void tft_espi_set_touch(uint16_t * calData)
{
    tft.setTouch(calData);
}

bool tft_espi_get_touch(uint16_t * touchX, uint16_t * touchY, uint16_t threshold)
{
    return tft.getTouch(touchX, touchY, threshold);
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
