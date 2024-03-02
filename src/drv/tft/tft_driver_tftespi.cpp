/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && defined(USER_SETUP_LOADED)
#include "tft_driver_tftespi.h"

namespace dev {

void TftEspi::init(int w, int h)
{
#ifdef USE_DMA_TO_TFT
    // DMA - should work with STM32F2xx/F4xx/F7xx processors
    // NOTE: >>>>>> DMA IS FOR SPI DISPLAYS ONLY <<<<<<
    tft.initDMA(); // Initialise the DMA engine (tested with STM32F446 and STM32F767)
#endif

    /* TFT init */
    tft.begin();
    tft.setSwapBytes(true); /* set endianness */
}

void TftEspi::show_info()
{
    splashscreen();

    setup_t tftSetup;
    tft.getSetup(tftSetup);

    LOG_VERBOSE(TAG_TFT, F("TFT_eSPI   : v%s"), tftSetup.version.c_str());
    LOG_VERBOSE(TAG_TFT, F("Transactns : %s"), (tftSetup.trans == 1) ? PSTR(D_YES) : PSTR(D_NO));
    LOG_VERBOSE(TAG_TFT, F("Interface  : %s"), (tftSetup.serial == 1) ? PSTR("SPI") : PSTR("Parallel"));

#if defined(ARDUINO_ARCH_ESP8266)
    LOG_VERBOSE(TAG_TFT, F("SPI overlap: %s"), (tftSetup.overlap == 1) ? PSTR(D_YES) : PSTR(D_NO));
#endif

    if(tftSetup.tft_driver != 0xE9D) // For ePaper displays the size is defined in the sketch
    {
        LOG_VERBOSE(TAG_TFT, F("Driver     : %s"), haspTft.get_tft_model()); // tftSetup.tft_driver);
        LOG_VERBOSE(TAG_TFT, F("Resolution : %ix%i"), tftSetup.tft_width, tftSetup.tft_height);
    } else if(tftSetup.tft_driver == 0xE9D)
        LOG_VERBOSE(TAG_TFT, F("Driver = ePaper"));

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

    tftPinInfo(F("TFT_MOSI"), tftSetup.pin_tft_mosi);
    tftPinInfo(F("TFT_MISO"), tftSetup.pin_tft_miso);
    tftPinInfo(F("TFT_SCLK"), tftSetup.pin_tft_clk);

#if defined(ARDUINO_ARCH_ESP8266)
    if(tftSetup.overlap == true) {
        LOG_VERBOSE(TAG_TFT, F("Overlap selected, following pins MUST be used:"));

        LOG_VERBOSE(TAG_TFT, F("MOSI     : SD1 (GPIO 8)"));
        LOG_VERBOSE(TAG_TFT, F("MISO     : SD0 (GPIO 7)"));
        LOG_VERBOSE(TAG_TFT, F("SCK      : CLK (GPIO 6)"));
        LOG_VERBOSE(TAG_TFT, F("TFT_CS   : D3  (GPIO 0)"));

        LOG_VERBOSE(TAG_TFT, F("TFT_DC and TFT_RST pins can be tftSetup defined"));
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
        LOG_VERBOSE(TAG_TFT, F("Display SPI freq. : %d.%d MHz"), tftSetup.tft_spi_freq / 10,
                    tftSetup.tft_spi_freq % 10);
    }
    if(tftSetup.pin_tch_cs != -1) {
        LOG_VERBOSE(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), tftSetup.tch_spi_freq / 10,
                    tftSetup.tch_spi_freq % 10);
    }
}

void TftEspi::splashscreen()
{
    uint8_t fg[]       = logoFgColor;
    uint8_t bg[]       = logoBgColor;
    lv_color_t fgColor = lv_color_make(fg[0], fg[1], fg[2]);
    lv_color_t bgColor = lv_color_make(bg[0], bg[1], bg[2]);

    tft.fillScreen(bgColor.full);
    int x = (tft.width() - logoWidth) / 2;
    int y = (tft.height() - logoHeight) / 2;
    tft.drawXBitmap(x, y, logoImage, logoWidth, logoHeight, fgColor.full);
}

void TftEspi::set_rotation(uint8_t rotation)
{
    LOG_VERBOSE(TAG_TFT, F("Rotation    : %d"), rotation);
    tft.setRotation(rotation);
}

void TftEspi::set_invert(bool invert)
{
    char buffer[4];
    memcpy_P(buffer, invert ? PSTR(D_YES) : PSTR(D_NO), sizeof(buffer));

    LOG_VERBOSE(TAG_TFT, F("Invert Color: %s"), buffer);
    tft.invertDisplay(invert);
}

/* Update TFT */
void IRAM_ATTR TftEspi::flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    uint32_t w   = (area->x2 - area->x1 + 1);
    uint32_t h   = (area->y2 - area->y1 + 1);
    uint32_t len = w * h;

#ifdef USE_DMA_TO_TFT
    tft.startWrite();                            /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, w, h); /* set the working window */
    tft.pushPixelsDMA((uint16_t*)color_p, len);  /* Write words at once */
    tft.endWrite();                              /* terminate TFT transaction */
#else
    tft.startWrite();                            /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, w, h); /* set the working window */
    tft.pushPixels((uint16_t*)color_p, len);     /* Write words at once */
    tft.endWrite();                              /* terminate TFT transaction */
#endif

    /* Tell lvgl that flushing is done */
    lv_disp_flush_ready(disp);
}

bool TftEspi::is_driver_pin(uint8_t pin)
{
    if(false // start condition is always needed

// Use individual checks instead of switch statement, as some case labels could be duplicated
#ifdef TOUCH_CS
       || (pin == TOUCH_CS)
#endif
#ifdef TFT_MOSI
       || (pin == TFT_MOSI)
#endif
#ifdef TFT_MISO
       || (pin == TFT_MISO)
#endif
#ifdef TFT_SCLK
       || (pin == TFT_SCLK)
#endif
#ifdef TFT_CS
       || (pin == TFT_CS)
#endif
#ifdef TFT_DC
       || (pin == TFT_DC)
#endif
#ifdef TFT_BL
       || (pin == TFT_BL)
#endif
#ifdef TFT_RST
       || (pin == TFT_RST)
#endif
#ifdef TFT_WR
       || (pin == TFT_WR)
#endif
#ifdef TFT_RD
       || (pin == TFT_RD)
#endif
#ifdef TFT_D0
       || (pin == TFT_D0)
#endif
#ifdef TFT_D1
       || (pin == TFT_D1)
#endif
#ifdef TFT_D2
       || (pin == TFT_D2)
#endif
#ifdef TFT_D3
       || (pin == TFT_D3)
#endif
#ifdef TFT_D4
       || (pin == TFT_D4)
#endif
#ifdef TFT_D5
       || (pin == TFT_D5)
#endif
#ifdef TFT_D6
       || (pin == TFT_D6)
#endif
#ifdef TFT_D7
       || (pin == TFT_D7)
#endif
#ifdef TFT_D8
       || (pin == TFT_D8)
#endif
#ifdef TFT_D9
       || (pin == TFT_D9)
#endif
#ifdef TFT_D10
       || (pin == TFT_D10)
#endif
#ifdef TFT_D11
       || (pin == TFT_D11)
#endif
#ifdef TFT_D12
       || (pin == TFT_D12)
#endif
#ifdef TFT_D13
       || (pin == TFT_D13)
#endif
#ifdef TFT_D14
       || (pin == TFT_D14)
#endif
#ifdef TFT_D15
       || (pin == TFT_D15)
#endif
    ) {
        return true;
    }

#ifdef ARDUINO_ARCH_ESP8266
#ifndef TFT_SPI_OVERLAP
    if((pin >= 12) && (pin <= 14)) return true; // HSPI
#endif
#endif

    return false;
}

const char* TftEspi::get_tft_model()
{
#if defined(ILI9341_DRIVER)
    return "ILI9341";
#elif defined(ST7735_DRIVER)
    return "ST7735";
#elif defined(ILI9163_DRIVER)
    return "ILI9163";
#elif defined(S6D02A1_DRIVER)
    return "S6D02A1";
#elif defined(ST7796_DRIVER)
    return "ST7796";
#elif defined(ILI9486_DRIVER)
    return "ILI9486";
#elif defined(ILI9481_DRIVER)
    return "ILI9481";
#elif defined(ILI9488_DRIVER)
    return "ILI9488";
#elif defined(HX8357D_DRIVER)
    return "HX8357D";
#elif defined(EPD_DRIVER)
    return "EPD";
#elif defined(ST7789_DRIVER)
    return "ST7789";
#elif defined(R61581_DRIVER)
    return "R61581";
#elif defined(ST7789_2_DRIVER)
    return "ST7789_2";
#elif defined(RM68140_DRIVER)
    return "RM68140";
#else
    return "Other";
#endif
}

} // namespace dev

dev::TftEspi haspTft;
#endif