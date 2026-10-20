/* CrowPanel Advance device support. See the project LICENSE for terms. */

#include "crowpanel_advanced.h"

#if defined(HASP_CROWPANEL_ADVANCED)

#include <Arduino.h>
#include <Wire.h>

#include "hasp_debug.h"

namespace dev {

static constexpr uint8_t controllerAddress = 0x30;
static constexpr uint8_t touchAddress      = 0x5D;

bool CrowPanelAdvanced::probe(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

bool CrowPanelAdvanced::command(uint8_t value)
{
    Wire.beginTransmission(controllerAddress);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

void CrowPanelAdvanced::set_backlight_pin(uint8_t)
{
    if(_initialized) return;
    _initialized = true;

    Wire.begin(TOUCH_SDA, TOUCH_SCL, I2C_TOUCH_FREQUENCY);
    Wire.setTimeOut(50);
    delay(50);

    // Avoid driving the touch recovery line on an unexpected board revision.
    _controller_present = probe(controllerAddress);
    if(!_controller_present) {
        LOG_ERROR(TAG_GUI, F("CrowPanel backlight controller at 0x30 not found"));
        return;
    }

    bool touchPresent = probe(touchAddress);
    for(unsigned attempt = 0; !touchPresent && attempt < 5; ++attempt) {
        if(!command(250)) break; // Vendor touch-activation command.
        pinMode(1, OUTPUT);
        digitalWrite(1, LOW);
        delay(120);
        pinMode(1, INPUT);
        delay(100);
        touchPresent = probe(touchAddress);
    }
    if(!touchPresent) LOG_ERROR(TAG_GUI, F("CrowPanel touch controller at 0x5D not found"));

    update_backlight();
}

void CrowPanelAdvanced::set_backlight_level(uint8_t level)
{
    _backlight_level = level;
    update_backlight();
}

uint8_t CrowPanelAdvanced::get_backlight_level()
{
    return _backlight_level;
}

void CrowPanelAdvanced::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool CrowPanelAdvanced::get_backlight_power()
{
    return _backlight_power;
}

void CrowPanelAdvanced::update_backlight()
{
    if(!_controller_present) return;

    // Controller range is inverted: 0 is brightest and 245 is off.
    // Values 246..255 are special commands and must not be sent as brightness.
    uint8_t value = _backlight_power ? 245u - (uint32_t(_backlight_level) * 245u / 255u) : 245u;
    if(!command(value)) LOG_ERROR(TAG_GUI, F("CrowPanel backlight command failed"));
}

bool CrowPanelAdvanced::is_system_pin(uint8_t pin)
{
    if(Esp32Device::is_system_pin(pin)) return true;

    // RGB bus, shared I2C bus, and touch recovery line.
    const uint8_t pins[] = {1, 3, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
                            21, 38, 39, 40, 41, 42, 45, 46, 47, 48};
    for(uint8_t used : pins) {
        if(pin == used) return true;
    }
    return false;
}

} // namespace dev

dev::CrowPanelAdvanced haspDevice;

#endif
