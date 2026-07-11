/* MIT License - Copyright (c) 2019-2026 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GT911_TOUCH_DRIVER_H
#define HASP_GT911_TOUCH_DRIVER_H

#ifdef ARDUINO
#include "lvgl.h"
#include "touch_driver.h"

namespace dev {

class TouchGt911 : public BaseTouch {
  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data);
    void init(int w, int h);
  private:
    // Function pointer type matching the LVGL read signature
    bool (*_read_strategy)(TouchGt911* instance, lv_indev_drv_t* indev_driver, lv_indev_data_t* data);

    // The two operational strategies
    static bool read_active(TouchGt911* instance, lv_indev_drv_t* indev_driver, lv_indev_data_t* data);
    static bool read_failed(TouchGt911* instance, lv_indev_drv_t* indev_driver, lv_indev_data_t* data);

    uint16_t xResolution = 0;
    uint16_t yResolution = 0;
    uint16_t tftWidth = 0;
    uint16_t tftHeight = 0;
};

} // namespace dev

using dev::TouchGt911;
extern dev::TouchGt911 haspTouch;

#endif // ARDUINO

#endif // HASP_GT911_TOUCH_DRIVER_H