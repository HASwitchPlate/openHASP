/*********************
 *      INCLUDES
 *********************/
#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "lvgl.h"

#include "hasp.h"
#include "hasp_gui.h"
#include "hasp_wifi.h"
#include "hasp_config.h"

#if HASP_USE_QRCODE != 0
#include "lv_qrcode.h"
#endif

static lv_obj_t * oobepage[2];
static lv_obj_t * oobekb;
extern lv_font_t * defaultFont;
static bool oobeAutoCalibrate = true;

void oobeSetAutoCalibrate(bool cal)
{
    oobeAutoCalibrate = cal;
}

static inline void oobeSetPage(uint8_t pageid)
{
    lv_scr_load(oobepage[pageid]);
}

void gotoPage1_cb(lv_obj_t * event_kb, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED) {
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
        oobeSetPage(1);
    }
}

static void kb_event_cb(lv_obj_t * event_kb, lv_event_t event)
{
    if(event == LV_EVENT_APPLY) {
        char ssid[32];
        char pass[32];

        DynamicJsonDocument settings(256);

        lv_obj_t * child;
        child = lv_obj_get_child(oobepage[1], NULL);
        while(child) {
            if(child->user_data) {
                lv_obj_user_data_t objid = 10;
                if(objid == child->user_data) {
                    strncpy(ssid, lv_textarea_get_text(child), sizeof(ssid));
                    settings[FPSTR(F_CONFIG_SSID)] = ssid;
                    if(oobekb != NULL) lv_keyboard_set_textarea(oobekb, child);
                }
                objid = 20;
                if(objid == child->user_data) {
                    strncpy(pass, lv_textarea_get_text(child), sizeof(pass));
                    settings[FPSTR(F_CONFIG_PASS)] = pass;
                }
            }

            /* next sibling */
            child = lv_obj_get_child(oobepage[1], child);
        }

        if(strlen(ssid) > 0) {
            wifiSetConfig(settings.as<JsonObject>());
            // dispatchReboot(true);
        }

    } else if(event == LV_EVENT_CANCEL) {
        oobeSetPage(0);
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
    } else {

        /* prevent double presses, swipes and ghost press on tiny keyboard */
        if(event == LV_EVENT_RELEASED) lv_keyboard_def_event_cb(event_kb, LV_EVENT_VALUE_CHANGED);
        /* Just call the regular event handler */
        // lv_kb_def_event_cb(event_kb, event);
    }
}
static void ta_event_cb(lv_obj_t * ta, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if(oobekb != NULL) lv_keyboard_set_textarea(oobekb, ta);
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

static void oobeSetupQR(const char * ssid, const char * pass)
{
    lv_disp_t * disp = lv_disp_get_default();
    oobepage[0]      = lv_obj_create(NULL, NULL);

    lv_obj_t * container = lv_cont_create(oobepage[0], NULL);
    lv_obj_set_pos(container, 5, 5);
    lv_obj_set_style_local_bg_opa(container, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_opa(container, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);

    Log.verbose(F("OOBE: line %d"), __LINE__);

#if HASP_USE_QRCODE != 0
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI:S:%s;T:WPA;P:%s;;"), ssid, pass);

    Log.verbose(F("OOBE: line %d"), __LINE__);

    lv_obj_t * qr = lv_qrcode_create(oobepage[0], 120, LV_COLOR_BLACK, LV_COLOR_WHITE);
    lv_qrcode_update(qr, buffer, strlen(buffer));

    Log.verbose(F("OOBE: line %d"), __LINE__);

    lv_obj_t * qrlabel = lv_label_create(oobepage[0], NULL);
    lv_label_set_text(qrlabel, "Scan to connect:");

    Log.verbose(F("OOBE: line %d"), __LINE__);

    if(disp->driver.hor_res <= disp->driver.ver_res) {
        lv_obj_align(qr, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
        lv_obj_set_size(container, disp->driver.hor_res - 10, disp->driver.ver_res - 10 - 125);
        lv_obj_align(qrlabel, container, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    } else {
        lv_obj_align(qr, NULL, LV_ALIGN_IN_RIGHT_MID, -5, 0);
        lv_obj_set_size(container, disp->driver.hor_res - 10 - 125, disp->driver.ver_res - 10);
        lv_obj_align(qrlabel, qr, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    }

    Log.verbose(F("OOBE: line %d"), __LINE__);

#else
    Log.verbose(F("OOBE: line %d"), __LINE__);
    lv_obj_set_size(container, disp->driver.hor_res, disp->driver.ver_res);
#endif

    Log.verbose(F("OOBE: line %d"), __LINE__);

    lv_obj_t * aplabel = lv_label_create(container, NULL);
    lv_label_set_text(aplabel, "Tap the screen to setup WiFi or\nconnect to this Access Point:");
    lv_label_set_long_mode(aplabel, LV_LABEL_LONG_BREAK);

    Log.verbose(F("OOBE: line %d"), __LINE__);
    lv_obj_set_width(aplabel, lv_obj_get_width(container));
    lv_label_set_align(aplabel, LV_LABEL_ALIGN_CENTER);

    Log.verbose(F("OOBE: line %d"), __LINE__);
    lv_obj_align(aplabel, container, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t * panel = lv_cont_create(container, NULL);
    lv_obj_align(panel, aplabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    Log.verbose(F("OOBE: line %d"), __LINE__);
    lv_cont_set_fit(panel, LV_FIT_TIGHT);

    Log.verbose(F("OOBE: line %d"), __LINE__);
    lv_cont_set_layout(panel, LV_LAYOUT_COLUMN_MID);

    Log.verbose(F("OOBE: line %d"), __LINE__);
    String txt((char *)0);
    txt.reserve(64);

    Log.verbose(F("OOBE: line %d"), __LINE__);
    txt                = String(LV_SYMBOL_WIFI) + " " + String(ssid);
    lv_obj_t * network = lv_label_create(panel, NULL);
    lv_label_set_text(network, txt.c_str());

    lv_obj_t * password = lv_label_create(panel, NULL);
    txt                 = String(F("\xef\x80\xA3")) + " " + String(pass);
    lv_label_set_text(password, txt.c_str());
}

static void oobeSetupSsid(void)
{
    lv_font_t * defaultfont;
#if defined(ARDUINO_ARCH_ESP32)
    defaultfont = &lv_font_roboto_12;
#else
    defaultfont = LV_FONT_DEFAULT;
#endif

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

    /*
        lv_style_copy(&rel_style, &lv_style_btn_rel);
        rel_style.body.radius       = 0;
        rel_style.body.border.width = 1;
        rel_style.text.font         = LV_FONT_DEFAULT;

        lv_style_copy(&pr_style, &lv_style_btn_pr);
        pr_style.body.radius       = 0;
        pr_style.body.border.width = 1;
        rel_style.text.font        = LV_FONT_DEFAULT;
    */

    // lv_style_list_t * list;
    // static lv_style_t robotofont;
    // lv_style_init(&robotofont);
    // lv_style_set_text_font(&robotofont, LV_STATE_DEFAULT, defaultfont);

    oobepage[1] = lv_obj_create(NULL, NULL);

    /* Create the password box */
    lv_obj_t * pwd_ta = lv_textarea_create(oobepage[1], NULL);
    lv_obj_set_style_local_text_font(pwd_ta, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, defaultfont);

    lv_textarea_set_text(pwd_ta, "");
    lv_textarea_set_max_length(pwd_ta, 32);
    lv_textarea_set_pwd_mode(pwd_ta, true);
    lv_textarea_set_one_line(pwd_ta, true);
    lv_textarea_set_cursor_hidden(pwd_ta, true);
    lv_obj_set_user_data(pwd_ta, 20);
    lv_obj_set_width(pwd_ta, disp->driver.hor_res - leftmargin - 20);
    lv_obj_set_event_cb(pwd_ta, ta_event_cb);
    lv_obj_align(pwd_ta, NULL, LV_ALIGN_CENTER, leftmargin / 2, topmargin - voffset);

    Log.notice(F("Create the one-line mode text area"));
    /* Create the one-line mode text area */
    lv_obj_t * oneline_ta = lv_textarea_create(oobepage[1], pwd_ta);

    lv_obj_set_style_local_text_font(oneline_ta, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, defaultfont);

    lv_textarea_set_pwd_mode(oneline_ta, false);
    lv_obj_set_user_data(oneline_ta, 10);
    lv_obj_align(oneline_ta, pwd_ta, LV_ALIGN_OUT_TOP_MID, 0, topmargin);

    Log.notice(F("Create a label and position it above the text box"));
    /* Create a label and position it above the text box */
    lv_obj_t * pwd_label = lv_label_create(oobepage[1], NULL);
    lv_label_set_text(pwd_label, "Password:");
    lv_obj_align(pwd_label, pwd_ta, labelpos, 0, 0);

    Log.notice(F("Create a label and position it above the text box"));
    /* Create a label and position it above the text box */
    lv_obj_t * oneline_label = lv_label_create(oobepage[1], NULL);
    lv_label_set_text(oneline_label, "Ssid:");
    lv_obj_align(oneline_label, oneline_ta, labelpos, 0, 0);

    Log.notice(F("Create a keyboard"));
    /* Create a keyboard and make it fill the width of the above text areas */
    oobekb = lv_keyboard_create(oobepage[1], NULL);

    lv_obj_set_style_local_text_font(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, defaultfont);
    lv_obj_set_style_local_radius(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_inner(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_pad_left(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_right(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);

    // lv_obj_set_pos(oobekb, 5, 90);
    Log.notice(F("Create a keyboard CB"));
    lv_obj_set_event_cb(oobekb, kb_event_cb);
    /* Setting a custom event handler stops the keyboard from closing automatically */
    //    lv_keybard_add_(oobekb, LV_KEYBOARD_PART_BG, &lv_style_transp_tight);
    //    lv_keyboard_set_style(oobekb, LV_keyboard_STYLE_BTN_REL, &rel_style);
    //    lv_keyboard_set_style(oobekb, LV_keyboard_STYLE_BTN_PR, &pr_style);

    Log.notice(F("Create a keyboard TA link"));
    lv_keyboard_set_textarea(oobekb, oneline_ta); /* Focus it on one of the text areas to start */
    lv_keyboard_set_cursor_manage(oobekb, true);  /* Automatically show/hide cursors on text areas */
}

static void oobe_calibrate_cb(lv_obj_t * ta, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        if(oobeAutoCalibrate) {
            guiSetDim(100);
            guiCalibrate();
            oobeAutoCalibrate = false;
            lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
            lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), gotoPage1_cb);
        } else {
            lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
            lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), NULL);
        }
    }
}

void oobeSetup()
{
    char ssid[32];
    char pass[32];
    if(wifiShowAP(ssid, pass)) {
        guiSetDim(100);
        oobeSetupQR(ssid, pass);
        oobeSetupSsid();
        oobeSetPage(0);
    }

    if(oobeAutoCalibrate) {
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
        lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), oobe_calibrate_cb);
        Log.verbose(F("OOBE: Enabled Auto Calibrate on touch"));
    } else {
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
        lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), gotoPage1_cb);
        Log.verbose(F("OOBE: Already calibrated"));
    }
}