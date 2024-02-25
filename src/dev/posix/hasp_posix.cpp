/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(POSIX)

#include <cstdint>
#include <sys/utsname.h>
#ifndef TARGET_OS_MAC
#include <sys/sysinfo.h> // uptime
#else
#include <chrono> // for all examples :)
#include <time.h>
#include <errno.h>
#include <sys/sysctl.h>
#endif

#include "hasp_posix.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#ifdef USE_MONITOR
#include "display/monitor.h"
#elif USE_FBDEV
#include "display/fbdev.h"
#include "drv/tft/tft_driver.h"
#endif

#include <fstream>
#include <unistd.h>

// extern monitor_t monitor;

namespace dev {

PosixDevice::PosixDevice()
{
    struct utsname uts;

    if(uname(&uts) < 0) {
        LOG_ERROR(0, "uname() error");
        _hostname     = "localhost";
        _core_version = "unknown";
        _chip_model   = "unknown";
    } else {
        char version[256];
        snprintf(version, sizeof(version), "%s %s", uts.sysname, uts.release);
        _core_version = version;
        _chip_model   = uts.machine;
        _hostname     = uts.nodename;
    }

    _backlight_power  = 1;
    _backlight_invert = 0;
    _backlight_level  = 255;
}

void PosixDevice::set_config(const JsonObject& settings)
{
    configOutput(settings, 0);
#if USE_FBDEV
    if(settings["fbdev"].is<std::string>()) {
        haspTft.fbdev_path = "/dev/" + settings["fbdev"].as<std::string>();
    }
#if USE_EVDEV
    if(settings["evdev"].is<std::string>()) {
        haspTft.evdev_names.push_back(settings["evdev"].as<std::string>());
    }
    if(settings["evdevs"].is<JsonArray>()) {
        for(auto v : settings["evdevs"].as<JsonArray>()) {
            haspTft.evdev_names.push_back(v.as<std::string>());
        }
    }
#endif
    if(settings["bldev"].is<std::string>()) {
        haspDevice.backlight_device = settings["bldev"].as<std::string>();
    }
    if(settings["blmax"].is<int>()) {
        haspDevice.backlight_max = settings["blmax"];
    }
#endif
}

void PosixDevice::reboot()
{}
void PosixDevice::show_info()
{
    struct utsname uts;

    if(uname(&uts) < 0) {
        LOG_ERROR(0, "uname() error");
    } else {
        LOG_VERBOSE(0, "Sysname    : %s", uts.sysname);
        LOG_VERBOSE(0, "Nodename   : %s", uts.nodename);
        LOG_VERBOSE(0, "Release    : %s", uts.release);
        LOG_VERBOSE(0, "Version    : %s", uts.version);
        LOG_VERBOSE(0, "Machine    : %s", uts.machine);
    }

    LOG_VERBOSE(0, "Processor  : %s", get_chip_model());
    LOG_VERBOSE(0, "CPU freq.  : %i MHz", get_cpu_frequency());
    LOG_VERBOSE(0, "OS Version : %s", get_core_version());
}

const char* PosixDevice::get_hostname()
{
    return _hostname.c_str();
}

void PosixDevice::set_hostname(const char* hostname)
{
    _hostname = hostname;
#if USE_MONITOR
    monitor_title(hostname);
#elif USE_FBDEV
    // fbdev doesn't really have a title bar
#endif
}

const char* PosixDevice::get_core_version()
{
    return _core_version.c_str();
}

const char* PosixDevice::get_chip_model()
{
    return _chip_model.c_str();
}

const char* PosixDevice::get_hardware_id()
{
    return "223344556677";
}

void PosixDevice::set_backlight_pin(uint8_t pin)
{
    // PosixDevice::backlight_pin = pin;
}

void PosixDevice::set_backlight_invert(bool invert)
{
    _backlight_invert = invert;
    update_backlight();
}

bool PosixDevice::get_backlight_invert()
{
    return _backlight_invert;
}

void PosixDevice::set_backlight_level(uint8_t level)
{
    uint8_t new_level = level;

    if(_backlight_level != new_level) {
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
    uint8_t level = _backlight_power ? _backlight_level : 0;
    if(_backlight_invert) level = 255 - level;
#if USE_MONITOR
    monitor_backlight(level);
#elif USE_FBDEV
    // set display backlight, if possible
    if(backlight_device != "") {
        if(backlight_max == 0) {
            std::ifstream f;
            f.open("/sys/class/backlight/" + backlight_device + "/max_brightness");
            if(!f.fail()) {
                f >> backlight_max;
                f.close();
            } else {
                perror("Max brightness read failed");
            }
        }

        int brightness = map(level, 0, 255, 0, backlight_max);
        LOG_VERBOSE(0, "Setting brightness to %d/255 (%d)", level, brightness);

        std::ofstream f;
        f.open("/sys/class/backlight/" + backlight_device + "/brightness");
        if(!f.fail()) {
            f << brightness;
            f.close();
        } else {
            perror("Brightness write failed (are you root?)");
        }
    }
#endif
}

size_t PosixDevice::get_free_max_block()
{
#ifndef TARGET_OS_MAC
    struct sysinfo s_info;
    if(sysinfo(&s_info) < 0) return 0;
    return s_info.freeram;
#else
    return 0;
#endif
}

size_t PosixDevice::get_free_heap(void)
{
#ifndef TARGET_OS_MAC
    struct sysinfo s_info;
    if(sysinfo(&s_info) < 0) return 0;
    return s_info.freeram;
#else
    return 0;
#endif
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

void PosixDevice::run_thread(void (*func)(void*), void* arg)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void* (*)(void*))func, arg);
}

#ifndef TARGET_OS_MAC
long PosixDevice::get_uptime()
{
    struct sysinfo s_info;
    if(sysinfo(&s_info) < 0) return 0;
    return s_info.uptime;
}
#else
long PosixDevice::get_uptime()
{
    using namespace std::chrono;
    timeval ts;
    auto ts_len = sizeof(ts);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    auto constexpr mib_len = sizeof(mib) / sizeof(mib[0]);
    std::chrono::seconds sec;
    if(sysctl(mib, mib_len, &ts, &ts_len, nullptr, 0) == 0) {
        system_clock::time_point boot{seconds{ts.tv_sec} + microseconds{ts.tv_usec}};
        sec = duration_cast<seconds>(system_clock::now() - boot);
    } else {
        sec = 0s;
    }
    return (long)sec.count();
}

#endif

} // namespace dev

static time_t tv_sec_start = 0;

unsigned long PosixMillis()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    if(tv_sec_start == 0) {
        tv_sec_start = spec.tv_sec;
    }
    unsigned long msec1 = (spec.tv_sec - tv_sec_start) * 1000;
    unsigned long msec2 = spec.tv_nsec / 1e6;
    return msec1 + msec2;
}

void msleep(unsigned long millis)
{
    usleep(millis * 1000);
}

dev::PosixDevice haspDevice;

#endif // POSIX
