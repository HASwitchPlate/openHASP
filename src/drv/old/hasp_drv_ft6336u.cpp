#if TOUCH_DRIVER == 0x6336

#include <Wire.h>
#include "FT6336U.h"
#include "ArduinoLog.h"

#include "hasp_drv_ft6336u.h"

#define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number

FT6336U* touchpanel;

// Read touch points
HASP_ATTRIBUTE_FAST_MEM bool FT6336U_getXY(int16_t* touchX, int16_t* touchY, bool debug)
{
    if(touchpanel->read_touch_number() == 1) {
        *touchX = touchpanel->read_touch1_x();
        *touchY = touchpanel->read_touch1_y();
        return true;
    } else {
        return false;
    }
}

static void scan(TwoWire& i2c)
{
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for(address = 1; address < 127; address++) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        i2c.beginTransmission(address);
        error = i2c.endTransmission();

        if(error == 0) {
            Serial.print("I2C device found at address 0x");
            if(address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        } else if(error == 4) {
            Serial.print("Unknown error at address 0x");
            if(address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if(nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
}

void FT6336U_init()
{
    scan(Wire1);

    LOG_INFO(TAG_DRVR, F("Touch SDA     : %d"), TOUCH_SDA);
    LOG_INFO(TAG_DRVR, F("Touch SCL     : %d"), TOUCH_SCL);
    LOG_INFO(TAG_DRVR, F("Touch freq.   : %d"), I2C_TOUCH_FREQUENCY);
    LOG_INFO(TAG_DRVR, F("Touch address : %x"), I2C_TOUCH_ADDRESS);

    touchpanel = new FT6336U(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_IRQ);
    touchpanel->begin();

    // From:  M5Core2/src/M5Touch.cpp
    // By default, the FT6336 will pulse the INT line for every touch
    // event. But because it shares the Wire1 TwoWire/I2C with other
    // devices, we cannot easily create an interrupt service routine to
    // handle these events. So instead, we set the INT wire to polled mode,
    // so it simply goes low as long as there is at least one valid touch.
    // touchpanel->writeByte(0xA4, 0x00);
    Wire1.beginTransmission(I2C_TOUCH_ADDRESS);
    Wire1.write(0xA4); // address
    Wire1.write(0x00); // data
    Wire1.endTransmission();

    if(touchpanel->read_chip_id() != 0) {
        LOG_INFO(TAG_DRVR, F("FT6336U touch driver started chipid: %d"), touchpanel->read_chip_id());
    } else {
        LOG_ERROR(TAG_DRVR, F("FT6336U touch driver failed to start"));
    }
}
#endif