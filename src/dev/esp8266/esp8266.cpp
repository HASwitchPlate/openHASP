#if defined(ESP8266)

#include "Arduino.h"
#include <Esp.h>

#include "dev/esp8266/esp8266.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#define BACKLIGHT_CHANNEL 0

namespace dev {

void Esp8266Device::reboot()
{
    ESP.restart();
}

void Esp8266Device::set_backlight_pin(uint8_t pin)
{
    Esp8266Device::backlight_pin = pin;
    /* Setup Backlight Control Pin */
    if(pin >= 0) {
        LOG_VERBOSE(TAG_GUI, F("Backlight  : Pin %d"), pin);
        pinMode(backlight_pin, OUTPUT);
        update_backlight();
    }
}

void Esp8266Device::set_backlight_level(uint8_t level)
{
    backlight_level = level >= 0 ? level : 0;
    backlight_level = backlight_level <= 100 ? backlight_level : 100;

    update_backlight();
}

uint8_t Esp8266Device::get_backlight_level()
{
    return backlight_level;
}

void Esp8266Device::set_backlight_power(bool power)
{
    backlight_power = power;
    update_backlight();
}

bool Esp8266Device::get_backlight_power()
{
    return backlight_power != 0;
}

void Esp8266Device::update_backlight()
{
    if(backlight_pin == -1) return;

    analogWrite(backlight_pin, backlight_power ? map(backlight_level, 0, 100, 0, 1023) : 0);
}

} // namespace dev

dev::Esp8266Device haspDevice;

#endif // ESP8266