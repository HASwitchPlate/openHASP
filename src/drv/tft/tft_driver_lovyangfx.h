/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_LOVYANGFX_DRIVER_H
#define HASP_LOVYANGFX_DRIVER_H

#if defined(ARDUINO) && defined(LGFX_USE_V1)
#include <Arduino.h>

#include "lvgl.h"
#include "LovyanGFX.hpp"

#include "tft_driver.h"
#include "hal/hasp_hal.h"
#include "dev/device.h"
#include "hasp_debug.h"

#ifdef HASP_CUSTOMIZE_BOOTLOGO
#include "custom/bootlogo.h" // Sketch tab header for xbm images
#else
#include "custom/bootlogo_template.h" // Sketch tab header for xbm images
#endif

namespace dev {
class LGFX : public lgfx::LGFX_Device {
  public:
    LGFX(void)
    {}
};

class LovyanGfx : BaseTft {

  public:
    LGFX tft;

    void init(int w, int h);
    void show_info();
    void splashscreen();

    void set_rotation(uint8_t rotation);
    void set_invert(bool invert);

    void flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    bool is_driver_pin(uint8_t pin);

    const char* get_tft_model();

    int32_t width()
    {
        return tft.width();
    }
    int32_t height()
    {
        return tft.height();
    }

  private:
    uint32_t tft_driver;

    uint32_t get_tft_driver();
    uint32_t get_touch_driver();

    lgfx::Panel_Device* _init_panel(lgfx::IBus* bus);

    void tftOffsetInfo(uint8_t pin, uint8_t x_offset, uint8_t y_offset)
    {
        if(x_offset != 0) {
            LOG_VERBOSE(TAG_TFT, F("R%u x offset = %i"), pin, x_offset);
        }
        if(y_offset != 0) {
            LOG_VERBOSE(TAG_TFT, F("R%u y offset = %i"), pin, y_offset);
        }
    }
};

} // namespace dev

using dev::LovyanGfx;
extern dev::LovyanGfx haspTft;

#endif // ARDUINO

#endif // HASP_LOVYANGFX_DRIVER_H