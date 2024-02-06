/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if USE_WIN32DRV && (defined(WINDOWS) || defined(POSIX))

#include "hasplib.h"
#include "lvgl.h"

#include "win32drv/win32drv.h"

#include "drv/tft/tft_driver.h"
#include "tft_driver_win32drv.h"

#include "dev/device.h"
#include "hasp_debug.h"

#ifdef HASP_CUSTOMIZE_BOOTLOGO
#include "custom/bootlogo.h" // Sketch tab header for xbm images
#else
#include "custom/bootlogo_template.h" // Sketch tab header for xbm images
#endif

namespace dev {

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static DWORD tick_thread(void* data)
{
    (void)data;

    while(1) {
        Sleep(5);       /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

int32_t TftWin32Drv::width()
{
    return _width;
}
int32_t TftWin32Drv::height()
{
    return _height;
}

void TftWin32Drv::init(int32_t w, int h)
{
    _width  = w;
    _height = h;

    /* Add a display
     * Use the 'win32drv' driver which creates window on PC's monitor to simulate a display
     * The following input devices are handled: mouse, keyboard, mousewheel */
    lv_win32_init(0, SW_SHOWNORMAL, w, h, 0);
    lv_win32_set_title(haspDevice.get_hostname());

    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
     * Create a Windows thread to do this*/
    CreateThread(NULL, 0, tick_thread, NULL, 0, NULL);
}
void TftWin32Drv::show_info()
{
    splashscreen();

    unsigned long version = GetVersion();
    unsigned long major   = LOBYTE(LOWORD(version));
    unsigned long minor   = HIBYTE(LOWORD(version));
    unsigned long build   = 0;
    if(version < 0x80000000) build = HIWORD(version);
    LOG_VERBOSE(TAG_TFT, F("Driver         : Win32Drv"));
    LOG_VERBOSE(TAG_TFT, F("Windows Version: %d.%d.%d"), major, minor, build);
}

void TftWin32Drv::splashscreen()
{
    uint8_t fg[]       = logoFgColor;
    uint8_t bg[]       = logoBgColor;
    lv_color_t fgColor = lv_color_make(fg[0], fg[1], fg[2]);
    lv_color_t bgColor = lv_color_make(bg[0], bg[1], bg[2]);
    lv_win32_splashscreen(logoImage, logoWidth, logoHeight, lv_color_to32(fgColor), lv_color_to32(bgColor));
}
void TftWin32Drv::set_rotation(uint8_t rotation)
{}
void TftWin32Drv::set_invert(bool invert)
{}
void TftWin32Drv::flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    lv_disp_flush_ready(disp);
}
bool TftWin32Drv::is_driver_pin(uint8_t pin)
{
    return false;
}
const char* TftWin32Drv::get_tft_model()
{
    return "Win32Drv";
}

} // namespace dev

dev::TftWin32Drv haspTft;

#endif // WINDOWS || POSIX
