/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_H
#define HASP_DEVICE_H

#ifdef ARDUINO
#include "Arduino.h"
#endif

#ifdef WINDOWS
#include <cstdint>
#include "Windows.h"
#endif

namespace dev {

class BaseDevice {
  public:
    bool has_battery           = false;
    bool has_backligth_control = true;

    virtual void reboot()
    {}
    virtual const char* get_hostname()
    {
        return "";
    }
    virtual void set_hostname(const char*)
    {}
    virtual const char* get_core_version()
    {
        return "";
    }
    virtual const char* get_display_driver()
    {
        return "";
    }

    virtual void init()
    {}
    virtual void post_setup()
    {}
    virtual void loop()
    {}
    virtual void loop_5s()
    {}
    virtual void set_backlight_pin(uint8_t pin)
    {}
    virtual void set_backlight_level(uint8_t level)
    {}
    virtual uint8_t get_backlight_level()
    {
        return -1;
    }
    virtual void set_backlight_power(bool power)
    {}
    virtual bool get_backlight_power()
    {
        return true;
    }
    virtual size_t get_free_max_block()
    {}
    virtual size_t get_free_heap()
    {}
    virtual uint8_t get_heap_fragmentation()
    {}
    virtual uint16_t get_cpu_frequency()
    {}
};

} // namespace dev

#if defined(ESP32)
#warning Building for ESP32 Devices
#include "esp32/esp32.h"
#elif defined(ESP8266)
#warning Building for ESP8266 Devices
#include "esp8266/esp8266.h"
#elif defined(STM32F4)
#warning Building for STM32F4xx Devices
#include "stm32f4/stm32f4.h"
#elif defined(WINDOWS)
#warning Building for Win32 Devices
#include "win32/hasp_win32.h"
#else
#warning Building for Generic Devices
using dev::BaseDevice;
extern dev::BaseDevice haspDevice;
#endif

#endif