/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if USE_FBDEV && HASP_TARGET_PC

#include "hasplib.h"
#include "lvgl.h"

#include "display/fbdev.h"

#include "drv/tft/tft_driver.h"
#include "tft_driver_posix_fbdev.h"

#if USE_EVDEV || USE_BSD_EVDEV
#include "indev/evdev.h"
#endif

#include "dev/device.h"
#include "hasp_debug.h"
#include "hasp_gui.h"

#ifdef HASP_CUSTOMIZE_BOOTLOGO
#include "custom/bootlogo.h" // Sketch tab header for xbm images
#else
#include "custom/bootlogo_template.h" // Sketch tab header for xbm images
#endif

#include <unistd.h>

extern uint16_t tft_width;
extern uint16_t tft_height;

namespace dev {

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static void* tick_thread(void* data)
{
    (void)data;

    while(1) {
        usleep(5000);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

int32_t TftFbdevDrv::width()
{
    return _width;
}
int32_t TftFbdevDrv::height()
{
    return _height;
}

static void* gui_entrypoint(void* arg)
{
#if HASP_USE_LVGL_TASK
#error "fbdev LVGL task is not implemented"
#else
    // create a LVGL tick thread
    pthread_t thread;
    pthread_create(&thread, 0, tick_thread, NULL);
#endif
    return 0;
}

void TftFbdevDrv::init(int32_t w, int h)
{
    /* Add a display
     * Use the 'fbdev' driver which uses POSIX framebuffer device as a display
     * The following input devices are handled: mouse, keyboard, mousewheel */
    fbdev_init();
    fbdev_get_sizes((uint32_t*)&_width, (uint32_t*)&_height);

    tft_width  = _width;
    tft_height = _height;

#if USE_EVDEV || USE_BSD_EVDEV
    evdev_register("/dev/input/event2", LV_INDEV_TYPE_POINTER, NULL);
#endif

#if HASP_USE_LVGL_TASK
#error "fbdev LVGL task is not implemented"
#else
    // do not use the gui_task(), just init the GUI and return
    gui_entrypoint(NULL);
#endif
}
void TftFbdevDrv::show_info()
{
    splashscreen();

    LOG_VERBOSE(TAG_TFT, F("Driver     : %s"), get_tft_model());
}

void TftFbdevDrv::splashscreen()
{
    uint8_t fg[]       = logoFgColor;
    uint8_t bg[]       = logoBgColor;
    lv_color_t fgColor = lv_color_make(fg[0], fg[1], fg[2]);
    lv_color_t bgColor = lv_color_make(bg[0], bg[1], bg[2]);
    // TODO show splashscreen
}
void TftFbdevDrv::set_rotation(uint8_t rotation)
{}
void TftFbdevDrv::set_invert(bool invert)
{}
void TftFbdevDrv::flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    lv_disp_flush_ready(disp);
}
bool TftFbdevDrv::is_driver_pin(uint8_t pin)
{
    return false;
}
const char* TftFbdevDrv::get_tft_model()
{
    return "POSIX fbdev";
}

} // namespace dev

dev::TftFbdevDrv haspTft;

#endif // WINDOWS || POSIX
