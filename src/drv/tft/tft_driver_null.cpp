/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_NULL_DRIVER && HASP_TARGET_PC

#include "hasplib.h"
#include "lvgl.h"

#include "drv/tft/tft_driver.h"
#include "tft_driver_null.h"

#include "dev/device.h"
#include "hasp_debug.h"

#include <unistd.h>
#include <pthread.h>

namespace dev {

static void* tick_thread(void* data)
{
    (void)data;

    while(1) {
        usleep(5000);
        lv_tick_inc(5);
    }

    return 0;
}

int32_t TftNullDrv::width()
{
    return _width;
}
int32_t TftNullDrv::height()
{
    return _height;
}

void TftNullDrv::init(int32_t w, int h)
{
    _width  = w;
    _height = h;

    pthread_t tick_pthread;
    pthread_create(&tick_pthread, 0, tick_thread, NULL);

    LOG_VERBOSE(TAG_TFT, F("Null driver initialized (%dx%d)"), w, h);
}

void TftNullDrv::show_info()
{
    LOG_VERBOSE(TAG_TFT, F("Driver     : Null (headless)"));
}

void TftNullDrv::splashscreen()
{}

void TftNullDrv::set_rotation(uint8_t rotation)
{}

void TftNullDrv::set_invert(bool invert)
{}

void TftNullDrv::flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    lv_disp_flush_ready(disp);
}

bool TftNullDrv::is_driver_pin(uint8_t pin)
{
    return false;
}

const char* TftNullDrv::get_tft_model()
{
    return "Null (headless)";
}

} // namespace dev

dev::TftNullDrv haspTft;

#endif // HASP_USE_NULL_DRIVER && HASP_TARGET_PC
