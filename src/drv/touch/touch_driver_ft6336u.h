/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FT6336T_TOUCH_DRIVER_H
#define HASP_FT6336T_TOUCH_DRIVER_H

#if defined(ARDUINO) && !defined(HASP_USE_LGFX_TOUCH)
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include <Wire.h>
#include "FT6336U.h"

#include "touch_driver.h"    // base class
#include "touch_helper.h"    // i2c scanner

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

// #define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number

// Read touch points
// HASP_ATTRIBUTE_FAST_MEM bool FT6336U_getXY(int16_t* touchX, int16_t* touchY)
// {
//     if(ft6336u_touch->read_touch_number() == 1) {
//         *touchX = ft6336u_touch->read_touch1_x();
//         *touchY = ft6336u_touch->read_touch1_y();
//         return true;
//     } else {
//         return false;
//     }
// }

// IRAM_ATTR bool touch_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
// {

//     if(ft6336u_touch->read_touch_number() == 1) {
//         if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

//         data->point.x = ft6336u_touch->read_touch1_x();
//         data->point.y = ft6336u_touch->read_touch1_y();
//         data->state   = LV_INDEV_STATE_PR;

//     } else {
//         data->state = LV_INDEV_STATE_REL;
//     }

//     /*Return `false` because we are not buffering and no more data to read*/
//     return false;
// }

namespace dev {

class TouchFt6336u : public BaseTouch {

  public:
    FT6336U* ft6336u_touch;

    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        if(ft6336u_touch->read_touch_number() == 1) {
            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

            data->point.x = ft6336u_touch->read_touch1_x();
            data->point.y = ft6336u_touch->read_touch1_y();
            data->state   = LV_INDEV_STATE_PR;
            hasp_set_sleep_offset(0); // Reset the offset

#if defined(TOUCH_SWAP_XY) && (TOUCH_SWAP_XY)
            data->point.x = ft6336u_touch->read_touch1_y();
            data->point.y = ft6336u_touch->read_touch1_x();
#else
            data->point.x = ft6336u_touch->read_touch1_x();
            data->point.y = ft6336u_touch->read_touch1_y();
#endif

#if defined(TOUCH_INVERSE_X) && (TOUCH_INVERSE_X)
            data->point.x = _width_max - data->point.x;
#endif
#if defined(TOUCH_INVERSE_Y) && (TOUCH_INVERSE_Y)
            data->point.y = _height_max - data->point.y;
#endif

        } else {
            data->state = LV_INDEV_STATE_REL;
        }

        /*Return `false` because we are not buffering and no more data to read*/
        return false;
    }

    void init(int w, int h)
    {
        _height_max = h - 1;
        _width_max  = w - 1;

        LOG_INFO(TAG_DRVR, F("Touch SDA     : %d"), TOUCH_SDA);
        LOG_INFO(TAG_DRVR, F("Touch SCL     : %d"), TOUCH_SCL);
        LOG_INFO(TAG_DRVR, F("Touch freq.   : %d"), I2C_TOUCH_FREQUENCY);
        LOG_INFO(TAG_DRVR, F("Touch address : %x"), I2C_TOUCH_ADDRESS);

        LOG_VERBOSE(TAG_DRVR, F("%s %d"), __FILE__, __LINE__);

        ft6336u_touch = new FT6336U(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_IRQ);
        LOG_VERBOSE(TAG_DRVR, F("%s %d"), __FILE__, __LINE__);
        ft6336u_touch->begin();

        LOG_VERBOSE(TAG_DRVR, F("%s %d"), __FILE__, __LINE__);
        // From:  M5Core2/src/M5Touch.cpp
        // By default, the FT6336 will pulse the INT line for every touch
        // event. But because it shares the Wire1 TwoWire/I2C with other
        // devices, we cannot easily create an interrupt service routine to
        // handle these events. So instead, we set the INT wire to polled mode,
        // so it simply goes low as long as there is at least one valid touch.
        // ft6336u_touch->writeByte(0xA4, 0x00);
        /* Wire1.beginTransmission(I2C_TOUCH_ADDRESS);
         Wire1.write(0xA4); // address
         Wire1.write(0x00); // data
         Wire1.endTransmission();

         LOG_VERBOSE(TAG_DRVR, F("%s %d"), __FILE__, __LINE__);
         touch_scan(Wire1);*/

        if(ft6336u_touch->read_chip_id() != 0) {
            LOG_INFO(TAG_DRVR, F("FT6336U touch driver started chipid: %d"), ft6336u_touch->read_chip_id());
        } else {
            LOG_ERROR(TAG_DRVR, F("FT6336U touch driver failed to start"));
        }
    }

  private:
    uint16_t _width_max;
    uint16_t _height_max;
};

} // namespace dev

#warning Using FT6336
using dev::TouchFt6336u;
dev::TouchFt6336u haspTouch;

#endif // ARDUINO

#endif // HASP_FT6336T_TOUCH_DRIVER_H