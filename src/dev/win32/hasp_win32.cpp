#if defined(WINDOWS)

#include <cstdint>
#include "Windows.h"

#include "hasp_win32.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

namespace dev {

void Win32Device::reboot()
{}

const char* Win32Device::get_hostname()
{
    return "winhasp";
}
const char* Win32Device::get_core_version()
{
    return "win32";
}
const char* Win32Device::get_display_driver()
{
    return "test";
}

void Win32Device::set_backlight_pin(uint8_t pin)
{
    Win32Device::backlight_pin = pin;
}

void Win32Device::set_backlight_level(uint8_t level)
{
    backlight_level = level >= 0 ? level : 0;
    backlight_level = backlight_level <= 100 ? backlight_level : 100;
    update_backlight();
}

uint8_t Win32Device::get_backlight_level()
{
    return backlight_level;
}

void Win32Device::set_backlight_power(bool power)
{
    backlight_power = power;
    update_backlight();
}

bool Win32Device::get_backlight_power()
{
    return backlight_power != 0;
}

void Win32Device::update_backlight()
{
    if(backlight_pin == -1) return;
}

size_t Win32Device::get_free_max_block()
{
    return 0;
}

size_t Win32Device::get_free_heap(void)
{
    return 0;
}

uint8_t Win32Device::get_heap_fragmentation()
{
    return 0;
}

uint16_t Win32Device::get_cpu_frequency()
{
    return 0;
}

} // namespace dev

dev::Win32Device haspDevice;

#endif // WINDOWS