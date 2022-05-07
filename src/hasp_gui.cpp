/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#include "lv_drv_conf.h"

// Filesystem Driver
#include "lv_misc/lv_fs.h"
#include "lv_fs_if.h"

// Device Drivers
#include "dev/device.h"
#include "drv/tft/tft_driver.h"
#include "drv/touch/touch_driver.h"

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp_oobe.h"

//#include "tpcal.h"

//#include "Ticker.h"

#define BACKLIGHT_CHANNEL 0 // pwm channel 0-15

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
File pFileOut;
#endif

#define LVGL_TICK_PERIOD 20

#ifndef TFT_BCKL
#define TFT_BCKL -1 // No Backlight Control
#endif
#ifndef TFT_ROTATION
#define TFT_ROTATION 0
#endif
#ifndef INVERT_COLORS
#define INVERT_COLORS 0
#endif

// HASP_ATTRIBUTE_FAST_MEM static void  lv_tick_handler(void);

gui_conf_t gui_settings = {.show_pointer   = false,
                           .backlight_pin  = TFT_BCKL,
                           .rotation       = TFT_ROTATION,
                           .invert_display = INVERT_COLORS,
                           .cal_data       = {0, 65535, 0, 65535, 0}};
lv_obj_t* cursor;

uint16_t tft_width  = TFT_WIDTH;
uint16_t tft_height = TFT_HEIGHT;

bool screenshotIsDirty = true;

static lv_disp_buf_t disp_buf;

// #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
// static Ticker tick; /* timer for interrupt handler */
// #else
// static Ticker tick(lv_tick_handler, LVGL_TICK_PERIOD); // guiTickPeriod);
// #endif

/* **************************** GUI TICKER ************************************** */

/* Interrupt driven periodic handler */
// static void ICACHE_RAM_ATTR lv_tick_handler(void)
// {
//     lv_tick_inc(LVGL_TICK_PERIOD);
// }

static inline void gui_init_lvgl()
{
    LOG_VERBOSE(TAG_LVGL, F("Version    : %u.%u.%u %s"), LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH,
                PSTR(LVGL_VERSION_INFO));
    lv_init();

#if LV_USE_LOG != 0
    // Register logger to capture lvgl_init output
    lv_log_register_print_cb(debugLvglLogEvent);
#endif

    /* Create the Virtual Device Buffers */
    // #if defined(ARDUINO_ARCH_ESP32)
    //
    // #ifdef USE_DMA_TO_TFT
    //     // DMA: len must be less than 32767
    //     const size_t guiVDBsize = 15 * 1024u; // 30 KBytes
    //     guiVdbBuffer1           = (lv_color_t*)heap_caps_calloc(guiVDBsize, sizeof(lv_color_t), MALLOC_CAP_DMA);
    //     // guiVdbBuffer2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * guiVDBsize,   MALLOC_CAP_DMA);
    //     // lv_disp_buf_init(&disp_buf, guiVdbBuffer1, guiVdbBuffer2, guiVDBsize);
    // #else
    //     const size_t guiVDBsize = 8 * 1024u; // 32 KBytes

    //     if(0 && psramFound()) {
    //         guiVdbBuffer1 = (lv_color_t*)ps_calloc(guiVDBsize, sizeof(lv_color_t)); // too slow for VDB
    //     } else {
    //         guiVdbBuffer1 = (lv_color_t*)calloc(guiVDBsize, sizeof(lv_color_t));
    //     }

    // #endif

    //     // static lv_color_t * guiVdbBuffer2 = (lv_color_t *)malloc(sizeof(lv_color_t) * guiVDBsize);
    //     // lv_disp_buf_init(&disp_buf, guiVdbBuffer1, guiVdbBuffer2, guiVDBsize);

    // #elif defined(ARDUINO_ARCH_ESP8266)
    //     /* allocate on heap */
    //     // static lv_color_t guiVdbBuffer1[2 * 512u]; // 4 KBytes
    //     // size_t guiVDBsize = sizeof(guiVdbBuffer1) / sizeof(guiVdbBuffer1[0]);
    //     // lv_disp_buf_init(&disp_buf, guiVdbBuffer1, NULL, guiVDBsize);

    //     const size_t guiVDBsize = 2 * 512u; // 4 KBytes * 2
    //     guiVdbBuffer1           = (lv_color_t*)malloc(sizeof(lv_color_t) * guiVDBsize);

    // #elif defined(WINDOWS) || defined(POSIX)
    //     const size_t guiVDBsize = LV_HOR_RES_MAX * 10;
    //     //  static lv_color_t guiVdbBuffer1[guiVDBsize]; /*Declare a buffer for 10 lines*/
    //     guiVdbBuffer1 = (lv_color_t*)calloc(guiVDBsize, sizeof(lv_color_t));

    // #else
    //     static lv_color_t guiVdbBuffer1[16 * 512u]; // 16 KBytes
    //     // static lv_color_t guiVdbBuffer2[16 * 512u]; // 16 KBytes
    //     size_t guiVDBsize = sizeof(guiVdbBuffer1) / sizeof(guiVdbBuffer1[0]);
    //     // lv_disp_buf_init(&disp_buf, guiVdbBuffer1, guiVdbBuffer2, guiVDBsize);
    // #endif

    /* Dynamic VDB allocation */
    const size_t guiVDBsize          = LV_VDB_SIZE / sizeof(lv_color_t);
    static lv_color_t* guiVdbBuffer1 = (lv_color_t*)malloc(sizeof(lv_color_t) * guiVDBsize);

    /* Static VDB allocation */
    // static lv_color_t guiVdbBuffer1[LV_VDB_SIZE * 512u];
    // const size_t guiVDBsize = sizeof(guiVdbBuffer1) / sizeof(lv_color_t);

    /* Initialize VDB */
    if(guiVdbBuffer1 && guiVDBsize > 0) {
        lv_disp_buf_init(&disp_buf, guiVdbBuffer1, NULL, guiVDBsize);
    } else {
        LOG_FATAL(TAG_GUI, F(D_ERROR_OUT_OF_MEMORY));
    }

#ifdef LV_MEM_SIZE
    LOG_VERBOSE(TAG_LVGL, F("MEM size   : %d"), LV_MEM_SIZE);
#endif
    LOG_VERBOSE(TAG_LVGL, F("VFB size   : %d"), (size_t)sizeof(lv_color_t) * guiVDBsize);
}

void gui_hide_pointer(bool hidden)
{
    if(cursor) lv_obj_set_hidden(cursor, hidden || !gui_settings.show_pointer);
}

IRAM_ATTR void gui_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    haspTft.flush_pixels(disp, area, color_p);
    screenshotIsDirty = true;
}

IRAM_ATTR bool gui_touch_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    return haspTouch.read(indev_driver, data);
}

void guiCalibrate(void)
{
#if TOUCH_DRIVER == 0x2046 && defined(USER_SETUP_LOADED)
#ifdef TOUCH_CS
    haspTouch.calibrate(gui_settings.cal_data);
#endif

    size_t len = sizeof(gui_settings.cal_data) / sizeof(gui_settings.cal_data[0]);
    for(int i = 0; i < len; i++) {
        Serial.print(gui_settings.cal_data[i]);
        if(i < len - 1) Serial.print(", ");
    }

    delay(500);
    lv_obj_invalidate(lv_disp_get_layer_sys(NULL));
#endif
}

// fast init
void gui_start_tft(void)
{
    /* Setup Backlight Control Pin */
    haspDevice.set_backlight_pin(gui_settings.backlight_pin);

    haspTft.init(tft_width, tft_height);
    haspTft.set_rotation(gui_settings.rotation);
    haspTft.set_invert(gui_settings.invert_display);
}

static inline void gui_init_tft(void)
{
    // Initialize TFT
    LOG_TRACE(TAG_TFT, F(D_SERVICE_STARTING));
    gui_start_tft();
    haspTft.show_info();

#ifdef USE_DMA_TO_TFT
    LOG_VERBOSE(TAG_TFT, F("DMA        : " D_SETTING_ENABLED));
#else
    LOG_VERBOSE(TAG_TFT, F("DMA        : " D_SETTING_DISABLED));
#endif
    LOG_INFO(TAG_TFT, F(D_SERVICE_STARTED));
}

// initialize the image decoders
static inline void gui_init_images()
{
#if HASP_USE_PNGDECODE > 0
    lv_png_init(); // Initialize PNG decoder
#endif

#if HASP_USE_BMPDECODE > 0
    lv_bmp_init(); // Initialize BMP decoder
#endif

#if HASP_USE_GIFDECODE > 0
    lv_gif_init(); // Initialize GIF decoder
#endif

#if HASP_USE_JPGDECODE > 0
    lv_split_jpeg_init(); // Initialize JPG decoder
#endif

#if defined(ARDUINO_ARCH_ESP32)
    if(hasp_use_psram()) lv_img_cache_set_size(LV_IMG_CACHE_DEF_SIZE_PSRAM);
#endif
}

static inline void gui_init_filesystems()
{
#if LV_USE_FS_IF != 0
    //_lv_fs_init(); // lvgl File System -- not needed, it done in lv_init() when LV_USE_FILESYSTEM is set
    LOG_VERBOSE(TAG_LVGL, F("Filesystem : " D_SETTING_ENABLED));
    lv_fs_if_init(); // auxilary file system drivers
    // filesystem_list_path("L:/");

    lv_fs_file_t f;
    lv_fs_res_t res;
    res = lv_fs_open(&f, "L:/config.json", LV_FS_MODE_RD);
    if(res == LV_FS_RES_OK) {
        LOG_VERBOSE(TAG_HASP, F("TEST Opening config.json OK"));
        lv_fs_close(&f);
    } else {
        LOG_ERROR(TAG_HASP, F("TEST Opening config.json from FS failed %d"), res);
    }

#else
    LOG_VERBOSE(TAG_LVGL, F("Filesystem : " D_SETTING_DISABLED));
#endif
}

void guiSetup()
{
    // Initialize hardware drivers
    gui_init_tft();
    haspDevice.show_info(); // debug info + preload app flash size

    // Initialize LVGL
    LOG_TRACE(TAG_LVGL, F(D_SERVICE_STARTING));
    gui_init_lvgl();
    gui_init_images();
    gui_init_filesystems();
    font_setup();

    /* Initialize the LVGL display driver with correct orientation */
#if(TOUCH_DRIVER == 0x2046) // || defined(LGFX_USE_V1) // Use native display driver to rotate display and touch
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.flush_cb = gui_flush_cb;

    if(gui_settings.rotation % 2) {
        disp_drv.hor_res = tft_height;
        disp_drv.ver_res = tft_width;
    } else {
        disp_drv.hor_res = tft_width;
        disp_drv.ver_res = tft_height;
    }

    lv_disp_t* display = lv_disp_drv_register(&disp_drv);
    lv_disp_set_rotation(display, LV_DISP_ROT_NONE);

    /*
    #elif defined(LANBONL8) // Screen is 0 deg. rotated
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.buffer   = &disp_buf;
        disp_drv.flush_cb = gui_flush_cb;

        disp_drv.hor_res = tft_width;
        disp_drv.ver_res = tft_height;

        lv_disp_rot_t rotation[] = {LV_DISP_ROT_NONE, LV_DISP_ROT_270, LV_DISP_ROT_180, LV_DISP_ROT_90};
        lv_disp_t* display       = lv_disp_drv_register(&disp_drv);
        lv_disp_set_rotation(display, rotation[(4 + gui_settings.rotation - TFT_ROTATION) % 4]);
    */

#elif defined(M5STACK) // Screen is 90 deg. rotated
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.flush_cb = gui_flush_cb;

    disp_drv.hor_res = tft_height;
    disp_drv.ver_res = tft_width;

    lv_disp_rot_t rotation[] = {LV_DISP_ROT_NONE, LV_DISP_ROT_270, LV_DISP_ROT_180, LV_DISP_ROT_90};
    lv_disp_t* display       = lv_disp_drv_register(&disp_drv);
    lv_disp_set_rotation(display, rotation[(4 + gui_settings.rotation - TFT_ROTATION) % 4]);

#else // Use lvgl transformations
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.flush_cb = gui_flush_cb;

    disp_drv.hor_res = tft_width;
    disp_drv.ver_res = tft_height;

    lv_disp_rot_t rotation[] = {LV_DISP_ROT_NONE, LV_DISP_ROT_270, LV_DISP_ROT_180, LV_DISP_ROT_90};
    lv_disp_t* display       = lv_disp_drv_register(&disp_drv);
    lv_disp_set_rotation(display, rotation[(4 + gui_settings.rotation - TFT_ROTATION) % 4]);

#endif

    /* Initialize the touch pad */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
#if defined(WINDOWS) || defined(POSIX)
    indev_drv.read_cb = mouse_read;
#else
    indev_drv.read_cb = gui_touch_read;
#endif
    lv_indev_t* mouse_indev  = lv_indev_drv_register(&indev_drv);
    mouse_indev->driver.type = LV_INDEV_TYPE_POINTER;

    /*Set a cursor for the mouse*/
    LOG_TRACE(TAG_GUI, F("Initialize Cursor"));
    lv_obj_t* mouse_layer = lv_disp_get_layer_sys(NULL); // default display

#if defined(ARDUINO_ARCH_ESP32)
    LV_IMG_DECLARE(mouse_cursor_icon);          /*Declare the image file.*/
    cursor = lv_img_create(mouse_layer, NULL);  /*Create an image object for the cursor */
    lv_img_set_src(cursor, &mouse_cursor_icon); /*Set the image source*/
#else
    cursor            = lv_obj_create(mouse_layer, NULL); // show cursor object on every page
    lv_obj_set_size(cursor, 9, 9);
    lv_obj_set_style_local_radius(cursor, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_bg_color(cursor, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_bg_opa(cursor, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
#endif
    gui_hide_pointer(false);
    lv_indev_set_cursor(mouse_indev, cursor); /*Connect the image  object to the driver*/

#if !(defined(WINDOWS) || defined(POSIX))
    // drv_touch_init(gui_settings.rotation); // Touch driver
    haspTouch.init(tft_width, tft_height);
    haspTouch.set_rotation(gui_settings.rotation);
#endif

    /* Initialize Global progress bar*/
    lv_obj_user_data_t udata = (lv_obj_user_data_t){10, 0, 10};
    lv_obj_t* bar            = lv_bar_create(lv_layer_sys(), NULL);
    lv_obj_set_user_data(bar, udata);
    lv_obj_set_hidden(bar, true);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 10, LV_ANIM_OFF);
    lv_obj_set_size(bar, 200, 15);
    lv_obj_align(bar, lv_layer_sys(), LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_local_value_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_value_align(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_value_font(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_FONT_DEFAULT);
    lv_obj_set_style_local_bg_color(lv_layer_sys(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(lv_layer_sys(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

    // guiStart(); // Ticker
    LOG_INFO(TAG_LVGL, F(D_SERVICE_STARTED));
}

IRAM_ATTR void guiLoop(void)
{
    lv_task_handler(); // process animations

#if defined(STM32F4xx)
    //  tick.update();
#endif

#if !(defined(WINDOWS) || defined(POSIX))
    // haspTouch.loop();
#endif
}

void guiEverySecond(void)
{
    // nothing
}

void guiStart()
{
    /*Initialize the graphics library's tick*/
    // #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    //     tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
    // #else
    //     tick.start();
    // #endif
}

void guiStop()
{
    /*Deinitialize the graphics library's tick*/
    // #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    //     tick.detach();
    // #else
    //     tick.stop();
    // #endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
bool guiGetConfig(const JsonObject& settings)
{
    bool changed          = false;
    bool backlight_invert = haspDevice.get_backlight_invert();
    uint16_t guiSleepTime1;
    uint16_t guiSleepTime2;
    hasp_get_sleep_time(guiSleepTime1, guiSleepTime2);

    // if(guiTickPeriod != settings[FPSTR(FP_GUI_TICKPERIOD)].as<uint8_t>()) changed = true;
    // settings[FPSTR(FP_GUI_TICKPERIOD)] = guiTickPeriod;

    if(guiSleepTime1 != settings[FPSTR(FP_GUI_IDLEPERIOD1)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_GUI_IDLEPERIOD1)] = guiSleepTime1;

    if(guiSleepTime2 != settings[FPSTR(FP_GUI_IDLEPERIOD2)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_GUI_IDLEPERIOD2)] = guiSleepTime2;

    if(gui_settings.backlight_pin != settings[FPSTR(FP_GUI_BACKLIGHTPIN)].as<int8_t>()) changed = true;
    settings[FPSTR(FP_GUI_BACKLIGHTPIN)] = gui_settings.backlight_pin;

    if(backlight_invert != settings[FPSTR(FP_GUI_BACKLIGHTINVERT)].as<bool>()) changed = true;
    settings[FPSTR(FP_GUI_BACKLIGHTINVERT)] = (uint8_t)backlight_invert;

    if(gui_settings.rotation != settings[FPSTR(FP_GUI_ROTATION)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_GUI_ROTATION)] = gui_settings.rotation;

    if(gui_settings.show_pointer != settings[FPSTR(FP_GUI_POINTER)].as<bool>()) changed = true;
    settings[FPSTR(FP_GUI_POINTER)] = (uint8_t)gui_settings.show_pointer;

    if(gui_settings.invert_display != settings[FPSTR(FP_GUI_INVERT)].as<bool>()) changed = true;
    settings[FPSTR(FP_GUI_INVERT)] = (uint8_t)gui_settings.invert_display;

    /* Check CalData array has changed */
    JsonArray array = settings[FPSTR(FP_GUI_CALIBRATION)].as<JsonArray>();
    uint8_t i       = 0;
    size_t len      = sizeof(gui_settings.cal_data) / sizeof(gui_settings.cal_data[0]);
    for(JsonVariant v : array) {
        LOG_VERBOSE(TAG_GUI, F("GUI CONF: %d: %d <=> %d"), i, gui_settings.cal_data[i], v.as<uint16_t>());
        if(i < len) {
            if(gui_settings.cal_data[i] != v.as<uint16_t>()) changed = true;
            v.set(gui_settings.cal_data[i]);
        } else {
            changed = true;

#if TOUCH_DRIVER == 0x2046 && defined(USER_SETUP_LOADED) && defined(TOUCH_CS)
            // tft_espi_set_touch(gui_settings.cal_data);
            haspTft.tft.setTouch(gui_settings.cal_data);
#endif
        }
        i++;
    }

    /* Build new CalData array if the count is not correct */
    if(i != len) {
        array = settings[FPSTR(FP_GUI_CALIBRATION)].to<JsonArray>(); // Clear JsonArray
        for(int i = 0; i < len; i++) {
            array.add(gui_settings.cal_data[i]);
        }
        changed = true;

#if TOUCH_DRIVER == 0x2046 && defined(USER_SETUP_LOADED) && defined(TOUCH_CS)
        // tft_espi_set_touch(gui_settings.cal_data);
        haspTft.tft.setTouch(gui_settings.cal_data);
#endif
    }

    if(changed) configOutput(settings, TAG_GUI);
    return changed;
}

/** Set GUI Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool guiSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_GUI);
    bool changed             = false;
    uint8_t backlight_invert = haspDevice.get_backlight_invert();
    uint16_t guiSleepTime1;
    uint16_t guiSleepTime2;

    hasp_get_sleep_time(guiSleepTime1, guiSleepTime2);

    // changed |= configSet(guiTickPeriod, settings[FPSTR(FP_GUI_TICKPERIOD)], F("guiTickPeriod"));
    changed |= configSet(gui_settings.backlight_pin, settings[FPSTR(FP_GUI_BACKLIGHTPIN)], F("guiBacklightPin"));
    changed |= configSet(backlight_invert, settings[FPSTR(FP_GUI_BACKLIGHTINVERT)], F("guiBacklightInvert"));
    changed |= configSet(guiSleepTime1, settings[FPSTR(FP_GUI_IDLEPERIOD1)], F("guiSleepTime1"));
    changed |= configSet(guiSleepTime2, settings[FPSTR(FP_GUI_IDLEPERIOD2)], F("guiSleepTime2"));
    changed |= configSet(gui_settings.rotation, settings[FPSTR(FP_GUI_ROTATION)], F("gui_settings.rotation"));
    changed |= configSet(gui_settings.invert_display, settings[FPSTR(FP_GUI_INVERT)], F("guiInvertDisplay"));

    hasp_set_sleep_time(guiSleepTime1, guiSleepTime2);
    haspDevice.set_backlight_invert(backlight_invert); // Update if changed

    if(!settings[FPSTR(FP_GUI_POINTER)].isNull()) {
        if(gui_settings.show_pointer != settings[FPSTR(FP_GUI_POINTER)].as<bool>()) {
            LOG_VERBOSE(TAG_GUI, F("guiShowPointer set"));
        }
        changed |= gui_settings.show_pointer != settings[FPSTR(FP_GUI_POINTER)].as<bool>();

        gui_settings.show_pointer = settings[FPSTR(FP_GUI_POINTER)].as<bool>();
        gui_hide_pointer(false);
    }

    if(!settings[FPSTR(FP_GUI_CALIBRATION)].isNull()) {
        bool status = false;
        int i       = 0;
        size_t len  = sizeof(gui_settings.cal_data) / sizeof(gui_settings.cal_data[0]);

        JsonArray array = settings[FPSTR(FP_GUI_CALIBRATION)].as<JsonArray>();
        for(JsonVariant v : array) {
            if(i < len) {
                if(gui_settings.cal_data[i] != v.as<uint16_t>()) status = true;
                gui_settings.cal_data[i] = v.as<uint16_t>();
            }
            i++;
        }

        if(gui_settings.cal_data[0] != 0 || gui_settings.cal_data[1] != 65535 || gui_settings.cal_data[2] != 0 ||
           gui_settings.cal_data[3] != 65535) {
            LOG_VERBOSE(TAG_GUI, F("calData set [%u, %u, %u, %u, %u]"), gui_settings.cal_data[0],
                        gui_settings.cal_data[1], gui_settings.cal_data[2], gui_settings.cal_data[3],
                        gui_settings.cal_data[4]);
            oobeSetAutoCalibrate(false);
        } else {
            LOG_TRACE(TAG_GUI, F("First Touch Calibration enabled"));
            oobeSetAutoCalibrate(true);
        }

#if TOUCH_DRIVER == 0x2046 && defined(USER_SETUP_LOADED) && defined(TOUCH_CS)
        if(status) // tft_espi_set_touch(gui_settings.cal_data);
            haspTft.tft.setTouch(gui_settings.cal_data);
#endif
        changed |= status;
    }

    return changed;
}
#endif // HASP_USE_CONFIG

/* **************************** SCREENSHOTS ************************************** */
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0 || HASP_USE_HTTP > 0

static void guiSetBmpHeader(uint8_t* buffer_p, int32_t data)
{
    *buffer_p++ = data & 0xFF;
    *buffer_p++ = (data >> 8) & 0xFF;
    *buffer_p++ = (data >> 16) & 0xFF;
    *buffer_p++ = (data >> 24) & 0xFF;
}

/** Send Bitmap Header.
 *
 * Sends a header in BMP format for the size of the screen.
 *
 * @note: send header before refreshing the whole screen
 *
 **/
static void gui_get_bitmap_header(uint8_t* buffer, size_t bufsize)
{
    lv_obj_t* scr     = lv_disp_get_scr_act(NULL);
    lv_coord_t width  = lv_obj_get_width(scr);
    lv_coord_t height = lv_obj_get_height(scr);

    const char* bm = "BM";
    memcpy(buffer, bm, strlen(bm));
    buffer += strlen(bm);

    bmp_header_t* bmp = (bmp_header_t*)buffer;
    bmp->bfSize       = (uint32_t)(width * height * LV_COLOR_DEPTH / 8);
    bmp->bfReserved   = 0;
    bmp->bfOffBits    = 66;

    bmp->biSize          = 40;
    bmp->biWidth         = width;
    bmp->biHeight        = -height;
    bmp->biPlanes        = 1;
    bmp->biBitCount      = LV_COLOR_DEPTH;
    bmp->biCompression   = 3; // BI_BITFIELDS
    bmp->biSizeImage     = bmp->bfSize;
    bmp->biXPelsPerMeter = 2836;
    bmp->biYPelsPerMeter = 2836;
    bmp->biClrUsed       = 0;
    bmp->biClrImportant  = 0;

    bmp->bdMask[0] = 0xF800; // Red bitmask  : 1111 1000 | 0000 0000
    bmp->bdMask[1] = 0x07E0; // Green bitmask: 0000 0111 | 1110 0000
    bmp->bdMask[2] = 0x001F; // Blue bitmask : 0000 0000 | 0001 1111
}

void gui_flush_not_complete()
{
    LOG_WARNING(TAG_GUI, F("Pixelbuffer not completely sent"));
}
#endif // HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0 || HASP_USE_HTTP > 0

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
/* Flush VDB bytes to a file */
static void gui_screenshot_to_file(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    size_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1); /* Number of pixels */
    len *= sizeof(lv_color_t);                                          /* Number of bytes */
    size_t res = pFileOut.write((uint8_t*)color_p, len);
    if(res != len) gui_flush_not_complete();

    // indirect callback to flush screenshot data to the screen
    // drv_display_flush_cb(disp, area, color_p);
    haspTft.flush_pixels(disp, area, color_p);
}

/** Take Screenshot.
 *
 * Flush buffer into a binary file.
 *
 * @note: data pixel should be formated to uint16_t RGB. Set by Bitmap header.
 *
 * @param[in] pFileName   Output binary file name.
 *
 **/
void guiTakeScreenshot(const char* pFileName)
{
    uint8_t buffer[128];
    gui_get_bitmap_header(buffer, sizeof(buffer));

    pFileOut = HASP_FS.open(pFileName, "w");
    if(pFileOut) {

        size_t len = pFileOut.write(buffer, 66);
        if(len == 66) {
            LOG_VERBOSE(TAG_GUI, F("Bitmap header written"));

            /* Refresh screen to screenshot callback */
            lv_disp_t* disp = lv_disp_get_default();
            void (*flush_cb)(struct _disp_drv_t * disp_drv, const lv_area_t* area, lv_color_t* color_p);
            flush_cb              = disp->driver.flush_cb; /* store callback */
            disp->driver.flush_cb = gui_screenshot_to_file;

            lv_obj_invalidate(lv_scr_act());
            lv_refr_now(NULL);                /* Will call our disp_drv.disp_flush function */
            disp->driver.flush_cb = flush_cb; /* restore callback */

            LOG_VERBOSE(TAG_GUI, F("Bitmap data flushed to %s"), pFileName);

        } else {
            LOG_ERROR(TAG_GUI, F("Data written does not match header size"));
        }
        pFileOut.close();

    } else {
        LOG_WARNING(TAG_GUI, F(D_FILE_SAVE_FAILED), pFileName);
    }
}
#endif

#if HASP_USE_HTTP > 0
/* Flush VDB bytes to a webclient */
static void gui_screenshot_to_http(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    size_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1); /* Number of pixels */
    len *= sizeof(lv_color_t);                                          /* Number of bytes */
    size_t res = httpClientWrite((uint8_t*)color_p, len);
    if(res != len) gui_flush_not_complete();

    // indirect callback to flush screenshot data to the screen
    // drv_display_flush_cb(disp, area, color_p);
    haspTft.flush_pixels(disp, area, color_p);
}

/** Take Screenshot.
 *
 * Flush buffer into a http client.
 *
 * @note: data pixel should be formated to uint16_t RGB. Set by Bitmap header.
 *
 **/
void guiTakeScreenshot()
{
    uint8_t buffer[128];
    gui_get_bitmap_header(buffer, sizeof(buffer));

    if(httpClientWrite(buffer, 66) == 66) { // 122
        LOG_VERBOSE(TAG_GUI, F("Bitmap header sent"));

        /* Refresh screen to screenshot callback */
        lv_disp_t* disp = lv_disp_get_default();
        void (*flush_cb)(struct _disp_drv_t * disp_drv, const lv_area_t* area, lv_color_t* color_p);
        flush_cb              = disp->driver.flush_cb; /* store callback */
        disp->driver.flush_cb = gui_screenshot_to_http;
        lv_obj_invalidate(lv_scr_act());
        lv_refr_now(NULL);                /* Will call our disp_drv.disp_flush function */
        disp->driver.flush_cb = flush_cb; /* restore callback */
        screenshotIsDirty     = false;

        LOG_VERBOSE(TAG_GUI, F("Bitmap data flushed to webclient"));
    } else {
        LOG_ERROR(TAG_GUI, F("Data sent does not match header size"));
    }
}

bool guiScreenshotIsDirty()
{
    return screenshotIsDirty;
}
#endif
