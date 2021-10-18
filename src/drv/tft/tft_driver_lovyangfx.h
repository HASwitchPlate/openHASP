/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_LOVYANGFX_DRIVER_H
#define HASP_LOVYANGFX_DRIVER_H

#if defined(ARDUINO) && defined(LGFX_USE_V1)
#include "Arduino.h"

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
    // lgfx::Panel_ILI9481 _panel_instance;
    lgfx::Panel_LCD* _panel_instance;
    lgfx::IBus* _bus_instance; // SPIバスのインスタンス
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

    LGFX(void)
    {
        _bus_instance = new lgfx::v1::Bus_SPI();
    }
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

  private:
    void tftOffsetInfo(uint8_t pin, uint8_t x_offset, uint8_t y_offset)
    {
        if(x_offset != 0) {
            LOG_VERBOSE(TAG_TFT, F("R%u x offset = %i"), pin, x_offset);
        }
        if(y_offset != 0) {
            LOG_VERBOSE(TAG_TFT, F("R%u y offset = %i"), pin, y_offset);
        }
    }

    void tftPinInfo(const __FlashStringHelper* pinfunction, int8_t pin)
    {
        if(pin != -1) {
            char buffer[64];
            snprintf_P(buffer, sizeof(buffer), PSTR("%-11s: %s (GPIO %02d)"), String(pinfunction).c_str(),
                       haspDevice.gpio_name(pin).c_str(), pin);
            LOG_VERBOSE(TAG_TFT, buffer);
        }
    }
};

} // namespace dev

using dev::LovyanGfx;
extern dev::LovyanGfx haspTft;

#endif // ARDUINO

#endif // HASP_LOVYANGFX_DRIVER_H