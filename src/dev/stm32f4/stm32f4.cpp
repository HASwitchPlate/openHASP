/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(STM32F4xx)

#include <Arduino.h>

#include "stm32f4.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#define BACKLIGHT_CHANNEL 0

namespace dev {

void Stm32f4Device::reboot()
{
    //  ESP.restart();
}

void Stm32f4Device::show_info()
{
    LOG_VERBOSE(TAG_DEV, F("Processor  : STM32F4xx"));
    // LOG_VERBOSE(TAG_DEV, F("CPU freq.  : %i MHz"), get_cpu_frequency());
    // LOG_VERBOSE(TAG_DEV, F("Voltage    : %2.2f V"), ESP.getVcc() / 918.0); // 918 empirically determined
}

const char* Stm32f4Device::get_hostname()
{
    return _hostname.c_str();
}

void Stm32f4Device::set_hostname(const char* hostname)
{
    _hostname = hostname;
}

const char* Stm32f4Device::get_core_version()
{
    // return ESP.getCoreVersion().c_str();
}

const char* Stm32f4Device::get_hardware_id()
{
    // https://stm32duinoforum.com/forum/viewtopic_f_29_t_2909_start_10.html
    //   Serial.println("UID [HEX]  : "+String(*(uint32_t*)(UID_BASE), HEX)+" "+String(*(uint32_t*)(UID_BASE+0x04),
    //   HEX)+" "+String(*(uint32_t*)(UID_BASE+0x08), HEX));
    return _hardware_id.c_str();
}

void Stm32f4Device::set_backlight_pin(uint8_t pin)
{
    _backlight_pin = pin;
    /* Setup Backlight Control Pin */
    if(pin >= 0) {
        LOG_VERBOSE(TAG_GUI, F("Backlight  : Pin %d"), pin);
        pinMode(_backlight_pin, OUTPUT);
        update_backlight();
    }
}

const char* Stm32f4Device::get_chip_model()
{
#if defined(STM32F407ZG)
    return "STM32F407ZG";
#elif defined(STM32F407ZE)
    return "STM32F407ZE";
#elif defined(STM32F407VE)
    return "STM32F407VE";
#elif defined(STM32F407VG)
    return "STM32F407VG";
#elif defined(STM32F4xx) || defined(ARDUINO_ARCH_STM32F4)
    return "STM32F4";
#else
    return "Unknown STM32";
#endif
}

void Stm32f4Device::set_backlight_invert(bool invert)
{
    _backlight_invert = invert;
    update_backlight();
}

bool Stm32f4Device::get_backlight_invert()
{
    return _backlight_invert;
}

void Stm32f4Device::set_backlight_level(uint8_t level)
{
    _backlight_level = level;
    update_backlight();
}

uint8_t Stm32f4Device::get_backlight_level()
{
    return _backlight_level;
}

void Stm32f4Device::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool Stm32f4Device::get_backlight_power()
{
    return _backlight_power != 0;
}

void Stm32f4Device::update_backlight()
{
    if(_backlight_pin == -1) return;

    // analogWrite(_backlight_pin, _backlight_power ? map(_backlight_level, 0, 255, 0, 1023) : 0);
}

size_t Stm32f4Device::get_free_max_block()
{
    // return ESP.getMaxFreeBlockSize();
}

size_t Stm32f4Device::get_free_heap(void)
{
    // return ESP.getFreeHeap();
}

uint8_t Stm32f4Device::get_heap_fragmentation()
{
    // return ESP.getHeapFragmentation();
}

uint16_t Stm32f4Device::get_cpu_frequency()
{
    // return ESP.getCpuFreqMHz();
}

bool Stm32f4Device::is_system_pin(uint8_t pin)
{
    // if((pin >= 6) && (pin <= 11)) return true;  // integrated SPI flash
    // if((pin >= 12) && (pin <= 14)) return true; // HSPI
    return false;
}

} // namespace dev

dev::Stm32f4Device haspDevice;

#endif // STM32F4xx