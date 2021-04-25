/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ESP32)

#include "Arduino.h"
#include <Esp.h>
#include <WiFi.h>
#include "esp_system.h"

#include "hasp_conf.h"

#include "../device.h"
#include "esp32.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "hasp_debug.h"

#define BACKLIGHT_CHANNEL 0

namespace dev {

Esp32Device::Esp32Device()
{
    _hostname         = MQTT_NODENAME;
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
    ESP.restart();
}

void Esp32Device::show_info()
{
    LOG_VERBOSE(TAG_DEV, F("Processor  : ESP32"));
    LOG_VERBOSE(TAG_DEV, F("CPU freq.  : %i MHz"), get_cpu_frequency());
    // LOG_VERBOSE(TAG_DEV, F("Voltage    : %2.2f V"), ESP.getVcc() / 918.0); // 918 empirically determined
}

const char* Esp32Device::get_hostname()
{
    return _hostname.c_str();
}

void Esp32Device::set_hostname(const char* hostname)
{
    _hostname = hostname;
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

#ifdef CHIP_ESP32S2
        case CHIP_ESP32S2:
            return "ESP32-S2";
#endif

#ifdef CHIP_ESP32S3
        case CHIP_ESP32S3:
            return "ESP32-S3";
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
}

size_t Esp32Device::get_free_max_block()
{
    return ESP.getMaxAllocHeap();
}

size_t Esp32Device::get_free_heap()
{
    return ESP.getFreeHeap();
}

uint8_t Esp32Device::get_heap_fragmentation()
{
    uint32_t free = ESP.getFreeHeap();
    if(free) {
        return (int8_t)(100.00f - (float)ESP.getMaxAllocHeap() * 100.00f / (float)free);
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