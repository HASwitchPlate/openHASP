/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
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
#include <dirent.h>
#include <fcntl.h>
#include <algorithm>
#include <fstream>
#include <linux/vt.h>

#if USE_BSD_EVDEV
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

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
    // create an LVGL GUI task thread
    pthread_t gui_pthread;
    pthread_create(&gui_pthread, 0, (void* (*)(void*))gui_task, NULL);
#endif
    // create an LVGL tick thread
    pthread_t tick_pthread;
    pthread_create(&tick_pthread, 0, tick_thread, NULL);
    return 0;
}

void TftFbdevDrv::init(int32_t w, int h)
{
    // try to switch the active tty to tty7
    int tty_fd = open("/dev/tty0", O_WRONLY);
    if(tty_fd == -1) {
        perror("Couldn't open /dev/tty0 (try running as root)");
    } else {
        if(ioctl(tty_fd, VT_ACTIVATE, 7) == -1) perror("Couldn't change active tty");
    }
    close(tty_fd);

    // check active tty
    std::ifstream f;
    f.open("/sys/class/tty/tty0/active");
    std::string tty;
    f >> tty;
    tty = "/dev/" + tty;
    f.close();

    // try to hide the cursor
    tty_fd = open(tty.c_str(), O_WRONLY);
    if(tty_fd == -1) {
        perror("Couldn't open active tty (try running as root)");
    } else {
        write(tty_fd, "\033[?25l", 6);
    }
    close(tty_fd);

    /* Add a display
     * Use the 'fbdev' driver which uses POSIX framebuffer device as a display
     * The following input devices are handled: mouse, keyboard, mousewheel */
    fbdev_init(fbdev_path.empty() ? NULL : fbdev_path.c_str());
    fbdev_get_sizes((uint32_t*)&_width, (uint32_t*)&_height);

    // show the splashscreen early
    splashscreen();

    tft_width  = _width;
    tft_height = _height;

#if USE_EVDEV || USE_BSD_EVDEV
    DIR* dir = opendir("/dev/input");
    if(dir == NULL) {
        perror("/dev/input opendir failed");
    } else {
        // iterate through /dev/input devices
        struct dirent* dirent;
        unsigned char ev_type[EV_MAX / 8 + 1];
        while((dirent = readdir(dir)) != NULL) {
            // make sure it's a block device matching /dev/input/event*
            if(strncmp(dirent->d_name, "event", 5) != 0 || strlen(dirent->d_name) <= 5) continue;
            if(dirent->d_type != DT_CHR) continue;
            // skip device if not specified on command line
            if(!evdev_names.empty() &&
               std::find(evdev_names.begin(), evdev_names.end(), std::string(dirent->d_name)) == evdev_names.end())
                continue;
            // get full path
            char dev_path[64];
            strcpy(dev_path, "/dev/input/");
            strcat(dev_path, dirent->d_name);
#if USE_BSD_EVDEV
            // open the device
            int fd = open(dev_path, O_RDONLY | O_NOCTTY);
#else
            int fd = open(dev_path, O_RDONLY | O_NOCTTY | O_NDELAY);
#endif
            if(fd == -1) {
                perror("input open failed");
                continue;
            }
            // read supported event types
            memset(ev_type, 0, sizeof(ev_type));
            if(ioctl(fd, EVIOCGBIT(0, sizeof(ev_type)), ev_type) < 0) {
                perror("ioctl failed");
                close(fd);
                continue;
            }
            // read device name
            char dev_name[256];
            if(ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name) < 0) {
                perror("ioctl failed");
                close(fd);
                continue;
            }
            // check which types are supported; judge LVGL device type
            lv_indev_type_t dev_type;
            const char* dev_type_name;
            if(ev_type[EV_REL / 8] & (1 << (EV_REL % 8))) {
                dev_type      = LV_INDEV_TYPE_POINTER;
                dev_type_name = "EV_REL";
            } else if(ev_type[EV_ABS / 8] & (1 << (EV_ABS % 8))) {
                dev_type      = LV_INDEV_TYPE_POINTER;
                dev_type_name = "EV_ABS";
            } else if(ev_type[EV_KEY / 8] & (1 << (EV_KEY % 8))) {
                dev_type      = LV_INDEV_TYPE_KEYPAD;
                dev_type_name = "EV_KEY";
            } else {
                close(fd);
                continue;
            }
            // register the device
            switch(dev_type) {
                case LV_INDEV_TYPE_POINTER:
                    LOG_VERBOSE(TAG_TFT, F("Pointer    : %s %s (%s)"), dev_path, dev_type_name, dev_name);
                    break;
                case LV_INDEV_TYPE_KEYPAD:
                    LOG_VERBOSE(TAG_TFT, F("Keypad     : %s %s (%s)"), dev_path, dev_type_name, dev_name);
                    break;
                default:
                    LOG_VERBOSE(TAG_TFT, F("Input      : %s %s (%s)"), dev_path, dev_type_name, dev_name);
                    break;
            }
            close(fd);
            // print verbose resolution info
            lv_indev_t* indev;
            if(!evdev_register(dev_path, dev_type, &indev) || indev == NULL) {
                printf("Failed to register evdev\n");
                continue;
            }
            evdev_data_t* user_data = (evdev_data_t*)indev->driver.user_data;
            LOG_VERBOSE(TAG_TFT, F("Resolution : X=%d (%d..%d), Y=%d (%d..%d)"), user_data->x_max,
                        user_data->x_absinfo.minimum, user_data->x_absinfo.maximum, user_data->y_max,
                        user_data->y_absinfo.minimum, user_data->y_absinfo.maximum);
        }
        closedir(dir);
    }
#endif

    gui_entrypoint(NULL);
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
    fbdev_splashscreen(logoImage, logoWidth, logoHeight, fgColor, bgColor);
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
