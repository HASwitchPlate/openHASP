/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "m5stackcore2.h"

#if defined(M5STACK)

#include "AXP192.h" // Power Mgmt
#include "dev/esp32/esp32.h"

AXP192 Axp;

// AXP192 Axp;
namespace dev {

void M5StackCore2::init(void)
{
    Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
    Axp.begin();

    Axp.SetCHGCurrent(AXP192::kCHG_100mA);
    Axp.SetLcdVoltage(2800);
    _backlight_power = true;

    Axp.SetBusPowerMode(0);
    Axp.SetCHGCurrent(AXP192::kCHG_190mA);

    Axp.SetLDOEnable(3, true);
    //    CoverScrollText("Motor Test", M5.Lcd.color565(SUCCE_COLOR));
    delay(150);
    Axp.SetLDOEnable(3, false);

    Axp.SetLed(1);
    //   CoverScrollText("LED Test", M5.Lcd.color565(SUCCE_COLOR));
    delay(100);
    Axp.SetLed(0);

    // FastLED.addLeds<SK6812, LEDS_PIN>(ledsBuff, LEDS_NUM);
    // for(int i = 0; i < LEDS_NUM; i++) {
    //     ledsBuff[i].setRGB(20, 20, 20);
    // }
    // FastLED.show();

    Axp.SetLDOVoltage(3, 3300);
    Axp.SetLed(1);
}

void M5StackCore2::set_backlight_level(uint8_t level)
{
    _backlight_level = level;
    update_backlight();
}

uint8_t M5StackCore2::get_backlight_level()
{
    return _backlight_level;
}

void M5StackCore2::set_backlight_power(bool power)
{
    _backlight_power = power;
    update_backlight();
}

bool M5StackCore2::get_backlight_power()
{
    return _backlight_power;
}

void M5StackCore2::update_backlight()
{
    if(_backlight_power) {
        uint16_t voltage = map(_backlight_level, 0, 255, 2500, 3300);
        Axp.SetLcdVoltage(voltage);
    } else {
        Axp.SetDCVoltage(2, 2200);
    }
    // Axp.SetDCDC3(_backlight_power); // LCD backlight
}

void M5StackCore2::get_sensors(JsonDocument& doc)
{
    Esp32Device::get_sensors(doc);

    JsonObject sensor        = doc.createNestedObject(F("AXP192"));
    sensor[F("BattVoltage")] = Axp.GetBatVoltage();
    sensor[F("BattPower")]   = Axp.GetBatPower();
    // sensor[F("Batt%")]             = Axp.getBattPercentage();
    sensor[F("BattChargeCurrent")] = Axp.GetBatChargeCurrent();
    sensor[F("Temperature")]       = Axp.GetTempInAXP192();
    sensor[F("Charging")]          = Axp.isCharging();
}

} // namespace dev

dev::M5StackCore2 haspDevice;

#endif
