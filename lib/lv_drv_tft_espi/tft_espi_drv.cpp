/**
 * @file tft_espi_drv.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../src/hasp_hal.h" // for halGpioName()
#include "ArduinoLog.h"
#include "tft_espi_drv.h"

#if USE_TFT_ESPI != 0

#include <stdbool.h>
#include "TFT_eSPI.h"

#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE
#include "bootscreen.h" // Sketch tab header for xbm images

/*********************
 *      DEFINES
 *********************/
#define TAG_TFT 22

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void tftShowConfig(TFT_eSPI & tft);

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

    tftShowConfig(tft);
}

void tft_espi_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    size_t len = lv_area_get_size(area);
    // size_t len = (x2 - x1 + 1) * (y2 - y1 + 1); /* Number of pixels */

    /* Update TFT */
    tft.startWrite();                                      /* Start new TFT transaction */
    tft.setWindow(area->x1, area->y1, area->x2, area->y2); /* set the working window */
#ifdef USE_DMA_TO_TFT
    tft.pushPixelsDMA((uint16_t *)color_p, len); /* Write words at once */
#else
    tft.pushPixels((uint16_t *)color_p, len); /* Write words at once */
#endif
    tft.endWrite(); /* terminate TFT transaction */

    /* Tell lvgl that flushing is done */
    lv_disp_flush_ready(disp);
}

// void tft_espi_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
// {
//     tft.fillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, color.full);
// }

// void tft_espi_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
// {
//     tft_espi_flush(x1, y1, x2, y2, color_p);
// }

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

#if defined(ARDUINO_ARCH_ESP8266)
ADC_MODE(ADC_VCC); // tftShowConfig measures the voltage on the pin
#endif

static void tftOffsetInfo(uint8_t pin, uint8_t x_offset, uint8_t y_offset)
{
    if(x_offset != 0) {
        Log.verbose(TAG_TFT, F("R%u x offset = %i"), pin, x_offset);
    }
    if(y_offset != 0) {
        Log.verbose(TAG_TFT, F("R%u y offset = %i"), pin, y_offset);
    }
}

static void tftPinInfo(const __FlashStringHelper * pinfunction, int8_t pin)
{
    if(pin != -1) {
        char buffer[64];
        snprintf_P(buffer, sizeof(buffer), PSTR("%-11s: %s (GPIO %02d)"), pinfunction, halGpioName(pin).c_str(), pin);
        Log.verbose(TAG_TFT, buffer);
    }
}

static void tftShowConfig(TFT_eSPI & tft)
{
    setup_t tftSetup;
    tft.getSetup(tftSetup);

    Log.verbose(TAG_TFT, F("TFT_eSPI   : v%s"), tftSetup.version.c_str());
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
    Log.verbose(TAG_TFT, F("Processor  : ESP%x"), tftSetup.esp);
#else
    Log.verbose(TAG_TFT, F("Processor  : STM%x"), tftSetup.esp);
#endif
    Log.verbose(TAG_TFT, F("CPU freq.  : %i MHz"), halGetCpuFreqMHz());

#if defined(ARDUINO_ARCH_ESP8266)
    Log.verbose(TAG_TFT, F("Voltage    : %2.2f V"), ESP.getVcc() / 918.0); // 918 empirically determined
#endif
    Log.verbose(TAG_TFT, F("Transactns : %s"), (tftSetup.trans == 1) ? PSTR("Yes") : PSTR("No"));
    Log.verbose(TAG_TFT, F("Interface  : %s"), (tftSetup.serial == 1) ? PSTR("SPI") : PSTR("Parallel"));
#if defined(ARDUINO_ARCH_ESP8266)
    Log.verbose(TAG_TFT, F("SPI overlap: %s"), (tftSetup.overlap == 1) ? PSTR("Yes") : PSTR("No"));
#endif

    if(tftSetup.tft_driver != 0xE9D) // For ePaper displays the size is defined in the sketch
    {
        Log.verbose(TAG_TFT, F("Driver     : %s"), halDisplayDriverName().c_str()); // tftSetup.tft_driver);
        Log.verbose(TAG_TFT, F("Resolution : %ix%i"), tftSetup.tft_width, tftSetup.tft_height);
    } else if(tftSetup.tft_driver == 0xE9D)
        Log.verbose(TAG_TFT, F("Driver = ePaper"));

    // Offsets, not all used yet
    tftOffsetInfo(0, tftSetup.r0_x_offset, tftSetup.r0_y_offset);
    tftOffsetInfo(1, tftSetup.r1_x_offset, tftSetup.r1_y_offset);
    tftOffsetInfo(2, tftSetup.r2_x_offset, tftSetup.r2_y_offset);
    tftOffsetInfo(3, tftSetup.r3_x_offset, tftSetup.r3_y_offset);
    /* replaced by tftOffsetInfo
    //    if(tftSetup.r1_x_offset != 0) Serial.printf("R1 x offset = %i \n", tftSetup.r1_x_offset);
    //    if(tftSetup.r1_y_offset != 0) Serial.printf("R1 y offset = %i \n", tftSetup.r1_y_offset);
    //    if(tftSetup.r2_x_offset != 0) Serial.printf("R2 x offset = %i \n", tftSetup.r2_x_offset);
    //    if(tftSetup.r2_y_offset != 0) Serial.printf("R2 y offset = %i \n", tftSetup.r2_y_offset);
    //    if(tftSetup.r3_x_offset != 0) Serial.printf("R3 x offset = %i \n", tftSetup.r3_x_offset);
    //    if(tftSetup.r3_y_offset != 0) Serial.printf("R3 y offset = %i \n", tftSetup.r3_y_offset);
    */

    tftPinInfo(F("MOSI"), tftSetup.pin_tft_mosi);
    tftPinInfo(F("MISO"), tftSetup.pin_tft_miso);
    tftPinInfo(F("SCLK"), tftSetup.pin_tft_clk);

#if defined(ARDUINO_ARCH_ESP8266)
    if(tftSetup.overlap == true) {
        Log.verbose(TAG_TFT, F("Overlap selected, following pins MUST be used:"));

        Log.verbose(TAG_TFT, F("MOSI     : SD1 (GPIO 8)"));
        Log.verbose(TAG_TFT, F("MISO     : SD0 (GPIO 7)"));
        Log.verbose(TAG_TFT, F("SCK      : CLK (GPIO 6)"));
        Log.verbose(TAG_TFT, F("TFT_CS   : D3  (GPIO 0)"));

        Log.verbose(TAG_TFT, F("TFT_DC and TFT_RST pins can be tftSetup defined"));
    }
#endif

    tftPinInfo(F("TFT_CS"), tftSetup.pin_tft_cs);
    tftPinInfo(F("TFT_DC"), tftSetup.pin_tft_dc);
    tftPinInfo(F("TFT_RST"), tftSetup.pin_tft_rst);

    tftPinInfo(F("TOUCH_CS"), tftSetup.pin_tch_cs);

    tftPinInfo(F("TFT_WR"), tftSetup.pin_tft_wr);
    tftPinInfo(F("TFT_RD"), tftSetup.pin_tft_rd);

    tftPinInfo(F("TFT_D0"), tftSetup.pin_tft_d0);
    tftPinInfo(F("TFT_D1"), tftSetup.pin_tft_d1);
    tftPinInfo(F("TFT_D2"), tftSetup.pin_tft_d2);
    tftPinInfo(F("TFT_D3"), tftSetup.pin_tft_d3);
    tftPinInfo(F("TFT_D4"), tftSetup.pin_tft_d4);
    tftPinInfo(F("TFT_D5"), tftSetup.pin_tft_d5);
    tftPinInfo(F("TFT_D6"), tftSetup.pin_tft_d6);
    tftPinInfo(F("TFT_D7"), tftSetup.pin_tft_d7);

    if(tftSetup.serial == 1) {
        Log.verbose(TAG_TFT, F("Display SPI freq. : %d.%d MHz"), tftSetup.tft_spi_freq / 10,
                    tftSetup.tft_spi_freq % 10);
    }
    if(tftSetup.pin_tch_cs != -1) {
        Log.verbose(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), tftSetup.tch_spi_freq / 10,
                    tftSetup.tch_spi_freq % 10);
    }
}

#endif
