/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_BASE_TOUCH_DRIVER_H
#define HASP_BASE_TOUCH_DRIVER_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "hasplib.h"
#include "lvgl.h"

namespace dev {

class BaseTouch {
  public:
    void init(int w, int h)
    {}
    // void loop()
    // {}
    void show_info()
    {}
    void set_rotation(uint8_t rotation)
    {}
    void set_invert(bool invert_display)
    {}
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        data->state = LV_INDEV_STATE_REL;
        return false;
    }
    void calibrate(uint16_t* calData)
    {}
    void set_calibration(uint16_t* calData)
    {}
    bool is_driver_pin(uint8_t)
    {
        return false;
    }
    const char* get_touch_model()
    {
        return "";
    }
};

} // namespace dev

#ifndef TOUCH_DRIVER
#define TOUCH_DRIVER -1 // No Touch
#endif

#if TOUCH_DRIVER == 0x2046 && defined(USER_SETUP_LOADED)
#warning Building for TFT_eSPI XPT2046
//#include "touch_driver_xpt2046.h"
#include "touch_driver_tftespi.h"
#elif TOUCH_DRIVER == 0x2046 && defined(LGFX_USE_V1) && defined(HASP_USE_LGFX_TOUCH)
#warning Building for LovyanGFX XPT2046
#include "touch_driver_lovyangfx.h"
#elif TOUCH_DRIVER == 0x0911 && defined(LGFX_USE_V1) && defined(HASP_USE_LGFX_TOUCH)
#warning Building for LovyanGFX GT911
#include "touch_driver_lovyangfx.h"
#elif TOUCH_DRIVER == 0x6336 && defined(LGFX_USE_V1) && defined(HASP_USE_LGFX_TOUCH)
#warning Building for LovyanGFX FT6336
#include "touch_driver_lovyangfx.h"
#elif defined(LGFX_USE_V1) && defined(HASP_USE_LGFX_TOUCH)
#warning Building for LovyanGFX
#include "touch_driver_lovyangfx.h"
#elif TOUCH_DRIVER == 0x5206
#warning Building for FT5206
#include "touch_driver_ft5206.h"
#elif TOUCH_DRIVER == 0x6336
#warning Building for FT6336
#include "touch_driver_ft6336u.h"
#elif TOUCH_DRIVER == 0x0610
#warning Building for STMPE610
#include "touch_driver_stmpe610.h"
#elif TOUCH_DRIVER == 0x0911
#warning Building for GT911
#include "touch_driver_gt911.h"
#elif TOUCH_DRIVER == 0x0ADC
#warning Building for analog touch
#include "touch_driver_analog.h"
#elif TOUCH_DRIVER == 0x1680
#warning Building for GSL1680
#include "touch_driver_gslx680.h"
#elif TOUCH_DRIVER == 0x2007
#warning Building for TSC2007
#include "touch_driver_tsc2007.h"
#elif defined(LGFX_USE_V1)
#warning Building for LovyanGfx Touch
#include "touch_driver_lovyangfx.h"
#elif TOUCH_DRIVER == 0x3240
#warning Building for CST3240
#include "touch_driver_cst3240.h"
#else
#warning Building for Generic Touch
using dev::BaseTouch;
extern dev::BaseTouch haspTouch;
// IRAM_ATTR bool touch_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
// {
//     data->state = LV_INDEV_STATE_REL;
//     return false;
// }
#endif

#endif

// #elif TOUCH_DRIVER == 0x2046B
//     touched = XPT2046_getXY(&normal_x, &normal_y, true);
