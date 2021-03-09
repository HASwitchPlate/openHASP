#if defined(POSIX)

#include <cstdint>
#include <sys/utsname.h>

#include "hasp_posix.h"

#include "hasp_conf.h"
#include "hasp/hasp_utilities.h"
#include "hasp_debug.h"

#include "display/monitor.h"

// extern monitor_t monitor;

namespace dev {

PosixDevice::PosixDevice()    {
        struct utsname uts;

        if (uname(&uts) < 0) {
          LOG_ERROR(0,"uname() error");
          _hostname = "localhost";
          _core_version = "unknown";
        } else {
        //   LOG_VERBOSE(0,"Sysname:  %s", uts.sysname);
        //   LOG_VERBOSE(0,"Nodename: %s", uts.nodename);
        //   LOG_VERBOSE(0,"Release:  %s", uts.release);
        //   LOG_VERBOSE(0,"Version:  %s", uts.version);
        //   LOG_VERBOSE(0,"Machine:  %s", uts.machine);

          char version[128];
          snprintf(version, sizeof(version), "%s %s", uts.sysname, uts.release);
          _core_version = version;
          _hostname = uts.nodename;
        }

        _backlight_power = 1;
        _backlight_level = 100;
    }

void PosixDevice::reboot()
{}
void PosixDevice::show_info()
{
  struct utsname uts;

  if (uname(&uts) < 0) {
    LOG_ERROR(0,"uname() error");
  } else {
    LOG_VERBOSE(0,"Sysname:  %s", uts.sysname);
    LOG_VERBOSE(0,"Nodename: %s", uts.nodename);
    LOG_VERBOSE(0,"Release:  %s", uts.release);
    LOG_VERBOSE(0,"Version:  %s", uts.version);
    LOG_VERBOSE(0,"Machine:  %s", uts.machine);
  }

    LOG_VERBOSE(0, "Processor  : %s", "unknown");
    LOG_VERBOSE(0, "CPU freq.  : %i MHz", 0);
}


const char* PosixDevice::get_hostname()
{
    return _hostname.c_str();
}
void PosixDevice::set_hostname(const char* hostname)
{
    _hostname = hostname;
    monitor_title(hostname);
    // SDL_SetWindowTitle(monitor.window, hostname);
}
const char* PosixDevice::get_core_version()
{
    return _core_version.c_str();
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
    uint8_t level = _backlight_power ? map(_backlight_level, 0, 100, 0, 255) : 0;
    monitor_backlight(level);
    // SDL_SetTextureColorMod(monitor.texture, level, level, level);
    // window_update(&monitor);
    // monitor.sdl_refr_qry = true;
    // monitor_sdl_refr(NULL);
    // const lv_area_t area = {1,1,0,0};
    //monitor_flush(NULL,&area,NULL);
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
