/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && defined(LGFX_USE_V1)
#include "tft_driver_lovyangfx.h"

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

    LOG_VERBOSE(TAG_DRVR, "[Autodetect] read cmd:%u = %u", cmd, res);
    LOG_VERBOSE(TAG_DRVR, "[Autodetect] read cmd:%02x = %08x", cmd, res);
    return res;
}

void LovyanGfx::init(int w, int h)
{
#ifdef USE_DMA_TO_TFT
    int dma_channel = 1; // Set the DMA channel (1 or 2. 0=disable)
#else
    int dma_channel = 1; // Set the DMA channel (1 or 2. 0=disable)
#endif

    uint32_t tft_driver = 0;

#ifdef ST7796_DRIVER
    tft_driver = 0x7796;
#endif

#ifdef ILI9481_DRIVER
    tft_driver = 0x9481;
#endif

#ifdef ILI9341_DRIVER
    tft_driver = 0x9481;
#endif

#ifdef ILI9488_DRIVER
    tft_driver = 0x9488;
#endif

if (tft_driver == 0x9341)
    tft._panel_instance = new lgfx::Panel_ILI9341();
else if (tft_driver == 0x9481)
    tft._panel_instance = new lgfx::Panel_ILI9481();
else if (tft_driver == 0x9488)
    tft._panel_instance = new lgfx::Panel_ILI9488();
else if (tft_driver == 0x7796)
    tft._panel_instance = new lgfx::Panel_ST7796();

    { // バス制御の設定を行います。
        auto bus     = (lgfx::v1::Bus_SPI*)tft._bus_instance;
        auto cfg     = bus->config(); // バス設定用の構造体を取得します。
        cfg.spi_host = VSPI_HOST;     // 使用するSPIを選択  (VSPI_HOST or HSPI_HOST)
        cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
        cfg.freq_write = SPI_FREQUENCY; // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
        cfg.freq_read   = SPI_READ_FREQUENCY; // 受信時のSPIクロック
        cfg.spi_3wire   = false;              // 受信をMOSIピンで行う場合はtrueを設定
        cfg.use_lock    = true;               // トランザクションロックを使用する場合はtrueを設定
        cfg.dma_channel = dma_channel;        // Set the DMA channel (1 or 2. 0=disable)
        cfg.pin_sclk    = TFT_SCLK;           // SPIのSCLKピン番号を設定
        cfg.pin_mosi    = TFT_MOSI;           // SPIのMOSIピン番号を設定
        cfg.pin_miso    = TFT_MISO;           // SPIのMISOピン番号を設定 (-1 = disable)
        cfg.pin_dc      = TFT_DC;             // SPIのD/Cピン番号を設定  (-1 = disable)
        bus->config(cfg);                     // 設定値をバスに反映します。
        bus->init();
        _read_panel_id(bus, TFT_CS, 0x00);
        _read_panel_id(bus, TFT_CS, 0x04);
        _read_panel_id(bus, TFT_CS, 0x09);
        _read_panel_id(bus, TFT_CS, 0xBF);
        tft._panel_instance->setBus(bus); // バスをパネルにセットします。
    }

    {                                                      // 表示パネル制御の設定を行います。
        auto cfg          = tft._panel_instance->config(); // 表示パネル設定用の構造体を取得します。
        cfg.pin_cs        = TFT_CS;                        // CSが接続されているピン番号   (-1 = disable)
        cfg.pin_rst       = TFT_RST;                       // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_busy      = -1;                            // BUSYが接続されているピン番号 (-1 = disable)
        cfg.memory_width  = w;                             // ドライバICがサポートしている最大の幅
        cfg.memory_height = h;                             // ドライバICがサポートしている最大の高さ
        cfg.panel_width   = w;                             // 実際に表示可能な幅
        cfg.panel_height  = h;                             // 実際に表示可能な高さ
        cfg.offset_x      = 0;                             // パネルのX方向オフセット量
        cfg.offset_y      = 0;                             // パネルのY方向オフセット量
        cfg.offset_rotation  = 0;     // 回転方向の値のオフセット 0~7 (4~7は上下反転)
        cfg.dummy_read_pixel = 8;     // ピクセル読出し前のダミーリードのビット数
        cfg.dummy_read_bits  = 1;     // ピクセル以外のデータ読出し前のダミーリードのビット数
        cfg.readable         = true;  // データ読出しが可能な場合 trueに設定
        cfg.invert           = false; // パネルの明暗が反転してしまう場合 trueに設定
        cfg.rgb_order        = false; // パネルの赤と青が入れ替わってしまう場合 trueに設定
        cfg.dlen_16bit       = false; // データ長を16bit単位で送信するパネルの場合 trueに設定
        cfg.bus_shared = true; // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

        tft._panel_instance->config(cfg);
    }

    { // バックライト制御の設定を行います。（必要なければ削除）
        auto cfg = tft._light_instance.config(); // バックライト設定用の構造体を取得します。

        cfg.pin_bl      = TFT_BCKL; // バックライトが接続されているピン番号
        cfg.invert      = false;    // バックライトの輝度を反転させる場合 true
        cfg.freq        = 44100;    // バックライトのPWM周波数
        cfg.pwm_channel = 0;        // 使用するPWMのチャンネル番号

        tft._light_instance.config(cfg);
        tft._panel_instance->setLight(&tft._light_instance); // バックライトをパネルにセットします。
    }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
        auto cfg            = tft._touch_instance.config();
        cfg.x_min           = 0;    // タッチスクリーンから得られる最小のX値(生の値)
        cfg.x_max           = 319;  // タッチスクリーンから得られる最大のX値(生の値)
        cfg.y_min           = 0;    // タッチスクリーンから得られる最小のY値(生の値)
        cfg.y_max           = 479;  // タッチスクリーンから得られる最大のY値(生の値)
        cfg.pin_int         = -1;   // INTが接続されているピン番号
        cfg.bus_shared      = true; // 画面と共通のバスを使用している場合 trueを設定
        cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
        cfg.spi_host        = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
        cfg.pin_sclk        = TFT_SCLK;  // SCLKが接続されているピン番号
        cfg.pin_mosi        = TFT_MOSI;  // MOSIが接続されているピン番号
        cfg.pin_miso        = TFT_MISO;  // MISOが接続されているピン番号
#ifdef TOUCH_CS
        cfg.pin_cs = TOUCH_CS;            //   CSが接続されているピン番号
        cfg.freq   = SPI_TOUCH_FREQUENCY; // SPIクロックを設定
#else
        cfg.pin_sda = TOUCH_SDA;
        cfg.pin_scl = TOUCH_SCL;
        cfg.freq    = I2C_TOUCH_FREQUENCY; // SPIクロックを設定
#endif
        tft._touch_instance.config(cfg);
        tft._panel_instance->setTouch(&tft._touch_instance); // タッチスクリーンをパネルにセットします。
    }
    tft.setPanel(tft._panel_instance); // 使用するパネルをセットします。

    /* TFT init */
    tft.begin();
    tft.setSwapBytes(true); /* set endianess */
}

void LovyanGfx::show_info()
{
    splashscreen();

    LOG_VERBOSE(TAG_TFT, F("LovyanGFX  : v%d.%d.%d"), LGFX_VERSION_MAJOR, LGFX_VERSION_MINOR, LGFX_VERSION_PATCH);

    //     LOG_VERBOSE(TAG_TFT, F("Transactns : %s"), (tftSetup.trans == 1) ? PSTR(D_YES) : PSTR(D_NO));
    //     LOG_VERBOSE(TAG_TFT, F("Interface  : %s"), (tftSetup.serial == 1) ? PSTR("SPI") : PSTR("Parallel"));

    {
        auto bus = (lgfx::v1::Bus_SPI*)tft._bus_instance;
        auto cfg = bus->config(); // バス設定用の構造体を取得します。
        tftPinInfo(F("MOSI"), cfg.pin_mosi);
        tftPinInfo(F("MISO"), cfg.pin_miso);
        tftPinInfo(F("SCLK"), cfg.pin_sclk);
        tftPinInfo(F("TFT_DC"), cfg.pin_dc);
    }

    {
        auto cfg = tft._panel_instance->config(); // バス設定用の構造体を取得します。
        tftPinInfo(F("TFT_CS"), cfg.pin_cs);
        tftPinInfo(F("TFT_RST"), cfg.pin_rst);
    }

    {
        auto bus      = (lgfx::v1::Bus_SPI*)tft._bus_instance;
        auto cfg      = bus->config(); // バス設定用の構造体を取得します。
        uint32_t freq = cfg.freq_write / 100000;
        LOG_VERBOSE(TAG_TFT, F("Display SPI freq. : %d.%d MHz"), freq / 10, freq % 10);
    }

    {
        auto cfg = tft._touch_instance.config(); // バス設定用の構造体を取得します。
        if(cfg.pin_cs != -1) {
            tftPinInfo(F("TOUCH_CS"), cfg.pin_cs);
            uint32_t freq = cfg.freq / 100000;
            LOG_VERBOSE(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), freq / 10, freq % 10);
        }
    }

    //     tftPinInfo(F("TFT_WR"), tftSetup.pin_tft_wr);
    //     tftPinInfo(F("TFT_RD"), tftSetup.pin_tft_rd);

    //     tftPinInfo(F("TFT_D0"), tftSetup.pin_tft_d0);
    //     tftPinInfo(F("TFT_D1"), tftSetup.pin_tft_d1);
    //     tftPinInfo(F("TFT_D2"), tftSetup.pin_tft_d2);
    //     tftPinInfo(F("TFT_D3"), tftSetup.pin_tft_d3);
    //     tftPinInfo(F("TFT_D4"), tftSetup.pin_tft_d4);
    //     tftPinInfo(F("TFT_D5"), tftSetup.pin_tft_d5);
    //     tftPinInfo(F("TFT_D6"), tftSetup.pin_tft_d6);
    //     tftPinInfo(F("TFT_D7"), tftSetup.pin_tft_d7);

    //     if(tftSetup.serial == 1) {
    //         LOG_VERBOSE(TAG_TFT, F("Display SPI freq. : %d.%d MHz"), tftSetup.tft_spi_freq / 10,
    //                     tftSetup.tft_spi_freq % 10);
    //     }
    //     if(tftSetup.pin_tch_cs != -1) {
    //         LOG_VERBOSE(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), tftSetup.tch_spi_freq / 10,
    //                     tftSetup.tch_spi_freq % 10);
    //     }
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

} // namespace dev

dev::LovyanGfx haspTft;
#endif