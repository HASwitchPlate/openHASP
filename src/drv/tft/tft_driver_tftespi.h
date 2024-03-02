/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TFTESPI_DRIVER_H
#define HASP_TFTESPI_DRIVER_H

#if defined(ARDUINO) && defined(USER_SETUP_LOADED)
#include <Arduino.h>

#include "lvgl.h"
#include "TFT_eSPI.h"

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

class TftEspi : BaseTft {

  public:
    TFT_eSPI tft;

    void init(int w, int h);
    void show_info();
    void splashscreen();

    void set_rotation(uint8_t rotation);
    void set_invert(bool invert);

    void flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    bool is_driver_pin(uint8_t pin);

    const char* get_tft_model();

    int32_t width(){
        return tft.width();
    }
    int32_t height(){
        return tft.height();
    }

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
            snprintf_P(buffer, sizeof(buffer), PSTR("%-12s: %s (GPIO %02d)"), String(pinfunction).c_str(),
                       haspDevice.gpio_name(pin).c_str(), pin);
            LOG_VERBOSE(TAG_TFT, buffer);
        }
    }
};

} // namespace dev

using dev::TftEspi;
extern dev::TftEspi haspTft;

#endif // ARDUINO

#endif // HASP_TFTESPI_DRIVER_H