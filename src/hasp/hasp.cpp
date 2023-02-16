/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"
#include "dev/device.h"
#include "drv/tft/tft_driver.h"
// #include "lv_datetime.h"
#include "hasp_gui.h"

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#if defined(WINDOWS) || defined(POSIX)
#include <iostream>
#include <fstream>
#include <sstream>
#endif

#if HASP_USE_EEPROM > 0
#include "StreamUtils.h" // For EEPromStream
#endif

#if HASP_USE_DEBUG > 0
#include "../hasp_debug.h"
#endif

#if HASP_USE_CONFIG > 0
#include "lv_fs_if.h"
#include "font/hasp_font_loader.h"
// #include "hasp_filesystem.h" included in hasp_conf.h
#endif

#if HASP_USE_EEPROM > 0
#include "EEPROM.h"
#endif

// #if LV_USE_HASP

/*********************
 *      DEFINES
 *********************/
#define PAGE_START_INDEX 1 // Page number of array index 0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
// void hasp_background(uint16_t pageid, uint16_t imageid);

/**********************
 *  STATIC VARIABLES
 **********************/
#if LV_USE_ANIMATION
// static void kb_hide_anim_end(lv_anim_t * a);
#endif

#if LV_DEMO_WALLPAPER
LV_IMG_DECLARE(img_bubble_pattern)
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t hasp_sleep_state        = HASP_SLEEP_OFF; // Used in hasp_drv_touch.cpp
bool hasp_first_touch_state     = false;          // Track first touch state
static uint16_t sleepTimeShort  = 60;             // 1 second resolution
static uint16_t sleepTimeLong   = 120;            // 1 second resolution
static uint32_t sleepTimeOffset = 0;              // 1 second resolution

uint8_t haspStartDim       = HASP_START_DIM;
uint8_t haspStartPage      = HASP_START_PAGE;
uint8_t haspThemeId        = HASP_THEME_ID;
uint16_t haspThemeHue      = 200;
lv_color_t color_primary   = lv_color_hsv_to_rgb(200, 100, 100);
lv_color_t color_secondary = lv_color_hsv_to_rgb(200, 100, 100);

char haspPagesPath[32] = "/pages.jsonl";
char haspZiFontPath[32];

lv_style_t style_mbox_bg; /*Black bg. style with opacity*/
lv_obj_t* kb;
// lv_font_t * defaultFont;

static lv_font_t* haspFonts[12] = {nullptr};
uint8_t current_page            = 1;

/**
 * Get Font ID
 */
lv_font_t* hasp_get_font(uint8_t fontid)
{
    if(fontid >= sizeof(haspFonts) / sizeof(haspFonts[0])) {
        return nullptr;
    } else {
        return haspFonts[fontid];
    }
}

/**
 * Check if sleep state needs to be updated
 */
HASP_ATTRIBUTE_FAST_MEM void hasp_update_sleep_state()
{
    if(hasp_first_touch_state) return; // don't update sleep when first touch is still active

    uint32_t idle = lv_disp_get_inactive_time(lv_disp_get_default()) / 1000;
    idle += sleepTimeOffset; // To force a specific state

    if(sleepTimeLong > 0 && idle >= (sleepTimeShort + sleepTimeLong)) {
        if(hasp_sleep_state != HASP_SLEEP_LONG) {
            gui_hide_pointer(true);
            hasp_sleep_state = HASP_SLEEP_LONG;
            dispatch_idle_state(HASP_SLEEP_LONG);
        }
    } else if(sleepTimeShort > 0 && idle >= sleepTimeShort) {
        if(hasp_sleep_state != HASP_SLEEP_SHORT) {
            gui_hide_pointer(true);
            hasp_sleep_state = HASP_SLEEP_SHORT;
            dispatch_idle_state(HASP_SLEEP_SHORT);
        }
    } else {
        if(hasp_sleep_state != HASP_SLEEP_OFF) {
            gui_hide_pointer(false);
            hasp_sleep_state = HASP_SLEEP_OFF;
            dispatch_idle_state(HASP_SLEEP_OFF);
        }
    }
}

void hasp_set_sleep_offset(uint32_t offset)
{
    sleepTimeOffset = offset;
}

uint8_t hasp_get_sleep_state()
{
    return hasp_sleep_state;
}

void hasp_set_sleep_state(uint8_t state)
{
    switch(state) {
        case HASP_SLEEP_LONG:
            hasp_set_sleep_offset(sleepTimeShort + sleepTimeLong);
            break;
        case HASP_SLEEP_SHORT:
            hasp_set_sleep_offset(sleepTimeShort);
            break;
        case HASP_SLEEP_OFF:
            hasp_set_sleep_offset(0);
            hasp_set_wakeup_touch(false);
            break;
        default:
            return;
    }
    lv_disp_trig_activity(NULL);
    hasp_sleep_state = state;
}

void hasp_get_sleep_payload(uint8_t state, char* payload)
{
    switch(state) {
        case HASP_SLEEP_LONG:
            memcpy_P(payload, PSTR("long"), 5);
            break;
        case HASP_SLEEP_SHORT:
            memcpy_P(payload, PSTR("short"), 6);
            break;
        default:
            memcpy_P(payload, PSTR("off"), 4);
    }
}

/**
 * Anti Burn-in protection
 */
static lv_task_t* antiburn_task;

bool hasp_stop_antiburn()
{
    bool changed = false;

    /* Refresh screen to flush callback */
    lv_disp_t* disp       = lv_disp_get_default();
    disp->driver.flush_cb = gui_flush_cb;

    // lv_obj_t* layer = lv_disp_get_layer_sys(NULL);
    // if(layer) lv_obj_set_style_local_bg_opa(layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    if(antiburn_task) {
        lv_task_del(antiburn_task);
        lv_obj_invalidate(lv_scr_act());
        changed = true;
    }
    antiburn_task = NULL;
    hasp_set_wakeup_touch(haspDevice.get_backlight_power() == false); // enabled if backlight is OFF

    // gui_hide_pointer(false);
    return changed;
}

void hasp_antiburn_cb(lv_task_t* task)
{
    lv_obj_t* layer = lv_disp_get_layer_sys(NULL);
    if(layer) {
        // Fill a buffer with random colors
        lv_color_t color[1223];
        size_t len = sizeof(color) / sizeof(color[0]);
        for(size_t x = 0; x < len; x++) {
            color[x].full = HASP_RANDOM(UINT16_MAX);
        }

        // list of possible draw widths; prime numbers combat recurring patterns on the screen
        uint8_t prime[] = {61,  67,  73,  79,  83,  89,  97,  103, 109, 113, 127, 131, 137, 139, 149,
                           157, 163, 167, 173, 179, 181, 191, 197, 211, 223, 227, 229, 233, 251};

        lv_disp_t* disp         = lv_disp_get_default();
        lv_disp_drv_t* disp_drv = &disp->driver;

        lv_coord_t scr_h = lv_obj_get_height(layer) - 1;
        lv_coord_t scr_w = lv_obj_get_width(layer) - 1;
        lv_coord_t w     = 487; // first prime larger than 480
        lv_area_t area;

        area.y1 = 0;
        while(area.y1 <= scr_h) {
            if(w > scr_w) w = scr_w; // limit to the actual screenwidth
            if(w > len) w = len;     // don't overrun the buffer
            lv_coord_t h    = len / w;
            size_t headroom = len % w; // additional bytes in the buffer that can be used for a random offset

            area.y2 = area.y1 + h - 1;
            if(area.y2 > scr_h) area.y2 = scr_h;

            area.x1 = 0;
            while(area.x1 <= scr_w) {
                area.x2 = area.x1 + w - 1;
                if(area.x2 > scr_w) area.x2 = scr_w;

                size_t offset = headroom ? HASP_RANDOM(headroom) : 0;
                haspTft.flush_pixels(disp_drv, &area, color + offset);
                area.x1 += w;
            }

            w = prime[HASP_RANDOM(sizeof(prime))]; // new random width
            area.y1 += h;
        }
    }

    if(task->repeat_count != 1) return; // don't stop yet

    // task is about to get deleted
    hasp_stop_antiburn();
    dispatch_state_antiburn(HASP_EVENT_OFF);
}

/**
 * Enable/Disable Anti Burn-in protection
 */
void hasp_set_antiburn(int32_t repeat_count, uint32_t period)
{
    if(repeat_count != 0) {
        lv_obj_t* layer = lv_disp_get_layer_sys(NULL);
        if(!layer) return;

        if(!antiburn_task) antiburn_task = lv_task_create(hasp_antiburn_cb, period, LV_TASK_PRIO_LOW, NULL);
        if(antiburn_task) {
            // lv_obj_set_style_local_bg_color(layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            // lv_obj_set_style_local_bg_opa(layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
            hasp_set_wakeup_touch(true);
            lv_task_set_repeat_count(antiburn_task, repeat_count);
            lv_task_set_period(antiburn_task, period);
            //  gui_hide_pointer(true);

            /* Refresh screen to antiburn callback */
            lv_disp_t* disp       = lv_disp_get_default();
            disp->driver.flush_cb = gui_antiburn_cb;
            lv_obj_invalidate(lv_scr_act());

        } else {
            LOG_INFO(TAG_HASP, F("Antiburn %s"), D_INFO_FAILED);
        }
    } else {
        hasp_stop_antiburn();
    }
}

/**
 * Check if Anti Burn-in protection is enabled
 */
hasp_event_t hasp_get_antiburn()
{
    return (antiburn_task != NULL) ? HASP_EVENT_ON : HASP_EVENT_OFF;
}

/**
 * Enable/Disable Wake-up Touch
 */
void hasp_set_wakeup_touch(bool en)
{
    lv_obj_t* layer = lv_disp_get_layer_sys(NULL);
    if(!layer) return;

    if(lv_obj_get_click(layer) != en) {
        hasp_first_touch_state = en;
        lv_obj_set_event_cb(layer, first_touch_event_handler);
        lv_obj_set_click(layer, en);
        LOG_INFO(TAG_HASP, F("First touch %s"), en ? D_SETTING_ENABLED : D_SETTING_DISABLED);
    }
}

/**
 * Return the sleep times
 */
void hasp_get_sleep_time(uint16_t& short_time, uint16_t& long_time)
{
    short_time = sleepTimeShort;
    long_time  = sleepTimeLong;
}

/**
 * Set the sleep times
 */
void hasp_set_sleep_time(uint16_t short_time, uint16_t long_time)
{
    sleepTimeShort = short_time;
    sleepTimeLong  = long_time;
}

/**
 * Checks if we went to sleep, wake up is handled in the event handlers
 */
void haspEverySecond()
{
    hasp_update_sleep_state();
    dispatchEverySecond();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void haspDisconnect()
{

    /* Create a dark plain style for a message box's background (modal)*/
    // lv_style_copy(&style_mbox_bg, &lv_style_plain);
    // style_mbox_bg.body.main_color = LV_COLOR_BLACK;
    // style_mbox_bg.body.grad_color = LV_COLOR_BLACK;
    // style_mbox_bg.body.opa        = LV_OPA_60;

    // lv_obj_set_style(lv_disp_get_layer_sys(NULL), &style_mbox_bg);

    /*
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
        lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), NULL);
        lv_obj_set_user_data(lv_disp_get_layer_sys(NULL), 255);
        */

    /*
        lv_obj_t * obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), NULL);
        lv_obj_set_hidden(obj, false);
        obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), obj);
        lv_obj_set_hidden(obj, false);*/
}

void haspReconnect()
{
    /*Revert the top layer to not block*/
    // lv_obj_set_style(lv_disp_get_layer_sys(NULL), &lv_style_transp);
    // lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
    // lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), btn_event_handler);
    /*
        lv_obj_t * obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), NULL);
        lv_obj_set_hidden(obj, true);
        obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), obj);
        lv_obj_set_hidden(obj, true);*/
}

// String progress_str((char *)0);

// Shows/hides the the global progress bar and updates the value
void haspProgressVal(uint8_t val)
{
    lv_obj_t* layer = lv_disp_get_layer_sys(NULL);
    lv_obj_t* bar   = hasp_find_obj_from_page_id(255U, 10U);
    if(layer && bar) {
        if(val == 255) {
            if(!lv_obj_get_hidden(bar)) {
                lv_obj_set_hidden(bar, true);
                lv_obj_set_style_local_bg_opa(layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

                // lv_obj_set_style_local_value_str(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
                // lv_obj_set_value_str_txt(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
                // TODO: call our custom function to free the memory

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
                // progress_str.clear();
#endif
            }
        } else {
            if(lv_obj_get_hidden(bar)) {
                lv_obj_set_hidden(bar, false);
                lv_obj_set_style_local_bg_opa(layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
            }
            lv_bar_set_value(bar, val, LV_ANIM_OFF);
        }
        lv_task_handler(); // needed to let the GUI do its work during long updates
    }
}

// Sets the value string of the global progress bar
void haspProgressMsg(const char* msg)
{
    if(lv_obj_t* bar = hasp_find_obj_from_page_id(255U, 10U)) {
        char value_str[10];
        snprintf_P(value_str, sizeof(value_str), PSTR("value_str"));
        hasp_process_obj_attribute(bar, value_str, msg, true);
    }

    lv_task_handler(); // needed to let the GUI do its work during long updates

    /* if(bar) {
         progress_str.reserve(64);
         progress_str = msg;
         lv_obj_set_style_local_value_str(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, progress_str.c_str());

        // lv_task_handler(); // let the GUI do its work
     } */
}

#ifdef ARDUINO
// Sets the value string of the global progress bar
void haspProgressMsg(const __FlashStringHelper* msg)
{
    haspProgressMsg(String(msg).c_str());
}
#endif

/*Add a custom apply callback*/
static void custom_font_apply_cb(lv_theme_t* th, lv_obj_t* obj, lv_theme_style_t name)
{
    /*    lv_style_list_t* list;

        switch(name) {
            case LV_THEME_BTN:
                list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
                // _lv_style_list_add_style(list, &my_style);
                break;
            default:
                // nothing
                ;
        } */
}

void hasp_set_theme(uint8_t themeid)
{
    lv_theme_t* th = NULL;

    /* ********** Theme Initializations ********** */
    if(themeid == 8) themeid = 1;          // update old HASP id
    if(themeid == 9) themeid = 5;          // update old material id
    if(themeid < 0 || themeid > 5) return; // check bounds

#if(LV_USE_THEME_HASP == 1)
    uint32_t hasp_flags = LV_THEME_HASP_FLAG_LIGHT + LV_THEME_HASP_FLAG_NO_TRANSITION + LV_THEME_HASP_FLAG_NO_FOCUS;
#endif

#if(LV_USE_THEME_MATERIAL == 1)
    // LV_THEME_MATERIAL_FLAG_NO_TRANSITION : disable transitions(state change animations)
    // LV_THEME_MATERIAL_FLAG_NO_FOCUS: disable indication of focused state)
    uint32_t material_flags =
        LV_THEME_MATERIAL_FLAG_LIGHT + LV_THEME_MATERIAL_FLAG_NO_TRANSITION + LV_THEME_MATERIAL_FLAG_NO_FOCUS;
#endif

    switch(themeid) {
#if(LV_USE_THEME_EMPTY == 1)
        case 0:
            th = lv_theme_empty_init(lv_color_hsv_to_rgb(haspThemeHue, 100, 100),
                                     lv_color_hsv_to_rgb(haspThemeHue, 100, 100), LV_THEME_DEFAULT_FLAGS, haspFonts[0],
                                     haspFonts[1], haspFonts[2], haspFonts[3]);
            break;
#endif

#if(LV_USE_THEME_HASP == 1)
        case 2: // Dark
            hasp_flags = LV_THEME_HASP_FLAG_DARK + LV_THEME_HASP_FLAG_NO_TRANSITION + LV_THEME_HASP_FLAG_NO_FOCUS;
        case 1: // Light
        case 8: // Light (old id)
            th = lv_theme_hasp_init(color_primary, color_secondary, hasp_flags, haspFonts[0], haspFonts[1],
                                    haspFonts[2], haspFonts[3]);
            break;
#endif

#if(LV_USE_THEME_MONO == 1)
        case 3:
            th = lv_theme_mono_init(color_primary, color_secondary, LV_THEME_DEFAULT_FLAGS, haspFonts[0], haspFonts[1],
                                    haspFonts[2], haspFonts[3]);
            break;
#endif

#if LV_USE_THEME_MATERIAL == 1
        case 5: // Dark
            material_flags =
                LV_THEME_MATERIAL_FLAG_DARK + LV_THEME_MATERIAL_FLAG_NO_TRANSITION + LV_THEME_MATERIAL_FLAG_NO_FOCUS;
        case 4: // Light
        case 9: // Light (old id)
            th = lv_theme_material_init(color_primary, color_secondary, material_flags, haspFonts[0], haspFonts[1],
                                        haspFonts[2], haspFonts[3]);
            break;
#endif

#if LV_USE_THEME_TEMPLATE == 1
        case 7:
            th = lv_theme_template_init(LV_COLOR_PURPLE, LV_COLOR_ORANGE, LV_THEME_DEFAULT_FLAGS, haspFonts[0],
                                        haspFonts[1], haspFonts[2], haspFonts[3]);
            break;
#endif

        default:

            LOG_ERROR(TAG_HASP, F("Unknown theme selected"));
    }

    if(th) {
        lv_theme_set_act(th);
        LOG_INFO(TAG_HASP, F("Custom theme loaded"));
    } else {
        LOG_ERROR(TAG_HASP, F("Theme could not be loaded"));
    }
}

/**
 * Create a demo application
 */
void haspSetup(void)
{
    haspDevice.set_backlight_level(haspStartDim);

    /******* File System Test ********************************************************************/
    // lv_fs_file_t f;
    // lv_fs_res_t res;
    // res = lv_fs_open(&f, "L:/config.json", LV_FS_MODE_RD);
    // if(res == LV_FS_RES_OK)
    //     LOG_VERBOSE(TAG_HASP, F("Opening config.json OK"));
    // else
    //     LOG_ERROR(TAG_HASP, F("Opening config.json from FS failed %d"), res);

    // uint32_t btoread = 128;
    // uint32_t bread   = 0;
    // char buffer[129];

    // res = lv_fs_read(&f, buffer, btoread, &bread);
    // if(res == LV_FS_RES_OK) {
    //     LOG_VERBOSE(TAG_HASP, F("Reading config.json OK %u"), bread);
    //     buffer[127] = '\0';
    //     LOG_INFO(TAG_HASP, buffer);
    // } else
    //     LOG_ERROR(TAG_HASP, F("Reading config.json from FS failed %d"), res);

    // res = lv_fs_close(&f);
    // if(res == LV_FS_RES_OK)
    //     LOG_VERBOSE(TAG_HASP, F("Closing config.json OK"));
    // else
    //     LOG_ERROR(TAG_HASP, F("Closing config.json on FS failed %d"), res);
    /******* File System Test ********************************************************************/

    /* ********** Font Initializations ********** */

    // LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, nullptr);
    // LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, haspFonts[0]);
    // LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, &robotocondensed_regular_16);

    haspFonts[0] = LV_THEME_DEFAULT_FONT_SMALL;
    haspFonts[1] = LV_THEME_DEFAULT_FONT_NORMAL;
    haspFonts[2] = LV_THEME_DEFAULT_FONT_SUBTITLE;
    haspFonts[3] = LV_THEME_DEFAULT_FONT_TITLE;

    hasp_set_theme(haspThemeId);

    /* Create all screens using the theme */

#if HASP_USE_WIFI > 0
    if(!wifiShowAP()) {
        haspDisconnect();
    }
#endif

    hasp_init();
    hasp_load_json();
    haspPages.set(haspStartPage, LV_SCR_LOAD_ANIM_NONE, 0, 0);

    // lv_obj_t* obj        = lv_datetime_create(haspPages.get_obj(haspPages.get()), NULL);
    // obj->user_data.objid = LV_HASP_DATETIME;
    // obj->user_data.id    = 199;
    // lv_datetime_set_text_fmt(obj,"%A, %B %d");
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

IRAM_ATTR void haspLoop(void)
{
    dispatchLoop();
}

// Replaces all pages with new ones
void hasp_init(void)
{
    haspPages.init(haspStartPage); // StartPage is used for the BACK action
}

void hasp_load_json(void)
{
    haspPages.load_jsonl(haspPagesPath);
}

/*
void hasp_background(uint16_t pageid, uint16_t imageid)
{
    lv_obj_t * page = get_page_obj(pageid);
    if(!page) return;

    return;

    page               = lv_scr_act();
    lv_obj_t * thisobj = lv_obj_get_child_back(page, NULL);

    if(!thisobj) return;

    switch (imageid)
    {
    case 0:
        lv_img_set_src(thisobj, &frame00);
        break;
    case 1:
        lv_img_set_src(thisobj, &frame02);
        break;
    case 2:
        lv_img_set_src(thisobj, &frame04);
        break;
    case 3:
        lv_img_set_src(thisobj, &frame06);
        break;
    case 4:
        lv_img_set_src(thisobj, &frame08);
        break;
    case 5:
        lv_img_set_src(thisobj, &frame10);
        break;
    case 6:
        lv_img_set_src(thisobj, &frame12);
        break;
    case 7:
        lv_img_set_src(thisobj, &frame14);
        break;
    }
    //printf("Image set to %u\n", imageid);

    lv_img_set_auto_size(thisobj, false);
    lv_obj_set_width(thisobj, lv_disp_get_hor_res(NULL));
    lv_obj_set_height(thisobj, lv_disp_get_ver_res(NULL));
    // lv_obj_set_protect(wp, LV_PROTECT_POS);
    // lv_obj_invalidate(thisobj);
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////

// void haspGetVersion(char* version, size_t len)
// {
//     snprintf_P(version, len, PSTR("%u.%u.%u"), HASP_VER_MAJ, HASP_VER_MIN, HASP_VER_REV);
// }

void haspClearPage(uint16_t pageid)
{
    lv_obj_t* page = haspPages.get_obj(pageid);
    if(!page || (pageid > HASP_NUM_PAGES)) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    } else if(page == lv_layer_sys() /*|| page == lv_layer_top()*/) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_LAYER));
    } else {
        LOG_TRACE(TAG_HASP, F(D_HASP_CLEAR_PAGE), pageid);
        lv_obj_clean(page);
    }
}

void hasp_get_info(JsonDocument& doc)
{
    std::string buffer;
    buffer.reserve(64);
    char size_buf[32];
    JsonObject info = doc.createNestedObject(F(D_MANUFACTURER));

    buffer = haspDevice.get_version();
#ifdef COMMIT_HASH
    buffer += " ";
    buffer += COMMIT_HASH;
#endif
    info[F(D_INFO_VERSION)] = buffer;

    buffer = __DATE__;
    buffer += (" ");
    buffer += __TIME__;
    buffer += (" UTC"); // Github buildservers are in UTC
    info[F(D_INFO_BUILD_DATETIME)] = buffer;

    unsigned long time = millis() / 1000;
    uint16_t day       = time / 86400;
    time               = time % 86400;
    uint8_t hour       = time / 3600;
    time               = time % 3600;
    uint8_t min        = time / 60;
    time               = time % 60;
    uint8_t sec        = time;

    buffer.clear();
    if(day > 0) {
        itoa(day, size_buf, DEC);
        buffer += size_buf;
        buffer += "d ";
    }
    if(day > 0 || hour > 0) {
        itoa(hour, size_buf, DEC);
        buffer += size_buf;
        buffer += "h ";
    }
    if(day > 0 || hour > 0 || min > 0) {
        itoa(min, size_buf, DEC);
        buffer += size_buf;
        buffer += "m ";
    }
    itoa(sec, size_buf, DEC);
    buffer += size_buf;
    buffer += "s";
    info[F(D_INFO_ENVIRONMENT)] = PIOENV;
    info[F(D_INFO_UPTIME)]      = buffer;
    hasp_get_sleep_payload(hasp_get_sleep_state(), size_buf);
    info[F("Idle")]        = size_buf;
    info[F("Active Page")] = haspPages.get();

    info = doc.createNestedObject(F(D_INFO_DEVICE_MEMORY));
    Parser::format_bytes(haspDevice.get_free_heap(), size_buf, sizeof(size_buf));
    info[F(D_INFO_FREE_HEAP)] = size_buf;
    Parser::format_bytes(haspDevice.get_free_max_block(), size_buf, sizeof(size_buf));
    info[F(D_INFO_FREE_BLOCK)]    = size_buf;
    info[F(D_INFO_FRAGMENTATION)] = std::to_string(haspDevice.get_heap_fragmentation()) + "%";

#if ARDUINO_ARCH_ESP32
    if(psramFound()) {
        Parser::format_bytes(ESP.getFreePsram(), size_buf, sizeof(size_buf));
        info[F(D_INFO_PSRAM_FREE)] = size_buf;
        Parser::format_bytes(ESP.getPsramSize(), size_buf, sizeof(size_buf));
        info[F(D_INFO_PSRAM_SIZE)] = size_buf;
    }
#endif

#if LV_MEM_CUSTOM == 0
    info = doc.createNestedObject(F(D_INFO_LVGL_MEMORY));
    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);
    Parser::format_bytes(mem_mon.total_size, size_buf, sizeof(size_buf));
    info[F(D_INFO_TOTAL_MEMORY)] = size_buf;
    Parser::format_bytes(mem_mon.free_size, size_buf, sizeof(size_buf));
    info[F(D_INFO_FREE_MEMORY)]   = size_buf;
    info[F(D_INFO_FRAGMENTATION)] = std::to_string(mem_mon.frag_pct) + "%";
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
bool haspGetConfig(const JsonObject& settings)
{
    char buffer1[8];
    char buffer2[8];
    bool changed = false;

    Parser::ColorToHaspPayload(color_primary, buffer1, sizeof(buffer1));
    Parser::ColorToHaspPayload(color_secondary, buffer2, sizeof(buffer2));

    if(haspStartPage != settings[FPSTR(FP_CONFIG_STARTPAGE)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_STARTPAGE)] = haspStartPage;

    if(haspStartDim != settings[FPSTR(FP_CONFIG_STARTDIM)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_STARTDIM)] = haspStartDim;

    if(haspThemeId != settings[FPSTR(FP_CONFIG_THEME)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_THEME)] = haspThemeId;

    // if(haspThemeHue != settings[FPSTR(FP_CONFIG_HUE)].as<uint16_t>()) changed = true;
    // settings[FPSTR(FP_CONFIG_HUE)] = haspThemeHue;

    if(strcmp(buffer1, settings[FPSTR(FP_CONFIG_COLOR1)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_COLOR1)] = buffer1;

    if(strcmp(buffer2, settings[FPSTR(FP_CONFIG_COLOR2)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_COLOR2)] = buffer2;

    if(strcmp(haspZiFontPath, settings[FPSTR(FP_CONFIG_ZIFONT)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_ZIFONT)] = haspZiFontPath;

    if(strcmp(haspPagesPath, settings[FPSTR(FP_CONFIG_PAGES)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_PAGES)] = haspPagesPath;

    if(changed) configOutput(settings, TAG_HASP);
    return changed;
}

/** Set HASP Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool haspSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_HASP);
    lv_color32_t c;
    JsonVariant color_str;
    bool changed = false;

    changed |= configSet(haspStartPage, settings[FPSTR(FP_CONFIG_STARTPAGE)], F("haspStartPage"));
    changed |= configSet(haspStartDim, settings[FPSTR(FP_CONFIG_STARTDIM)], F("haspStartDim"));

    { // Theme related settings
        // Set from Hue first
        bool theme_changed = false;
        theme_changed |= configSet(haspThemeId, settings[FPSTR(FP_CONFIG_THEME)], F("haspThemeId"));
        theme_changed |= configSet(haspThemeHue, settings[FPSTR(FP_CONFIG_HUE)], F("haspThemeHue"));
        color_primary   = lv_color_hsv_to_rgb(haspThemeHue, 100, 100);
        color_secondary = lv_color_hsv_to_rgb(20, 60, 100);

        // Check for color1 and color2
        theme_changed |= configSet(color_primary, settings[FPSTR(FP_CONFIG_COLOR1)], F("haspColor1"));
        theme_changed |= configSet(color_secondary, settings[FPSTR(FP_CONFIG_COLOR2)], F("haspColor2"));

        changed |= theme_changed;
        // if(theme_changed) hasp_set_theme(haspThemeId); // LVGL is not inited at config load time
    }

    if(haspStartPage == 0) { // TODO: fase out migration code
        haspStartPage = 1;
        changed       = true;
    }

    if(!settings[FPSTR(FP_CONFIG_PAGES)].isNull()) {
        changed |= strcmp(haspPagesPath, settings[FPSTR(FP_CONFIG_PAGES)]) != 0;
        strncpy(haspPagesPath, settings[FPSTR(FP_CONFIG_PAGES)], sizeof(haspPagesPath));
    }

    if(!settings[FPSTR(FP_CONFIG_ZIFONT)].isNull()) {
        changed |= strcmp(haspZiFontPath, settings[FPSTR(FP_CONFIG_ZIFONT)]) != 0;
        strncpy(haspZiFontPath, settings[FPSTR(FP_CONFIG_ZIFONT)], sizeof(haspZiFontPath));
    }

    return changed;
}
#endif // HASP_USE_CONFIG
