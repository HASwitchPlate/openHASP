/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ESP32)

#include <Arduino.h>
#include <Esp.h>
#include <WiFi.h>
#include "esp_system.h"
#include <rom/rtc.h> // needed to get the ResetInfo
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "hasp_conf.h"
#include "../device.h"
#include "esp32.h"
#include "hasp_debug.h"

#include "../../drv/tft/tft_driver.h" // for haspTft

#define BACKLIGHT_CHANNEL 0

#ifndef ESP32S2
uint8_t temprature_sens_read();
#endif

namespace dev {

static String esp32ResetReason(uint8_t cpuid)
{
    if(cpuid > 1) {
        return F("Invalid CPU id");
    }
    RESET_REASON reason = rtc_get_reset_reason(cpuid);

    String resetReason((char*)0);
    resetReason.reserve(128);

    resetReason += F("CPU");
    resetReason += cpuid;
    resetReason += F(": ");

    switch(reason) {
        case 1:
            resetReason += F("POWERON");
            break; /**<1, Vbat power on reset*/
        case 3:
            resetReason += F("SW");
            break; /**<3, Software reset digital core*/
        case 4:
            resetReason += F("OWDT");
            break; /**<4, Legacy watch dog reset digital core*/
        case 5:
            resetReason += F("DEEPSLEEP");
            break; /**<5, Deep Sleep reset digital core*/
        case 6:
            resetReason += F("SDIO");
            break; /**<6, Reset by SLC module, reset digital core*/
        case 7:
            resetReason += F("TG0WDT_SYS");
            break; /**<7, Timer Group0 Watch dog reset digital core*/
        case 8:
            resetReason += F("TG1WDT_SYS");
            break; /**<8, Timer Group1 Watch dog reset digital core*/
        case 9:
            resetReason += F("RTCWDT_SYS");
            break; /**<9, RTC Watch dog Reset digital core*/
        case 10:
            resetReason += F("INTRUSION");
            break; /**<10, Instrusion tested to reset CPU*/
        case 11:
            resetReason += F("TGWDT_CPU");
            break; /**<11, Time Group reset CPU*/
        case 12:
            resetReason += F("SW_CPU");
            break; /**<12, Software reset CPU*/
        case 13:
            resetReason += F("RTCWDT_CPU");
            break; /**<13, RTC Watch dog Reset CPU*/
        case 14:
            resetReason += F("EXT_CPU");
            break; /**<14, for APP CPU, reseted by PRO CPU*/
        case 15:
            resetReason += F("RTCWDT_BROWN_OUT");
            break; /**<15, Reset when the vdd voltage is not stable*/
        case 16:
            resetReason += F("RTCWDT_RTC");
            break; /**<16, RTC Watch dog reset digital core and rtc module*/
        default:
            resetReason += F("NO_MEAN");
            return resetReason;
    }
    resetReason += F("_RESET");
    return resetReason;
}

static void halGetResetInfo(String& resetReason)
{
    resetReason = String(esp32ResetReason(0));
    resetReason += F(" / ");
    resetReason += String(esp32ResetReason(1));
}

Esp32Device::Esp32Device()
{
    BaseDevice::set_hostname(MQTT_NODENAME);
    _backlight_invert = (TFT_BACKLIGHT_ON == LOW);
    _backlight_power  = 1;
    _backlight_level  = 255;
    _backlight_pin    = TFT_BCKL;

    /* fill unique identifier with wifi mac */
    byte mac[6];
    WiFi.macAddress(mac);
    _hardware_id.reserve(13);
    for(int i = 0; i < 6; ++i) {
        if(mac[i] < 0x10) _hardware_id += "0";
        _hardware_id += String(mac[i], HEX).c_str();
    }
}

void Esp32Device::reboot()
{
    esp_sleep_enable_timer_wakeup(50 * 1000);
    esp_deep_sleep_start();
    //    ESP.restart();
}

void Esp32Device::show_info()
{
    LOG_VERBOSE(TAG_DEV, F("Processor  : ESP32"));
    LOG_VERBOSE(TAG_DEV, F("CPU freq.  : %i MHz"), get_cpu_frequency());
    // LOG_VERBOSE(TAG_DEV, F("Voltage    : %2.2f V"), ESP.getVcc() / 918.0); // 918 empirically determined

    if(_sketch_size == 0) _sketch_size = ESP.getSketchSize(); // slow: takes ~1 second
}

const char* Esp32Device::get_core_version()
{
    return esp_get_idf_version(); // == ESP.getSdkVersion();
}
const char* Esp32Device::get_chip_model()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    //  model = chip_info.cores;
    //  model += F("core ");
    switch(chip_info.model) {
        case CHIP_ESP32:
            return "ESP32";

#ifdef ESP32
        case CHIP_ESP32S2:
            return "ESP32-S2";

        case CHIP_ESP32S3:
            return "ESP32-S3";

        case CHIP_ESP32C3:
            return "ESP32-C3";

        case CHIP_ESP32H2:
            return "ESP32-H2";
#endif

        default:
            return "Unknown ESP32";
    }
    // model += F(" rev");
    // model += chip_info.revision;
}

const char* Esp32Device::get_hardware_id()
{
    return _hardware_id.c_str();
}

void Esp32Device::set_backlight_pin(uint8_t pin)
{
    _backlight_pin = pin;

    /* Setup Backlight Control Pin */
    if(pin < GPIO_NUM_MAX) {
        LOG_VERBOSE(TAG_GUI, F("Backlight  : Pin %d"), pin);
        ledcSetup(BACKLIGHT_CHANNEL, 20000, 12);
        ledcAttachPin(pin, BACKLIGHT_CHANNEL);
        update_backlight();
    } else {
        LOG_VERBOSE(TAG_GUI, F("Backlight  : Pin not set"));
    }
}

void Esp32Device::set_backlight_level(uint8_t level)
{
    _backlight_level = level;
    update_backlight();
}

uint8_t Esp32Device::get_backlight_level()
{
    return _backlight_level;
}

void Esp32Device::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool Esp32Device::get_backlight_power()
{
    return _backlight_power != 0;
}

void Esp32Device::update_backlight()
{
    if(_backlight_pin < GPIO_NUM_MAX) {
        uint32_t duty = _backlight_power ? map(_backlight_level, 0, 255, 0, 4095) : 0;
        if(_backlight_invert) duty = 4095 - duty;
        ledcWrite(BACKLIGHT_CHANNEL, duty); // ledChannel and value
    }

    // haspTft.tft.writecommand(0x53); // Write CTRL Display
    // if(_backlight_power)
    //     haspTft.tft.writedata(0x24); // BL on, show image
    // else
    //     haspTft.tft.writedata(0x20); // BL off, white screen

    // // if(_backlight_power)
    // //     haspTft.tft.writecommand(0x29); // BL on, show image
    // // else
    // //     haspTft.tft.writecommand(0x28); // BL off, white screen

    // // haspTft.tft.writecommand(0x55); // Write Content Adaptive Brightness Control and Color Enhancement
    // // haspTft.tft.writedata(0x0);     // Off

    // haspTft.tft.writecommand(0x5E);          // minimum brightness
    // haspTft.tft.writedata(_backlight_level); // 0-255

    // haspTft.tft.writecommand(0x51);          //  Write Display Brightness
    // haspTft.tft.writedata(_backlight_level); // 0-255
}

size_t Esp32Device::get_free_max_block()
{
    // return ESP.getMaxAllocHeap();
    return heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

size_t Esp32Device::get_free_heap()
{
    // return ESP.getFreeHeap();
    return heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

uint8_t Esp32Device::get_heap_fragmentation()
{
    uint32_t free = get_free_heap();
    if(free) {
        return (int8_t)(100.00f - (float)get_free_max_block() * 100.00f / (float)free);
    } else {
        return 100; // no free memory
    }
}

uint16_t Esp32Device::get_cpu_frequency()
{
    return ESP.getCpuFreqMHz();
}

bool Esp32Device::is_system_pin(uint8_t pin)
{
    if((pin >= 6) && (pin <= 11)) return true;  // integrated SPI flash
    if((pin == 37) || (pin == 38)) return true; // unavailable
    if(psramFound()) {
        if((pin == 16) || (pin == 17)) return true; // PSRAM
    }
    return false;
}

void Esp32Device::get_info(JsonDocument& doc)
{
    char size_buf[64];
    String buffer((char*)0);
    buffer.reserve(64);

    JsonObject info = doc.createNestedObject(F(D_INFO_MODULE));

    /* ESP Stats */
    buffer = String(get_cpu_frequency());
    buffer += F("MHz");
    info[F(D_INFO_MODEL)]     = get_chip_model(); // 10ms
    info[F(D_INFO_FREQUENCY)] = buffer;

    info[F(D_INFO_CORE_VERSION)] = get_core_version();
    halGetResetInfo(buffer);
    info[F(D_INFO_RESET_REASON)] = buffer;

    Parser::format_bytes(ESP.getFlashChipSize(), size_buf, sizeof(size_buf)); // 25ms
    info[F(D_INFO_FLASH_SIZE)] = size_buf;

    if(_sketch_size == 0) _sketch_size = ESP.getSketchSize(); // slow: takes ~1 second
    Parser::format_bytes(_sketch_size, size_buf, sizeof(size_buf));
    info[F(D_INFO_SKETCH_USED)] = size_buf;

    Parser::format_bytes(ESP.getFreeSketchSpace(), size_buf, sizeof(size_buf));
    info[F(D_INFO_SKETCH_FREE)] = size_buf;
}

void Esp32Device::get_sensors(JsonDocument& doc)
{
#ifndef ESP32S2
    JsonObject sensor        = doc.createNestedObject(F("ESP32"));
    uint32_t temp            = (temprature_sens_read() - 32) * 100 / 1.8;
    sensor[F("Temperature")] = serialized(String(1.0f * temp / 100, 2));
#endif
}

long Esp32Device::get_uptime()
{
    return esp_timer_get_time() / 1000000U;
}

} // namespace dev

#if defined(LANBONL8)
// #warning Building for Lanbon L8
#include "dev/esp32/lanbonl8.h"
#elif defined(M5STACK)
// #warning Building for M5Stack core2
#include "dev/esp32/m5stackcore2.h"
#else
dev::Esp32Device haspDevice;
#endif

#endif // ESP32