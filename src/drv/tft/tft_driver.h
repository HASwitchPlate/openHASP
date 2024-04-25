/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_BASE_TFT_DRIVER_H
#define HASP_BASE_TFT_DRIVER_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "lvgl.h"
#include "tft_defines.h"

namespace dev {

enum lv_hasp_obj_type_t {
    TFT_PANEL_UNKNOWN = 0,
    TFT_PANEL_ILI9341,
    TFT_PANEL_ILI9342,
    TFT_PANEL_ILI9163,
    TFT_PANEL_ILI9486,
    TFT_PANEL_ILI9481,
    TFT_PANEL_ILI9488,
    TFT_PANEL_HX8357D,
    TFT_PANEL_ST7735,
    TFT_PANEL_ST7789,
    TFT_PANEL_ST7789B,
    TFT_PANEL_ST7796,
    TFT_PANEL_S6D02A1,
    TFT_PANEL_R61581,
    TFT_PANEL_R61529,
    TFT_PANEL_RM68140,
    TFT_PANEL_RGB,
    TFT_PANEL_EPD,
    TFT_PANEL_GC9A01,
    TFT_PANEL_LAST,
};

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
#elif defined(HASP_USE_ARDUINOGFX)
#warning Building for ESP32 ArduinoGfx
#include "tft_driver_arduinogfx.h"
#elif defined(ESP8266)
// #warning Building for ESP8266 Tfts
#include "tft_driver_tftespi.h"
#elif defined(STM32F4)
// #warning Building for STM32F4xx Tfts
#include "tft_driver_tftespi.h"
#elif defined(STM32F7)
#warning Building for STM32F7xx Tfts
#include "tft_driver_tftespi.h"
#elif USE_MONITOR && HASP_TARGET_PC
// #warning Building for SDL2
#include "tft_driver_sdl2.h"
#elif USE_WIN32DRV && HASP_TARGET_PC
// #warning Building for Win32Drv
#include "tft_driver_win32drv.h"
#elif USE_FBDEV && HASP_TARGET_PC
// #warning Building for POSIX fbdev
#include "tft_driver_posix_fbdev.h"
#else
// #warning Building for Generic Tfts
using dev::BaseTft;
extern dev::BaseTft haspTft;
#endif

#endif
