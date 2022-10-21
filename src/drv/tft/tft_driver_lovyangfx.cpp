/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && defined(LGFX_USE_V1)
#include "tft_driver_lovyangfx.h"
#include <Preferences.h>

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

#if defined(ESP32S2) || defined(ESP32S3)
static lgfx::Bus_Parallel16* init_parallel_16_bus(Preferences* prefs, int8_t data_pins[], uint8_t num)
{
    lgfx::Bus_Parallel16* bus = new lgfx::v1::Bus_Parallel16();
    auto cfg                  = bus->config(); // バス設定用の構造体を取得します。
    cfg.pin_rd                = prefs->getInt("rd", TFT_RD);
    cfg.pin_wr                = prefs->getInt("wr", TFT_WR);
    cfg.pin_rs                = prefs->getInt("rs", TFT_DC);
    cfg.freq_write            = prefs->getUInt("write_freq", SPI_FREQUENCY);
#if !defined(CONFIG_IDF_TARGET_ESP32S3)
    uint8_t port = prefs->getUInt("i2s_port", 0);
    switch(port) {
#if SOC_I2S_NUM > 1
        case 1:
            cfg.i2s_port = I2S_NUM_1;
            break;
#endif
        default:
            cfg.i2s_port = I2S_NUM_0;
    }
#endif
    for(uint8_t i = 0; i < num; i++) {
        cfg.pin_data[i] = data_pins[i];
    }
    bus->config(cfg); // The set value is reflected on the bus.
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    return bus;
}
#endif // ESP32S2

static lgfx::Bus_Parallel8* init_parallel_8_bus(Preferences* prefs, int8_t data_pins[], uint8_t num)
{
    lgfx::Bus_Parallel8* bus = new lgfx::v1::Bus_Parallel8();
    auto cfg                 = bus->config(); // バス設定用の構造体を取得します。
    cfg.pin_rd               = prefs->getInt("rd", TFT_RD);
    cfg.pin_wr               = prefs->getInt("wr", TFT_WR);
    cfg.pin_rs               = prefs->getInt("rs", TFT_DC);
    cfg.freq_write           = prefs->getUInt("write_freq", SPI_FREQUENCY);
#if !defined(CONFIG_IDF_TARGET_ESP32S3)
    uint8_t port = prefs->getUInt("i2s_port", 0);
    switch(port) {
#if SOC_I2S_NUM > 1
        case 1:
            cfg.i2s_port = I2S_NUM_1;
            break;
#endif
        default:
            cfg.i2s_port = I2S_NUM_0;
    }
#endif
    for(uint8_t i = 0; i < num; i++) {
        cfg.pin_data[i] = data_pins[i];
    }
    bus->config(cfg); // The set value is reflected on the bus.
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    return bus;
}

static lgfx::Bus_SPI* init_spi_bus(Preferences* prefs)
{
    lgfx::Bus_SPI* bus = new lgfx::v1::Bus_SPI();
    auto cfg           = bus->config(); // バス設定用の構造体を取得します。
    cfg.pin_miso       = prefs->getInt("miso", TFT_MISO);
    cfg.pin_mosi       = prefs->getInt("mosi", TFT_MOSI);
    cfg.pin_sclk       = prefs->getInt("sclk", TFT_SCLK);
    cfg.pin_dc         = prefs->getInt("dc", TFT_DC);
    cfg.spi_3wire      = prefs->getBool("3wire", false);
    cfg.use_lock       = prefs->getBool("use_lock", true);
    cfg.freq_write     = prefs->getUInt("write_freq", SPI_FREQUENCY);
    cfg.freq_read      = prefs->getUInt("read_freq", SPI_READ_FREQUENCY);
    cfg.dma_channel    = prefs->getUInt("dma_channel", 0);
    cfg.spi_mode       = prefs->getUInt("spi_mode", 0);
    uint8_t host       = prefs->getUInt("host", 3);
    LOG_VERBOSE(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

    // #if CONFIG_IDF_TARGET_ESP32C3
    // #define FSPI 0
    // #define HSPI 1
    // #else
    // #define FSPI 1 // SPI bus attached to the flash (can use the same data lines but different SS)
    // #define HSPI 2 // SPI bus normally mapped to pins 12 - 15, but can be matrixed to any pins
    // #if CONFIG_IDF_TARGET_ESP32
    // #define VSPI 3 // SPI bus normally attached to pins 5, 18, 19 and 23, but can be matrixed to any pins
    // #endif
    // #endif

    // //alias for different chips, deprecated for the chips after esp32s2
    // #ifdef CONFIG_IDF_TARGET_ESP32
    // #define SPI_HOST    SPI1_HOST
    // #define HSPI_HOST   SPI2_HOST
    // #define VSPI_HOST   SPI3_HOST
    // #elif CONFIG_IDF_TARGET_ESP32S2
    // // SPI_HOST (SPI1_HOST) is not supported by the SPI Master and SPI Slave driver on ESP32-S2 and later
    // #define SPI_HOST    SPI1_HOST
    // #define FSPI_HOST   SPI2_HOST
    // #define HSPI_HOST   SPI3_HOST
    // #endif

    switch(host) {
#ifdef CONFIG_IDF_TARGET_ESP32
        case 1:
            // SPI_HOST (SPI1_HOST) is not supported by the SPI Master and SPI Slave driver on ESP32-S2 and later
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            cfg.spi_host = SPI_HOST;
            break;
#endif
        case 2: // HSPI on ESP32 and FSPI on ESP32-S2
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
#ifdef CONFIG_IDF_TARGET_ESP32
            cfg.spi_host = HSPI_HOST;
#elif CONFIG_IDF_TARGET_ESP32S2
            cfg.spi_host = FSPI_HOST;
#endif
            break;
        case 3:
        default: // VSPI on ESP32 and HSPI on ESP32-S2
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
#ifdef CONFIG_IDF_TARGET_ESP32
            cfg.spi_host = VSPI_HOST;
#elif CONFIG_IDF_TARGET_ESP32S2
            cfg.spi_host = HSPI_HOST;
#endif
    }
    bus->config(cfg); // The set value is reflected on the bus.
    bus->init();
    return bus;
}

static void configure_panel(lgfx::Panel_Device* panel, Preferences* prefs)
{
    auto cfg = panel->config(); // Get the structure for display panel settings.

    cfg.pin_cs   = prefs->getInt("cs", TFT_CS);     // CS required
    cfg.pin_rst  = prefs->getInt("rst", TFT_RST);   // RST sum development board RST linkage
    cfg.pin_busy = prefs->getInt("busy", TFT_BUSY); // Pin number to which BUSY is connected (-1 = disable)

    // The following setting values are set to general initial values for each panel, so please comment out any unknown
    // items and try them.

    cfg.panel_width   = prefs->getUInt("panel_width", TFT_WIDTH);          // Actually displayable width
    cfg.panel_height  = prefs->getUInt("panel_height", TFT_HEIGHT);        // Height that can actually be displayed
    cfg.memory_width  = prefs->getUInt("memory_width", cfg.panel_width);   // Maximum width supported by driver IC
    cfg.memory_height = prefs->getUInt("memory_height", cfg.panel_height); // Maximum height supported by driver IC

    cfg.offset_x = prefs->getUInt("offset_x", 0); // Amount of offset in the X direction of the panel
    cfg.offset_y = prefs->getUInt("offset_y", 0); // Amount of offset in the Y direction of the panel
    cfg.offset_rotation =
        prefs->getUInt("offset_rotation", TFT_OFFSET_ROTATION); // Offset of the rotation 0 ~ 7 (4 ~ 7 is upside down)

    cfg.dummy_read_pixel = prefs->getUInt("dummy_read_pixel", 8); // Number of dummy read bits before pixel read
    cfg.dummy_read_bits =
        prefs->getUInt("dummy_read_bits", 1);         // bits of dummy read before reading data other than pixels
    cfg.readable = prefs->getBool("readable", false); // true if data can be read

#ifdef INVERT_COLORS // This is configurable un Web UI
    cfg.invert =
        prefs->getBool("invert", INVERT_COLORS != 0); // true if the light and darkness of the panel is reversed
#else
    cfg.invert = prefs->getBool("invert", false);       // true if the light and darkness of the panel is reversed
#endif
#ifdef TFT_RGB_ORDER
    cfg.rgb_order =
        prefs->getBool("rgb_order", TFT_RGB_ORDER != 0); // true if the red and blue of the panel are swapped
#else
    cfg.rgb_order = prefs->getBool("rgb_order", false); // true if the red and blue of the panel are swapped
#endif
    cfg.dlen_16bit = prefs->getBool("dlen_16bit", false); // true for panels that send data length in 16-bit units
    cfg.bus_shared = prefs->getBool("bus_shared", true);  // true if the bus is shared with the SD card
                                                          // (bus control is performed with drawJpgFile etc.)
    panel->config(cfg);
}

// Initialize the bus
lgfx::IBus* _init_bus(Preferences* preferences)
{
    if(!preferences) return nullptr;

    char key[8];
    int8_t data_pins[16] = {TFT_D0, TFT_D1, TFT_D2,  TFT_D3,  TFT_D4,  TFT_D5,  TFT_D6,  TFT_D7,
                            TFT_D8, TFT_D9, TFT_D10, TFT_D11, TFT_D12, TFT_D13, TFT_D14, TFT_D15};
    for(uint8_t i = 0; i < 16; i++) {
        snprintf(key, sizeof(key), "d%d", i + 1);
        data_pins[i] = preferences->getInt(key, data_pins[i]);
        LOG_DEBUG(TAG_TFT, F("D%d: %d"), i + 1, data_pins[i]);
    }

    LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    bool is_8bit  = true;
    bool is_16bit = true;
    for(uint8_t i = 0; i < 16; i++) {
        if(i < 8) is_8bit = is_8bit && (data_pins[i] >= 0);
        is_16bit = is_16bit && (data_pins[i] >= 0);
    }

    LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
#if defined(ESP32S2)
    if(is_16bit) {
        is_8bit = false;
        return init_parallel_16_bus(preferences, data_pins, 16);
    } else
#endif // ESP32S2
        if(is_8bit) {
            is_16bit = false;
            return init_parallel_8_bus(preferences, data_pins, 8);
        } else {
            return init_spi_bus(preferences);
        }
    return nullptr;
}

lgfx::Panel_Device* LovyanGfx::_init_panel(lgfx::IBus* bus)
{
    lgfx::Panel_Device* panel = nullptr;
    switch(tft_driver) {
        case 0x9341: {
            panel = new lgfx::Panel_ILI9341();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        case 0x9342: {
            panel = new lgfx::Panel_ILI9342();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        case 0x61529:
        case 0x9481: {
            panel = new lgfx::Panel_ILI9481();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        case 0x9488: {
            panel = new lgfx::Panel_ILI9488();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        case 0x7789: {
            panel = new lgfx::Panel_ST7789();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        case 0x7796: {
            panel = new lgfx::Panel_ST7796();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        case 0x8357D: {
            panel = new lgfx::Panel_HX8357D();
            LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
            break;
        }
        default: { // Needs to be in curly braces
            LOG_FATAL(TAG_TFT, F(D_SERVICE_START_FAILED ": %s line %d"), __FILE__, __LINE__);
        }
    }

    return panel;
}

lgfx::ITouch* _init_touch(Preferences* preferences)
{
    // lgfx::ITouch* touch = nullptr;
    // switch(tft_driver) {
    //     default:
    // case 0x9341: {
    LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    // break;
    //       }

#if defined(HASP_USE_LGFX_TOUCH)

#if TOUCH_DRIVER == 0x0911
    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
        auto touch = new lgfx::Touch_GT911();
        auto cfg   = touch->config();

        cfg.x_min      = 0;              // タッチスクリーンから得られる最小のX値(生の値)
        cfg.x_max      = TFT_WIDTH - 1;  // タッチスクリーンから得られる最大のX値(生の値)
        cfg.y_min      = 0;              // タッチスクリーンから得られる最小のY値(生の値)
        cfg.y_max      = TFT_HEIGHT - 1; // タッチスクリーンから得られる最大のY値(生の値)
        cfg.bus_shared = false;          // 画面と共通のバスを使用している場合 trueを設定
        cfg.offset_rotation = TOUCH_OFFSET_ROTATION; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

        // SPI接続の場合
        // cfg.spi_host = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
        // cfg.freq     = 1000000;   // SPIクロックを設定
        // cfg.pin_sclk = 18;        // SCLKが接続されているピン番号
        // cfg.pin_mosi = 23;        // MOSIが接続されているピン番号
        // cfg.pin_miso = 19;        // MISOが接続されているピン番号
        // cfg.pin_cs   = 5;         //   CSが接続されているピン番号

        // I2C接続の場合
        cfg.i2c_port = I2C_TOUCH_PORT;      // 使用するI2Cを選択 (0 or 1)
        cfg.i2c_addr = I2C_TOUCH_ADDRESS;   // I2Cデバイスアドレス番号
        cfg.pin_sda  = TOUCH_SDA;           // SDAが接続されているピン番号
        cfg.pin_scl  = TOUCH_SCL;           // SCLが接続されているピン番号
        cfg.pin_int  = -1;                  // INTが接続されているピン番号
        cfg.freq     = I2C_TOUCH_FREQUENCY; // I2Cクロックを設定

        touch->config(cfg);
        return touch;
    }
#endif

#if TOUCH_DRIVER == 0x6336
    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
        auto touch = new lgfx::Touch_FT5x06();
        auto cfg   = touch->config();

        cfg.x_min      = 0;              // タッチスクリーンから得られる最小のX値(生の値)
        cfg.x_max      = TFT_WIDTH - 1;  // タッチスクリーンから得られる最大のX値(生の値)
        cfg.y_min      = 0;              // タッチスクリーンから得られる最小のY値(生の値)
        cfg.y_max      = TFT_HEIGHT - 1; // タッチスクリーンから得られる最大のY値(生の値)
        cfg.pin_int    = TOUCH_IRQ;      // INTが接続されているピン番号
        cfg.bus_shared = true;           // 画面と共通のバスを使用している場合 trueを設定
        cfg.offset_rotation = TOUCH_OFFSET_ROTATION; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

        // SPI接続の場合
        // cfg.spi_host = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
        // cfg.freq     = 1000000;   // SPIクロックを設定
        // cfg.pin_sclk = 18;        // SCLKが接続されているピン番号
        // cfg.pin_mosi = 23;        // MOSIが接続されているピン番号
        // cfg.pin_miso = 19;        // MISOが接続されているピン番号
        // cfg.pin_cs   = 5;         //   CSが接続されているピン番号

        // I2C接続の場合
        cfg.i2c_port = I2C_TOUCH_PORT;      // 使用するI2Cを選択 (0 or 1)
        cfg.i2c_addr = I2C_TOUCH_ADDRESS;   // I2Cデバイスアドレス番号
        cfg.pin_sda  = TOUCH_SDA;           // SDAが接続されているピン番号
        cfg.pin_scl  = TOUCH_SCL;           // SCLが接続されているピン番号
        cfg.freq     = I2C_TOUCH_FREQUENCY; // I2Cクロックを設定

        touch->config(cfg);
        return touch;
    }
#endif

#if TOUCH_DRIVER == 0x2046

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
        auto touch = new lgfx::Touch_XPT2046();
        auto cfg   = touch->config();
        LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);

        cfg.bus_shared      = true; // 画面と共通のバスを使用している場合 trueを設定
        cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

        // SPI接続の場合
        cfg.spi_host = VSPI_HOST;   // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
        cfg.pin_sclk = 18;          // SCLKが接続されているピン番号
        cfg.pin_mosi = 23;          // MOSIが接続されているピン番号
        cfg.pin_miso = 19;          // MISOが接続されているピン番号
        cfg.pin_cs   = 21;          //   CSが接続されているピン番号
        cfg.pin_int  = GPIO_NUM_NC; // INTが接続されているピン番号

        touch->config(cfg);
        return touch;
    }
#endif

#endif // HASP_USE_LGFX_TOUCH

    return nullptr;
}

void LovyanGfx::init(int w, int h)
{
    LOG_TRACE(TAG_TFT, F(D_SERVICE_STARTING));

#ifdef WTSC01PLUS
    auto _panel_instance = new lgfx::Panel_ST7796();
    auto _bus_instance   = new lgfx::v1::Bus_Parallel8();
    auto _touch_instance = new lgfx::Touch_FT5x06(); // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436
    {                                                // バス制御の設定を行います。
        auto cfg = _bus_instance->config();          // バス設定用の構造体を取得します。
                                                     // 8ビットパラレルバスの設定
        // cfg.i2s_port = I2S_NUM_0;     // 使用するI2Sポートを選択 (I2S_NUM_0 or I2S_NUM_1) (ESP32のI2S
        // LCDモードを使用します)
        cfg.freq_write = 20000000; // 送信クロック (最大20MHz, 80MHzを整数で割った値に丸められます)
        cfg.pin_wr     = 47;       // WR を接続しているピン番号
        cfg.pin_rd     = -1;       // RD を接続しているピン番号
        cfg.pin_rs     = 0;        // RS(D/C)を接続しているピン番号
        cfg.pin_d0     = 9;        // D0を接続しているピン番号
        cfg.pin_d1     = 46;       // D1を接続しているピン番号
        cfg.pin_d2     = 3;        // D2を接続しているピン番号
        cfg.pin_d3     = 8;        // D3を接続しているピン番号
        cfg.pin_d4     = 18;       // D4を接続しているピン番号
        cfg.pin_d5     = 17;       // D5を接続しているピン番号
        cfg.pin_d6     = 16;       // D6を接続しているピン番号
        cfg.pin_d7     = 15;       // D7を接続しているピン番号
        _bus_instance->config(cfg);             // 設定値をバスに反映します。
        _panel_instance->setBus(_bus_instance); // バスをパネルにセットします。
    }

    {                                         // 表示パネル制御の設定を行います。
        auto cfg = _panel_instance->config(); // 表示パネル設定用の構造体を取得します。

        cfg.pin_cs   = -1; // CSが接続されているピン番号   (-1 = disable)
        cfg.pin_rst  = 4;  // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

        // ※
        // 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

        cfg.panel_width      = 320;   // 実際に表示可能な幅
        cfg.panel_height     = 480;   // 実際に表示可能な高さ
        cfg.offset_x         = 0;     // パネルのX方向オフセット量
        cfg.offset_y         = 0;     // パネルのY方向オフセット量
        cfg.offset_rotation  = 0;     // 回転方向の値のオフセット 0~7 (4~7は上下反転)
        cfg.dummy_read_pixel = 8;     // ピクセル読出し前のダミーリードのビット数
        cfg.dummy_read_bits  = 1;     // ピクセル以外のデータ読出し前のダミーリードのビット数
        cfg.readable         = true;  // データ読出しが可能な場合 trueに設定
        cfg.invert           = true;  // パネルの明暗が反転してしまう場合 trueに設定
        cfg.rgb_order        = false; // パネルの赤と青が入れ替わってしまう場合 trueに設定
        cfg.dlen_16bit = false; // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
        cfg.bus_shared = true; // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

        // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
        //    cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
        //    cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ

        _panel_instance->config(cfg);
    }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
        auto cfg = _touch_instance->config();

        cfg.x_min           = 0;    // タッチスクリーンから得られる最小のX値(生の値)
        cfg.x_max           = 319;  // タッチスクリーンから得られる最大のX値(生の値)
        cfg.y_min           = 0;    // タッチスクリーンから得られる最小のY値(生の値)
        cfg.y_max           = 479;  // タッチスクリーンから得られる最大のY値(生の値)
        cfg.pin_int         = 7;    // INTが接続されているピン番号
        cfg.bus_shared      = true; // 画面と共通のバスを使用している場合 trueを設定
        cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
                                 // I2C接続の場合
        cfg.i2c_port = 1;        // 使用するI2Cを選択 (0 or 1)
        cfg.i2c_addr = 0x38;     // I2Cデバイスアドレス番号
        cfg.pin_sda  = 6;        // SDAが接続されているピン番号
        cfg.pin_scl  = 5;        // SCLが接続されているピン番号
        cfg.freq     = 400000;   // I2Cクロックを設定

        _touch_instance->config(cfg);
        _panel_instance->setTouch(_touch_instance); // タッチスクリーンをパネルにセットします。
    }
#else

    Preferences preferences;
    preferences.begin("tft", false);
    this->tft_driver = preferences.getUInt("DRIVER", get_tft_driver());

    lgfx::IBus* _bus_instance = _init_bus(&preferences);
    lgfx::Panel_Device* _panel_instance = _init_panel(_bus_instance);
    lgfx::ITouch* _touch_instance = _init_touch(&preferences);

    if(_panel_instance != nullptr) {
        _panel_instance->setBus(_bus_instance);
        configure_panel(_panel_instance, &preferences);
    }
#endif

    tft.setPanel(_panel_instance);

    if(_touch_instance) {
        LOG_INFO(TAG_TFT, F("Touch " D_SERVICE_STARTED));
    } else {
        LOG_WARNING(TAG_TFT, F("Touch " D_SERVICE_START_FAILED));
    }

    /* TFT init */
    LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    tft.begin();
    LOG_DEBUG(TAG_TFT, F("%s - %d"), __FILE__, __LINE__);
    tft.setSwapBytes(true); /* set endianess */
    LOG_INFO(TAG_TFT, F(D_SERVICE_STARTED));
}

void LovyanGfx::show_info()
{
    splashscreen();

    LOG_VERBOSE(TAG_TFT, F("LovyanGFX   : v%d.%d.%d"), LGFX_VERSION_MAJOR, LGFX_VERSION_MINOR, LGFX_VERSION_PATCH);
    auto panel = tft.getPanel();
    if(!panel) return;

    if(!panel->getBus()) return;
    lgfx::v1::bus_type_t bus_type = panel->getBus()->busType();

    if(bus_type == lgfx::v1::bus_parallel8) {
        LOG_VERBOSE(TAG_TFT, F("Interface  : Parallel 8bit"));
        auto bus = (lgfx::Bus_Parallel8*)panel->getBus();
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

#if defined(ESP32S2) || defined(ESP32S3)
    if(bus_type == lgfx::v1::bus_parallel16) {
        LOG_VERBOSE(TAG_TFT, F("Interface  : Parallel 16bit"));
        auto bus = (lgfx::v1::Bus_Parallel16*)panel->getBus();
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
        tftPinInfo(F("TFT_D1"), cfg.pin_d8);
        tftPinInfo(F("TFT_D1"), cfg.pin_d9);
        tftPinInfo(F("TFT_D1"), cfg.pin_d10);
        tftPinInfo(F("TFT_D1"), cfg.pin_d11);
        tftPinInfo(F("TFT_D1"), cfg.pin_d12);
        tftPinInfo(F("TFT_D1"), cfg.pin_d13);
        tftPinInfo(F("TFT_D1"), cfg.pin_d14);
        tftPinInfo(F("TFT_D1"), cfg.pin_d15);
    }
#endif

    if(bus_type == lgfx::v1::bus_spi) {
        LOG_VERBOSE(TAG_TFT, F("Interface   : Serial"));
        auto bus = (lgfx::Bus_SPI*)panel->getBus();
        auto cfg = bus->config(); // Get the structure for bus configuration.
        tftPinInfo(F("TFT_MOSI"), cfg.pin_mosi);
        tftPinInfo(F("TFT_MISO"), cfg.pin_miso);
        tftPinInfo(F("TFT_SCLK"), cfg.pin_sclk);
        tftPinInfo(F("TFT_DC"), cfg.pin_dc);
    }

    {
        auto cfg = panel->config(); // Get the structure for panel configuration.
        tftPinInfo(F("TFT_CS"), cfg.pin_cs);
        tftPinInfo(F("TFT_RST"), cfg.pin_rst);
        tftPinInfo(F("TFT_BUSY"), cfg.pin_busy);
    }

    if(bus_type == lgfx::v1::bus_spi) {
        auto bus      = (lgfx::Bus_SPI*)panel->getBus();
        auto cfg      = bus->config(); // Get the structure for bus configuration.
        uint32_t freq = cfg.freq_write / 100000;
        LOG_VERBOSE(TAG_TFT, F("TFT SPI freq: %d.%d MHz"), freq / 10, freq % 10);
    }

    lgfx::v1::ITouch* touch = panel->getTouch();
    if(touch) {
        auto cfg      = touch->config(); // Get the structure for bus configuration.
        uint32_t freq = cfg.freq / 100000;
        tftPinInfo(F("TOUCH_INT"), cfg.pin_int);
        tftPinInfo(F("TOUCH_RST"), cfg.pin_rst);
        if(touch->isSPI()) {
            tftPinInfo(F("TOUCH_MISO"), cfg.pin_miso);
            tftPinInfo(F("TOUCH_MOSI"), cfg.pin_mosi);
            tftPinInfo(F("TOUCH_SCLK"), cfg.pin_sclk);
            tftPinInfo(F("TOUCH_CS"), cfg.pin_cs);
            LOG_VERBOSE(TAG_TFT, F("Touch SPI freq.   : %d.%d MHz"), freq / 10, freq % 10);
        } else {
            tftPinInfo(F("TOUCH_SDA"), cfg.pin_sda);
            tftPinInfo(F("TOUCH_SCL"), cfg.pin_scl);
            tftPinInfo(F("TOUCH_ADDR"), cfg.i2c_addr);
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
    // tft.fillSmoothRoundRect(x, y, logoWidth, logoWidth, 15, fgColor.full);
}

void LovyanGfx::set_rotation(uint8_t rotation)
{
    LOG_VERBOSE(TAG_TFT, F("Rotation    : %d"), rotation);
    tft.setRotation(rotation);
}

void LovyanGfx::set_invert(bool invert)
{
    char buffer[4];
    memcpy_P(buffer, invert ? PSTR(D_YES) : PSTR(D_NO), sizeof(buffer));

    LOG_VERBOSE(TAG_TFT, F("Invert Color: %s"), buffer);
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
    auto panel = tft.getPanel();
    if(panel && panel->getBus()) {
        lgfx::v1::bus_type_t bus_type = panel->getBus()->busType();

        if(bus_type == lgfx::v1::bus_parallel8) {
            auto bus = (lgfx::Bus_Parallel8*)panel->getBus();
            auto cfg = bus->config(); // Get the structure for bus configuration.
            if(pin == cfg.pin_wr || pin == cfg.pin_rd || pin == cfg.pin_rs || pin == cfg.pin_d0 || pin == cfg.pin_d1 ||
               pin == cfg.pin_d2 || pin == cfg.pin_d3 || pin == cfg.pin_d4 || pin == cfg.pin_d5 || pin == cfg.pin_d6 ||
               pin == cfg.pin_d7)
                return true;

#if defined(ESP32S2) || defined(ESP32S3)
        } else if(bus_type == lgfx::v1::bus_parallel16) {
            auto bus = (lgfx::v1::Bus_Parallel16*)panel->getBus();
            auto cfg = bus->config(); // Get the structure for bus configuration.
            if(cfg.pin_wr || pin == cfg.pin_rd || pin == cfg.pin_rs || pin == cfg.pin_d0 || pin == cfg.pin_d1 ||
               pin == cfg.pin_d2 || pin == cfg.pin_d3 || pin == cfg.pin_d4 || pin == cfg.pin_d5 || pin == cfg.pin_d6 ||
               pin == cfg.pin_d7 || pin == cfg.pin_d8 || pin == cfg.pin_d9 || pin == cfg.pin_d10 ||
               pin == cfg.pin_d11 || pin == cfg.pin_d12 || pin == cfg.pin_d13 || pin == cfg.pin_d14 ||
               pin == cfg.pin_d15)
                return true;
#endif

        } else if(bus_type == lgfx::v1::bus_spi) {
            auto bus = (lgfx::Bus_SPI*)panel->getBus();
            auto cfg = bus->config(); // Get the structure for bus configuration.
            if(pin == cfg.pin_mosi || pin == cfg.pin_miso || pin == cfg.pin_sclk || pin == cfg.pin_dc) return true;
        }

        {
            auto cfg = panel->config(); // Get the structure for panel configuration.
            if(pin == cfg.pin_cs || pin == cfg.pin_rst || pin == cfg.pin_busy) return true;
        }

        lgfx::v1::ITouch* touch = panel->getTouch();
        if(touch) {
            auto cfg = touch->config(); // Get the structure for bus configuration.
            if(pin == cfg.pin_int || pin == cfg.pin_rst) return true;
            if(touch->isSPI()) {
                if(pin == cfg.pin_miso || pin == cfg.pin_mosi || pin == cfg.pin_sclk || pin == cfg.pin_cs) return true;
            } else {
                if(pin == cfg.pin_sda || pin == cfg.pin_scl) return true;
            }
        }
    }

    return false;
}

const char* LovyanGfx::get_tft_model()
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
#elif defined(ST7789_2_DRIVER)
    return 0x77892;
#elif defined(R61581_DRIVER)
    return 0x61581;
#elif defined(R61529_DRIVER)
    return 0x61529;
#elif defined(RM68140_DRIVER)
    return 0x68140;
#else
    return 0x0;
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