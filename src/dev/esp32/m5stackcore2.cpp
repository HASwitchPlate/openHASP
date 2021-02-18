#include "m5stackcore2.h"

#if defined(M5STACK)

#include "AXP192.h" // Power Mgmt
#include "dev/esp32/esp32.h"

// AXP192 Axp;
namespace dev {

void M5StackCore2::init(void)
{
    AXP192 Axp;
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Axp.begin();

    Axp.SetCHGCurrent(AXP192::kCHG_100mA);
    Axp.SetLcdVoltage(2800);

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

} // namespace dev

dev::M5StackCore2 haspDevice;

#endif
