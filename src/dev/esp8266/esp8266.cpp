/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ESP8266)

#include <Arduino.h>
#include <Esp.h>
#include <ESP8266WiFi.h>

#include "esp8266.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#define BACKLIGHT_CHANNEL 0

namespace dev {

Esp8266Device::Esp8266Device()
{
    _hostname         = MQTT_NODENAME;
    _backlight_invert = (TFT_BACKLIGHT_ON == LOW);
    _backlight_power  = 1;
    _backlight_level  = 255;
    _core_version     = ESP.getCoreVersion().c_str();
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

void Esp8266Device::reboot()
{
    ESP.restart();
}

void Esp8266Device::show_info()
{
    LOG_VERBOSE(TAG_DEV, F("Processor  : ESP8266"));
    LOG_VERBOSE(TAG_DEV, F("CPU freq.  : %i MHz"), get_cpu_frequency());
    LOG_VERBOSE(TAG_DEV, F("Voltage    : %2.2f V"), ESP.getVcc() / 918.0); // 918 empirically determined
}

const char* Esp8266Device::get_hostname()
{
    return _hostname.c_str();
}
void Esp8266Device::set_hostname(const char* hostname)
{
    _hostname = hostname;
}

const char* Esp8266Device::get_core_version()
{
    return _core_version.c_str();
}

const char* Esp8266Device::get_chip_model()
{
    return "ESP8266";
}

const char* Esp8266Device::get_hardware_id()
{
    return _hardware_id.c_str();
}

void Esp8266Device::set_backlight_pin(uint8_t pin)
{
    _backlight_pin = pin;
    /* Setup Backlight Control Pin */
    if(pin >= 0) {
        LOG_VERBOSE(TAG_GUI, F("Backlight  : Pin %d"), pin);
        pinMode(_backlight_pin, OUTPUT);
        update_backlight();
    }
}

void Esp8266Device::set_backlight_invert(bool invert)
{
    _backlight_invert = invert;
    update_backlight();
}

bool Esp8266Device::get_backlight_invert()
{
    return _backlight_invert;
}

void Esp8266Device::set_backlight_level(uint8_t level)
{
    _backlight_level = level;
    update_backlight();
}

uint8_t Esp8266Device::get_backlight_level()
{
    return _backlight_level;
}

void Esp8266Device::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool Esp8266Device::get_backlight_power()
{
    return _backlight_power != 0;
}

void Esp8266Device::update_backlight()
{
    if(_backlight_pin < 17) {
        uint32_t duty = _backlight_power ? map(_backlight_level, 0, 255, 0, 1023) : 0;
        if(_backlight_invert) duty = 1023 - duty;
        analogWrite(_backlight_pin, duty);
    }
}

size_t Esp8266Device::get_free_max_block()
{
    return ESP.getMaxFreeBlockSize();
}

size_t Esp8266Device::get_free_heap(void)
{
    return ESP.getFreeHeap();
}

uint8_t Esp8266Device::get_heap_fragmentation()
{
    return ESP.getHeapFragmentation();
}

uint16_t Esp8266Device::get_cpu_frequency()
{
    return ESP.getCpuFreqMHz();
}

bool Esp8266Device::is_system_pin(uint8_t pin)
{
    if((pin >= 6) && (pin <= 11)) return true;  // integrated SPI flash
    if((pin >= 12) && (pin <= 14)) return true; // HSPI
    return false;
}

long Esp8266Device::get_uptime()
{
    return millis() / 1000U;
}

} // namespace dev

dev::Esp8266Device haspDevice;

#endif // ESP8266