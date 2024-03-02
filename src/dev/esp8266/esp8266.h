/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_ESP8266_H
#define HASP_DEVICE_ESP8266_H

#include "hasp_conf.h"
#include "dev/device.h"

#if defined(ESP8266)

namespace dev {

class Esp8266Device : public BaseDevice {

  public:
    Esp8266Device();

    void reboot() override;
    void show_info() override;

    const char* get_hostname();
    void set_hostname(const char*);
    const char* get_core_version();
    const char* get_chip_model();
    const char* get_hardware_id();

    void set_backlight_pin(uint8_t pin) override;
    void set_backlight_invert(bool invert) override;
    void set_backlight_level(uint8_t val) override;
    uint8_t get_backlight_level() override;
    void set_backlight_power(bool power) override;
    bool get_backlight_invert() override;
    bool get_backlight_power() override;

    size_t get_free_max_block() override;
    size_t get_free_heap() override;
    uint8_t get_heap_fragmentation() override;
    uint16_t get_cpu_frequency() override;
    long get_uptime();

    bool is_system_pin(uint8_t pin) override;

  private:
    std::string _hostname;
    std::string _hardware_id;
    std::string _core_version;

    uint8_t _backlight_pin;
    uint8_t _backlight_level;
    uint8_t _backlight_power;
    uint8_t _backlight_invert;

    void update_backlight();
};

} // namespace dev

using dev::Esp8266Device;
extern dev::Esp8266Device haspDevice;

#endif // ESP8266

#endif // HASP_DEVICE_ESP8266_H