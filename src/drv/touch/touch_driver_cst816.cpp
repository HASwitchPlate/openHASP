/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && (TOUCH_DRIVER == 0x816)
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"
#include "touch_driver_cst816.h"

#include <Wire.h>
#include "cst816t.h"

#include "touch_driver.h" // base class
#include "touch_helper.h" // i2c scanner

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

cst816t touchpad(Wire, TOUCH_RST, TOUCH_IRQ);

namespace dev {

IRAM_ATTR bool TouchCst816::read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    if(touchpad.available()) {

        if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle
        //LOG_INFO(TAG_DRVR, "CST816 touched x:%d, y:%d", touchpad.x, touchpad.y);

#ifdef TOUCH_WIDTH
        data->point.x = map(x, 0, TOUCH_WIDTH - 1, 0, TFT_WIDTH - 1);
#else
        data->point.x = touchpad.x;
#endif

#ifdef TOUCH_HEIGHT
        data->point.y = map(y, 0, TOUCH_HEIGHT - 1, 0, TFT_HEIGHT - 1);
#else
        data->point.y = touchpad.y;
#endif

        data->state = LV_INDEV_STATE_PR;
        hasp_set_sleep_offset(0); // Reset the offset

    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

void TouchCst816::init(int w, int h)
{
    Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
    if(touchpad.begin(mode_touch) == true) {
        LOG_INFO(TAG_DRVR, "CST816 %s (170X320)", D_SERVICE_STARTED);
    } else {
        LOG_WARNING(TAG_DRVR, "CST816 %s", D_SERVICE_START_FAILED);
    }

    Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
    touch_scan(Wire); // The address could change during begin, so scan afterwards
}

} // namespace dev

dev::TouchCst816 haspTouch;

#endif // ARDUINO
