  /* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_SDL2_DRIVER_H
#define HASP_SDL2_DRIVER_H

#if defined(WINDOWS) || defined(POSIX)

#include "lvgl.h"
#include "tft_driver.h"
#include "indev/mouse.h"

namespace dev {

class TftSdl2 : BaseTft {
  public:
    void init(int w, int h);
    void show_info();
    void splashscreen();
    void set_rotation(uint8_t rotation);
    void set_invert(bool invert);
    static void flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    bool is_driver_pin(uint8_t pin);
    const char* get_tft_model();
};

} // namespace dev

using dev::TftSdl2;
extern dev::TftSdl2 haspTft;

#endif // defined(WINDOWS) || defined(POSIX)

#endif // HASP_SDL2_DRIVER_H
