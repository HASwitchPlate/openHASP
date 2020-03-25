#include "Arduino.h"
#include "ArduinoLog.h"

#include "lvgl.h"
#include "hasp.h"
#include "hasp_attr_get.h"

#define LVGL7 1

lv_opa_t haspGetOpacity(lv_obj_t * obj)
{
#if LVGL7
    return lv_obj_get_style_opa_scale(obj, LV_OBJ_PART_MAIN);
#else
    return (!lv_obj_get_opa_scale_enable(obj) ? 255 : lv_obj_get_opa_scale(obj))
#endif
}

uint32_t get_cpicker_value(lv_obj_t * obj)
{
    lv_color32_t c32;
    c32.full = lv_color_to32(lv_cpicker_get_color(obj));
    return (uint32_t)c32.full;
}

bool haspGetLabelText(lv_obj_t * obj, std::string & strPayload)
{
    if(!obj) {
        Log.warning(F("HASP: Button not defined"));
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
        Log.warning(F("HASP: haspGetLabelText NULL Pointer encountered"));
    }

    return false;
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
#if LVGL7
                        strPayload = lv_checkbox_get_text(obj);
#else
                        strPayload = lv_cb_get_text(obj);
#endif
                        return true;
                    }
                    if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                        char buffer[128];
#if LVGL7
                        lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
#else
                        lv_ddlist_get_selected_str(obj, buffer, sizeof(buffer));
#endif
                        strPayload = String(buffer).c_str();
                        return true;
                    }
                    if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
                        char buffer[128];
                        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
                        strPayload = String(buffer).c_str();
                        return true;
                    }
                    return false;
                }

                if(strAttr == F(".val")) {
                    if(check_obj_type(list.type[0], LV_HASP_PRELOADER)) return false;

                    if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {

#if LVGL7
                        if(lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_PRESSED ||
                           lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED)
#else
                        if(lv_btn_get_state(obj) == LV_BTN_STATE_TGL_PR ||
                           lv_btn_get_state(obj) == LV_BTN_STATE_TGL_REL)
#endif
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
#if LVGL7
                        strPayload = String(lv_linemeter_get_value(obj)).c_str();
#else
                        strPayload = String(lv_lmeter_get_value(obj)).c_str();
#endif
                    if(check_obj_type(list.type[0], LV_HASP_CPICKER))
                        strPayload = String(get_cpicker_value(obj)).c_str();
                    if(check_obj_type(list.type[0], LV_HASP_CHECKBOX))
#if LVGL7
                        strPayload = String(!lv_checkbox_is_checked(obj) ? 0 : 1).c_str();
#else
                        strPayload = String(!lv_cb_is_checked(obj) ? 0 : 1).c_str();
#endif
                    if(check_obj_type(list.type[0], LV_HASP_DDLIST))
#if LVGL7
                        strPayload = String(lv_dropdown_get_selected(obj)).c_str();
#else
                        strPayload = String(lv_ddlist_get_selected(obj)).c_str();
#endif
                    if(check_obj_type(list.type[0], LV_HASP_ROLLER))
                        strPayload = String(lv_roller_get_selected(obj)).c_str();

                    if(check_obj_type(list.type[0], LV_HASP_LED)) strPayload = String(lv_led_get_bright(obj)).c_str();
#if LVGL7
                    if(check_obj_type(list.type[0], LV_HASP_SWITCH))
                        strPayload = String(lv_switch_get_state(obj)).c_str();
#else
                    if(check_obj_type(list.type[0], LV_HASP_SWITCH)) strPayload = String(lv_sw_get_state(obj)).c_str();
#endif

                    return true;
                }
            }
            break;
        case 6:
            if(strAttr == F(".color")) {
                lv_color_t color;
                lv_obj_get_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &color);
                strPayload = String(lv_color_to32(lv_cpicker_get_color(obj))).c_str();
                return true;
            } else {
                return false;
            }
            return false;
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
                strPayload = String(haspGetOpacity(obj)).c_str();
            } else if(strAttr == F(".enabled")) {
                strPayload = String(lv_obj_get_click(obj)).c_str();
            } else if(strAttr == F(".options")) {
                /* options depend on objecttype */
                lv_obj_type_t list;
                lv_obj_get_type(obj, &list);

                if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
#if LVGL7
                    strPayload = lv_dropdown_get_options(obj);
#else
                    strPayload = lv_ddlist_get_options(obj);
#endif
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
            Log.warning(F("HASP: Unknown property %s"), strAttr.c_str());
            return false;
    }
    return false;
}