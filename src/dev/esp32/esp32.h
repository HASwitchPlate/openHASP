/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_ESP32_H
#define HASP_DEVICE_ESP32_H

#include "../device.h"

#if defined(ESP32)

namespace dev {

class Esp32Device : public BaseDevice {

  public:
    void reboot() override;

    void set_backlight_pin(uint8_t pin) override;

    void set_backlight_level(uint8_t val) override;

    uint8_t get_backlight_level() override;

    void set_backlight_power(bool power) override;

    bool get_backlight_power() override;

    size_t get_free_max_block() override;

    size_t get_free_heap() override;

    uint8_t get_heap_fragmentation() override;

    uint16_t get_cpu_frequency() override;

  private:
    uint8_t backlight_pin;
    uint8_t backlight_level;
    uint8_t backlight_power;

    void update_backlight();
};

} // namespace dev

using dev::Esp32Device;

#if defined(LANBONL8)
#warning Building for Lanbon L8
#include "lanbonl8.h"
#elif defined(M5STACK)
#warning Building for M5Stack core2
#include "m5stackcore2.h"
#else
extern dev::Esp32Device haspDevice;
#endif
#endif // ESP32

#endif // HASP_DEVICE_ESP32_H