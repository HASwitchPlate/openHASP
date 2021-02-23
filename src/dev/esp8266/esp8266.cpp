/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ESP8266)

#include "Arduino.h"
#include <Esp.h>

#include "esp8266.h"

#include "hasp_conf.h"
#include "hasp_debug.h"
#include "hasp/hasp_utilities.h"

#define BACKLIGHT_CHANNEL 0

namespace dev {

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
    return ESP.getCoreVersion().c_str();
}

const char* Esp8266Device::get_display_driver()
{
    return Utilities::tft_driver_name().c_str();
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

void Esp8266Device::set_backlight_level(uint8_t level)
{
    _backlight_level = level >= 0 ? level : 0;
    _backlight_level = _backlight_level <= 100 ? _backlight_level : 100;

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
    if(_backlight_pin == -1) return;

    analogWrite(_backlight_pin, _backlight_power ? map(_backlight_level, 0, 100, 0, 1023) : 0);
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

} // namespace dev

dev::Esp8266Device haspDevice;

#endif // ESP8266