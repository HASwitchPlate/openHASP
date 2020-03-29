/*********************
 *      INCLUDES
 *********************/
#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "lvgl.h"
#include "lv_conf.h"

#define LVGL7 1

#if LVGL7
#include "../lib/lvgl/src/lv_widgets/lv_roller.h"
#else
#include "lv_theme_hasp.h"
#include "lv_objx/lv_roller.h"
#endif

#if HASP_USE_QRCODE != 0
#include "lv_qrcode.h"
#endif

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
//#include "lv_zifont.h"
#include "SPIFFS.h"
#endif
#include "lv_zifont.h"
#include <FS.h> // Include the SPIFFS library
#endif

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_wifi.h"
#include "hasp_gui.h"
#include "hasp_tft.h"
#include "hasp_attr_get.h"
#include "hasp_attr_set.h"
#include "hasp.h"

#include "hasp_conf.h"
#if HASP_USE_MQTT
#include "hasp_mqtt.h"
#endif

//#if LV_USE_HASP

/*********************
 *      DEFINES
 *********************/

uint8_t haspStartDim   = 100;
uint8_t haspStartPage  = 0;
uint8_t haspThemeId    = 0;
uint16_t haspThemeHue  = 200;
char haspPagesPath[32] = "/pages.jsonl";
char haspZiFontPath[32];

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void btn_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_event_handler(lv_obj_t * obj, lv_event_t event);
// void hasp_background(uint16_t pageid, uint16_t imageid);

#if LV_USE_ANIMATION
// static void kb_hide_anim_end(lv_anim_t * a);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_style_t style_mbox_bg; /*Black bg. style with opacity*/
static lv_obj_t * kb;
static lv_font_t * defaultFont;

#if LV_DEMO_WALLPAPER
LV_IMG_DECLARE(img_bubble_pattern)
#endif

/*
LV_IMG_DECLARE(xmass)

LV_IMG_DECLARE(frame00)
LV_IMG_DECLARE(frame02)
LV_IMG_DECLARE(frame04)
LV_IMG_DECLARE(frame06)
LV_IMG_DECLARE(frame08)
LV_IMG_DECLARE(frame10)
LV_IMG_DECLARE(frame12)
LV_IMG_DECLARE(frame14)
*/

/*
static const char * btnm_map1[] = {" ", "\n", " ", "\n", " ", "\n", " ", "\n", "P1", "P2", "P3", ""};

static const char * btnm_map2[] = {"0",  "1", "\n", "2",  "3",  "\n", "4",  "5",
                                   "\n", "6", "7",  "\n", "P1", "P2", "P3", ""};
*/

#if defined(ARDUINO_ARCH_ESP8266)
static lv_obj_t * pages[4];
// static lv_font_t * haspFonts[4];
// static lv_style_t labelStyles[4];
// static lv_style_t rollerStyles[4];
#else
static lv_obj_t * pages[12];
// static lv_font_t * haspFonts[8];
// static lv_style_t labelStyles[8];
// static lv_style_t rollerStyles[8];
#endif
uint16_t current_page = 0;
// uint16_t current_style = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void haspLoadPage(String pages);

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get Page Object by PageID
 */
lv_obj_t * get_page(uint8_t pageid)
{
    if(pageid == 254) return lv_layer_top();
    if(pageid == 255) return lv_layer_sys();
    if(pageid >= sizeof pages / sizeof *pages) return NULL;
    return pages[pageid];
}
bool get_page_id(lv_obj_t * obj, uint8_t * pageid)
{
    lv_obj_t * page = lv_obj_get_screen(obj);

    if(!page) return false;

    if(page == lv_layer_top()) {
        *pageid = 254;
        return true;
    }
    if(page == lv_layer_sys()) {
        *pageid = 255;
        return true;
    }

    for(uint8_t i = 0; i < sizeof pages / sizeof *pages; i++) {
        if(page == pages[i]) {
            *pageid = i;
            return true;
        }
    }
    return false;
}

lv_obj_t * FindObjFromId(lv_obj_t * parent, uint8_t objid)
{
    if(objid == 0) return parent;
    lv_obj_t * child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        if(child->user_data && (lv_obj_user_data_t)objid == child->user_data) return child; // object found

        /* check grandchildren */
        lv_obj_t * grandchild = FindObjFromId(child, objid);
        if(grandchild) return grandchild;

        /* next sibling */
        child = lv_obj_get_child(parent, child);
    }
    return NULL;
}
lv_obj_t * FindObjFromId(uint8_t pageid, uint8_t objid)
{
    return FindObjFromId(get_page(pageid), objid);
}

bool FindIdFromObj(lv_obj_t * obj, uint8_t * pageid, lv_obj_user_data_t * objid)
{
    if(!get_page_id(obj, pageid)) return false;
    if(!(obj->user_data > 0)) return false;
    memcpy(objid, &obj->user_data, sizeof(lv_obj_user_data_t));
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void hasp_send_event_attribute(lv_obj_t * obj, const char * event)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
#if HASP_USE_MQTT > 0
        mqtt_send_event_attribute(pageid, objid, event);
#endif
    }
}

static void hasp_send_val_attribute(lv_obj_t * obj, int32_t val)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
#if HASP_USE_MQTT > 0
        mqtt_send_val_attribute(pageid, objid, val);
#endif
    }
}

static void hasp_send_txt_attribute(lv_obj_t * obj, String & txt)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
#if HASP_USE_MQTT > 0
        mqtt_send_txt_attribute(pageid, objid, txt.c_str());
#endif
    }
}

static void hasp_send_txt_attribute(lv_obj_t * obj, const char * txt)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
#if HASP_USE_MQTT > 0
        mqtt_send_txt_attribute(pageid, objid, txt);
#endif
    }
}

static inline void hasp_send_val_attribute(lv_obj_t * obj, int16_t val)
{
    hasp_send_val_attribute(obj, (int32_t)val);
}

static inline void hasp_send_color_attribute(lv_obj_t * obj, lv_color_t color)
{
    hasp_send_val_attribute(obj, (int32_t)get_cpicker_value(obj)); // Needs a color function
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool check_obj_type(const char * lvobjtype, lv_hasp_obj_type_t haspobjtype)
{
    switch(haspobjtype) {
        case LV_HASP_BUTTON:
            return (strcmp_P(lvobjtype, PSTR("lv_btn")) == 0);
        case LV_HASP_LABEL:
            return (strcmp_P(lvobjtype, PSTR("lv_label")) == 0);
        case LV_HASP_CHECKBOX:
            return (strcmp_P(lvobjtype, PSTR("lv_cb")) == 0) || (strcmp_P(lvobjtype, PSTR("lv_checkbox")) == 0);
        case LV_HASP_DDLIST:
            return (strcmp_P(lvobjtype, PSTR("lv_ddlist")) == 0) || (strcmp_P(lvobjtype, PSTR("lv_dropdown")) == 0);
        case LV_HASP_CPICKER:
            return (strcmp_P(lvobjtype, PSTR("lv_cpicker")) == 0);
        case LV_HASP_PRELOADER:
            return (strcmp_P(lvobjtype, PSTR("lv_preload")) == 0) || (strcmp_P(lvobjtype, PSTR("lv_spinner")) == 0);
        case LV_HASP_SLIDER:
            return (strcmp_P(lvobjtype, PSTR("lv_slider")) == 0);
        case LV_HASP_GAUGE:
            return (strcmp_P(lvobjtype, PSTR("lv_gauge")) == 0);
        case LV_HASP_BAR:
            return (strcmp_P(lvobjtype, PSTR("lv_bar")) == 0);
        case LV_HASP_LMETER:
            return (strcmp_P(lvobjtype, PSTR("lv_lmeter")) == 0) || (strcmp_P(lvobjtype, PSTR("lv_linemeter")) == 0);
        case LV_HASP_ROLLER:
            return (strcmp_P(lvobjtype, PSTR("lv_roller")) == 0);
        case LV_HASP_SWITCH:
            return (strcmp_P(lvobjtype, PSTR("lv_sw")) == 0) || (strcmp_P(lvobjtype, PSTR("lv_switch")) == 0);
        case LV_HASP_LED:
            return (strcmp_P(lvobjtype, PSTR("lv_led")) == 0);
        case LV_HASP_CONTAINER:
            return (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0) || (strcmp_P(lvobjtype, PSTR("lv_container")) == 0);
        default:
            return false;
    }
}

void haspSetToggle(lv_obj_t * obj, bool toggle)
{
#if LVGL7
    lv_btn_set_checkable(obj, toggle);
#else
    lv_btn_set_toggle(obj, toggle);
#endif
    lv_obj_set_event_cb(obj, toggle ? toggle_event_handler : btn_event_handler);
}

// Used in the dispatcher
void hasp_process_attribute(uint8_t pageid, uint8_t objid, String strAttr, String strPayload)
{
    lv_obj_t * obj = FindObjFromId((uint8_t)pageid, (uint8_t)objid);
    if(obj) {
        if(strPayload != "")
            hasp_set_obj_attribute(obj, strAttr.c_str(), strPayload.c_str());
        else {
            // publish the change
            std::string strValue = "";
            if(haspGetObjAttribute(obj, strAttr, strValue)) {
#if HASP_USE_MQTT > 0
                mqtt_send_attribute(pageid, objid, strAttr.c_str(), strValue.c_str());
#endif
            } else {
                Log.warning(F("HASP: Unknown property: %s"), strAttr.c_str());
            }
        } // payload
    }     // obj
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Connection lost GUI
 */

void haspWakeUp()
{
    lv_disp_trig_activity(NULL);
}

void haspDisconnect()
{
#if LVGL7

#else
    /* Create a dark plain style for a message box's background (modal)*/
    lv_style_copy(&style_mbox_bg, &lv_style_plain);
    style_mbox_bg.body.main_color = LV_COLOR_BLACK;
    style_mbox_bg.body.grad_color = LV_COLOR_BLACK;
    style_mbox_bg.body.opa        = LV_OPA_60;

    lv_obj_set_style(lv_disp_get_layer_sys(NULL), &style_mbox_bg);
#endif
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
    lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), NULL);
    lv_obj_set_user_data(lv_disp_get_layer_sys(NULL), 255);
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
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
    lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), btn_event_handler);
    /*
        lv_obj_t * obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), NULL);
        lv_obj_set_hidden(obj, true);
        obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), obj);
        lv_obj_set_hidden(obj, true);*/
}

void gotoPage1(lv_obj_t * event_kb, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED) {
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
        haspSetPage(1);
    }
}

void haspDisplayAP(const char * ssid, const char * pass)
{
    guiSetDim(100);

    String txt((char *)0);
    txt.reserve(64);

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI:S:%s;T:WPA;P:%s;;"), ssid, pass);

    /*Clear all screens*/
    for(uint8_t i = 0; i < (sizeof pages / sizeof *pages); i++) {
        lv_obj_clean(pages[i]);
    }

#if HASP_USE_QRCODE != 0
    lv_obj_t * qr = lv_qrcode_create(pages[0], 120, LV_COLOR_BLACK, LV_COLOR_WHITE);
    lv_obj_align(qr, NULL, LV_ALIGN_CENTER, 0, 50);
    lv_qrcode_update(qr, buffer, strlen(buffer));
#endif

    lv_obj_t * panel = lv_cont_create(pages[0], NULL);
    // lv_obj_set_style(panel, &lv_style_pretty);
#if HASP_USE_QRCODE != 0
    lv_obj_align(panel, qr, LV_ALIGN_OUT_TOP_MID, 0, -20);
#endif
    lv_label_set_align(panel, LV_LABEL_ALIGN_CENTER);
    lv_cont_set_fit(panel, LV_FIT_TIGHT);
    // lv_cont_set_layout(panel, LV_LAYOUT_COL_M);

    txt                = String(LV_SYMBOL_WIFI) + String(ssid);
    lv_obj_t * network = lv_label_create(panel, NULL);
    lv_label_set_text(network, txt.c_str());

    lv_obj_t * password = lv_label_create(panel, NULL);
    txt                 = String(F("\xef\x80\xA3")) + String(pass);
    lv_label_set_text(password, txt.c_str());

    haspSetPage(0);
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
    lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), gotoPage1);

    haspFirstSetup();
    // lv_obj_set_style(lv_disp_get_layer_sys(NULL), &lv_style_transp);
}

static void kb_event_cb(lv_obj_t * event_kb, lv_event_t event)
{
    if(event == LV_EVENT_APPLY) {
        char ssid[32];
        char pass[32];

        DynamicJsonDocument settings(256);

        lv_obj_t * child;
        child = lv_obj_get_child(pages[1], NULL);
        while(child) {
            if(child->user_data) {
                lv_obj_user_data_t objid = 10;
                if(objid == child->user_data) {
#if LVGL7
                    strncpy(ssid, lv_textarea_get_text(child), sizeof(ssid));
                    settings[FPSTR(F_CONFIG_SSID)] = ssid;
                    // if(kb != NULL) lv_keyboard_set_ta(kb, child);
#else
                    strncpy(ssid, lv_ta_get_text(child), sizeof(ssid));
                    settings[FPSTR(F_CONFIG_SSID)] = ssid;
                    if(kb != NULL) lv_kb_set_ta(kb, child);
#endif
                }
                objid = 20;
                if(objid == child->user_data) {
#if LVGL7
                    strncpy(pass, lv_textarea_get_text(child), sizeof(pass));
#else
                    strncpy(pass, lv_ta_get_text(child), sizeof(pass));
#endif
                    settings[FPSTR(F_CONFIG_PASS)] = pass;
                }
            }

            /* next sibling */
            child = lv_obj_get_child(pages[1], child);
        }

        if(strlen(ssid) > 0) {
            wifiSetConfig(settings.as<JsonObject>());
            dispatchReboot(true);
        }

    } else if(event == LV_EVENT_CANCEL) {
        haspSetPage(0);
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
    } else {

        /* prevent double presses, swipes and ghost press on tiny keyboard */
#if LVGL7
        if(event == LV_EVENT_RELEASED) lv_keyboard_def_event_cb(event_kb, LV_EVENT_VALUE_CHANGED);
#else
        if(event == LV_EVENT_RELEASED) lv_kb_def_event_cb(event_kb, LV_EVENT_VALUE_CHANGED);
#endif
        /* Just call the regular event handler */
        // lv_kb_def_event_cb(event_kb, event);
    }
}
static void ta_event_cb(lv_obj_t * ta, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
#if LVGL7
        // if(kb != NULL) lv_keyboard_set_ta(kb, ta);
#else
        if(kb != NULL) lv_kb_set_ta(kb, ta);
#endif
    }

    else if(event == LV_EVENT_INSERT) {
        const char * str = (const char *)lv_event_get_data();
        if(str[0] == '\n') {
            // printf("Ready\n");
        } else {
            // printf("%s\n", lv_ta_get_text(ta));
        }
    }
}

void haspFirstSetup(void)
{
    /*Create styles for the keyboard*/
    static lv_style_t rel_style, pr_style;

    lv_coord_t leftmargin, topmargin, voffset;
    lv_align_t labelpos;

    lv_disp_t * disp = lv_disp_get_default();
    if(disp->driver.hor_res <= disp->driver.ver_res) {
        leftmargin = 0;
        topmargin  = -35;
        voffset    = 12;
        labelpos   = LV_ALIGN_OUT_TOP_LEFT;
    } else {
        leftmargin = 100;
        topmargin  = -14;
        voffset    = 20;
        labelpos   = LV_ALIGN_OUT_LEFT_MID;
    }

#if LVGL7
#else
    lv_style_copy(&rel_style, &lv_style_btn_rel);
    rel_style.body.radius       = 0;
    rel_style.body.border.width = 1;
    rel_style.text.font         = LV_FONT_DEFAULT;

    lv_style_copy(&pr_style, &lv_style_btn_pr);
    pr_style.body.radius       = 0;
    pr_style.body.border.width = 1;
    rel_style.text.font        = LV_FONT_DEFAULT;

    /* Create the password box */
    lv_obj_t * pwd_ta = lv_ta_create(pages[1], NULL);
    lv_ta_set_text(pwd_ta, "");
    lv_ta_set_max_length(pwd_ta, 32);
    lv_ta_set_pwd_mode(pwd_ta, true);
    lv_ta_set_one_line(pwd_ta, true);
    lv_obj_set_user_data(pwd_ta, 20);
    lv_obj_set_width(pwd_ta, disp->driver.hor_res - leftmargin - 20);
    lv_obj_set_event_cb(pwd_ta, ta_event_cb);
    lv_obj_align(pwd_ta, NULL, LV_ALIGN_CENTER, leftmargin / 2, topmargin - voffset);

    /* Create the one-line mode text area */
    lv_obj_t * oneline_ta = lv_ta_create(pages[1], pwd_ta);
    lv_ta_set_pwd_mode(oneline_ta, false);
    lv_obj_set_user_data(oneline_ta, 10);
    lv_ta_set_cursor_type(oneline_ta, LV_CURSOR_LINE | LV_CURSOR_HIDDEN);
    lv_obj_align(oneline_ta, pwd_ta, LV_ALIGN_OUT_TOP_MID, 0, topmargin);

    /* Create a label and position it above the text box */
    lv_obj_t * pwd_label = lv_label_create(pages[1], NULL);
    lv_label_set_text(pwd_label, "Password:");
    lv_obj_align(pwd_label, pwd_ta, labelpos, 0, 0);

    /* Create a label and position it above the text box */
    lv_obj_t * oneline_label = lv_label_create(pages[1], NULL);
    lv_label_set_text(oneline_label, "Ssid:");
    lv_obj_align(oneline_label, oneline_ta, labelpos, 0, 0);

    /* Create a keyboard and make it fill the width of the above text areas */
    kb = lv_kb_create(pages[1], NULL);
    // lv_obj_set_pos(kb, 5, 90);
    lv_obj_set_event_cb(kb,
                        kb_event_cb); /* Setting a custom event handler stops the keyboard from closing automatically */
    lv_kb_set_style(kb, LV_KB_STYLE_BG, &lv_style_transp_tight);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &rel_style);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &pr_style);

    lv_kb_set_ta(kb, oneline_ta);              /* Focus it on one of the text areas to start */
    lv_kb_set_cursor_manage(oneline_ta, true); /* Automatically show/hide cursors on text areas */
#endif
}

/**
 * Create a demo application
 */
void haspSetup(JsonObject settings)
{
    guiSetDim(haspStartDim);

    /* Create all screens */
    for(uint8_t i = 0; i < (sizeof pages / sizeof *pages); i++) {
        pages[i] = lv_obj_create(NULL, NULL);
        // lv_obj_set_size(pages[0], hres, vres);
    }

    // haspSetConfig(settings);

    /*
    #ifdef LV_HASP_HOR_RES_MAX
        lv_coord_t hres = LV_HASP_HOR_RES_MAX;
    #else
        lv_coord_t hres = lv_disp_get_hor_res(NULL);
    #endif

    #ifdef LV_HASP_VER_RES_MAX
        lv_coord_t vres = LV_HASP_VER_RES_MAX;
    #else
        lv_coord_t vres = lv_disp_get_ver_res(NULL);
    #endif
    */

    // static lv_font_t *
    //    my_font = (lv_font_t *)lv_mem_alloc(sizeof(lv_font_t));

    lv_zifont_init();

    /*  if(lv_zifont_font_init(&defaultFont, haspZiFontPath, 24) != 0) {
          errorPrintln(String(F("HASP: %sFailed to set the custom font to ")) + String(haspZiFontPath));
          defaultFont = LV_FONT_DEFAULT; // Use default font
      }*/

    lv_theme_t * th = lv_theme_material_init(LV_COLOR_PURPLE, LV_COLOR_ORANGE, LV_THEME_DEFAULT_FLAGS, LV_FONT_DEFAULT,
                                             LV_FONT_DEFAULT, LV_FONT_DEFAULT, LV_FONT_DEFAULT);

    /*
            lv_theme_t * th;
            switch(haspThemeId) {
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
        #if LV_USE_THEME_MONO == 1
                case 3:
                    th = lv_theme_mono_init(haspThemeHue, defaultFont); // lightweight
                    break;
        #endif
        #if LV_USE_THEME_MATERIAL == 1
                case 4:
                    // th = lv_theme_material_init(haspThemeHue, defaultFont);
                    break;
        #endif
        #if LV_USE_THEME_ZEN == 1
                case 5:
                    th = lv_theme_zen_init(haspThemeHue, defaultFont); // lightweight
                    break;
        #endif
        #if LV_USE_THEME_NEMO == 1
                case 6:
                    th = lv_theme_nemo_init(haspThemeHue, defaultFont); // heavy
                    break;
        #endif
        #if LV_USE_THEME_TEMPL == 1
                case 7:
                    th = lv_theme_templ_init(haspThemeHue, defaultFont); // lightweight, not for production...
                    break;
        #endif
        #if LV_USE_THEME_HASP == 1
                case 8:
                    th = lv_theme_hasp_init(haspThemeHue, defaultFont);
                    break;
        #endif
                case 0:
        #if LV_USE_THEME_DEFAULT == 1
                    th = lv_theme_default_init(haspThemeHue, defaultFont);
        #else
                    th = lv_theme_hasp_init(512, defaultFont);
        #endif
                    break;

                default:
                    th = lv_theme_hasp_init(512, defaultFont);
                    debugPrintln(F("HASP: Unknown theme selected"));
            }

            if(th) {
                debugPrintln(F("HASP: Custom theme loaded"));
            } else {
                errorPrintln(F("HASP: %sNo theme could be loaded"));
            }
            // lv_theme_set_current(th);
        */

    /*
        if(lv_zifont_font_init(&haspFonts[0], "/fonts/HMI FrankRuhlLibre 24.zi", 24) != 0) {
            errorPrintln(String(F("HASP: %sFailed to set the custom font to 0")));
            defaultFont = NULL; // Use default font
        }
        if(lv_zifont_font_init(&haspFonts[1], "/fonts/HMI FiraSans 24.zi", 24) != 0) {
            errorPrintln(String(F("HASP: %sFailed to set the custom font to 1")));
            defaultFont = NULL; // Use default font
        }
        if(lv_zifont_font_init(&haspFonts[2], "/fonts/HMI AbrilFatface 24.zi", 24) != 0) {
            errorPrintln(String(F("HASP: %sFailed to set the custom font to 2")));
            defaultFont = NULL; // Use default font
        }

        for(int i = 0; i < 3; i++) {
            //lv_style_copy(&labelStyles[i], &lv_style_pretty_color);
            labelStyles[i].text.font  = haspFonts[i];
            labelStyles[i].text.color = LV_COLOR_BLUE;
        }
    */

    haspDisconnect();
    haspLoadPage(haspPagesPath);
    haspSetPage(haspStartPage);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void haspLoop(void)
{}

/*
void hasp_background(uint16_t pageid, uint16_t imageid)
{
    lv_obj_t * page = get_page(pageid);
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

/**
 * Called when a a list button is clicked on the List tab
 * @param btn pointer to a list button
 * @param event type of event that occured
 */
static void IRAM_ATTR btn_event_handler(lv_obj_t * obj, lv_event_t event)
{
    char buffer[128];
    sprintf(buffer, PSTR("HASP: "));

    /*

        // int16_t id = get_obj_id(obj);

        // uint8_t eventid = 0;
        uint8_t pageid = 0;
        lv_obj_user_data_t objid;

        char buffer[128];

        if(obj != lv_disp_get_layer_sys(NULL)) {
            if(!FindIdFromObj(obj, &pageid, &objid)) {
                Log.warning(F("HASP: Event for unknown object"));
                return;
            }
        }

    */

    switch(event) {
        case LV_EVENT_PRESSED:
            memcpy_P(buffer, PSTR("DOWN"), sizeof(buffer));
            break;
        case LV_EVENT_CLICKED:
            // UP = the same object was release then was pressed and press was not lost!
            memcpy_P(buffer, PSTR("UP"), sizeof(buffer));
            break;
        case LV_EVENT_SHORT_CLICKED:
            memcpy_P(buffer, PSTR("SHORT"), sizeof(buffer));
            break;
        case LV_EVENT_LONG_PRESSED:
            memcpy_P(buffer, PSTR("LONG"), sizeof(buffer));
            break;
        case LV_EVENT_LONG_PRESSED_REPEAT:
            memcpy_P(buffer, PSTR("HOLD"), sizeof(buffer));
            break;
        case LV_EVENT_PRESS_LOST:
            memcpy_P(buffer, PSTR("LOST"), sizeof(buffer));
            break;
        case LV_EVENT_PRESSING:
        case LV_EVENT_FOCUSED:
        case LV_EVENT_DEFOCUSED:
        case LV_EVENT_RELEASED:
            return;

        case LV_EVENT_VALUE_CHANGED:
            strcat_P(buffer, PSTR("Value Changed"));
            Log.notice(buffer);
            return;

        case LV_EVENT_DELETE:
            Log.notice(PSTR("HASP: Object deleted"), event);
            return;
        default:
            Log.warning(F("HASP: Unknown Event %d occured"), event);
            return;
    }

    if(obj == lv_disp_get_layer_sys(NULL)) {
#if HASP_USE_MQTT > 0
        mqtt_send_state(F("wakeuptouch"), buffer);
#endif
    } else {
#if HASP_USE_MQTT > 0
        hasp_send_event_attribute(obj, buffer);
#endif
    }
}

// ##################### Event Handlers by Version ########################################################

#if LVGL7
static void btnmap_event_handler(lv_obj_t * obj, lv_event_t event)
{
    // if(event == LV_EVENT_VALUE_CHANGED) haspSendNewValue(obj, lv_btnmatrix_get_pressed_btn(obj));
}

static void toggle_event_handler(lv_obj_t * obj, lv_event_t event)
{
    bool toggled =
        lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED;
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, toggled);
}

static void switch_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, lv_switch_get_state(obj));
}

static void checkbox_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, lv_checkbox_is_checked(obj));
}

static void ddlist_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_send_val_attribute(obj, lv_dropdown_get_selected(obj));
        char buffer[128];
        lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
        hasp_send_txt_attribute(obj, buffer);
    }
}
#else
static void btnmap_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, lv_btnm_get_pressed_btn(obj));
}

static void toggle_event_handler(lv_obj_t * obj, lv_event_t event)
{
    bool toggled = lv_btn_get_state(obj) == LV_BTN_STATE_TGL_PR || lv_btn_get_state(obj) == LV_BTN_STATE_TGL_REL;
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, toggled);
}

static void switch_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, lv_sw_get_state(obj));
}

static void checkbox_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, lv_cb_is_checked(obj));
}

static void ddlist_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_send_val_attribute(obj, lv_ddlist_get_selected(obj));
        char buffer[128];
        lv_ddlist_get_selected_str(obj, buffer, sizeof(buffer));
        hasp_send_txt_attribute(obj, String(buffer));
    }
}
#endif

static void slider_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_val_attribute(obj, lv_slider_get_value(obj));
}

static void cpicker_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_color_attribute(obj, lv_cpicker_get_color(obj));
}

static void roller_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_send_val_attribute(obj, lv_roller_get_selected(obj));
        char buffer[128];
        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
        hasp_send_txt_attribute(obj, buffer);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void haspSetNodename(String name)
{}

// String haspGetNodename()
//{
//    return String(F("plate11"));
//}

String haspGetVersion()
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), "%u.%u.%u", HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);
    return buffer;
}

void haspClearPage(uint16_t pageid)
{
    lv_obj_t * page = get_page(pageid);
    if(!page) {
        Log.warning(F("HASP: Page ID %u not defined"), pageid);
    } else if(page == lv_layer_sys() || page == lv_layer_top()) {
        Log.warning(F("HASP: Cannot clear a layer"));
    } else {
        Log.notice(F("HASP: Clearing page %u"), pageid);
        lv_obj_clean(pages[pageid]);
    }
}

uint16_t haspGetPage()
{
    return current_page;
}

void haspSetPage(uint16_t pageid)
{
    lv_obj_t * page = get_page(pageid);
    if(!page) {
        Log.warning(F("HASP: Page ID %u not defined"), pageid);
    } else if(page == lv_layer_sys() || page == lv_layer_top()) {
        Log.warning(F("HASP: %sCannot change to a layer"));
    } else {
        Log.notice(F("HASP: Changing page to %u"), pageid);
        current_page = pageid;
        lv_scr_load(page);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void haspNewObject(const JsonObject & config)
{
    /* Validate page */
    uint8_t pageid = config[F("page")].isNull() ? current_page : config[F("page")].as<uint8_t>();

    /* Page selection */
    lv_obj_t * page = get_page(pageid);
    if(!page) {
        Log.warning(F("HASP: Page ID %u not defined"), pageid);
        return;
    }
    /* save the current pageid */
    current_page = pageid;

    /* Validate type */
    if(config[F("objid")].isNull()) return; // comments

    lv_obj_t * parent_obj = page;
    if(!config[F("parentid")].isNull()) {
        uint8_t parentid = config[F("parentid")].as<uint8_t>();
        parent_obj       = FindObjFromId(page, parentid);
        if(!parent_obj) {
            Log.warning(F("HASP: Parent ID p[%u].b[%u] not found"), pageid, parentid);
            parent_obj = page; // create on the page instead ??
        } else {
            Log.trace(F("HASP: Parent ID p[%u].b[%u] found"), pageid, parentid);
        }
    }

    /* Input cache and validation */
    // int16_t min = config[F("min")].as<int16_t>();
    // int16_t max = config[F("max")].as<int16_t>();
    // int16_t val = config[F("val")].as<int16_t>();
    // if(min >= max) {
    //    min = 0;
    //    max = 100;
    //}
    // bool enabled      = config[F("enable")].as<bool>() | true;
    // lv_coord_t width  = config[F("w")].as<lv_coord_t>();
    // lv_coord_t height = config[F("h")].as<lv_coord_t>();
    // if(width == 0) width = 32;
    // if(height == 0) height = 32;
    uint8_t objid = config[F("objid")].as<uint8_t>();
    uint8_t id    = config[F("id")].as<uint8_t>();
    // uint8_t styleid = config[F("styleid")].as<uint8_t>();

    /* Define Objects*/
    lv_obj_t * obj = FindObjFromId(parent_obj, id);
    if(obj) {
        Log.warning(F("HASP: Object ID %u already exists!"), id);
        return;
    }

    switch(objid) {
        /* ----- Basic Objects ------ */
        case LV_HASP_BUTTON: {
            obj = lv_btn_create(parent_obj, NULL);
            // bool toggle = config[F("toggle")].as<bool>();
            // haspSetToggle(obj, toggle);
            // lv_btn_set_toggle(obj, toggle);
            // if(config[F("txt")]) {
            lv_obj_t * label = lv_label_create(obj, NULL);
            // lv_label_set_text(label, config[F("txt")].as<String>().c_str());
            // haspSetOpacity(obj, LV_OPA_COVER);
            //}
            lv_obj_set_event_cb(obj, btn_event_handler);

            break;
        }
        case LV_HASP_CHECKBOX: {
#if LVGL7
            obj = lv_checkbox_create(parent_obj, NULL);
            // if(config[F("txt")]) lv_checkbox_set_text(obj, config[F("txt")].as<String>().c_str());
#else
            obj = lv_cb_create(parent_obj, NULL);
            // if(config[F("txt")]) lv_cb_set_text(obj, config[F("txt")].as<String>().c_str());
#endif
            lv_obj_set_event_cb(obj, checkbox_event_handler);
            break;
        }
        case LV_HASP_LABEL: {
            obj = lv_label_create(parent_obj, NULL);
            // if(config[F("txt")]) {
            //    lv_label_set_text(obj, config[F("txt")].as<String>().c_str());
            // }
            /*if(styleid < sizeof labelStyles / sizeof *labelStyles) {
                debugPrintln(String(F("HASP: Styleid set to ")) + styleid);
                lv_label_set_style(obj, LV_LABEL_STYLE_MAIN, &labelStyles[styleid]);
            }*/
            /* click area padding */
            uint8_t padh = config[F("padh")].as<uint8_t>();
            uint8_t padv = config[F("padv")].as<uint8_t>();
            /* text align */
            if(padh > 0 || padv > 0) {
                lv_obj_set_ext_click_area(obj, padh, padh, padv, padv);
            }
            if(!config[F("align")].isNull()) {
                lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
            }
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_ARC: {
            obj = lv_arc_create(parent_obj, NULL);
            break;
        }
        case LV_HASP_CONTAINER: {
            obj = lv_cont_create(parent_obj, NULL);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }

        /* ----- Color Objects ------ */
        case LV_HASP_CPICKER: {
            obj = lv_cpicker_create(parent_obj, NULL);
            // lv_cpicker_set_value(obj, (uint8_t)val);
            // bool rect = config[F("rect")].as<bool>();
            // lv_cpicker_set_type(obj, rect ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
            lv_obj_set_event_cb(obj, cpicker_event_handler);
            break;
        }
#if LV_USE_PRELOAD != 0
        case LV_HASP_PRELOADER: {
#if LVGL7
            obj = lv_spinner_create(parent_obj, NULL);
#else
            obj = lv_preload_create(parent_obj, NULL);
#endif
            break;
        }
#endif
        /* ----- Range Objects ------ */
        case LV_HASP_SLIDER: {
            obj = lv_slider_create(parent_obj, NULL);
            lv_slider_set_range(obj, 0, 100);
            lv_obj_set_event_cb(obj, slider_event_handler);
            // bool knobin = config[F("knobin")].as<bool>() | true;
            // lv_slider_set_knob_in(obj, knobin);
            break;
        }
        case LV_HASP_GAUGE: {
            obj = lv_gauge_create(parent_obj, NULL);
            lv_gauge_set_range(obj, 0, 100);
            // lv_gauge_set_value(obj, val, LV_ANIM_OFF);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_BAR: {
            obj = lv_bar_create(parent_obj, NULL);
            lv_bar_set_range(obj, 0, 100);
            // lv_bar_set_value(obj, val, LV_ANIM_OFF);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_LMETER: {
#if LVGL7
            obj = lv_linemeter_create(parent_obj, NULL);
            lv_linemeter_set_range(obj, 0, 100);
            // lv_linemeter_set_value(obj, val);
#else
            obj = lv_lmeter_create(parent_obj, NULL);
            lv_lmeter_set_range(obj, 0, 100);
            // lv_lmeter_set_value(obj, val);
#endif
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }

        /* ----- On/Off Objects ------ */
        case LV_HASP_SWITCH: {
            // bool state = config[F("val")].as<bool>();
#if LVGL7
            obj = lv_switch_create(parent_obj, NULL);
            // if(state) lv_switch_on(obj, LV_ANIM_OFF);
#else
            obj = lv_sw_create(parent_obj, NULL);
            // if(state) lv_sw_on(obj, LV_ANIM_OFF);
#endif
            lv_obj_set_event_cb(obj, switch_event_handler);
            break;
        }
        case LV_HASP_LED: {
            obj = lv_led_create(parent_obj, NULL);
            // lv_led_set_bright(obj, config[F("val")].as<uint8_t>() | 0);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
            /**/
        case LV_HASP_DDLIST: {
#if LVGL7
            obj = lv_dropdown_create(parent_obj, NULL);
            // if(config[F("txt")]) lv_dropdown_set_options(obj, config[F("txt")].as<String>().c_str());
            // lv_dropdown_set_selected(obj, val);
            // lv_dropdown_set_fix_width(obj, width);
            lv_dropdown_set_draw_arrow(obj, true);
            lv_dropdown_set_anim_time(obj, 250);
#else
            obj = lv_ddlist_create(parent_obj, NULL);
            // if(config[F("txt")]) lv_ddlist_set_options(obj, config[F("txt")].as<String>().c_str());
            // lv_ddlist_set_selected(obj, val);
            // lv_ddlist_set_fix_width(obj, width);
            lv_ddlist_set_draw_arrow(obj, true);
            lv_ddlist_set_anim_time(obj, 250);
#endif
            lv_obj_set_top(obj, true);
            // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
            lv_obj_set_event_cb(obj, ddlist_event_handler);
            break;
        }
        case LV_HASP_ROLLER: {
            obj           = lv_roller_create(parent_obj, NULL);
            bool infinite = config[F("infinite")].as<bool>();
            // if(config[F("txt")]) lv_roller_set_options(obj, config[F("txt")].as<String>().c_str(), infinite);
            // lv_roller_set_selected(obj, val, LV_ANIM_ON);
#if LVGL7
            // lv_roller_set_fix_width(obj, width);
#else
            // lv_roller_set_fix_width(obj, width);
#endif
            // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
            lv_obj_set_event_cb(obj, roller_event_handler);
            break;
        }

        /* ----- Other Object ------ */
        default:
            Log.warning(F("HASP: Unsupported Object ID %u"), objid);
            return;
    }

    if(!obj) {
        Log.warning(F("HASP: Object is NULL, skipping..."));
        return;
    }

    /*
        if(!config[F("opacity")].isNull()) {
            uint8_t opacity = config[F("opacity")].as<uint8_t>();
            haspSetOpacity(obj, opacity);
        }

        bool hidden = config[F("hidden")].as<bool>();
        lv_obj_set_hidden(obj, hidden);
        lv_obj_set_click(obj, enabled);

        lv_obj_set_pos(obj, config[F("x")].as<lv_coord_t>(), config[F("y")].as<lv_coord_t>());
        lv_obj_set_width(obj, width);
    */

    String k, v;
    lv_obj_set_user_data(obj, id);

    for(JsonPair keyValue : config) {
        k = keyValue.key().c_str();
        if(k != F("page") && k != F("id") && k != F("objid") && k != F("parentid")) {
            v = keyValue.value().as<String>();
            hasp_set_obj_attribute(obj, k.c_str(), v.c_str());
            Log.trace(F("     * %s => %s"), k.c_str(), v.c_str());
        }
    }

    /** testing start **/
    lv_obj_user_data_t temp;
    if(!FindIdFromObj(obj, &pageid, &temp)) {
        Log.error(F("HASP: Lost track of the created object, not found!"));
        return;
    }
    /** testing end **/

    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    Log.verbose(F("HASP:     * p[%u].b[%u] = %s"), pageid, temp, list.type[0]);

    /* Double-check */
    lv_obj_t * test = FindObjFromId(pageid, (uint8_t)temp);
    if(test != obj) {
        Log.error(F("HASP: Objects DO NOT match!"));
    } else {
        Log.trace(F("Objects match!"));
    }
}

void haspLoadPage(String pages)
{
    if(!SPIFFS.begin()) {
        Log.error(F("HASP: FS not mounted. Failed to load %s"), pages.c_str());
        return;
    }

    if(!SPIFFS.exists(pages)) {
        Log.error(F("HASP: Non existing file %s"), pages.c_str());
        return;
    }

    Log.notice(F("HASP: Loading file %s"), pages.c_str());

    File file = SPIFFS.open(pages, "r");
    //    ReadBufferingStream bufferingStream(file, 256);
    DynamicJsonDocument config(256);

    uint8_t savedPage = current_page;
    while(deserializeJson(config, file) == DeserializationError::Ok) {
        serializeJson(config, Serial);
        Serial.println();
        haspNewObject(config.as<JsonObject>());
    }
    current_page = savedPage;

    Log.notice(F("HASP: File %s loaded"), pages.c_str());
    file.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool haspGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_STARTPAGE)] = haspStartPage;
    settings[FPSTR(F_CONFIG_STARTDIM)]  = haspStartDim;
    settings[FPSTR(F_CONFIG_THEME)]     = haspThemeId;
    settings[FPSTR(F_CONFIG_HUE)]       = haspThemeHue;
    settings[FPSTR(F_CONFIG_ZIFONT)]    = haspZiFontPath;
    settings[FPSTR(F_CONFIG_PAGES)]     = haspPagesPath;

    configOutput(settings);
    return true;
}

/** Set HASP Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool haspSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    changed |= configSet(haspStartPage, settings[FPSTR(F_CONFIG_STARTPAGE)], PSTR("haspStartPage"));
    changed |= configSet(haspStartDim, settings[FPSTR(F_CONFIG_STARTDIM)], PSTR("haspStartDim"));
    changed |= configSet(haspThemeId, settings[FPSTR(F_CONFIG_THEME)], PSTR("haspThemeId"));
    changed |= configSet(haspThemeHue, settings[FPSTR(F_CONFIG_HUE)], PSTR("haspThemeHue"));

    if(!settings[FPSTR(F_CONFIG_PAGES)].isNull()) {
        changed |= strcmp(haspPagesPath, settings[FPSTR(F_CONFIG_PAGES)]) != 0;
        strncpy(haspPagesPath, settings[FPSTR(F_CONFIG_PAGES)], sizeof(haspPagesPath));
    }

    if(!settings[FPSTR(F_CONFIG_ZIFONT)].isNull()) {
        changed |= strcmp(haspZiFontPath, settings[FPSTR(F_CONFIG_ZIFONT)]) != 0;
        strncpy(haspZiFontPath, settings[FPSTR(F_CONFIG_ZIFONT)], sizeof(haspZiFontPath));
    }

    return changed;
}
