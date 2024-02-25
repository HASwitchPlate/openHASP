/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_STMPE610_TOUCH_DRIVER_H
#define HASP_STMPE610_TOUCH_DRIVER_H

#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_STMPE610.h"

#include "touch_driver.h" // base class

#include "hasp_conf.h"

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

static Adafruit_STMPE610 stmpe610_touchpanel = Adafruit_STMPE610(TOUCH_CS);

bool touch_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    data->state = LV_INDEV_STATE_REL;

    // while touched, but the state is released => read next point
    while(data->state == LV_INDEV_STATE_REL && stmpe610_touchpanel.touched()) {

        TS_Point point = stmpe610_touchpanel.getPoint();
        Log.trace(TAG_DRVR, F("STMPE610: x=%i y=%i z=%i"), point.x, point.y, point.z);

        if(point.z && point.x < 4096 && point.y < 4096) {                     // valid reading
            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle
            data->state = LV_INDEV_STATE_PR;
        hasp_set_sleep_offset(0);  // Reset the offset

#if HX8357D_DRIVER == 1
            data->point.x = map(point.x, TS_MINX, TS_MAXX, TFT_WIDTH, 0);
            data->point.y = map(point.y, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
#else
            data->point.x = map(point.x, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
            data->point.y = map(point.y, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
#endif
        }
    }

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

namespace dev {

class TouchStmpe610 : public BaseTouch {

  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        return touch_read(indev_driver, data);
    }

    void init(int w, int h)
    {
        LOG_TRACE(TAG_DRVR, F("STMPE610 " D_SERVICE_STARTING));
        if(!stmpe610_touchpanel.begin()) {
            LOG_ERROR(TAG_DRVR, F("STMPE610 " D_SERVICE_START_FAILED));
        } else {
            LOG_INFO(TAG_DRVR, F("STMPE610 " D_SERVICE_STARTED));
        }
    }
};

} // namespace dev

using dev::TouchStmpe610;
extern dev::TouchStmpe610 haspTouch;

#endif // ARDUINO

#endif // HASP_STMPE610_TOUCH_DRIVER_H