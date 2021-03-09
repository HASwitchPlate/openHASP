#if defined(POSIX)

#include <cstdint>

#include "hasp_posix.h"

#include "hasp_conf.h"
#include "hasp/hasp_utilities.h"
#include "hasp_debug.h"

#include "display/monitor.h"

namespace dev {

void PosixDevice::reboot()
{}
void PosixDevice::show_info()
{
    LOG_VERBOSE(0, F("Processor  : %s"), "unknown");
    LOG_VERBOSE(0, F("CPU freq.  : %i MHz"), 0);
}


const char* PosixDevice::get_hostname()
{
    return _hostname.c_str();
}
void PosixDevice::set_hostname(const char* hostname)
{
    _hostname = hostname;
}
const char* PosixDevice::get_core_version()
{
    return "posix";
}
const char* PosixDevice::get_display_driver()
{
    return "SDL2";
}

void PosixDevice::set_backlight_pin(uint8_t pin)
{
    // PosixDevice::backlight_pin = pin;
}

void PosixDevice::set_backlight_level(uint8_t level)
{
    uint8_t new_level = level >= 0 ? level : 0;
    new_level         = new_level <= 100 ? new_level : 100;

    if(_backlight_level != new_level) 
    {
        _backlight_level = new_level;
        update_backlight();
    }
}

uint8_t PosixDevice::get_backlight_level()
{
    return _backlight_level;
}

void PosixDevice::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool PosixDevice::get_backlight_power()
{
    return _backlight_power != 0;
}

void PosixDevice::update_backlight()
{
    monitor_backlight(_backlight_power ? map(_backlight_level, 0, 100, 0, 255) : 0);
}

size_t PosixDevice::get_free_max_block()
{
    return 0;
}

size_t PosixDevice::get_free_heap(void)
{
    return 0;
}

uint8_t PosixDevice::get_heap_fragmentation()
{
    return 0;
}

uint16_t PosixDevice::get_cpu_frequency()
{
    return 0;
}

bool PosixDevice::is_system_pin(uint8_t pin)
{
    return false;
}

} // namespace dev

dev::PosixDevice haspDevice;

#endif // POSIX
