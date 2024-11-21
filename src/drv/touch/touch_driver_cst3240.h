/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_CST3240_TOUCH_DRIVER_H
#define HASP_CST3240_TOUCH_DRIVER_H

#ifdef ARDUINO
#include "lvgl.h"
#include "touch_driver.h"

namespace dev {

class TouchCst3240 : public BaseTouch {
  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data);
    void init(int w, int h);
};

} // namespace dev

using dev::TouchCst3240;
extern dev::TouchCst3240 haspTouch;

#endif // ARDUINO

#endif // HASP_CST3240_TOUCH_DRIVER_H
