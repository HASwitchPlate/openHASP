/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && defined(LGFX_USE_V1)
#include "tft_driver_lovyangfx.h"
#include <Preferences.h>

namespace dev {

static void _pin_level(int_fast16_t pin, bool level)
{
    lgfx::pinMode(pin, lgfx::pin_mode_t::output);
    if(level)
        lgfx::gpio_hi(pin);
    else
        lgfx::gpio_lo(pin);
}

static uint32_t _read_panel_id(lgfx::Bus_SPI* bus, int32_t pin_cs, uint32_t cmd = 0x04,
                               uint8_t dummy_read_bit = 1) // 0x04 = RDDID command
{
    bus->beginTransaction();
    _pin_level(pin_cs, true);
    bus->writeCommand(0, 8);
    bus->wait();
    _pin_level(pin_cs, false);
    bus->writeCommand(cmd, 8);
    if(dummy_read_bit) bus->writeData(0, dummy_read_bit); // dummy read bit
    bus->beginRead();
    uint32_t res = bus->readData(32);
    bus->endTransaction();
    _pin_level(pin_cs, true);

    LOG_VERBOSE(TAG_TFT, "[Autodetect] read cmd:%u = %u", cmd, res);
    return res;
}

void LovyanGfx::init(int w, int h)
{
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

    Preferences preferences;
    preferences.begin("gfx", false);

#ifdef USE_DMA_TO_TFT
    int dma_channel = 1; // Set the DMA channel (1 or 2. 0=disable)
#else
    int dma_channel = 0; // Set the DMA channel (1 or 2. 0=disable)
#endif

    uint32_t tft_driver = preferences.getUInt("DRIVER", get_tft_driver());
    switch(tft_driver) {
        case 0x9341:
            tft._panel_instance = new lgfx::Panel_ILI9341();
            break;
        case 0x9481:
            tft._panel_instance = new lgfx::Panel_ILI9481();
            break;
        case 0x9488:
            tft._panel_instance = new lgfx::Panel_ILI9488();
            break;
        case 0x7796:
            tft._panel_instance = new lgfx::Panel_ST7796();
            break;
        case 0x8357D:
            tft._panel_instance = new lgfx::Panel_HX8357D();
            break;
        default: {
            LOG_ERROR(TAG_TFT, F("Unknown display driver")); // Needs to be in curly braces
        }
    }
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

#ifdef ESP32_PARALLEL
    { // Set 8-bit parallel bus control
        auto bus       = (lgfx::v1::Bus_Parallel8*)tft._bus_instance;
        auto cfg       = bus->config(); // バス設定用の構造体を取得します。
        cfg.i2s_port   = I2S_NUM_0;
        cfg.freq_write = 20000000;
        cfg.pin_wr     = TFT_WR;
        cfg.pin_rd     = TFT_RD;
        cfg.pin_rs     = TFT_DC;
        cfg.pin_d0     = TFT_D0;
        cfg.pin_d1     = TFT_D1;
        cfg.pin_d2     = TFT_D2;
        cfg.pin_d3     = TFT_D3;
        cfg.pin_d4     = TFT_D4;
        cfg.pin_d5     = TFT_D5;
        cfg.pin_d6     = TFT_D6;
        cfg.pin_d7     = TFT_D7;
        bus->config(cfg); // 設定値をバスに反映します。
        tft._panel_instance->setBus(bus);          // Set the bus on the panel.
    }
#else
    {                    // Set SPI bus control
        auto bus       = (lgfx::v1::Bus_SPI*)tft._bus_instance;
        auto cfg       = bus->config(); // Get the structure for bus configuration.
        cfg.spi_host   = FSPI_HOST;     // Select the SPI to use  (VSPI_HOST or HSPI_HOST)
        cfg.spi_mode   = 0;             // Set SPI communication mode  (0 ~ 3)
        cfg.freq_write = SPI_FREQUENCY; // SPI clock during transmission  (Max 80MHz, 80MHz Can be rounded to the value
                                        // divided by an integer )
        cfg.freq_read   = SPI_READ_FREQUENCY;                    // SPI clock when receiving
        cfg.spi_3wire   = (TFT_MOSI == -1);                      // true when receiving with MOSI pin
        cfg.use_lock    = true;                                  // Set to true when using transaction lock
        cfg.dma_channel = dma_channel;                           // Set the DMA channel (1 or 2. 0=disable)
        cfg.pin_sclk    = preferences.getChar("SCLK", TFT_SCLK); // Set SPI SCLK pin number
        cfg.pin_mosi    = preferences.getChar("MOSI", TFT_MOSI); // Set SPI MOSI pin number
        cfg.pin_miso    = preferences.getChar("MISO", TFT_MISO); // Set SPI MISO pin number  (-1 = disable)
        cfg.pin_dc      = preferences.getChar("DC", TFT_DC);     // Set SPI D/C pin number   (-1 = disable)
        bus->config(cfg);                                        // The set value is reflected on the bus.
        bus->init();
        int8_t cs = preferences.getChar("CS", TFT_CS);
        _read_panel_id(bus, cs, 0x00);    // NOP
        _read_panel_id(bus, cs, 0x04);    // ST7789/ILI9488: RDDID (04h): Read Display ID
        _read_panel_id(bus, cs, 0x09);    // ST7789/ILI9488: RDDST (09h): Read Display Status
        _read_panel_id(bus, cs, 0xBF);    // /ILI9481: Device Code Read
        tft._panel_instance->setBus(bus); // Set the bus on the panel.
    }
#endif
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

    {                                                               // Set the display panel control.
        auto cfg             = tft._panel_instance->config();       // Gets the structure for display panel settings.
        cfg.pin_cs           = preferences.getChar("CS", TFT_CS);   // CS Pin Number   (-1 = disable)
        cfg.pin_rst          = preferences.getChar("RST", TFT_RST); // RST Pin Number  (-1 = disable)
        cfg.pin_busy         = -1;                                  // BUSY Pin Number (-1 = disable)
        cfg.memory_width     = w;                                   // Maximum width supported by  driver IC
        cfg.memory_height    = h;                                   // Maximum height supported by driver IC
        cfg.panel_width      = w;                                   // Actually displayable width
        cfg.panel_height     = h;                                   // Actually displayable height
        cfg.offset_x         = 0;                                   // Amount of X-direction offset of the panel
        cfg.offset_y         = 0;                                   // Amount of Y-direction offset of the panel
        cfg.offset_rotation  = 0;     // Offset of values in the direction of rotation 0 ~ 7 (4 ~ 7 are upside down)
        cfg.dummy_read_pixel = 8;     // Number of dummy read bits before pixel reading
        cfg.dummy_read_bits  = 1;     // Number of bits of dummy read before reading data other than pixels
        cfg.readable         = true;  // Set to true if data can be read
        cfg.invert           = false; // Set to true if the light and darkness of the panel is reversed
        cfg.rgb_order        = false; // Set to true if the red and blue of the panel are swapped
        cfg.dlen_16bit       = false; // Set to true for panels that send data length in 16-bit units
        cfg.bus_shared =
            true; // Set to true if the bus is shared with the SD card (bus control is performed with drawJpgFile etc.)
        tft._panel_instance->config(cfg);
    }

    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

#if 0
    {                                            // Set the backlight control. (Delete if not needed)
        auto cfg = tft._light_instance.config(); // Get the backlight structure for configuration.

        cfg.pin_bl      = preferences.getChar("BCKL", TFT_BCKL); // Backlight Pin Number
        cfg.invert      = false; // True if you want to invert the brightness of the Backlight
        cfg.freq        = 44100; // Backlight PWM frequency
        cfg.pwm_channel = 0;     // PWM channel number to use

        tft._light_instance.config(cfg);
        tft._panel_instance->setLight(&tft._light_instance); // Set the Backlight on the panel.
    }
#endif

    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

    uint32_t touch_driver = preferences.getUInt("T_DRIVER", get_touch_driver());
    switch(touch_driver) {
        case 0x0911:
            tft._touch_instance = new lgfx::Touch_GT911();
            break;
        case 0x5206:
        case 0x6336:
            tft._touch_instance = new lgfx::Touch_FT5x06();
            break;
        case 0x2046:
            tft._touch_instance = new lgfx::Touch_XPT2046();
            break;
        case 0x0610:
            tft._touch_instance = new lgfx::Touch_STMPE610();
            break;
        default: {
            LOG_ERROR(TAG_TFT, F("Unknown touch driver")); // Needs to be in curly braces
        };
    }

    if(touch_driver == 0x2046 || touch_driver == 0x0610) { // Set the touch screen control. (Delete if not needed)
        auto cfg            = tft._touch_instance->config();
        cfg.pin_int         = TOUCH_IRQ;      // INT Pin Number
        cfg.offset_rotation = 0;              // Adjustment when the display and touch orientation do not match:
                                              // Set with a value from 0 to 7
        cfg.bus_shared = true;                // Set to true if you are using the same bus as the screen
        cfg.spi_host   = FSPI_HOST;           // Select the SPI to use  (HSPI_HOST or VSPI_HOST)
        cfg.pin_sclk   = TFT_SCLK;            // SCLK Pin Number
        cfg.pin_mosi   = TFT_MOSI;            // MOSI Pin Number
        cfg.pin_miso   = TFT_MISO;            // MISO Pin Number
        cfg.pin_cs     = TOUCH_CS;            //   CS Pin Number
        cfg.freq       = SPI_TOUCH_FREQUENCY; // Set SPI clock
        cfg.x_min      = 0;                   // Minimum X value (raw value) obtained from touch screen
        cfg.x_max      = w - 1;               // Maximum X value (raw value) obtained from touch screen
        cfg.y_min      = 0;                   // Minimum Y value (raw value) obtained from touch screen
        cfg.y_max      = h - 1;               // Maximum Y value (raw value) obtained from touch screen

        tft._touch_instance->config(cfg);
        tft._panel_instance->setTouch(tft._touch_instance); // Set the touch screen on the panel.
    }

    if(touch_driver == 0x6336 || touch_driver == 0x5206 ||
       touch_driver == 0x0911) { // Set the touch screen control. (Delete if not needed)
        auto cfg            = tft._touch_instance->config();
        cfg.pin_int         = TOUCH_IRQ; // INT Pin Number
        cfg.offset_rotation = 0;         // Adjustment when the display and touch orientation do not match:

        cfg.bus_shared = false; // Set to true if you are using the same bus as the screen
        cfg.pin_sda    = TOUCH_SDA;
        cfg.pin_scl    = TOUCH_SCL;
        cfg.i2c_port   = I2C_TOUCH_PORT;      // Select I2C to use (0 or 1)
        cfg.i2c_addr   = I2C_TOUCH_ADDRESS;   // I2C device address number
        cfg.freq       = I2C_TOUCH_FREQUENCY; // Set I2C clock
        cfg.x_min      = 0;                   // Minimum X value (raw value) obtained from touch screen
        cfg.x_max      = w - 1;               // Maximum X value (raw value) obtained from touch screen
        cfg.y_min      = 0;                   // Minimum Y value (raw value) obtained from touch screen
        cfg.y_max      = h - 1;               // Maximum Y value (raw value) obtained from touch screen
        tft._touch_instance->config(cfg);
        tft._panel_instance->setTouch(tft._touch_instance); // Set the touch screen on the panel.
    }

    tft.setPanel(tft._panel_instance); // Set the panel to be used.
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    preferences.end();

    /* TFT init */
    tft.begin();
    tft.setSwapBytes(true); /* set endianess */

    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
}

void LovyanGfx::show_info()
{
    splashscreen();

    LOG_VERBOSE(TAG_TFT, F("LovyanGFX  : v%d.%d.%d"), LGFX_VERSION_MAJOR, LGFX_VERSION_MINOR, LGFX_VERSION_PATCH);

#ifdef ESP32_PARALLEL
    {
        LOG_VERBOSE(TAG_TFT, F("Interface  : Parallel"));
        auto bus = (lgfx::v1::Bus_Parallel8*)tft._bus_instance;
        auto cfg = bus->config(); // Get the structure for bus configuration.
        tftPinInfo(F("TFT_WR"), cfg.pin_wr);
        tftPinInfo(F("TFT_RD"), cfg.pin_rd);
        tftPinInfo(F("TFT_RS"), cfg.pin_rs);

        tftPinInfo(F("TFT_D0"), cfg.pin_d0);
        tftPinInfo(F("TFT_D1"), cfg.pin_d1);
        tftPinInfo(F("TFT_D2"), cfg.pin_d2);
        tftPinInfo(F("TFT_D3"), cfg.pin_d3);
        tftPinInfo(F("TFT_D4"), cfg.pin_d4);
        tftPinInfo(F("TFT_D5"), cfg.pin_d5);
        tftPinInfo(F("TFT_D6"), cfg.pin_d6);
        tftPinInfo(F("TFT_D7"), cfg.pin_d7);
    }
#else
    {
        LOG_VERBOSE(TAG_TFT, F("Interface  : Serial"));
        auto bus = (lgfx::v1::Bus_SPI*)tft._bus_instance;
        auto cfg = bus->config(); // Get the structure for bus configuration.
        tftPinInfo(F("MOSI"), cfg.pin_mosi);
        tftPinInfo(F("MISO"), cfg.pin_miso);
        tftPinInfo(F("SCLK"), cfg.pin_sclk);
        tftPinInfo(F("TFT_DC"), cfg.pin_dc);
    }
#endif

    {
        auto cfg = tft._panel_instance->config(); // Get the structure for bus configuration.
        tftPinInfo(F("TFT_CS"), cfg.pin_cs);
        tftPinInfo(F("TFT_RST"), cfg.pin_rst);
    }

#ifndef ESP32_PARALLEL
    {
        auto bus      = (lgfx::v1::Bus_SPI*)tft._bus_instance;
        auto cfg      = bus->config(); // Get the structure for bus configuration.
        uint32_t freq = cfg.freq_write / 100000;
        LOG_VERBOSE(TAG_TFT, F("Display SPI freq. : %d.%d MHz"), freq / 10, freq % 10);
    }
#endif

    {
        auto cfg = tft._touch_instance->config(); // Get the structure for bus configuration.
        if(cfg.pin_cs != -1) {
            tftPinInfo(F("TOUCH_CS"), cfg.pin_cs);
            uint32_t freq = cfg.freq / 100000;
            LOG_VERBOSE(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), freq / 10, freq % 10);
        }
        if(cfg.pin_sda != -1) {
            tftPinInfo(F("TOUCH_SDA"), cfg.pin_sda);
        }
        if(cfg.pin_scl != -1) {
            tftPinInfo(F("TOUCH_SCL"), cfg.pin_scl);
            uint32_t freq = cfg.freq / 100000;
            LOG_VERBOSE(TAG_TFT, F("Touch I2C freq.   : %d.%d MHz"), freq / 10, freq % 10);
        }
    }
}

void LovyanGfx::splashscreen()
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

void LovyanGfx::set_rotation(uint8_t rotation)
{
    LOG_VERBOSE(TAG_TFT, F("Rotation   : %d"), rotation);
    tft.setRotation(rotation);
}

void LovyanGfx::set_invert(bool invert)
{
    char buffer[4];
    memcpy_P(buffer, invert ? PSTR(D_YES) : PSTR(D_NO), sizeof(buffer));

    LOG_VERBOSE(TAG_TFT, F("Invert Disp: %s"), buffer);
    tft.invertDisplay(invert);
}

/* Update TFT */
void IRAM_ATTR LovyanGfx::flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    uint32_t w   = (area->x2 - area->x1 + 1);
    uint32_t h   = (area->y2 - area->y1 + 1);
    uint32_t len = w * h;

    tft.startWrite();                            /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, w, h); /* set the working window */
    tft.writePixels((uint16_t*)color_p, len);    /* Write words at once */
    tft.endWrite();                              /* terminate TFT transaction */

    /* Tell lvgl that flushing is done */
    lv_disp_flush_ready(disp);
}

bool LovyanGfx::is_driver_pin(uint8_t pin)
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

const char* LovyanGfx::get_tft_model()
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

uint32_t LovyanGfx::get_tft_driver()
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
#elif defined(R61581_DRIVER)
    return 0x61581;
#elif defined(ST7789_2_DRIVER)
    return 0x77892;
#elif defined(RM68140_DRIVER)
    return 0x68140;
#else
    return 0;
#endif
}

uint32_t LovyanGfx::get_touch_driver()
{
#ifdef TOUCH_DRIVER
    return TOUCH_DRIVER > 0 ? TOUCH_DRIVER : 0;
#else
    return 0;
#endif
}

} // namespace dev

dev::LovyanGfx haspTft;
#endif