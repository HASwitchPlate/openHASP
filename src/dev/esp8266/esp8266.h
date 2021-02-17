/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_ESP8266_H
#define HASP_DEVICE_ESP8266_H

#include "hasp_conf.h"
#include "dev/device.h"

#if defined(ESP8266)

namespace dev {

class Esp8266Device : public BaseDevice {

  public:
    void reboot() override;

    void set_backlight_pin(uint8_t pin) override;

    void set_backlight_level(uint8_t val) override;

    uint8_t get_backlight_level() override;

    void set_backlight_power(bool power) override;

    bool get_backlight_power() override;

  private:
    uint8_t backlight_pin;
    uint8_t backlight_level;
    uint8_t backlight_power;

    void update_backlight();
};

} // namespace dev

using dev::Esp8266Device;

extern dev::Esp8266Device haspDevice;

#endif // ESP8266

#endif // HASP_DEVICE_ESP8266_H