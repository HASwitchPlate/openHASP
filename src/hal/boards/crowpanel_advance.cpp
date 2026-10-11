/* CrowPanel Advance board support. See the project LICENSE for terms. */

#if defined(ESP32) && defined(HASP_CROWPANEL_ADVANCE_STC)

#include <Arduino.h>
#include <Wire.h>

#include "crowpanel_advance.h"
#include "hasp_debug.h"

namespace {

constexpr uint8_t controllerAddress = 0x30;
constexpr uint8_t touchAddress      = 0x5D;
bool initialized                   = false;
bool controllerPresent             = false;
bool touchReady                    = false;

bool probe(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

bool command(uint8_t value)
{
    Wire.beginTransmission(controllerAddress);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

} // namespace

bool crowpanelAdvanceBegin()
{
    if(initialized) return touchReady;
    initialized = true;

    Wire.begin(15, 16, 400000);
    Wire.setTimeOut(50);
    delay(50);

    // Avoid driving the touch recovery line on an unexpected board revision.
    controllerPresent = probe(controllerAddress);
    if(!controllerPresent) {
        LOG_ERROR(TAG_GUI, F("CrowPanel backlight controller at 0x30 not found"));
        return false;
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

    command(0); // Initial full brightness; saved brightness is applied later.
    touchReady = touchPresent;
    return touchReady;
}

void crowpanelAdvanceBacklight(uint8_t level, bool power)
{
    if(!initialized || !controllerPresent) return;

    // Controller range is inverted: 0 is brightest and 245 is off.
    // Values 246..255 are special commands and must not be sent as brightness.
    uint8_t value = power ? 245u - (uint32_t(level) * 245u / 255u) : 245u;
    if(!command(value)) LOG_ERROR(TAG_GUI, F("CrowPanel backlight command failed"));
}

bool crowpanelAdvancePinInUse(uint8_t pin)
{
    // RGB bus, shared I2C bus, and touch recovery line.
    const uint8_t pins[] = {1, 3, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
                            21, 38, 39, 40, 41, 42, 45, 46, 47, 48};
    for(uint8_t used : pins) {
        if(pin == used) return true;
    }
    return false;
}

#endif
