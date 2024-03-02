/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_STM32F4_H
#define HASP_DEVICE_STM32F4_H

#include "hasp_conf.h"
#include "dev/device.h"

#if defined(STM32F7xx)
extern "C" {
int _gettimeofday(struct timeval* tv, struct timezone* tz);
}

namespace dev {

class Stm32f7Device : public BaseDevice {

  public:
    Stm32f7Device()
    {
        _hostname         = MQTT_NODENAME;
        _backlight_power  = 1;
        _backlight_invert = 0;
        _backlight_level  = 255;
        _backlight_pin    = TFT_BCKL;
    }

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

    bool is_system_pin(uint8_t pin) override;

  private:
    std::string _hostname;
    std::string _hardware_id;

    uint8_t _backlight_pin;
    uint8_t _backlight_level;
    uint8_t _backlight_power;
    uint8_t _backlight_invert;

    void update_backlight();
};

} // namespace dev

using dev::Stm32f7Device;
extern dev::Stm32f7Device haspDevice;

#endif // STM32F7xx

#endif // HASP_DEVICE_STM32F4_H