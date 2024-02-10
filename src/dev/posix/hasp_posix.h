/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_POSIX_H
#define HASP_DEVICE_POSIX_H

#include <cstdint>
#include <cstddef>

extern "C" {
#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/utsname.h>
}

#include "hasp_conf.h"
#include "../device.h"

#if defined(POSIX)
static inline void itoa(int i, char* out, int unused_)
{
    (void)unused_;
    sprintf(out, "%d", i);
}

namespace dev {

class PosixDevice : public BaseDevice {

  public:
    PosixDevice();

    void set_config(const JsonObject& settings);

    void reboot() override;
    void show_info() override;

    const char* get_hostname();
    void set_hostname(const char*);
    const char* get_core_version();
    const char* get_chip_model();
    const char* get_hardware_id();

    void set_backlight_pin(uint8_t pin);
    void set_backlight_invert(bool invert) override;
    void set_backlight_level(uint8_t val);
    uint8_t get_backlight_level();
    void set_backlight_power(bool power);
    bool get_backlight_invert() override;
    bool get_backlight_power();

    size_t get_free_max_block();
    size_t get_free_heap();
    uint8_t get_heap_fragmentation();
    uint16_t get_cpu_frequency();
    long get_uptime();

    bool is_system_pin(uint8_t pin) override;

    void run_thread(void (*func)(void*), void* arg);

  public:
    std::string backlight_device;
    int backlight_max = 0;

  private:
    std::string _hostname;
    std::string _core_version;
    std::string _chip_model;

    uint8_t _backlight_pin;
    uint8_t _backlight_level;
    uint8_t _backlight_power;
    uint8_t _backlight_invert;

    void update_backlight();
};

} // namespace dev

extern unsigned long PosixMillis();
extern void msleep(unsigned long millis);

using dev::PosixDevice;
extern dev::PosixDevice haspDevice;

#endif // POSIX

#endif // HASP_DEVICE_POSIX_H
