/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_CONFIG > 0

#include "hasp_conf.h"

#include "lvgl.h"
#if LVGL_VERSION_MAJOR != 7
#include "../lv_components.h"
#endif

#include "hasp_object.h"
#include "hasp_gui.h"
#include "hasp_wifi.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"

static bool oobeAutoCalibrate = true;

#if HASP_USE_WIFI > 0

#if HASP_USE_QRCODE > 0
#include "lv_qrcode.h"
#endif

static lv_obj_t * oobepage[2];
static lv_obj_t * oobekb;
extern lv_font_t * defaultFont;

lv_obj_t * pwd_ta;

static inline void oobeSetPage(uint8_t pageid)
{
    lv_scr_load(oobepage[pageid]);
    lv_obj_invalidate(lv_disp_get_layer_sys(NULL));
}

void gotoPage1_cb(lv_obj_t * event_kb, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED) {
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
        oobeSetPage(1);
    }
}

static void peek_password_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,
                                         lv_btn_get_state(obj) ? LV_SYMBOL_EYE_OPEN : LV_SYMBOL_EYE_CLOSE);
        lv_textarea_set_pwd_mode(pwd_ta, !lv_btn_get_state(obj));
    }
}

static void kb_event_cb(lv_obj_t * event_kb, lv_event_t event)
{
    if(event == LV_EVENT_APPLY) {
        DynamicJsonDocument settings(256);
        char ssid[32];
        char pass[32];
        lv_obj_t * obj;

        obj = hasp_find_obj_from_parent_id(oobepage[1], (uint8_t)10);
        if(obj) {
            strncpy(ssid, lv_textarea_get_text(obj), sizeof(ssid));
            settings[FPSTR(F_CONFIG_SSID)] = ssid;
            if(oobekb != NULL) lv_keyboard_set_textarea(oobekb, obj);
        }

        obj = hasp_find_obj_from_parent_id(oobepage[1],(uint8_t) 20);
        if(obj) {
            strncpy(pass, lv_textarea_get_text(obj), sizeof(pass));
            settings[FPSTR(F_CONFIG_PASS)] = pass;
        }

        if(strlen(ssid) > 0) {
            if(wifiValidateSsid(ssid, pass)) {
                wifiSetConfig(settings.as<JsonObject>());
                Log.notice(TAG_OOBE, F("SSID validated, rebooting..."));
                dispatch_reboot(true);
            }
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
            lv_obj_t * obj;

            obj = hasp_find_obj_from_parent_id(oobepage[1], (uint8_t)10);
            if(ta == obj) { // now ssid, goto pass
                obj = hasp_find_obj_from_parent_id(oobepage[1], (uint8_t)20);
            }

            if(oobekb && obj) {
                lv_keyboard_set_textarea(oobekb, obj);
            }

        } else {
            // printf("%s\n", lv_ta_get_text(ta));
        }
    }
}

static void oobeSetupQR(const char * ssid, const char * pass)
{
    lv_disp_t * disp = lv_disp_get_default();
    oobepage[0]      = lv_obj_create(NULL, NULL);
    char buffer[128];
    lv_obj_t * container = lv_cont_create(oobepage[0], NULL);
    lv_obj_set_pos(container, 5, 5);
    lv_obj_set_style_local_bg_opa(container, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_opa(container, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);

#if HASP_USE_QRCODE > 0
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI:S:%s;T:WPA;P:%s;;"), ssid, pass);

    lv_obj_t * qr = lv_qrcode_create(oobepage[0], 120, LV_COLOR_BLACK, LV_COLOR_WHITE);
    lv_qrcode_update(qr, buffer, strlen(buffer));

    lv_obj_t * qrlabel = lv_label_create(oobepage[0], NULL);
    snprintf(buffer, sizeof(buffer), PSTR("Scan to connect"));
    lv_label_set_text(qrlabel, buffer);

    if(disp->driver.hor_res <= disp->driver.ver_res) {
        lv_obj_align(qr, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
        lv_obj_set_size(container, disp->driver.hor_res - 10, disp->driver.ver_res - 10 - 125);
        lv_obj_align(qrlabel, container, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    } else {
        lv_obj_align(qr, NULL, LV_ALIGN_IN_RIGHT_MID, -5, 0);
        lv_obj_set_size(container, disp->driver.hor_res - 10 - 125, disp->driver.ver_res - 10);
        lv_obj_align(qrlabel, qr, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    }

#else

    lv_obj_set_size(container, disp->driver.hor_res, disp->driver.ver_res);
#endif

    lv_obj_t * aplabel = lv_label_create(container, NULL);
    snprintf(buffer, sizeof(buffer), PSTR("Tap the screen to setup WiFi or connect to this Access Point:"));
    lv_label_set_text(aplabel, buffer);
    lv_label_set_long_mode(aplabel, LV_LABEL_LONG_BREAK);

    lv_obj_set_width(aplabel, lv_obj_get_width(container));
    lv_label_set_align(aplabel, LV_LABEL_ALIGN_CENTER);

    lv_obj_align(aplabel, container, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t * panel = lv_cont_create(container, NULL);
    lv_obj_align(panel, aplabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_cont_set_fit(panel, LV_FIT_TIGHT);

    lv_cont_set_layout(panel, LV_LAYOUT_COLUMN_MID);

    String txt((char *)0);
    txt.reserve(64);

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
    defaultfont = &lv_font_montserrat_12;
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

    oobepage[1] = lv_obj_create(NULL, NULL);
    char buffer[32];

    /* Create the password box */
    pwd_ta = lv_textarea_create(oobepage[1], NULL);
    lv_obj_set_style_local_text_font(pwd_ta, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, defaultfont);

    lv_textarea_set_text(pwd_ta, "");
    lv_textarea_set_max_length(pwd_ta, 32);
    lv_textarea_set_pwd_mode(pwd_ta, true);
    lv_textarea_set_one_line(pwd_ta, true);
    lv_textarea_set_cursor_hidden(pwd_ta, true);
    lv_obj_user_data_t udata = (lv_obj_user_data_t){20, 1, 0};
    lv_obj_set_user_data(pwd_ta, udata);
    lv_obj_set_width(pwd_ta, disp->driver.hor_res - leftmargin - 20 - lv_obj_get_height(pwd_ta));
    lv_obj_set_event_cb(pwd_ta, ta_event_cb);
    lv_obj_align(pwd_ta, NULL, LV_ALIGN_CENTER, leftmargin / 2 - lv_obj_get_height(pwd_ta) / 2, topmargin - voffset);

    lv_obj_t * pwd_icon = lv_btn_create(oobepage[1], NULL);
    lv_obj_set_size(pwd_icon, lv_obj_get_height(pwd_ta), lv_obj_get_height(pwd_ta));
    lv_obj_set_pos(pwd_icon, lv_obj_get_x(pwd_ta) + lv_obj_get_width(pwd_ta), lv_obj_get_y(pwd_ta));
    lv_obj_set_style_local_value_str(pwd_icon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_EYE_CLOSE);
    lv_obj_set_style_local_radius(pwd_icon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,
                                  lv_obj_get_style_radius(pwd_ta, LV_TEXTAREA_PART_BG));
    lv_obj_set_event_cb(pwd_icon, peek_password_cb);
    lv_btn_set_checkable(pwd_icon, true);

    /* Create the one-line mode text area */
    lv_obj_t * oneline_ta = lv_textarea_create(oobepage[1], pwd_ta);

    lv_obj_set_style_local_text_font(oneline_ta, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, defaultfont);

    lv_textarea_set_pwd_mode(oneline_ta, false);
    lv_obj_set_user_data(oneline_ta, (lv_obj_user_data_t){10, 1, 0});
    lv_obj_align(oneline_ta, pwd_ta, LV_ALIGN_OUT_TOP_MID, 0, topmargin);

    /* Create a label and position it above the text box */
    lv_obj_t * pwd_label = lv_label_create(oobepage[1], NULL);
    snprintf(buffer, sizeof(buffer), PSTR("Password:"));
    lv_label_set_text(pwd_label, buffer);
    lv_obj_align(pwd_label, pwd_ta, labelpos, 0, 0);

    /* Create a label and position it above the text box */
    lv_obj_t * oneline_label = lv_label_create(oobepage[1], NULL);
    snprintf(buffer, sizeof(buffer), PSTR("Ssid:"));
    lv_label_set_text(oneline_label, buffer);
    lv_obj_align(oneline_label, oneline_ta, labelpos, 0, 0);

    /* Create a keyboard and make it fill the width of the above text areas */
#if LVGL_VERSION_MAJOR == 8
    oobekb = lv_keyboard_create(oobepage[1]);
#else
    oobekb      = lv_keyboard_create(oobepage[1], NULL);
#endif

    lv_obj_set_style_local_pad_inner(oobekb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(oobekb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_width(oobekb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_radius(oobekb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_text_font(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, defaultfont);
    lv_obj_set_style_local_radius(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_color(oobekb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_obj_set_style_local_border_color(oobekb, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, LV_COLOR_PURPLE);
    lv_obj_set_style_local_bg_color(oobekb, LV_KEYBOARD_PART_BTN, LV_BTN_STATE_PRESSED, LV_COLOR_PURPLE);
    // lv_obj_set_style_local_pad_inner(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 1);
    // lv_obj_set_style_local_pad_left(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    // lv_obj_set_style_local_pad_right(oobekb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);

    // lv_obj_set_pos(oobekb, 5, 90);
    lv_obj_set_event_cb(oobekb, kb_event_cb);
    /* Setting a custom event handler stops the keyboard from closing automatically */
    //    lv_keybard_add_(oobekb, LV_KEYBOARD_PART_BG, &lv_style_transp_tight);
    //    lv_keyboard_set_style(oobekb, LV_keyboard_STYLE_BTN_REL, &rel_style);
    //    lv_keyboard_set_style(oobekb, LV_keyboard_STYLE_BTN_PR, &pr_style);

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
#endif // HASP_USE_WIFI

void oobeSetAutoCalibrate(bool cal)
{
    oobeAutoCalibrate = cal;
}

bool oobeSetup()
{
#if HASP_USE_ETHERNET > 0
    if(eth_connected) return false;
#endif
#if HASP_USE_WIFI > 0
    char ssid[32];
    char pass[32];

    if(wifiShowAP(ssid, pass)) {
        guiSetDim(100);
        oobeSetupQR(ssid, pass);
        oobeSetupSsid();

        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
        if(oobeAutoCalibrate) {
            lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), oobe_calibrate_cb);
            Log.trace(TAG_OOBE, F("Enabled Auto Calibrate on touch"));
        } else {
            lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), gotoPage1_cb);
            Log.trace(TAG_OOBE, F("Already calibrated"));
        }
        oobeSetPage(0);
        return true;
    } else {
        return false;
    }
#endif
    return false;
}

// Thist is used for testing only !!
void oobeFakeSetup(const char *, const char *)
{
#if HASP_USE_WIFI > 0
    char ssid[32] = "HASP-ABCDEF";
    char pass[32] = "haspadmin";

    guiSetDim(100);
    oobeSetupQR(ssid, pass);
    oobeSetupSsid();
    oobeSetPage(0);
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
    lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), gotoPage1_cb);

    if(oobeAutoCalibrate) {
        lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
        lv_obj_set_event_cb(lv_disp_get_layer_sys(NULL), oobe_calibrate_cb);
        Log.trace(TAG_OOBE, F("Enabled Auto Calibrate on touch"));
    } else {
        Log.trace(TAG_OOBE, F("Already calibrated"));
    }
#endif
}
#endif // HASP_USE_CONFIG