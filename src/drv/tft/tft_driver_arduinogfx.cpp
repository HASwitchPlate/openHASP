/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && defined(HASP_USE_ARDUINOGFX)
#include "tft_driver_arduinogfx.h"
#include <Preferences.h>

#include "Arduino_RGBPanel_mod.h"
#include "Arduino_RGB_Display_mod.h"
#include "Arduino_PCA9535SWSPI.h"

namespace dev {

void tftPinInfo(const __FlashStringHelper* pinfunction, int8_t pin)
{
    if(pin != -1) {
        char buffer[64];
        snprintf_P(buffer, sizeof(buffer), PSTR("%-12s: %s (GPIO %02d)"), String(pinfunction).c_str(),
                   haspDevice.gpio_name(pin).c_str(), pin);
        LOG_VERBOSE(TAG_TFT, buffer);
    }
}

void ArduinoGfx::init(int w, int h)
{
    LOG_TRACE(TAG_TFT, F(D_SERVICE_STARTING));

#if(TFT_WIDTH == 170) && (TFT_HEIGHT == 320)
    Arduino_DataBus* bus = new Arduino_ESP32PAR8(TFT_DC, TFT_CS, TFT_WR, TFT_RD, TFT_D0, TFT_D1, TFT_D2, TFT_D3, TFT_D4,
                                                 TFT_D5, TFT_D6, TFT_D7);
    tft = new Arduino_ST7789(bus, TFT_RST /* RST */, TFT_ROTATION /* rotation */, true /* IPS */, TFT_WIDTH /* width */,
                             TFT_HEIGHT /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */,
                             35 /* col offset 2 */, 0 /* row offset 2 */
    );
    tft->begin(SPI_FREQUENCY); // Used for SPI displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 480) && defined(LILYGO_T_PANEL)
    Wire.begin(17, 18);
    Arduino_DataBus* bus = new Arduino_XL9535SWSPI(17 /* SDA */, 18 /* SCL */, -1 /* XL PWD */, 17 /* XL CS */,
                                                   15 /* XL SCK */, 16 /* XL MOSI */, &Wire);
    Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
        -1 /* DE */, TFT_VSYNC /* VSYNC */, TFT_HSYNC /* HSYNC */, TFT_PCLK /* PCLK */, TFT_B0 /* B0 */,
        TFT_B1 /* B1 */, TFT_B2 /* B2 */, TFT_B3 /* B3 */, TFT_B4 /* B4 */, TFT_G0 /* G0 */, TFT_G1 /* G1 */,
        TFT_G2 /* G2 */, TFT_G3 /* G3 */, TFT_G4 /* G4 */, TFT_G5 /* G5 */, TFT_R0 /* R0 */, TFT_R1 /* R1 */,
        TFT_R2 /* R2 */, TFT_R3 /* R3 */, TFT_R4 /* R4 */, 1 /* hsync_polarity */, 20 /* hsync_front_porch */,
        2 /* hsync_pulse_width */, 0 /* hsync_back_porch */, 1 /* vsync_polarity */, 30 /* vsync_front_porch */,
        8 /* vsync_pulse_width */, 1 /* vsync_back_porch */, 10 /* pclk_active_neg */, 6000000L /* prefer_speed */,
        false /* useBigEndian */, 0 /* de_idle_high*/, 0 /* pclk_idle_high */);

    tft = new Arduino_RGB_Display(TFT_WIDTH /* width */, TFT_HEIGHT /* height */, rgbpanel, 0 /* rotation */,
                                  true /* auto_flush */, bus, -1 /* RST */, st7701_t_panel_init_operations,
                                  sizeof(st7701_t_panel_init_operations));
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 480) && defined(LILYGO_T_RGB)
    Wire.begin(8 /* SDA */, 48 /* SCL */, 800000L /* speed */);
    Arduino_DataBus* bus            = new Arduino_XL9535SWSPI(8 /* SDA */, 48 /* SCL */, 2 /* XL PWD */, 3 /* XL CS */,
                                                              5 /* XL SCK */, 4 /* XL MOSI */);
    Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
        45 /* DE */, 41 /* VSYNC */, 47 /* HSYNC */, 42 /* PCLK */, 21 /* R0 */, 18 /* R1 */, 17 /* R2 */, 46 /* R3 */,
        15 /* R4 */, 14 /* G0 */, 13 /* G1 */, 12 /* G2 */, 11 /* G3 */, 10 /* G4 */, 9 /* G5 */, 7 /* B0 */,
        6 /* B1 */, 5 /* B2 */, 3 /* B3 */, 2 /* B4 */, 1 /* hsync_polarity */, 50 /* hsync_front_porch */,
        1 /* hsync_pulse_width */, 30 /* hsync_back_porch */, 1 /* vsync_polarity */, 20 /* vsync_front_porch */,
        1 /* vsync_pulse_width */, 30 /* vsync_back_porch */, 1 /* pclk_active_neg */);
    tft = new Arduino_RGB_Display(w, h, rgbpanel, 0 /* rotation */, TFT_AUTO_FLUSH, bus, TFT_RST,
                                  st7701_type4_init_operations, sizeof(st7701_type4_init_operations));
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 480) && defined(SENSECAP_INDICATOR_D1)
    Wire.begin(TOUCH_SDA, TOUCH_SCL, I2C_TOUCH_FREQUENCY);
    pinMode(TFT_SCLK, OUTPUT);
    pinMode(TFT_MOSI, OUTPUT);
    pinMode(TFT_MISO, OUTPUT);
    Arduino_DataBus* bus            = new Arduino_PCA9535SWSPI(TOUCH_SDA, TOUCH_SCL, 5 /* XL PWD */, 4 /* XL CS */,
                                                               TFT_SCLK /* XL SCK */, TFT_MOSI /* XL MOSI */, &Wire);
    Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK, TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_G0, TFT_G1, TFT_G2, TFT_G3,
        TFT_G4, TFT_G5, TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_HSYNC_POLARITY, TFT_HSYNC_FRONT_PORCH,
        TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH, TFT_VSYNC_POLARITY, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH,
        TFT_VSYNC_BACK_PORCH);
    tft = new Arduino_RGB_Display(w, h, rgbpanel, 0 /* rotation */, TFT_AUTO_FLUSH, bus, TFT_RST,
                                  st7701_sensecap_indicator_init_operations,
                                  sizeof(st7701_sensecap_indicator_init_operations));
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 480) && defined(GC9503V_DRIVER)
    Arduino_DataBus* bus            = new Arduino_SWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
    Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK, TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_G0, TFT_G1, TFT_G2, TFT_G3,
        TFT_G4, TFT_G5, TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_HSYNC_POLARITY, TFT_HSYNC_FRONT_PORCH,
        TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH, TFT_VSYNC_POLARITY, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH,
        TFT_VSYNC_BACK_PORCH);
    tft = new Arduino_RGB_Display(w, h, rgbpanel, 0 /* rotation */, TFT_AUTO_FLUSH, bus, TFT_RST,
                                  gc9503v_type1_init_operations, sizeof(gc9503v_type1_init_operations));
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 480) && defined(ST7701_DRIVER) && defined(ST7701_4848S040)
    Arduino_DataBus* bus            = new Arduino_SWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
    Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK, TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_G0, TFT_G1, TFT_G2, TFT_G3,
        TFT_G4, TFT_G5, TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_HSYNC_POLARITY, TFT_HSYNC_FRONT_PORCH,
        TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH, TFT_VSYNC_POLARITY, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH,
        TFT_VSYNC_BACK_PORCH);

    tft = new Arduino_RGB_Display(w, h, rgbpanel, 0 /* rotation */, TFT_AUTO_FLUSH, bus, TFT_RST,
                                  st7701_4848S040_init_operations, sizeof(st7701_4848S040_init_operations));
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 480) && defined(ST7701_DRIVER)
    /* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
    Arduino_DataBus* bus            = new Arduino_SWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
    Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK, TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_G0, TFT_G1, TFT_G2, TFT_G3,
        TFT_G4, TFT_G5, TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_HSYNC_POLARITY, TFT_HSYNC_FRONT_PORCH,
        TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH, TFT_VSYNC_POLARITY, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH,
        TFT_VSYNC_BACK_PORCH);

    /* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
    tft = new Arduino_RGB_Display(w, h, rgbpanel, 0 /* rotation */, TFT_AUTO_FLUSH, bus, TFT_RST,
                                  st7701_type1_init_operations, sizeof(st7701_type1_init_operations));
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif(TFT_WIDTH == 480) && (TFT_HEIGHT == 272) && defined(NV3041A_DRIVER)
    Arduino_DataBus* bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
    tft                  = new Arduino_NV3041A(bus, TFT_RST, TFT_ROTATION, TFT_IPS);
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays

#elif 1
    /* Reset is not implemented in the panel */
    if(TFT_RST != GFX_NOT_DEFINED) {
        pinMode(TFT_RST, OUTPUT);
        digitalWrite(TFT_RST, HIGH);
        delay(100);
        digitalWrite(TFT_RST, LOW);
        delay(120);
        digitalWrite(TFT_RST, HIGH);
        delay(120);
    }

    Arduino_RGBPanel_Mod* bus = new Arduino_RGBPanel_Mod(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK, TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_G0, TFT_G1, TFT_G2, TFT_G3,
        TFT_G4, TFT_G5, TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_HSYNC_POLARITY, TFT_HSYNC_FRONT_PORCH,
        TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH, TFT_VSYNC_POLARITY, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH,
        TFT_VSYNC_BACK_PORCH, TFT_PCLK_ACTIVE_NEG, TFT_PREFER_SPEED);

    tft = new Arduino_RGB_Display_Mod(TFT_WIDTH, TFT_HEIGHT, bus);
    tft->begin(GFX_NOT_DEFINED); // Used for RFB displays
    // fb  = ((Arduino_RGBPanel_Mod*)tft)->getFramebuffer();
#endif

    // tft.setSwapBytes(true); /* set endianness */
    LOG_INFO(TAG_TFT, F(D_SERVICE_STARTED));
}

void ArduinoGfx::show_info()
{
    splashscreen();

    //     LOG_VERBOSE(TAG_TFT, F("ArduinoGfx   : v%d.%d.%d"), LGFX_VERSION_MAJOR, LGFX_VERSION_MINOR,
    //     LGFX_VERSION_PATCH); auto panel = tft.getPanel(); if(!panel) return;

    //     if(!panel->getBus()) return;
    //     lgfx::v1::bus_type_t bus_type = panel->getBus()->busType();

    //     if(bus_type == lgfx::v1::bus_parallel8) {
    //         LOG_VERBOSE(TAG_TFT, F("Interface  : Parallel 8bit"));
    //         auto bus = (lgfx::Bus_Parallel8*)panel->getBus();
    //         auto cfg = bus->config(); // Get the structure for bus configuration.
    //         tftPinInfo(F("TFT_WR"), cfg.pin_wr);
    //         tftPinInfo(F("TFT_RD"), cfg.pin_rd);
    //         tftPinInfo(F("TFT_RS"), cfg.pin_rs);

    //         tftPinInfo(F("TFT_D0"), cfg.pin_d0);
    //         tftPinInfo(F("TFT_D1"), cfg.pin_d1);
    //         tftPinInfo(F("TFT_D2"), cfg.pin_d2);
    //         tftPinInfo(F("TFT_D3"), cfg.pin_d3);
    //         tftPinInfo(F("TFT_D4"), cfg.pin_d4);
    //         tftPinInfo(F("TFT_D5"), cfg.pin_d5);
    //         tftPinInfo(F("TFT_D6"), cfg.pin_d6);
    //         tftPinInfo(F("TFT_D7"), cfg.pin_d7);
    //     }

    // #if defined(ESP32S2) || defined(ESP32S3)
    //     if(bus_type == lgfx::v1::bus_parallel16) {
    //         LOG_VERBOSE(TAG_TFT, F("Interface  : Parallel 16bit"));
    //         auto bus = (lgfx::v1::Bus_Parallel16*)panel->getBus();
    //         auto cfg = bus->config(); // Get the structure for bus configuration.
    //         tftPinInfo(F("TFT_WR"), cfg.pin_wr);
    //         tftPinInfo(F("TFT_RD"), cfg.pin_rd);
    //         tftPinInfo(F("TFT_RS"), cfg.pin_rs);

    //         tftPinInfo(F("TFT_D0"), cfg.pin_d0);
    //         tftPinInfo(F("TFT_D1"), cfg.pin_d1);
    //         tftPinInfo(F("TFT_D2"), cfg.pin_d2);
    //         tftPinInfo(F("TFT_D3"), cfg.pin_d3);
    //         tftPinInfo(F("TFT_D4"), cfg.pin_d4);
    //         tftPinInfo(F("TFT_D5"), cfg.pin_d5);
    //         tftPinInfo(F("TFT_D6"), cfg.pin_d6);
    //         tftPinInfo(F("TFT_D7"), cfg.pin_d7);
    //         tftPinInfo(F("TFT_D8"), cfg.pin_d8);
    //         tftPinInfo(F("TFT_D9"), cfg.pin_d9);
    //         tftPinInfo(F("TFT_D01"), cfg.pin_d10);
    //         tftPinInfo(F("TFT_D11"), cfg.pin_d11);
    //         tftPinInfo(F("TFT_D12"), cfg.pin_d12);
    //         tftPinInfo(F("TFT_D13"), cfg.pin_d13);
    //         tftPinInfo(F("TFT_D14"), cfg.pin_d14);
    //         tftPinInfo(F("TFT_D15"), cfg.pin_d15);
    //     }
    // #endif

    //     if(bus_type == lgfx::v1::bus_spi) {
    //         LOG_VERBOSE(TAG_TFT, F("Interface   : Serial"));
    //         auto bus = (lgfx::Bus_SPI*)panel->getBus();
    //         auto cfg = bus->config(); // Get the structure for bus configuration.
    //         tftPinInfo(F("TFT_MOSI"), cfg.pin_mosi);
    //         tftPinInfo(F("TFT_MISO"), cfg.pin_miso);
    //         tftPinInfo(F("TFT_SCLK"), cfg.pin_sclk);
    //         tftPinInfo(F("TFT_DC"), cfg.pin_dc);
    //     }

    //     {
    //         auto cfg = panel->config(); // Get the structure for panel configuration.
    //         tftPinInfo(F("TFT_CS"), cfg.pin_cs);
    //         tftPinInfo(F("TFT_RST"), cfg.pin_rst);
    //         tftPinInfo(F("TFT_BUSY"), cfg.pin_busy);
    //     }

    //     if(bus_type == lgfx::v1::bus_spi) {
    //         auto bus      = (lgfx::Bus_SPI*)panel->getBus();
    //         auto cfg      = bus->config(); // Get the structure for bus configuration.
    //         uint32_t freq = cfg.freq_write / 100000;
    //         LOG_VERBOSE(TAG_TFT, F("TFT SPI freq: %d.%d MHz"), freq / 10, freq % 10);
    //     }

    //     lgfx::v1::ITouch* touch = panel->getTouch();
    //     if(touch) {
    //         auto cfg      = touch->config(); // Get the structure for bus configuration.
    //         uint32_t freq = cfg.freq / 100000;
    //         tftPinInfo(F("TOUCH_INT"), cfg.pin_int);
    //         tftPinInfo(F("TOUCH_RST"), cfg.pin_rst);
    //         if(touch->isSPI()) {
    //             tftPinInfo(F("TOUCH_MISO"), cfg.pin_miso);
    //             tftPinInfo(F("TOUCH_MOSI"), cfg.pin_mosi);
    //             tftPinInfo(F("TOUCH_SCLK"), cfg.pin_sclk);
    //             tftPinInfo(F("TOUCH_CS"), cfg.pin_cs);
    //             LOG_VERBOSE(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), freq / 10, freq % 10);
    //         } else {
    //             tftPinInfo(F("TOUCH_SDA"), cfg.pin_sda);
    //             tftPinInfo(F("TOUCH_SCL"), cfg.pin_scl);
    //             tftPinInfo(F("TOUCH_ADDR"), cfg.i2c_addr);
    //             LOG_VERBOSE(TAG_TFT, F("Touch I2C freq.   : %d.%d MHz"), freq / 10, freq % 10);
    //         }
    //     }
}

void ArduinoGfx::splashscreen()
{
    uint8_t fg[]       = logoFgColor;
    uint8_t bg[]       = logoBgColor;
    lv_color_t fgColor = lv_color_make(fg[0], fg[1], fg[2]);
    lv_color_t bgColor = lv_color_make(bg[0], bg[1], bg[2]);

    tft->fillScreen(bgColor.full);
    int x = (tft->width() - logoWidth) / 2;
    int y = (tft->height() - logoHeight) / 2;
    tft->drawXBitmap(x, y, logoImage, logoWidth, logoHeight, fgColor.full);
}

void ArduinoGfx::set_rotation(uint8_t rotation)
{
    LOG_VERBOSE(TAG_TFT, F("Rotation    : %d"), rotation);
    tft->setRotation(rotation);
}

void ArduinoGfx::set_invert(bool invert)
{
    char buffer[4];
    memcpy_P(buffer, invert ? PSTR(D_YES) : PSTR(D_NO), sizeof(buffer));

    LOG_VERBOSE(TAG_TFT, F("Invert Color: %s"), buffer);
    tft->invertDisplay(invert);
}

/* Update TFT */
void IRAM_ATTR ArduinoGfx::flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if(LV_COLOR_16_SWAP != 0)
    tft->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);
#else
    tft->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

bool ArduinoGfx::is_driver_pin(uint8_t pin)
{
    if(false // start condition is always needed

// Use individual checks instead of switch statement, as some case labels could be duplicated
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
#ifdef TFT_DE
       || (pin == TFT_DE)
#endif
#ifdef TFT_PCLK
       || (pin == TFT_PCLK)
#endif
#ifdef TFT_VSYNC
       || (pin == TFT_VSYNC)
#endif
#ifdef TFT_HSYNC
       || (pin == TFT_HSYNC)
#endif
#ifdef TFT_RST
       || (pin == TFT_RST)
#endif
#ifdef TFT_BUSY
       || (pin == TFT_BUSY)
#endif
#ifdef TFT_RD
       || (pin == TFT_RD)
#endif
#ifdef TFT_R0
       || (pin == TFT_R0)
#endif
#ifdef TFT_R1
       || (pin == TFT_R1)
#endif
#ifdef TFT_R2
       || (pin == TFT_R2)
#endif
#ifdef TFT_R3
       || (pin == TFT_R3)
#endif
#ifdef TFT_R4
       || (pin == TFT_R4)
#endif
#ifdef TFT_G0
       || (pin == TFT_G0)
#endif
#ifdef TFT_G1
       || (pin == TFT_G1)
#endif
#ifdef TFT_G2
       || (pin == TFT_G2)
#endif
#ifdef TFT_G3
       || (pin == TFT_G3)
#endif
#ifdef TFT_G4
       || (pin == TFT_G4)
#endif
#ifdef TFT_G5
       || (pin == TFT_G5)
#endif
#ifdef TFT_B0
       || (pin == TFT_B0)
#endif
#ifdef TFT_B1
       || (pin == TFT_B1)
#endif
#ifdef TFT_B2
       || (pin == TFT_B2)
#endif
#ifdef TFT_B3
       || (pin == TFT_B3)
#endif
#ifdef TFT_B4
       || (pin == TFT_B4)
#endif
#ifdef TOUCH_SDA
       || (pin == TOUCH_SDA)
#endif
#ifdef TOUCH_SCL
       || (pin == TOUCH_SCL)
#endif
#ifdef TOUCH_RST
       || (pin == TOUCH_RST)
#endif
#ifdef TOUCH_IRQ
       || (pin == TOUCH_IRQ)
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
    return false;
}

const char* ArduinoGfx::get_tft_model()
{
#if defined(ILI9341_DRIVER)
    return "ILI9341";
#elif defined(ILI9342_DRIVER)
    return "ILI9342";
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
#elif defined(R61529_DRIVER)
    return "R61529";
#elif defined(RM68140_DRIVER)
    return "RM68140";
#elif defined(NV3041A_DRIVER)
    return "NV3041A";
#else
    return "Other";
#endif
}

uint32_t ArduinoGfx::get_tft_driver()
{
#if defined(ILI9341_DRIVER)
    return 0x9341;
#elif defined(ST7735_DRIVER)
    return 0x7735;
#elif defined(ILI9163_DRIVER)
    return 0x9163;
#elif defined(S6D02A1_DRIVER)
    return 0x6D02A1;
#elif defined(ST7796_DRIVER)
    return 0x7796;
#elif defined(ILI9486_DRIVER)
    return 0x9486;
#elif defined(ILI9481_DRIVER)
    return 0x9481;
#elif defined(ILI9488_DRIVER)
    return 0x9488;
#elif defined(HX8357D_DRIVER)
    return 0x8357D;
#elif defined(EPD_DRIVER)
    return 0xED;
#elif defined(ST7789_DRIVER)
    return 0x7789;
#elif defined(ST7789_2_DRIVER)
    return 0x77892;
#elif defined(R61581_DRIVER)
    return 0x61581;
#elif defined(R61529_DRIVER)
    return 0x61529;
#elif defined(RM68140_DRIVER)
    return 0x68140;
#elif defined(NV3041A_DRIVER)
    return 0x3041A;
#else
    return 0x0;
#endif
}

} // namespace dev

dev::ArduinoGfx haspTft;
#endif