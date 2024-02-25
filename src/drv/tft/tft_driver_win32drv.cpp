/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if USE_WIN32DRV && HASP_TARGET_PC

#include "hasplib.h"
#include "lvgl.h"

#include "win32drv/win32drv.h"

#include "drv/tft/tft_driver.h"
#include "tft_driver_win32drv.h"

#include "dev/device.h"
#include "hasp_debug.h"
#include "hasp_gui.h"

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

static void win32_message_loop(lv_task_t* param)
{
    MSG Message;
#if HASP_USE_LVGL_TASK
    while(haspDevice.pc_is_running && GetMessageW(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessageW(&Message);
    }
    // apparently GetMessageW doesn't deliver WM_QUIT
    haspDevice.pc_is_running = false;
#else
    BOOL Result = PeekMessageW(&Message, NULL, 0, 0, TRUE);
    if(Result != 0 && Result != -1) {
        TranslateMessage(&Message);
        DispatchMessageW(&Message);
        if(Message.message == WM_QUIT) haspDevice.pc_is_running = false;
    }
#endif
}

static DWORD gui_entrypoint(HANDLE semaphore)
{
    /* Add a display
     * Use the 'win32drv' driver which creates window on PC's monitor to simulate a display
     * The following input devices are handled: mouse, keyboard, mousewheel */
    lv_win32_init(0, SW_SHOWNORMAL, haspTft.width(), haspTft.height(), 0);
    lv_win32_set_title(haspDevice.get_hostname());

#if HASP_USE_LVGL_TASK
    // let the init() function continue
    ReleaseSemaphore(semaphore, 1, NULL);
    // run the LVGL task as a thread
    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)gui_task, NULL, 0, NULL);
    // run a blocking message loop on this thread
    win32_message_loop(NULL);
    // wait for the LVGL task now
    WaitForSingleObject(thread, 4000);
#else
    // create a LVGL tick thread
    CreateThread(NULL, 0, tick_thread, NULL, 0, NULL);
    // create a LVGL task for the message loop
    lv_task_create(win32_message_loop, 5, LV_TASK_PRIO_HIGHEST, NULL);
#endif
    return 0;
}

void TftWin32Drv::init(int32_t w, int h)
{
    _width  = w;
    _height = h;

#if HASP_USE_LVGL_TASK
    // run a thread for creating the window and running the message loop
    HANDLE semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    HANDLE thread    = CreateThread(NULL, 0, gui_entrypoint, semaphore, 0, NULL);
    WaitForSingleObject(semaphore, INFINITE);
#else
    // do not use the gui_task(), just init the GUI and return
    gui_entrypoint(NULL);
#endif
}
void TftWin32Drv::show_info()
{
    splashscreen();

    LOG_VERBOSE(TAG_TFT, F("Driver     : %s"), get_tft_model());
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
