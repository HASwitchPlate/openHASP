/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
 For full license information read the LICENSE file in the project folder */

#ifndef HASP_WIN32DRV_DRIVER_H
#define HASP_WIN32DRV_DRIVER_H

#include "tft_driver.h"

#if USE_WIN32DRV && HASP_TARGET_PC
// #warning Building H driver WIN32DRV

#include "lvgl.h"

namespace dev {

class TftWin32Drv : BaseTft {
  public:
    void init(int w, int h);
    void show_info();
    void splashscreen();

    void set_rotation(uint8_t rotation);
    void set_invert(bool invert);

    void flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    bool is_driver_pin(uint8_t pin);

    const char* get_tft_model();

    int32_t width();
    int32_t height();

  private:
    int32_t _width, _height;
};

} // namespace dev

using dev::TftWin32Drv;
extern dev::TftWin32Drv haspTft;

#endif // HASP_TARGET_PC

#endif // HASP_WIN32DRV_DRIVER_H
