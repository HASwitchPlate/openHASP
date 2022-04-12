/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
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

#ifndef TOUCH_CS
#define TOUCH_CS -1
#endif

#ifndef TOUCH_IRQ
#define TOUCH_IRQ -1
#endif

#ifndef TFT_MOSI
#define TFT_MOSI -1
#endif
#ifndef TFT_MISO
#define TFT_MISO -1
#endif
#ifndef TFT_SCLK
#define TFT_SCLK -1
#endif
#ifndef TFT_BUSY
#define TFT_BUSY -1
#endif

#ifndef TFT_D0
#define TFT_D0 -1
#endif
#ifndef TFT_D1
#define TFT_D1 -1
#endif
#ifndef TFT_D2
#define TFT_D2 -1
#endif
#ifndef TFT_D3
#define TFT_D3 -1
#endif
#ifndef TFT_D4
#define TFT_D4 -1
#endif
#ifndef TFT_D5
#define TFT_D5 -1
#endif
#ifndef TFT_D6
#define TFT_D6 -1
#endif
#ifndef TFT_D7
#define TFT_D7 -1
#endif
#ifndef TFT_D8
#define TFT_D8 -1
#endif
#ifndef TFT_D9
#define TFT_D9 -1
#endif
#ifndef TFT_D10
#define TFT_D10 -1
#endif
#ifndef TFT_D11
#define TFT_D11 -1
#endif
#ifndef TFT_D12
#define TFT_D12 -1
#endif
#ifndef TFT_D13
#define TFT_D13 -1
#endif
#ifndef TFT_D14
#define TFT_D14 -1
#endif
#ifndef TFT_D15
#define TFT_D15 -1
#endif
#ifndef TFT_RD
#define TFT_RD -1
#endif
#ifndef TFT_WR
#define TFT_WR -1
#endif
#ifndef TFT_DC
#define TFT_DC -1
#endif
#ifndef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 0
#endif
#ifndef TFT_OFFSET_ROTATION
#define TFT_OFFSET_ROTATION 0
#endif
#ifndef TOUCH_OFFSET_ROTATION
#define TOUCH_OFFSET_ROTATION 0
#endif
#ifndef I2C_TOUCH_PORT
#define I2C_TOUCH_PORT 0
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