/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TOUCH_HELPER_H
#define HASP_TOUCH_HELPER_H

#ifdef ARDUINO
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include <Wire.h>
#include "hasp_debug.h"

void touch_scan(TwoWire& i2c)
{
    char buffer[64];
    byte error, address;
    int nDevices;

    LOG_VERBOSE(TAG_DRVR, F("Scanning I2C..."));

    nDevices = 0;
    for(address = 1; address < 127; address++) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        i2c.beginTransmission(address);
        error = i2c.endTransmission();

        if(error == 0) {
            snprintf_P(buffer, sizeof(buffer), PSTR(D_BULLET " Found device 0x%02x"), address);
            LOG_INFO(TAG_DRVR, buffer, address);
            nDevices++;
        } else if(error == 4) {
            snprintf_P(buffer, sizeof(buffer), PSTR(D_BULLET "Unknown error at address 0x%02x"), address);
            LOG_WARNING(TAG_DRVR, buffer, address);
        }
    }
    if(nDevices == 0)
        LOG_WARNING(TAG_DRVR, F("No I2C devices found"));
    else
        LOG_VERBOSE(TAG_DRVR, F("Scan complete"));
}

#endif // ARDUINO

#endif // HASP_TOUCH_HELPER_H
