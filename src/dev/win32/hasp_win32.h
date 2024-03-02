/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_WINDOWS_H
#define HASP_DEVICE_WINDOWS_H

#include <cstdint>
#include <string>
#include "Windows.h"

#include "hasp_conf.h"
#include "../device.h"

#if defined(WINDOWS)

namespace dev {

class Win32Device : public BaseDevice {

  public:
    Win32Device();

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

extern long Win32Millis();

using dev::Win32Device;
extern dev::Win32Device haspDevice;

#endif // WINDOWS

#endif // HASP_DEVICE_WINDOWS_H
