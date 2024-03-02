/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_ANALOG_TOUCH_DRIVER_H
#define HASP_ANALOG_TOUCH_DRIVER_H

#ifdef ARDUINO
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include "hasp_drv_analogTouch.h"
#include "touch_driver.h" // base class

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;
#define MINPRESSURE 200
#define MAXPRESSURE 2400

const int XP = TFT_D6, XM = TFT_DC, YP = TFT_WR, YM = TFT_D7; // ID=0x9341
const int TS_LEFT = 560, TS_RT = 3670, TS_TOP = 3850, TS_BOT = 580;
int max_x = 4095, max_y = 4095;

static TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

namespace dev {

class AnalogTouch : public BaseTouch {

  public:
    void init(int w, int h)
    {
        max_x = w;
        max_y = h;
    }

    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        static TSPoint tp;
        tp = ts.getPoint();
        if(tp.z < MINPRESSURE) {
            data->state = LV_INDEV_STATE_REL;
        } else {
            data->point.x = map(tp.x, TS_LEFT, TS_RT, 0, max_x);
            data->point.y = map(tp.y, TS_BOT, TS_TOP, max_y, 0);
            data->state   = LV_INDEV_STATE_PR;
            hasp_set_sleep_offset(0); // Reset the offset
        }

        return false;
    }
};

} // namespace dev

using dev::AnalogTouch;
extern dev::AnalogTouch haspTouch;

#endif // ARDUINO

#endif // HASP_ANALOG_TOUCH_DRIVER_H
