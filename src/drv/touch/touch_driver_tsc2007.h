/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TSC2007_TOUCH_DRIVER_H
#define HASP_TSC2007_TOUCH_DRIVER_H

#if defined(ARDUINO) && !defined(HASP_USE_LGFX_TOUCH)
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include <Wire.h>
#include "Adafruit_TSC2007.h"

#include "touch_driver.h"    // base class
#include "touch_helper.h"    // i2c scanner

#include "hasp_debug.h"

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000
#define TS_MIN_PRESSURE 100

namespace dev {

class TouchTsc2007 : public BaseTouch {
  public:
    Adafruit_TSC2007* ts;
        
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        uint16_t x, y, z1, z2;
        if (ts->read_touch(&x, &y, &z1, &z2) && (z1 > TS_MIN_PRESSURE)) {
            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

            data->state   = LV_INDEV_STATE_PR;
            hasp_set_sleep_offset(0); // Reset the offset
        
            // Scale from ~0->4000 to tft.width using the calibration #'s
            x = map(x, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
            y = map(y, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);

            // LOG_INFO(TAG_DRVR, F("Touch point: %i, %i"), x, y);

#if defined(TOUCH_SWAP_XY) && (TOUCH_SWAP_XY)
            data->point.x = y;
            data->point.y = x;
#else
            data->point.x = x;
            data->point.y = y;
#endif

#if defined(TOUCH_INVERSE_X) && (TOUCH_INVERSE_X)
            data->point.x = _width_max - x;
#endif
#if defined(TOUCH_INVERSE_Y) && (TOUCH_INVERSE_Y)
            data->point.y = _height_max - y;
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

        // tsc2007_touch = new Adafruit_TSC2007();
        LOG_VERBOSE(TAG_DRVR, F("%s %d"), __FILE__, __LINE__);

        ts = new Adafruit_TSC2007();

        // Startup sequence CONTROLLER parT
        if (!ts->begin()) {
            LOG_INFO(TAG_DRVR, F("Failed to find Adafruit TSC2007 chip"));
            while (1) { delay(10); }
        }
        LOG_INFO(TAG_DRVR, F("Found Adafruit TSC2007 chip"));
    }

  private:
    uint16_t _width_max;
    uint16_t _height_max;
};

} // namespace dev

using dev::TouchTsc2007;
dev::TouchTsc2007 haspTouch;

#endif // ARDUINO

#endif // HASP_TSC2007_TOUCH_DRIVER_H