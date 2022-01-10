/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_BASE_TFT_DRIVER_H
#define HASP_BASE_TFT_DRIVER_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "lvgl.h"

namespace dev {

class BaseTft {
  public:
    virtual void init(int w, int h)
    {}
    virtual void show_info()
    {}
    virtual void set_rotation(uint8_t rotation)
    {}
    virtual void set_invert(bool invert_display)
    {}
    static void flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
    {}
    virtual bool is_driver_pin(uint8_t)
    {
        return false;
    }
    virtual const char* get_tft_model()
    {
        return "";
    }
};

} // namespace dev

#if defined(ESP32) && defined(USER_SETUP_LOADED)
// #warning Building for ESP32 TFT_eSPI
#include "tft_driver_tftespi.h"
#elif defined(ESP32) && defined(LGFX_USE_V1)
// #warning Building for ESP32 LovyanGfx
#include "tft_driver_lovyangfx.h"
#elif defined(ESP8266)
// #warning Building for ESP8266 Tfts
#include "tft_driver_tftespi.h"
#elif defined(STM32F4)
// #warning Building for STM32F4xx Tfts
#include "tft_driver_tftespi.h"
#elif defined(STM32F7)
#warning Building for STM32F7xx Tfts
#include "tft_driver_tftespi.h"
#elif defined(WINDOWS) || defined(POSIX)
// #warning Building for SDL2
#include "tft_driver_sdl2.h"
#else
// #warning Building for Generic Tfts
using dev::BaseTft;
extern dev::BaseTft haspTft;
#endif

#endif
