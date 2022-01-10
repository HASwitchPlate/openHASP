/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(WINDOWS)

#include <cstdint>
#include "Windows.h"

#include "hasp_win32.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#include "display/monitor.h"

namespace dev {

static inline void native_cpuid(unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx)
{
    /* ecx is often an input as well as an output. */
    asm volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "0"(*eax), "2"(*ecx) : "memory");
}

void Win32Device::reboot()
{}

void Win32Device::show_info()
{

    unsigned int eax, ebx, ecx, edx;
    eax = 0;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    // printf("EAX: %08X EBX: %08X ECX: %08X EDX: %08X\n", eax, ebx, ecx, edx);
    char vendor[13];
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';

    LOG_VERBOSE(0, F("Processor  : %s"), vendor);
    LOG_VERBOSE(0, F("CPU freq.  : %i MHz"), get_cpu_frequency());
}

const char* Win32Device::get_hostname()
{
    return _hostname.c_str();
}
void Win32Device::set_hostname(const char* hostname)
{
    _hostname = hostname;
    monitor_title(hostname);
    // SDL_SetWindowTitle(monitor.window, hostname);
}
const char* Win32Device::get_core_version()
{
    return _core_version.c_str();
}

const char* Win32Device::get_chip_model()
{
    return "SDL2";
}

const char* Win32Device::get_hardware_id()
{
    return "112233445566";
}

void Win32Device::set_backlight_pin(uint8_t pin)
{
    // Win32Device::_backlight_pin = pin;
}

void Win32Device::set_backlight_level(uint8_t level)
{
    uint8_t new_level = level;

    if(_backlight_level != new_level) {
        _backlight_level = new_level;
        update_backlight();
    }
}

uint8_t Win32Device::get_backlight_level()
{
    return _backlight_level;
}

void Win32Device::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool Win32Device::get_backlight_power()
{
    return _backlight_power != 0;
}

void Win32Device::update_backlight()
{
    uint8_t level = _backlight_power ? _backlight_level : 0;
    if(_backlight_invert) level = 255 - level;
    monitor_backlight(level);
}

size_t Win32Device::get_free_max_block()
{
    return 0;
}

size_t Win32Device::get_free_heap(void)
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
}

uint8_t Win32Device::get_heap_fragmentation()
{
    return 0;
}

uint16_t Win32Device::get_cpu_frequency()
{
    return 0;
}

bool Win32Device::is_system_pin(uint8_t pin)
{
    return false;
}

long Win32Device::get_uptime()
{
    return GetTickCount64() / 1000;
}

} // namespace dev

dev::Win32Device haspDevice;

#endif // WINDOWS