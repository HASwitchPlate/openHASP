/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#if defined(WINDOWS) || defined(POSIX)
#include <iostream>
#include <fstream>
#include <sstream>
#endif

#include "ArduinoJson.h"

#if HASP_USE_EEPROM > 0
#include "StreamUtils.h" // For EEPromStream
#endif

#include "lvgl.h"
#include "lv_conf.h"

#if HASP_USE_DEBUG > 0
#include "../hasp_debug.h"
#endif

#if HASP_USE_CONFIG > 0
#include "lv_fs_if.h"
#include "hasp_gui.h"
#include "hasp_config.h"
//#include "hasp_filesystem.h" included in hasp_conf.h
#endif

#include "hasplib.h"
#include "lv_theme_hasp.h"

#include "dev/device.h"

#if HASP_USE_EEPROM > 0
#include "EEPROM.h"
#endif

//#if LV_USE_HASP

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
void haspLoadPage(const char* pages);

////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t hasp_sleep_state       = HASP_SLEEP_OFF; // Used in hasp_drv_touch.cpp
static uint16_t sleepTimeShort = 60;             // 1 second resolution
static uint16_t sleepTimeLong  = 120;            // 1 second resolution

uint8_t haspStartDim   = 100;
uint8_t haspStartPage  = 1;
uint8_t haspThemeId    = 2;
uint16_t haspThemeHue  = 200;
char haspPagesPath[32] = "/pages.jsonl";
char haspZiFontPath[32];

lv_style_t style_mbox_bg; /*Black bg. style with opacity*/
lv_obj_t* kb;
// lv_font_t * defaultFont;

lv_obj_t* pages[HASP_NUM_PAGES];
static lv_font_t* haspFonts[4] = {LV_THEME_DEFAULT_FONT_SMALL, LV_THEME_DEFAULT_FONT_NORMAL,
                                  LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE};
uint8_t current_page           = 1;

/**
 * Get Font ID
 */
lv_font_t* hasp_get_font(uint8_t fontid)
{
    if(fontid >= 4) {
        return nullptr;
    } else {
        return haspFonts[fontid];
    }
}

/**
 * Check if sleep state needs to be updated
 */
bool hasp_update_sleep_state()
{
    uint32_t idle = lv_disp_get_inactive_time(NULL);

    if(sleepTimeLong > 0 && idle >= (sleepTimeShort + sleepTimeLong) * 1000U) {
        if(hasp_sleep_state != HASP_SLEEP_LONG) {
            dispatch_output_idle_state(HASP_SLEEP_LONG);
            hasp_sleep_state = HASP_SLEEP_LONG;
        }
    } else if(sleepTimeShort > 0 && idle >= sleepTimeShort * 1000U) {
        if(hasp_sleep_state != HASP_SLEEP_SHORT) {
            dispatch_output_idle_state(HASP_SLEEP_SHORT);
            hasp_sleep_state = HASP_SLEEP_SHORT;
        }
    } else {
        if(hasp_sleep_state != HASP_SLEEP_OFF) {
            dispatch_output_idle_state(HASP_SLEEP_OFF);
            hasp_sleep_state = HASP_SLEEP_OFF;
        }
    }

    return (hasp_sleep_state != HASP_SLEEP_OFF);
}

void hasp_enable_wakeup_touch()
{
    LOG_VERBOSE(TAG_HASP, F("Wakeup touch enabled"));
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), true); // enable first touch
    lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), wakeup_event_handler);
}

void hasp_disable_wakeup_touch()
{
    LOG_VERBOSE(TAG_HASP, F("Wakeup touch disabled"));
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), false); // disable first touch
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
/**
 * Get Page Object by PageID
 */
lv_obj_t* get_page_obj(uint8_t pageid)
{
    if(pageid == 0) return lv_layer_top(); // 254
    if(pageid == 255) return lv_layer_sys();
    if(pageid > sizeof pages / sizeof *pages) return NULL; // >=0
    return pages[pageid - PAGE_START_INDEX];
}

bool get_page_id(lv_obj_t* obj, uint8_t* pageid)
{
    lv_obj_t* page = lv_obj_get_screen(obj);

    if(!page) return false;

    if(page == lv_layer_top()) {
        *pageid = 0; // 254
        return true;
    }
    if(page == lv_layer_sys()) {
        *pageid = 255;
        return true;
    }

    for(uint8_t i = 0; i < sizeof pages / sizeof *pages; i++) {
        if(page == pages[i]) {
            *pageid = i + PAGE_START_INDEX;
            return true;
        }
    }
    return false;
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
    lv_obj_t* bar   = hasp_find_obj_from_parent_id(get_page_obj(255), (uint8_t)10);
    if(layer && bar) {
        if(val == 255) {
            if(!lv_obj_get_hidden(bar)) {
                lv_obj_set_hidden(bar, true);
                lv_obj_set_style_local_bg_opa(layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

                // lv_obj_set_style_local_value_str(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
                // lv_obj_set_value_str_txt(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL); //TODO: call our custom
                // function to free the memory

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
    lv_obj_t* bar = hasp_find_obj_from_parent_id(get_page_obj(255), (uint8_t)10);

    if(bar) {
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
    lv_style_list_t* list;

    switch(name) {
        case LV_THEME_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            // _lv_style_list_add_style(list, &my_style);
            break;
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
    // res = lv_fs_open(&f, "E:/config.json", LV_FS_MODE_RD);
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

    LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, &robotocondensed_regular_16_nokern);
    LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, nullptr);
    LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, haspFonts[1]);

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    lv_font_t* hasp_font = nullptr; // required or font init will crash
    lv_zifont_init();

    // WARNING: hasp_font needs to be null !
    if(lv_zifont_font_init(&hasp_font, haspZiFontPath, 32) != 0) {
        LOG_ERROR(TAG_HASP, F("Failed to set font to %s"), haspZiFontPath);
        haspFonts[1] = LV_FONT_DEFAULT;
    } else {
        // defaultFont = haspFonts[0];
        haspFonts[0] = hasp_font; // save it
    }

    LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, robotocondensed_regular_16_nokern);
    LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, *hasp_font);

#endif
#endif

    if(haspFonts[0] == nullptr) haspFonts[0] = LV_THEME_DEFAULT_FONT_SMALL;
    // if(haspFonts[1] == nullptr) haspFonts[1] = LV_THEME_DEFAULT_FONT_NORMAL;
    if(haspFonts[2] == nullptr) haspFonts[2] = LV_THEME_DEFAULT_FONT_SUBTITLE;
    if(haspFonts[3] == nullptr) haspFonts[3] = LV_THEME_DEFAULT_FONT_TITLE;

    // haspFonts[0] = lv_font_load("E:/font_1.fnt");
    //  haspFonts[2] = lv_font_load("E:/font_2.fnt");
    //  haspFonts[3] = lv_font_load("E:/font_3.fnt");

    /* ********** Theme Initializations ********** */
    if(haspThemeId == 8) haspThemeId = 1;                   // update old HASP id
    if(haspThemeId == 9) haspThemeId = 5;                   // update old material id
    if(haspThemeId < 0 || haspThemeId > 5) haspThemeId = 1; // check bounds

    lv_theme_t* th = NULL;
#if(LV_USE_THEME_HASP == 1)
    lv_theme_hasp_flag_t hasp_flags = LV_THEME_HASP_FLAG_LIGHT;
#endif
#if(LV_USE_THEME_MATERIAL == 1)
    lv_theme_material_flag_t material_flags = LV_THEME_MATERIAL_FLAG_LIGHT;
#endif

    switch(haspThemeId) {
#if(LV_USE_THEME_EMPTY == 1)
        case 0:
            th = lv_theme_empty_init(lv_color_hsv_to_rgb(haspThemeHue, 100, 100),
                                     lv_color_hsv_to_rgb(haspThemeHue, 100, 100), LV_THEME_DEFAULT_FLAGS, haspFonts[0],
                                     haspFonts[1], haspFonts[2], haspFonts[3]);
            break;
#endif

#if(LV_USE_THEME_HASP == 1)
        case 2: // Dark
            hasp_flags = LV_THEME_HASP_FLAG_DARK;
        case 1: // Light
        case 8: // Light (old id)
            th = lv_theme_hasp_init(
                lv_color_hsv_to_rgb(haspThemeHue, 100, 100), lv_color_hsv_to_rgb(haspThemeHue, 100, 100),
                hasp_flags + LV_THEME_HASP_FLAG_NO_FOCUS, haspFonts[0], haspFonts[1], haspFonts[2], haspFonts[3]);
            break;
#endif

#if LV_USE_THEME_ALIEN == 1
        case 1:
            th = lv_theme_alien_init(haspThemeHue, defaultFont);
            break;
#endif

#if LV_USE_THEME_NIGHT == 1
        case 2:
            th = lv_theme_night_init(haspThemeHue, defaultFont); // heavy
            break;
#endif

#if(LV_USE_THEME_MONO == 1)
        case 3:
            th = lv_theme_mono_init(LV_COLOR_PURPLE, LV_COLOR_BLACK, LV_THEME_DEFAULT_FLAGS, haspFonts[0], haspFonts[1],
                                    haspFonts[2], haspFonts[3]);
            break;
#endif

            // LV_THEME_MATERIAL_FLAG_NO_TRANSITION : disable transitions(state change animations)
            // LV_THEME_MATERIAL_FLAG_NO_FOCUS: disable indication of focused state)
#if LV_USE_THEME_MATERIAL == 1
        case 5: // Dark
            material_flags = LV_THEME_MATERIAL_FLAG_DARK;
        case 4: // Light
        case 9: // Light (old id)
            th = lv_theme_material_init(
                lv_color_hsv_to_rgb(haspThemeHue, 100, 100), lv_color_hsv_to_rgb(haspThemeHue, 100, 100),
                material_flags + LV_THEME_MATERIAL_FLAG_NO_FOCUS + LV_THEME_MATERIAL_FLAG_NO_TRANSITION, haspFonts[0],
                haspFonts[1], haspFonts[2], haspFonts[3]);
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

    /* Create all screens using the theme */
    for(int i = 0; i < (sizeof pages / sizeof *pages); i++) {
        pages[i] = lv_obj_create(NULL, NULL);
    }

#if HASP_USE_WIFI > 0
    if(!wifiShowAP()) {
        haspDisconnect();
    }
#endif

    haspLoadPage(haspPagesPath);
    haspSetPage(haspStartPage);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void haspLoop(void)
{
    dispatchLoop();
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

void haspGetVersion(char* version, size_t len)
{
    snprintf_P(version, len, PSTR("%u.%u.%u"), HASP_VER_MAJ, HASP_VER_MIN, HASP_VER_REV);
}

void haspClearPage(uint16_t pageid)
{
    lv_obj_t* page = get_page_obj(pageid);
    if(!page || (pageid > HASP_NUM_PAGES)) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    } else if(page == lv_layer_sys() /*|| page == lv_layer_top()*/) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_LAYER));
    } else {
        LOG_TRACE(TAG_HASP, F(D_HASP_CLEAR_PAGE), pageid);
        lv_obj_clean(page);
    }
}

uint8_t haspGetPage()
{
    return current_page;
}

void haspSetPage(uint8_t pageid)
{
    lv_obj_t* page = get_page_obj(pageid);
    if(!page || pageid == 0 || pageid > HASP_NUM_PAGES) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    } else {
        LOG_TRACE(TAG_HASP, F(D_HASP_CHANGE_PAGE), pageid);
        current_page = pageid;
        lv_scr_load(page);
        hasp_object_tree(page, pageid, 0);
    }
}

void haspLoadPage(const char* pagesfile)
{
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    if(pagesfile[0] == '\0') return;

    if(!filesystemSetup()) {
        LOG_ERROR(TAG_HASP, F("FS not mounted. Failed to load %s"), pagesfile);
        return;
    }

    if(!HASP_FS.exists(pagesfile)) {
        LOG_ERROR(TAG_HASP, F("Non existing file %s"), pagesfile);
        return;
    }

    LOG_TRACE(TAG_HASP, F("Loading file %s"), pagesfile);

    File file = HASP_FS.open(pagesfile, "r");
    dispatch_parse_jsonl(file);
    file.close();

    LOG_INFO(TAG_HASP, F("File %s loaded"), pagesfile);
#else

#if HASP_USE_EEPROM > 0
    LOG_TRACE(TAG_HASP, F("Loading jsonl from EEPROM..."));
    EepromStream eepromStream(4096, 1024);
    dispatch_parse_jsonl(eepromStream);
    LOG_INFO(TAG_HASP, F("Loaded jsonl from EEPROM"));
#endif

    std::ifstream ifs("pages.json", std::ifstream::in);
    if(ifs) {
        LOG_TRACE(TAG_HASP, F("Loading file %s"), pagesfile);
        dispatch_parse_jsonl(ifs);
        LOG_INFO(TAG_HASP, F("File %s loaded"), pagesfile);
    } else {
        LOG_ERROR(TAG_HASP, F("Non existing file %s"), pagesfile);
    }

#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
bool haspGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(haspStartPage != settings[FPSTR(FP_CONFIG_STARTPAGE)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_STARTPAGE)] = haspStartPage;

    if(haspStartDim != settings[FPSTR(FP_CONFIG_STARTDIM)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_STARTDIM)] = haspStartDim;

    if(haspThemeId != settings[FPSTR(FP_CONFIG_THEME)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_THEME)] = haspThemeId;

    if(haspThemeHue != settings[FPSTR(FP_CONFIG_HUE)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_HUE)] = haspThemeHue;

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
    bool changed = false;

    changed |= configSet(haspStartPage, settings[FPSTR(FP_CONFIG_STARTPAGE)], F("haspStartPage"));
    changed |= configSet(haspStartDim, settings[FPSTR(FP_CONFIG_STARTDIM)], F("haspStartDim"));
    changed |= configSet(haspThemeId, settings[FPSTR(FP_CONFIG_THEME)], F("haspThemeId"));
    changed |= configSet(haspThemeHue, settings[FPSTR(FP_CONFIG_HUE)], F("haspThemeHue"));

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
