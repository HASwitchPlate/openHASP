#if TOUCH_DRIVER == 5206

#include <Wire.h>
#include "focaltech.h"
#include "ArduinoLog.h"

#include "hasp_drv_ft5206.h"

#define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number

FocalTech_Class* touchpanel;

// Read touch points
bool FT5206_getXY(int16_t* touchX, int16_t* touchY, bool debug)
{
    if(!touchpanel->getTouched()) return false;

    uint16_t x;
    uint16_t y;
    bool res = touchpanel->getPoint(x, y);
    *touchX  = x;
    *touchY  = y;

    if(debug) {
        LOG_VERBOSE(TAG_DRVR, F("FT5206 touched x: %d y: %d\n"), *touchX, *touchY);
    }

    return res;
}

void scan(TwoWire& i2c)
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

// void FT5206_init(TwoWire & i2c)
void FT5206_init()
{
    LOG_INFO(TAG_DRVR, F("Touch SDA     : %d"), TOUCH_SDA);
    LOG_INFO(TAG_DRVR, F("Touch SCL     : %d"), TOUCH_SCL);
    LOG_INFO(TAG_DRVR, F("Touch freq.   : %d"), I2C_TOUCH_FREQUENCY);
    LOG_INFO(TAG_DRVR, F("Touch address : %02x"), FT5206_address);

    Wire1.begin(TOUCH_SDA, TOUCH_SCL, I2C_TOUCH_FREQUENCY);
    scan(Wire1);
    touchpanel = new FocalTech_Class();

    if(touchpanel->begin(Wire1, FT5206_address)) {
        LOG_INFO(TAG_DRVR, F("FT5206 touch driver started"));
    } else {
        LOG_ERROR(TAG_DRVR, F("FT5206 touch driver failed to start"));
    }
}
#endif