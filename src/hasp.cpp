/*********************
 *      INCLUDES
 *********************/
#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"

#include "lvgl.h"
#include "lv_conf.h"
#include "lv_theme_hasp.h"
#include "lv_objx/lv_roller.h"

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

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_gui.h"
#include "hasp_tft.h"
#include "hasp.h"

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
static void keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event);
#if LV_USE_ANIMATION
static void kb_hide_anim_end(lv_anim_t * a);
#endif

void hasp_background(uint16_t pageid, uint16_t imageid);

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
static lv_font_t * haspFonts[4];
static lv_style_t labelStyles[4];
static lv_style_t rollerStyles[4];
#else
static lv_obj_t * pages[12];
static lv_font_t * haspFonts[8];
static lv_style_t labelStyles[8];
static lv_style_t rollerStyles[8];
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
void haspGetAttr(String hmiAttribute)
{ // Get the value of a Nextion component attribute
  // This will only send the command to the panel requesting the attribute, the actual
  // return of that value will be handled by nextionProcessInput and placed into mqttGetSubtopic
    /*Serial1.print("get " + hmiAttribute);
    Serial1.write(nextionSuffix, sizeof(nextionSuffix));*/
    debugPrintln(String(F("HMI OUT: 'get ")) + hmiAttribute + "'");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void haspProcessInput()
{
#if HASP_USE_MQTT > 0
    // mqttSend(topic, value);
#endif
}

void haspSendNewEvent(lv_obj_t * obj, uint8_t val)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
        // char buffer[127];
        // sprintf_P(buffer, PSTR("HASP: Send p[%u].b[%u].event=%d"), pageid, objid, val);
        // debugPrintln(buffer);

#if HASP_USE_MQTT > 0
        mqttSendNewEvent(pageid, objid, val);
#endif
    }
}

void haspSendNewValue(lv_obj_t * obj, int32_t val)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
        // char buffer[127];
        // sprintf_P(buffer, PSTR("HASP: Send p[%u].b[%u].val=%d"), pageid, objid, val);
        // debugPrintln(buffer);

#if HASP_USE_MQTT > 0
        mqttSendNewValue(pageid, objid, val);
#endif
    }
}

void haspSendNewValue(lv_obj_t * obj, String txt)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
        // char buffer[127];
        // sprintf_P(buffer, PSTR("HASP: Send p[%u].b[%u].txt='%s'"), pageid, objid, txt.c_str());
        // debugPrintln(buffer);

#if HASP_USE_MQTT > 0
        mqttSendNewValue(pageid, objid, txt);
#endif
    }
}

void haspSendNewValue(lv_obj_t * obj, const char * txt)
{
    uint8_t pageid;
    uint8_t objid;

    if(FindIdFromObj(obj, &pageid, &objid)) {
        // char buffer[127];
        // sprintf_P(buffer, PSTR("HASP: Send p[%u].b[%u].txt='%s'"), pageid, objid, txt);
        // debugPrintln(buffer);

#if HASP_USE_MQTT > 0
        mqttSendNewValue(pageid, objid, txt);
#endif
    }
}

void haspSendNewValue(lv_obj_t * obj, int16_t val)
{
    haspSendNewValue(obj, (int32_t)val);
}

uint32_t get_cpicker_value(lv_obj_t * obj)
{
    lv_color32_t c32;
    c32.full = lv_color_to32(lv_cpicker_get_color(obj));
    return (uint32_t)c32.full;
}
void set_cpicker_value(lv_obj_t * obj, uint32_t color)
{
    lv_cpicker_set_color(obj, lv_color_hex(color));
}

void haspSendNewValue(lv_obj_t * obj, lv_color_t color)
{
    haspSendNewValue(obj, (int32_t)get_cpicker_value(obj));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void haspSendCmd(String nextionCmd)
{ // Send a raw command to the Nextion panel
    /*Serial1.print(utf8ascii(nextionCmd));
    Serial1.write(nextionSuffix, sizeof(nextionSuffix));
    debugPrintln(String(F("HMI OUT: ")) + nextionCmd);*/
}

bool check_obj_type(const char * lvobjtype, lv_hasp_obj_type_t haspobjtype)
{
    switch(haspobjtype) {
        case LV_HASP_BUTTON:
            return (strcmp_P(lvobjtype, PSTR("lv_btn")) == 0);
        case LV_HASP_LABEL:
            return (strcmp_P(lvobjtype, PSTR("lv_label")) == 0);
        case LV_HASP_CHECKBOX:
            return (strcmp_P(lvobjtype, PSTR("lv_cb")) == 0);
        case LV_HASP_DDLIST:
            return (strcmp_P(lvobjtype, PSTR("lv_ddlist")) == 0);
        case LV_HASP_CPICKER:
            return (strcmp_P(lvobjtype, PSTR("lv_cpicker")) == 0);
        case LV_HASP_PRELOADER:
            return (strcmp_P(lvobjtype, PSTR("lv_preloader")) == 0);
        case LV_HASP_SLIDER:
            return (strcmp_P(lvobjtype, PSTR("lv_slider")) == 0);
        case LV_HASP_GAUGE:
            return (strcmp_P(lvobjtype, PSTR("lv_gauge")) == 0);
        case LV_HASP_BAR:
            return (strcmp_P(lvobjtype, PSTR("lv_bar")) == 0);
        case LV_HASP_LMETER:
            return (strcmp_P(lvobjtype, PSTR("lv_lmeter")) == 0);
        case LV_HASP_ROLLER:
            return (strcmp_P(lvobjtype, PSTR("lv_roller")) == 0);
        case LV_HASP_SWITCH:
            return (strcmp_P(lvobjtype, PSTR("lv_sw")) == 0);
        case LV_HASP_LED:
            return (strcmp_P(lvobjtype, PSTR("lv_led")) == 0);
        default:
            return false;
    }
}

bool haspGetLabelText(lv_obj_t * obj, std::string & strPayload)
{
    if(!obj) {
        errorPrintln(F("HASP: %sButton not defined"));
        return false;
    }

    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    if(label) {
        lv_obj_type_t list;
        lv_obj_get_type(label, &list);

        if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
            strPayload = lv_label_get_text(label);
            return true;
        }

    } else {
        warningPrintln(F("HASP: %shaspGetLabelText NULL Pointer encountered"));
    }

    return false;
}

void haspSetLabelText(lv_obj_t * obj, const char * value)
{
    if(!obj) {
        errorPrintln(F("HASP: %sButton not defined"));
        return;
    }

    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    if(label) {
        lv_obj_type_t list;
        lv_obj_get_type(label, &list);

        if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
            debugPrintln(String(F("HASP: Setting value to ")) + String(value));
            lv_label_set_text(label, value);
        }

    } else {
        warningPrintln(F("HASP: %shaspSetLabelText NULL Pointer encountered"));
    }
}

bool haspGetObjAttribute(lv_obj_t * obj, String strAttr, std::string & strPayload)
{
    if(!obj) return false;
    uint16_t val = 0;

    switch(strAttr.length()) {
        case 2:
            if(strAttr == F(".x")) {
                val = lv_obj_get_x(obj);
            } else if(strAttr == F(".y")) {
                val = lv_obj_get_y(obj);
            } else if(strAttr == F(".w")) {
                val = lv_obj_get_width(obj);
            } else if(strAttr == F(".h")) {
                val = lv_obj_get_height(obj);
            } else {
                return false;
            }
            strPayload = String(val).c_str();
            return true;
        case 4:
            if(strAttr == F(".vis")) {
                strPayload = String(!lv_obj_get_hidden(obj)).c_str();
                return true;
            } else {
                /* .txt and .val depend on objecttype */
                lv_obj_type_t list;
                lv_obj_get_type(obj, &list);

                if(strAttr == F(".txt")) {
                    if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                        return haspGetLabelText(obj, strPayload);
                    }
                    if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
                        strPayload = lv_label_get_text(obj);
                        return true;
                    }
                    if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
                        strPayload = lv_cb_get_text(obj);
                        return true;
                    }
                    if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                        char buffer[127];
                        lv_ddlist_get_selected_str(obj, buffer, sizeof(buffer));
                        strPayload = String(buffer).c_str();
                        return true;
                    }
                    if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
                        char buffer[127];
                        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
                        strPayload = String(buffer).c_str();
                        return true;
                    }
                    return false;
                }

                if(strAttr == F(".val")) {
                    if(check_obj_type(list.type[0], LV_HASP_PRELOADER)) return false;

                    if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                        if(lv_btn_get_state(obj) == LV_BTN_STATE_TGL_PR ||
                           lv_btn_get_state(obj) == LV_BTN_STATE_TGL_REL)
                            strPayload = "1"; // It's toggled
                        else
                            strPayload = "0"; // Normal btn has no toggle state
                    }

                    if(check_obj_type(list.type[0], LV_HASP_SLIDER))
                        strPayload = String(lv_slider_get_value(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_GAUGE))
                        strPayload = String(lv_gauge_get_value(obj, 0)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_BAR)) strPayload = String(lv_bar_get_value(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_LMETER))
                        strPayload = String(lv_lmeter_get_value(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_CPICKER))
                        strPayload = String(get_cpicker_value(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_CHECKBOX))
                        strPayload = String(!lv_cb_is_checked(obj) ? 0 : 1).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_DDLIST))
                        strPayload = String(lv_ddlist_get_selected(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_ROLLER))
                        strPayload = String(lv_roller_get_selected(obj)).c_str();

                    if(check_obj_type(list.type[0], LV_HASP_LED)) strPayload = String(lv_led_get_bright(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_SWITCH)) strPayload = String(lv_sw_get_state(obj)).c_str();

                    return true;
                }
            }
            break;
        case 7:
            if(strAttr == F(".hidden")) {
                strPayload = String(!lv_obj_get_hidden(obj)).c_str();
                return true;
            } else {
                return false;
            }
            return false;
        case 8:
            if(strAttr == F(".opacity")) {
                strPayload = String(!lv_obj_get_opa_scale_enable(obj) ? 255 : lv_obj_get_opa_scale(obj)).c_str();
            } else if(strAttr == F(".enabled")) {
                strPayload = String(lv_obj_get_click(obj)).c_str();
            } else if(strAttr == F(".options")) {
                /* options depend on objecttype */
                lv_obj_type_t list;
                lv_obj_get_type(obj, &list);

                if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                    strPayload = lv_ddlist_get_options(obj);
                    return true;
                }
                if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
                    strPayload = lv_roller_get_options(obj);
                    return true;
                }
                return false;
            }
            break;
        default:
            errorPrintln(F("HASP: %sUnknown property"));
            return false;
    }
    return false;
}

void haspSetObjAttribute(lv_obj_t * obj, String strAttr, String strPayload)
{
    if(!obj) return;
    uint16_t val = (uint16_t)strPayload.toInt();

    switch(strAttr.length()) {
        case 2:
            if(strAttr == F(".x")) {
                lv_obj_set_x(obj, val);
                return;
            } else if(strAttr == F(".y")) {
                lv_obj_set_y(obj, val);
                return;
            } else if(strAttr == F(".w")) {
                lv_obj_set_width(obj, val);
                return;
            } else if(strAttr == F(".h")) {
                lv_obj_set_height(obj, val);
                return;
            }
            break;
        case 4:
            if(strAttr == F(".vis")) {
                lv_obj_set_hidden(obj, val == 0);
                return;
            } else {
                /* .txt and .val depend on objecttype */
                lv_obj_type_t list;
                lv_obj_get_type(obj, &list);

                if(strAttr == F(".txt")) { // In order of likelihood to occur
                    if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                        haspSetLabelText(obj, strPayload.c_str());
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
                        lv_label_set_text(obj, strPayload.c_str());
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
                        lv_cb_set_text(obj, strPayload.c_str());
                        return;
                    }
                }

                if(strAttr == F(".val")) { // In order of likelihood to occur
                    int16_t intval = (int16_t)strPayload.toInt();

                    if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                        if(lv_btn_get_toggle(obj)) {
                            lv_btn_set_state(obj, val == 0 ? LV_BTN_STATE_REL : LV_BTN_STATE_TGL_REL);
                            return;
                        }

                    } else if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
                        lv_cb_set_checked(obj, val != 0);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_SLIDER)) {
                        lv_slider_set_value(obj, intval, LV_ANIM_ON);
                        return;
                    }

                    else if(check_obj_type(list.type[0], LV_HASP_SWITCH)) {
                        val == 0 ? lv_sw_off(obj, LV_ANIM_ON) : lv_sw_on(obj, LV_ANIM_ON);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_LED)) {
                        lv_led_set_bright(obj, (uint8_t)val);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_GAUGE)) {
                        lv_gauge_set_value(obj, 0, intval);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                        lv_ddlist_set_selected(obj, val);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
                        lv_roller_set_selected(obj, val, LV_ANIM_ON);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_BAR)) {
                        lv_bar_set_value(obj, intval, LV_ANIM_OFF);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_LMETER)) {
                        lv_lmeter_set_value(obj, intval);
                        return;
                    } else if(check_obj_type(list.type[0], LV_HASP_CPICKER)) {
                        set_cpicker_value(obj, (uint32_t)strPayload.toInt());
                        return;
                    }
                }
            }
            break;
        case 7:
            if(strAttr == F(".hidden")) {
                lv_obj_set_hidden(obj, val == 0);
                return;
            }
            break;
        case 8:
            if(strAttr == F(".opacity")) {
                lv_obj_set_opa_scale_enable(obj, val < 255);
                lv_obj_set_opa_scale(obj, val < 255 ? val : 255);
                return;
            } else if(strAttr == F(".enabled")) {
                lv_obj_set_click(obj, val != 0);
                return;
            } else if(strAttr == F(".options")) {
                /* .options depend on objecttype */
                lv_obj_type_t list;
                lv_obj_get_type(obj, &list);

                if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                    lv_ddlist_set_options(obj, strPayload.c_str());
                    return;
                } else if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
                    lv_roller_ext_t * ext = (lv_roller_ext_t *)lv_obj_get_ext_attr(obj);
                    lv_roller_set_options(obj, strPayload.c_str(), ext->mode);
                    return;
                }
            }
            break;
    }
    errorPrintln(F("HASP: %sUnknown property"));
}

void haspProcessAttribute(uint8_t pageid, uint8_t objid, String strAttr, String strPayload)
{
    lv_obj_t * obj = FindObjFromId((uint8_t)pageid, (uint8_t)objid);
    if(obj) {
        if(strPayload != "")
            haspSetObjAttribute(obj, strAttr, strPayload);
        else {
            /* publish the change */
            std::string strValue = "";
            if(haspGetObjAttribute(obj, strAttr, strValue)) {
                mqttSendNewValue(pageid, objid, String(strValue.c_str()));
            } else {
                warningPrintln(String(F("HASP: %sUnknown property: ")) + strAttr);
            }
        } // payload
    }     // obj
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static int16_t get_obj_id(lv_obj_t * obj)
{
    lv_obj_t * page    = lv_scr_act();
    int16_t id         = -1;
    lv_obj_t * thisobj = NULL;
    while(obj != thisobj && id < 256) {
        thisobj = lv_obj_get_child_back(page, thisobj);
        id++;
    }
    if(obj != thisobj) {
        return -1;
    } else {
        return id;
    }
}

static int16_t get_obj(uint8_t pageid, uint8_t objid, lv_obj_t * obj)
{
    lv_obj_t * page    = lv_scr_act();
    int16_t id         = -1;
    lv_obj_t * thisobj = NULL;
    while(obj != thisobj && id < 256) {
        thisobj = lv_obj_get_child_back(page, thisobj);
        id++;
    }
    if(obj != thisobj) {
        return -1;
    } else {
        return id;
    }
}
*/

/**
 * Connection lost GUI
 */

void haspWakeUp()
{
    lv_disp_trig_activity(NULL);
}

void haspDisconnect()
{
    /* Create a dark plain style for a message box's background (modal)*/
    lv_style_copy(&style_mbox_bg, &lv_style_plain);
    style_mbox_bg.body.main_color = LV_COLOR_BLACK;
    style_mbox_bg.body.grad_color = LV_COLOR_BLACK;
    style_mbox_bg.body.opa        = LV_OPA_60;

    lv_obj_set_style(lv_disp_get_layer_sys(NULL), &style_mbox_bg);
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), true);
    /*
        lv_obj_t * obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), NULL);
        lv_obj_set_hidden(obj, false);
        obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), obj);
        lv_obj_set_hidden(obj, false);*/
}

void haspReconnect()
{
    /*Revert the top layer to not block*/
    lv_obj_set_style(lv_disp_get_layer_sys(NULL), &lv_style_transp);
    lv_obj_set_click(lv_disp_get_layer_sys(NULL), false);
    /*
        lv_obj_t * obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), NULL);
        lv_obj_set_hidden(obj, true);
        obj = lv_obj_get_child(lv_disp_get_layer_sys(NULL), obj);
        lv_obj_set_hidden(obj, true);*/
}

void haspDisplayAP(const char * ssid, const char * pass)
{
    guiSetDim(100);

    String txt((char *)0);
    txt.reserve(64);

    char buffer[127];
    sprintf_P(buffer, PSTR("WIFI:S:%s;T:WPA;P:%s;;"), ssid, pass);

    /*Clear all screens*/
    for(uint8_t i = 0; i < (sizeof pages / sizeof *pages); i++) {
        lv_obj_clean(pages[i]);
    }

#if HASP_USE_QRCODE != 0
    lv_obj_t * qr = lv_qrcode_create(lv_disp_get_layer_sys(NULL), 120, LV_COLOR_BLACK, LV_COLOR_WHITE);
    lv_obj_align(qr, NULL, LV_ALIGN_CENTER, 0, 10);
    lv_qrcode_update(qr, buffer, strlen(buffer));
#endif

    lv_obj_t * panel = lv_cont_create(lv_disp_get_layer_sys(NULL), NULL);
    lv_obj_set_style(panel, &lv_style_pretty);
    lv_obj_align(panel, qr, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_label_set_align(panel, LV_LABEL_ALIGN_CENTER);
    lv_cont_set_fit(panel, LV_FIT_TIGHT);
    lv_cont_set_layout(panel, LV_LAYOUT_COL_M);

    txt                = String(LV_SYMBOL_WIFI) + String(ssid);
    lv_obj_t * network = lv_label_create(panel, NULL);
    lv_label_set_text(network, ssid);

    lv_obj_t * password = lv_label_create(panel, NULL);
    txt                 = String(LV_SYMBOL_WIFI) + String(pass);
    lv_label_set_text(password, txt.c_str());
}

static void kb_event_cb(lv_obj_t * event_kb, lv_event_t event)
{
    /* Just call the regular event handler */
    lv_kb_def_event_cb(event_kb, event);
}
static void ta_event_cb(lv_obj_t * ta, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if(kb != NULL) lv_kb_set_ta(kb, ta);
    }

    else if(event == LV_EVENT_INSERT) {
        const char * str = (const char *)lv_event_get_data();
        if(str[0] == '\n') {
            printf("Ready\n");
        } else {
            printf(lv_ta_get_text(ta));
            printf("\n");
        }
    }
}

void haspFirstSetup(void)
{
    /*Create styles for the keyboard*/
    static lv_style_t rel_style, pr_style;

    lv_style_copy(&rel_style, &lv_style_btn_rel);
    rel_style.body.radius       = 0;
    rel_style.body.border.width = 1;

    lv_style_copy(&pr_style, &lv_style_btn_pr);
    pr_style.body.radius       = 0;
    pr_style.body.border.width = 1;

    /* Create the password box */
    lv_obj_t * pwd_ta = lv_ta_create(lv_disp_get_layer_sys(NULL), NULL);
    lv_ta_set_text(pwd_ta, "");
    lv_ta_set_pwd_mode(pwd_ta, true);
    lv_ta_set_one_line(pwd_ta, true);
    lv_obj_set_width(pwd_ta, LV_HOR_RES - 20);
    lv_obj_set_pos(pwd_ta, 5, 20);
    lv_obj_set_event_cb(pwd_ta, ta_event_cb);
    lv_obj_align(pwd_ta, NULL, LV_ALIGN_OUT_TOP_MID, 0, 140);

    /* Create a label and position it above the text box */
    lv_obj_t * pwd_label = lv_label_create(lv_disp_get_layer_sys(NULL), NULL);
    lv_label_set_text(pwd_label, "Password:");
    lv_obj_align(pwd_label, pwd_ta, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

    /* Create the one-line mode text area */
    lv_obj_t * oneline_ta = lv_ta_create(lv_disp_get_layer_sys(NULL), pwd_ta);
    lv_ta_set_pwd_mode(oneline_ta, false);
    lv_ta_set_cursor_type(oneline_ta, LV_CURSOR_LINE | LV_CURSOR_HIDDEN);
    lv_obj_align(oneline_ta, NULL, LV_ALIGN_OUT_TOP_MID, 0, 100);

    /* Create a label and position it above the text box */
    lv_obj_t * oneline_label = lv_label_create(lv_disp_get_layer_sys(NULL), NULL);
    lv_label_set_text(oneline_label, "Ssid:");
    lv_obj_align(oneline_label, oneline_ta, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

    /* Create a keyboard and make it fill the width of the above text areas */
    kb = lv_kb_create(lv_disp_get_layer_sys(NULL), NULL);
    // lv_obj_set_pos(kb, 5, 90);
    lv_obj_set_event_cb(kb,
                        kb_event_cb); /* Setting a custom event handler stops the keyboard from closing automatically */
    // lv_obj_set_size(kb, LV_HOR_RES, 140);
    lv_kb_set_style(kb, LV_KB_STYLE_BG, &lv_style_transp_tight);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &rel_style);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &pr_style);

    lv_kb_set_ta(kb, pwd_ta);          /* Focus it on one of the text areas to start */
    lv_kb_set_cursor_manage(kb, true); /* Automatically show/hide cursors on text areas */
}

/**
 * Create a demo application
 */
void haspSetup(JsonObject settings)
{
    haspSetConfig(settings);

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

    if(lv_zifont_font_init(&defaultFont, haspZiFontPath, 24) != 0) {
        errorPrintln(String(F("HASP: %sFailed to set the custom font to ")) + String(haspZiFontPath));
        defaultFont = NULL; // Use default font
    }

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
            th = lv_theme_material_init(haspThemeHue, defaultFont);
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
    lv_theme_set_current(th);

    /*Create a screen*/
    for(uint8_t i = 0; i < (sizeof pages / sizeof *pages); i++) {
        pages[i] = lv_obj_create(NULL, NULL);
        // lv_obj_set_size(pages[0], hres, vres);
    }

    for(uint8_t i = 0; i < (sizeof pages / sizeof *pages); i++) {
        pages[i] = lv_obj_create(NULL, NULL);
        // lv_obj_set_size(pages[0], hres, vres);
    }

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
        lv_style_copy(&labelStyles[i], &lv_style_pretty_color);
        labelStyles[i].text.font  = haspFonts[i];
        labelStyles[i].text.color = LV_COLOR_BLUE;
    }

    /*
    lv_obj_t * obj;

        obj = lv_label_create(lv_layer_sys(), NULL);
        lv_obj_set_size(obj, hres, 30);
        lv_obj_set_pos(obj, lv_disp_get_hor_res(NULL) / 2 - 40, lv_disp_get_ver_res(NULL) / 2 + 40 + 20);
        lv_label_set_text(obj, String(F("#ffffff " LV_SYMBOL_WIFI " Connecting... #")).c_str());
        lv_label_set_recolor(obj, true);

        obj = lv_btn_create(lv_layer_sys(), NULL);
        lv_obj_set_size(obj, 80, 80);
        lv_obj_set_pos(obj, lv_disp_get_hor_res(NULL) / 2 - 40, lv_disp_get_ver_res(NULL) / 2 - 40 - 20);
    */
    haspDisconnect();
    haspLoadPage(haspPagesPath);
    haspSetPage(haspStartPage);
    guiSetDim(haspStartDim);

    // lv_obj_t * img_bin_t = lv_img_create(pages[current_page], NULL); /*Crate an image object*/
    // lv_img_set_src(img_bin_t, "F:/dogsmall(1).bin");                 /*Set the created file as image (a red rose)*/
    // lv_obj_set_pos(img_bin_t, 64, 64);                               /*Set the positions*/

    // // lv_page_set_style(page, LV_PAGE_STYLE_SB, &style_sb);           /*Set the scrollbar style*/

    // // lv_obj_t *img1 = lv_img_create(pages[2], NULL);
    // // lv_img_set_src(img1, &xmass);

    // // lv_obj_set_protect(page1, LV_PROTECT_POS);
    // // lv_obj_set_protect(page2, LV_PROTECT_POS);

    // /*Create a page*/
    // /*	lv_obj_t * page1 = lv_obj_create(screen, NULL);
    // lv_obj_set_size(page1, hres, vres);
    // lv_obj_align(page1, NULL, LV_ALIGN_CENTER, 0, 0);
    // //lv_page_set_style(page, LV_PAGE_STYLE_SB, &style_sb);           /*Set the scrollbar style*/

    // /* default style*/
    // static lv_style_t style_btnm_rel;                /* Une variable pour,!enregistrer le style normal */
    // lv_style_copy(&style_btnm_rel, &lv_style_plain); /* Initialise a partir d un,!style included */
    // style_btnm_rel.body.border.color   = lv_color_hex3(0x269);
    // style_btnm_rel.body.border.width   = 1;
    // style_btnm_rel.body.padding.inner  = -5;
    // style_btnm_rel.body.padding.left   = 0;
    // style_btnm_rel.body.padding.right  = 0;
    // style_btnm_rel.body.padding.bottom = 0;
    // style_btnm_rel.body.padding.top    = 0;
    // style_btnm_rel.body.main_color     = lv_color_hex3(0xADF);
    // style_btnm_rel.body.grad_color     = lv_color_hex3(0x46B);
    // style_btnm_rel.body.shadow.width   = 0;
    // style_btnm_rel.body.radius         = 0;
    // // style_btnm_rel.body.shadow.type = LV_SHADOW_BOTTOM;
    // // style_btnm_rel.body.radius = LV_RADIUS_CIRCLE;
    // style_btnm_rel.text.color = lv_color_hex3(0xDEF);

    // static lv_style_t style_btnm_pr;                /* Une variable pour,!enregistrer le style pressed */
    // lv_style_copy(&style_btnm_pr, &style_btnm_rel); /* Initialise a partir du,!style released */
    // style_btnm_pr.body.border.width = 1;
    // style_btnm_pr.body.border.color = lv_color_hex3(0x46B);
    // style_btnm_pr.body.main_color   = lv_color_hex3(0x8BD);
    // style_btnm_pr.body.grad_color   = lv_color_hex3(0x24A);
    // style_btnm_pr.body.shadow.width = 0;
    // style_btnm_pr.body.radius       = 0;
    // style_btnm_pr.text.color        = lv_color_hex3(0xBCD);

    // lv_obj_t * btnm1 = lv_btnm_create(pages[0], NULL);
    // /* style should set before map */
    // lv_btnm_set_style(btnm1, LV_BTN_STYLE_REL, &style_btnm_rel);     /* Dfinit le style,!released du bouton */
    // lv_btnm_set_style(btnm1, LV_BTN_STYLE_PR, &style_btnm_pr);       /* Dfinit le style,!pressed du bouton */
    // lv_btnm_set_style(btnm1, LV_BTN_STYLE_TGL_REL, &style_btnm_rel); /* Dfinit le style,!released du bouton */
    // lv_btnm_set_style(btnm1, LV_BTN_STYLE_TGL_PR, &style_btnm_pr);   /* Dfinit le style,!pressed du bouton */

    // lv_btnm_set_map(btnm1, btnm_map1);

    // lv_btnm_set_style(btnm1, LV_BTNM_STYLE_BG, &lv_style_transp);
    // lv_obj_set_size(btnm1, hres, vres);
    // // lv_btnm_set_style(btnm1, LV_BTNM_STYLE, &style)
    // // lv_btnm_set_btn_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
    // lv_obj_align(btnm1, NULL, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_event_cb(btnm1, btnmap_event_handler);

    // lv_obj_t * btnm2 = lv_btnm_create(pages[1], NULL);
    // /* style should set before map */
    // lv_btnm_set_style(btnm2, LV_BTNM_STYLE_BG, &lv_style_transp);
    // lv_btnm_set_map(btnm2, btnm_map2);
    // lv_obj_set_size(btnm2, hres, vres);
    // // lv_btnm_set_style(btnm1, LV_BTNM_STYLE, &style)
    // // lv_btnm_set_btn_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
    // lv_obj_align(btnm2, NULL, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_event_cb(btnm2, btnmap_event_handler);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void haspLoop(void)
{
    /* idle detection and dim */
}

void hasp_background(uint16_t pageid, uint16_t imageid)
{
    lv_obj_t * page = get_page(pageid);
    if(!page) return;

    return;

    page               = lv_scr_act();
    lv_obj_t * thisobj = lv_obj_get_child_back(page, NULL);

    if(!thisobj) return;

    /*
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
*/
    lv_img_set_auto_size(thisobj, false);
    lv_obj_set_width(thisobj, lv_disp_get_hor_res(NULL));
    lv_obj_set_height(thisobj, lv_disp_get_ver_res(NULL));
    // lv_obj_set_protect(wp, LV_PROTECT_POS);
    // lv_obj_invalidate(thisobj);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a a list button is clicked on the List tab
 * @param btn pointer to a list button
 * @param event type of event that occured
 */
static void btn_event_handler(lv_obj_t * obj, lv_event_t event)
{
    // int16_t id = get_obj_id(obj);

    uint8_t eventid = 0;
    uint8_t pageid  = 0;
    lv_obj_user_data_t objid;

    char buffer[127];
    sprintf(buffer, PSTR("HASP: "));

    if(!FindIdFromObj(obj, &pageid, &objid)) {
        errorPrintln(F("HASP: %sEvent for unknown object"));
        return;
    }

    switch(event) {
        case LV_EVENT_PRESSED:
            strcat_P(buffer, PSTR("Pressed Down"));
            debugPrintln(buffer);
            eventid = 1;
            break;

        case LV_EVENT_CLICKED:
            strcat_P(buffer, PSTR("Released Up"));
            debugPrintln(buffer);
            // UP = the same object was release then was pressed and press was not lost!
            eventid = 0;
            break;

        case LV_EVENT_SHORT_CLICKED:
            strcat_P(buffer, PSTR("Short Click"));
            debugPrintln(buffer);
            eventid = 2;
            break;

        case LV_EVENT_LONG_PRESSED:
            strcat_P(buffer, PSTR("Long Press"));
            debugPrintln(buffer);
            eventid = 3;
            break;

        case LV_EVENT_LONG_PRESSED_REPEAT:
            strcat_P(buffer, PSTR("Long Hold"));
            debugPrintln(buffer);
            eventid = 4;
            break;

        case LV_EVENT_PRESS_LOST:
            strcat_P(buffer, PSTR("Lost Press"));
            debugPrintln(buffer);
            eventid = 9;
            break;

        case LV_EVENT_PRESSING:
        case LV_EVENT_FOCUSED:
        case LV_EVENT_DEFOCUSED:
        case LV_EVENT_RELEASED:
            return;

        case LV_EVENT_VALUE_CHANGED:
            strcat_P(buffer, PSTR("Value Changed"));
            debugPrintln(buffer);
            return;

        default:
            sprintf(buffer, PSTR("HASP: Unknown Event %d occured"), event);
            debugPrintln(buffer);
            return;
    }

    // printf("p[%u].b[%u].val = %u  ", pageid, objid, eventid);
    mqttSendNewEvent(pageid, objid, eventid);
}

static void btnmap_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) haspSendNewValue(obj, lv_btnm_get_pressed_btn(obj));
}

static void slider_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) haspSendNewValue(obj, lv_slider_get_value(obj));
}

static void cpicker_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) haspSendNewValue(obj, lv_cpicker_get_color(obj));
}

static void switch_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) haspSendNewValue(obj, lv_sw_get_state(obj));
}

static void checkbox_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) haspSendNewValue(obj, lv_cb_is_checked(obj));
}

static void ddlist_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        haspSendNewValue(obj, lv_ddlist_get_selected(obj));
        char buffer[127];
        lv_ddlist_get_selected_str(obj, buffer, sizeof(buffer));
        haspSendNewValue(obj, String(buffer));
    }
}

static void roller_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        haspSendNewValue(obj, lv_roller_get_selected(obj));
        char buffer[127];
        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
        haspSendNewValue(obj, String(buffer));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void haspSetNodename(String name)
{}

String haspGetNodename()
{
    return String(F("plate11"));
}

String haspGetVersion()
{
    char buffer[127];
    snprintf_P(buffer, sizeof(buffer), "%u.%u.%u", HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);
    return buffer;
}

uint16_t haspGetPage()
{
    return current_page;
}

void haspSetPage(uint16_t pageid)
{
    lv_obj_t * page = get_page(pageid);
    if(!page) {
        errorPrintln(F("HASP: %sPage ID not defined"));
    } else if(page == lv_layer_sys() || page == lv_layer_top()) {
        errorPrintln(F("HASP: %sCannot change to a layer"));
    } else {
        debugPrintln(String(F("HASP: Changing page to ")) + String(pageid));
        lv_scr_load(page);
        current_page = pageid;
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
        errorPrintln(F("HASP: %sPage ID not defined"));
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
            errorPrintln(F("HASP: %sParent ID not found"));
            parent_obj = page;
        } else {
            debugPrintln(F("HASP: Parent ID found"));
        }
    }

    /* Input cache and validation */
    int16_t min = config[F("min")].as<int16_t>();
    int16_t max = config[F("max")].as<int16_t>();
    int16_t val = config[F("val")].as<int16_t>();
    if(min >= max) {
        min = 0;
        max = 100;
    }
    bool enabled      = config[F("enable")].as<bool>() | true;
    lv_coord_t width  = config[F("w")].as<lv_coord_t>();
    lv_coord_t height = config[F("h")].as<lv_coord_t>();
    if(width == 0) width = 32;
    if(height == 0) height = 32;
    uint8_t objid   = config[F("objid")].as<uint8_t>();
    uint8_t id      = config[F("id")].as<uint8_t>();
    uint8_t styleid = config[F("styleid")].as<uint8_t>();

    /* Define Objects*/
    lv_obj_t * obj;
    lv_obj_t * label;
    switch(objid) {
        /* ----- Basic Objects ------ */
        case LV_HASP_BUTTON: {
            obj         = lv_btn_create(parent_obj, NULL);
            bool toggle = config[F("toggle")].as<bool>();
            lv_btn_set_toggle(obj, toggle);
            if(config[F("txt")]) {
                label = lv_label_create(obj, NULL);
                lv_label_set_text(label, config[F("txt")].as<String>().c_str());
                lv_obj_set_opa_scale_enable(label, true);
                lv_obj_set_opa_scale(label, LV_OPA_COVER);
            }
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_CHECKBOX: {
            obj = lv_cb_create(parent_obj, NULL);
            if(config[F("txt")]) lv_cb_set_text(obj, config[F("txt")].as<String>().c_str());
            lv_obj_set_event_cb(obj, checkbox_event_handler);
            break;
        }
        case LV_HASP_LABEL: {
            obj = lv_label_create(parent_obj, NULL);
            if(config[F("txt")]) {
                lv_label_set_text(obj, config[F("txt")].as<String>().c_str());
            }
            if(styleid < sizeof labelStyles / sizeof *labelStyles) {
                debugPrintln(String(F("HASP: Styleid set to ")) + styleid);
                // lv_label_set_style(obj, LV_LABEL_STYLE_MAIN, &labelStyles[styleid]);
            }
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
            bool rect = config[F("rect")].as<bool>();
            lv_cpicker_set_type(obj, rect ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
            lv_obj_set_event_cb(obj, cpicker_event_handler);
            break;
        }
#if LV_USE_PRELOAD != 0
        case LV_HASP_PRELOADER: {
            obj = lv_preload_create(parent_obj, NULL);
            break;
        }
#endif
        /* ----- Range Objects ------ */
        case LV_HASP_SLIDER: {
            obj = lv_slider_create(parent_obj, NULL);
            lv_slider_set_range(obj, min, max);
            lv_slider_set_value(obj, val, LV_ANIM_OFF);
            lv_obj_set_event_cb(obj, slider_event_handler);
            // bool knobin = config[F("knobin")].as<bool>() | true;
            // lv_slider_set_knob_in(obj, knobin);
            break;
        }
        case LV_HASP_GAUGE: {
            obj = lv_gauge_create(parent_obj, NULL);
            lv_gauge_set_range(obj, min, max);
            lv_gauge_set_value(obj, val, LV_ANIM_OFF);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_BAR: {
            obj = lv_bar_create(parent_obj, NULL);
            lv_bar_set_range(obj, min, max);
            lv_bar_set_value(obj, val, LV_ANIM_OFF);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_LMETER: {
            obj = lv_lmeter_create(parent_obj, NULL);
            lv_lmeter_set_range(obj, min, max);
            lv_lmeter_set_value(obj, val);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }

        /* ----- On/Off Objects ------ */
        case LV_HASP_SWITCH: {
            obj        = lv_sw_create(parent_obj, NULL);
            bool state = config[F("val")].as<bool>();
            if(state) lv_sw_on(obj, LV_ANIM_OFF);
            lv_obj_set_event_cb(obj, switch_event_handler);
            break;
        }
        case LV_HASP_LED: {
            obj = lv_led_create(parent_obj, NULL);
            lv_led_set_bright(obj, config[F("val")].as<uint8_t>() | 0);
            lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
            /**/
        case LV_HASP_DDLIST: {
            obj = lv_ddlist_create(parent_obj, NULL);
            if(config[F("txt")]) lv_ddlist_set_options(obj, config[F("txt")].as<String>().c_str());
            lv_ddlist_set_selected(obj, val);
            lv_ddlist_set_fix_width(obj, width);
            lv_ddlist_set_draw_arrow(obj, true);
            lv_ddlist_set_anim_time(obj, 250);
            lv_obj_set_top(obj, true);
            // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
            lv_obj_set_event_cb(obj, ddlist_event_handler);
            break;
        }
        case LV_HASP_ROLLER: {
            obj           = lv_roller_create(parent_obj, NULL);
            bool infinite = config[F("infinite")].as<bool>();
            if(config[F("txt")]) lv_roller_set_options(obj, config[F("txt")].as<String>().c_str(), infinite);
            lv_roller_set_selected(obj, val, LV_ANIM_ON);
            lv_roller_set_fix_width(obj, width);
            lv_roller_set_visible_row_count(obj, config[F("rows")].as<uint8_t>());
            // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
            lv_obj_set_event_cb(obj, roller_event_handler);
            break;
        }

        /* ----- Other Object ------ */
        default:
            errorPrintln(F("HASP: %sUnsupported Object ID"));
            return;
    }

    if(!obj) {
        errorPrintln(F("HASP: %sObject is NULL"));
        return;
    }

    if(!config[F("opacity")].isNull()) {
        uint8_t opacity = config[F("opacity")].as<uint8_t>();
        lv_obj_set_opa_scale_enable(obj, opacity < 255);
        lv_obj_set_opa_scale(obj, opacity < 255 ? opacity : 255);
    }

    bool hidden = config[F("hidden")].as<bool>();
    lv_obj_set_hidden(obj, hidden);
    lv_obj_set_click(obj, enabled);

    lv_obj_set_pos(obj, config[F("x")].as<lv_coord_t>(), config[F("y")].as<lv_coord_t>());
    lv_obj_set_width(obj, width);
    if(objid != LV_HASP_DDLIST && objid != LV_HASP_ROLLER)
        lv_obj_set_height(obj, height); // ddlist and roller have auto height

    lv_obj_set_user_data(obj, id);

    /** testing start **/
    lv_obj_user_data_t temp;
    if(!FindIdFromObj(obj, &pageid, &temp)) {
        errorPrintln(F("HASP: %sLost track of the created object, not found!"));
        return;
    }
    /** testing end **/

    char msg[127];
    sprintf_P(msg, PSTR("HASP: Created object p[%u].b[%u]"), pageid, temp);
    debugPrintln(msg);

    /* Double-check */
    lv_obj_t * test = FindObjFromId(pageid, (uint8_t)temp);
    if(test == obj) {
        debugPrintln(F("Objects match!"));
    } else {
        errorPrintln(F("HASP: %sObjects DO NOT match!"));
    }
}

void haspLoadPage(String pages)
{
    char msg[127];

    if(!SPIFFS.begin()) {
        errorPrintln(String(F("HASP: %sFS not mounted. Failed to load ")) + pages.c_str());
        return;
    }

    if(!SPIFFS.exists(pages)) {
        errorPrintln(String(F("HASP: %sNon existing file ")) + pages.c_str());
        return;
    }

    sprintf_P(msg, PSTR("HASP: Loading file %s"), pages.c_str());
    debugPrintln(msg);

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

    sprintf_P(msg, PSTR("HASP: File %s loaded"), pages.c_str());
    debugPrintln(msg);

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

    serializeJson(settings, Serial);
    Serial.println();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool haspSetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(!settings[FPSTR(F_CONFIG_PAGES)].isNull()) {
        changed |= strcmp(haspPagesPath, settings[FPSTR(F_CONFIG_PAGES)]) != 0;
        strncpy(haspPagesPath, settings[FPSTR(F_CONFIG_PAGES)], sizeof(haspPagesPath));
    }

    if(!settings[FPSTR(F_CONFIG_ZIFONT)].isNull()) {
        changed |= strcmp(haspZiFontPath, settings[FPSTR(F_CONFIG_ZIFONT)]) != 0;
        strncpy(haspZiFontPath, settings[FPSTR(F_CONFIG_ZIFONT)], sizeof(haspZiFontPath));
    }

    if(!settings[FPSTR(F_CONFIG_STARTPAGE)].isNull()) {
        changed |= haspStartPage != settings[FPSTR(F_CONFIG_STARTPAGE)].as<uint8_t>();
        haspStartPage = settings[FPSTR(F_CONFIG_STARTPAGE)].as<uint8_t>();
    }

    if(!settings[FPSTR(F_CONFIG_STARTDIM)].isNull()) {
        changed |= haspStartDim != settings[FPSTR(F_CONFIG_STARTDIM)].as<uint8_t>();
        haspStartDim = settings[FPSTR(F_CONFIG_STARTDIM)].as<uint8_t>();
    }

    if(!settings[FPSTR(F_CONFIG_THEME)].isNull()) {
        changed |= haspThemeId != settings[FPSTR(F_CONFIG_THEME)].as<uint8_t>();
        haspThemeId = settings[FPSTR(F_CONFIG_THEME)].as<uint8_t>();
    }

    if(!settings[FPSTR(F_CONFIG_HUE)].isNull()) {
        changed |= haspThemeHue != settings[FPSTR(F_CONFIG_HUE)].as<uint16_t>();
        haspThemeHue = settings[FPSTR(F_CONFIG_HUE)].as<uint16_t>();
    }

    serializeJson(settings, Serial);
    Serial.println();

    return changed;
}