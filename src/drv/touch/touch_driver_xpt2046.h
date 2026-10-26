/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Standalone XPT2046 resistive-touch driver.
 *
 * openHASP's other XPT2046 support is welded to the TFT_eSPI / LovyanGFX
 * display drivers (they own the SPI touch read). RGB-parallel boards must use
 * ArduinoGFX for the display, which has no touch companion -- so those boards
 * fell through to the dead "Generic Touch" stub. This driver runs XPT2046 on
 * its OWN SPI bus so resistive touch works alongside an ArduinoGFX RGB panel
 * (e.g. Sunton ESP32-4827S043R). */

#ifndef HASP_XPT2046_TOUCH_DRIVER_H
#define HASP_XPT2046_TOUCH_DRIVER_H

#ifdef ARDUINO
#include "lvgl.h"
#include "touch_driver.h" // base class

namespace dev {

class TouchXpt2046 : public BaseTouch {
  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data);
    void init(int w, int h);
    void set_rotation(uint8_t rotation);
    const char* get_touch_model()
    {
        return "XPT2046";
    }
    bool is_driver_pin(uint8_t pin);

  private:
    uint8_t _rotation = 0;
    int _width        = 0;
    int _height       = 0;
};

} // namespace dev

using dev::TouchXpt2046;
extern dev::TouchXpt2046 haspTouch;

#endif // ARDUINO

#endif // HASP_XPT2046_TOUCH_DRIVER_H
