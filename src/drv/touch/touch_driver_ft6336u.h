/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FT6336T_TOUCH_DRIVER_H
#define HASP_FT6336T_TOUCH_DRIVER_H

#ifdef ARDUINO
#include "Arduino.h"
#include "lvgl.h"

#include <Wire.h>
#include "FT6336U.h"

#include "touch_driver.h" // base class
#include "touch_helper.h" // i2c scanner

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

#define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number

FT6336U* ft6336u_touch;

// Read touch points
HASP_ATTRIBUTE_FAST_MEM bool FT6336U_getXY(int16_t* touchX, int16_t* touchY)
{
    if(ft6336u_touch->read_touch_number() == 1) {
        *touchX = ft6336u_touch->read_touch1_x();
        *touchY = ft6336u_touch->read_touch1_y();
        return true;
    } else {
        return false;
    }
}

IRAM_ATTR void touch_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{

    if(ft6336u_touch->read_touch_number() == 1) {
        if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

        data->point.x = ft6336u_touch->read_touch1_x();
        data->point.y = ft6336u_touch->read_touch1_y();
        data->state   = LV_INDEV_STATE_PR;

    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static inline void FT6336U_drv_init()
{
    LOG_INFO(TAG_DRVR, F("Touch SDA     : %d"), TOUCH_SDA);
    LOG_INFO(TAG_DRVR, F("Touch SCL     : %d"), TOUCH_SCL);
    LOG_INFO(TAG_DRVR, F("Touch freq.   : %d"), TOUCH_FREQUENCY);
    LOG_INFO(TAG_DRVR, F("Touch address : %x"), I2C_ADDR_FT6336U);

    ft6336u_touch = new FT6336U(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_IRQ);
    ft6336u_touch->begin();

    // From:  M5Core2/src/M5Touch.cpp
    // By default, the FT6336 will pulse the INT line for every touch
    // event. But because it shares the Wire1 TwoWire/I2C with other
    // devices, we cannot easily create an interrupt service routine to
    // handle these events. So instead, we set the INT wire to polled mode,
    // so it simply goes low as long as there is at least one valid touch.
    // ft6336u_touch->writeByte(0xA4, 0x00);
    Wire1.beginTransmission(I2C_ADDR_FT6336U);
    Wire1.write(0xA4); // address
    Wire1.write(0x00); // data
    Wire1.endTransmission();

    touch_scan(Wire1);

    if(ft6336u_touch->read_chip_id() != 0) {
        LOG_INFO(TAG_DRVR, F("FT6336U touch driver started chipid: %d"), ft6336u_touch->read_chip_id());
    } else {
        LOG_ERROR(TAG_DRVR, F("FT6336U touch driver failed to start"));
    }
}

namespace dev {

class TouchFt6336u : public BaseTouch {

  public:
    void init(int w, int h)
    {
        FT6336U_drv_init();
    }

    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        int16_t touchX;
        int16_t touchY;
        if(FT6336U_getXY(&touchX, &touchY)) {
            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

            data->point.x = touchX;
            data->point.y = touchY;
            data->state   = LV_INDEV_STATE_PR;

        } else {
            data->state = LV_INDEV_STATE_REL;
        }

        /*Return `false` because we are not buffering and no more data to read*/
        return false;
    }
};

} // namespace dev

#warning Using FT6336
using dev::TouchFt6336u;
extern dev::TouchFt6336u haspTouch;

#endif // ARDUINO

#endif // HASP_FT6336T_TOUCH_DRIVER_H