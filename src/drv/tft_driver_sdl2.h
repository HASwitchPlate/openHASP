/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_SDL2_DRIVER_H
#define HASP_SDL2_DRIVER_H

#include "lvgl.h"
#include <SDL2/SDL.h>

#include "display/monitor.h"
#include "indev/mouse.h"

#include "tft_driver.h"
#include "dev/device.h"
#include "hasp_debug.h"

//#include "bootscreen.h" // Sketch tab header for xbm images

namespace dev {

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void* data)
{
    (void)data;

    while(1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

class TftSdl2 : BaseTft {
  public:
    void init(int w, int h)
    {

// Workaround for sdl2 `-m32` crash
// https://bugs.launchpad.net/ubuntu/+source/libsdl2/+bug/1775067/comments/7
#ifndef WIN32
        setenv("DBUS_FATAL_WARNINGS", "0", 1);
#endif

        /* Add a display
         * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
        monitor_init();
        monitor_title(haspDevice.get_hostname());

        /* Add the mouse as input device
         * Use the 'mouse' driver which reads the PC's mouse*/
        mouse_init();

        /* Tick init.
         * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
         * Create an SDL thread to do this*/
        SDL_CreateThread(tick_thread, "tick", NULL);
    }
    void show_info()
    {
        SDL_version linked;
        SDL_GetVersion(&linked);
        LOG_VERBOSE(TAG_TFT, F("SDL2       : v%d.%d.%d"), linked.major, linked.minor, linked.patch);
        LOG_VERBOSE(TAG_TFT, F("Driver     : SDL2"));
    }

    void splashscreen()
    {
        // tft.fillScreen(TFT_DARKCYAN);
        // int x = (tft.width() - logoWidth) / 2;
        // int y = (tft.height() - logoHeight) / 2;
        // tft.drawXBitmap(x, y, bootscreen, logoWidth, logoHeight, TFT_WHITE);
    }
    void set_rotation(uint8_t rotation)
    {}
    void set_invert(bool invert)
    {}
    static void flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
    {
        monitor_flush(disp, area, color_p);
    }
};

} // namespace dev

using dev::TftSdl2;
extern dev::TftSdl2 haspTft;

#endif