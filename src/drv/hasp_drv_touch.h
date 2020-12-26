/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DRV_TOUCH_H
#define HASP_DRV_TOUCH_H

#include "lvgl.h"

#ifndef TOUCH_DRIVER
    #define TOUCH_DRIVER -1 // No Touch
#endif

void drv_touch_init(uint8_t rotation);
bool drv_touch_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data);
void drv_touch_loop();

#endif