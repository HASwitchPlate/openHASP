/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_LOVYANGFX_TOUCH_DRIVER_H
#define HASP_LOVYANGFX_TOUCH_DRIVER_H

#if defined(ARDUINO) && defined(LGFX_USE_V1) && defined(HASP_USE_LGFX_TOUCH)
#include <Arduino.h>
#include <Wire.h>
#include "LovyanGFX.hpp"

#include "touch_driver.h" // base class
#include "touch_helper.h" // wire scan
#include "dev/device.h"   // for haspTft
#include "drv/tft/tft_driver.h"

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

namespace dev {

class TouchLovyanGfx : public BaseTouch {

  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        lgfx::v1::touch_point_t tp;

        if(haspTft.tft.getTouch(&tp, 1)) {
            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

            data->point.x = tp.x;
            data->point.y = tp.y;
            data->state   = LV_INDEV_STATE_PR;
            hasp_set_sleep_offset(0); // Reset the offset

            // LOG_DEBUG(TAG_DRVR, F("Touch: %d %d"), tp.x, tp.y);
            // HASP_SERIAL.print('#');
        } else {
            data->state = LV_INDEV_STATE_REL;
            // HASP_SERIAL.print('x');
        }

        /*Return `false` because we are not buffering and no more data to read*/
        return false;
    }

    void init(int w, int h)
    {
#if defined(TOUCH_SDA) && defined(TOUCH_SCL) && defined(I2C_TOUCH_FREQUENCY)
        // Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
        // touch_scan(Wire);
#endif
    }

    void set_calibration(uint16_t* calData)
    {
#if TOUCH_DRIVER == 0x2046
        if(haspTft.tft.panel() && haspTft.tft.width() && haspTft.tft.height()) {
            haspTft.tft.setTouchCalibrate(calData);
        }
#endif
    }

    void calibrate(uint16_t* calData)
    {
#if TOUCH_DRIVER == 0x2046
        if(haspTft.tft.panel() && haspTft.tft.width() && haspTft.tft.height()) {

            haspTft.tft.fillScreen(TFT_BLACK);
            // haspTft.tft.setCursor(20, 0);
            // haspTft.tft.setTextFont(1);
            // haspTft.tft.setTextSize(1);
            // haspTft.tft.setTextColor(TFT_WHITE, TFT_BLACK);

            // // tft.println(PSTR("Touch corners as indicated"));

            // haspTft.tft.setTextFont(1);
            delay(500);
            haspTft.tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
            set_calibration(calData);
            delay(500);
        }
#endif
    }
};

} // namespace dev

#warning Using Lovyan Touch
using dev::TouchLovyanGfx;
extern dev::TouchLovyanGfx haspTouch;

#endif // ARDUINO

#endif // HASP_LOVYANGFX_TOUCH_DRIVER_H