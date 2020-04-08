#include "Arduino.h"
#include "ArduinoLog.h"

#include "lvgl.h"
#include "hasp.h"
//#include "hasp_attr_get.h"

#define LVGL7 1

bool haspGetObjAttribute(lv_obj_t * obj, String strAttr, std::string & strPayload)
{
    if(!obj) return false;

    switch(strAttr.length()) {
        case 4:

            /* .txt and .val depend on objecttype */
            lv_obj_type_t list;
            lv_obj_get_type(obj, &list);

            if(strAttr == F(".val")) {
                if(check_obj_type(list.type[0], LV_HASP_PRELOADER)) return false;

                if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {

                    if(lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_PRESSED ||
                       lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED)
                        strPayload = "1"; // It's toggled
                    else
                        strPayload = "0"; // Normal btn has no toggle state
                }

                if(check_obj_type(list.type[0], LV_HASP_SLIDER)) strPayload = String(lv_slider_get_value(obj)).c_str();
                if(check_obj_type(list.type[0], LV_HASP_GAUGE)) strPayload = String(lv_gauge_get_value(obj, 0)).c_str();
                if(check_obj_type(list.type[0], LV_HASP_BAR)) strPayload = String(lv_bar_get_value(obj)).c_str();
                if(check_obj_type(list.type[0], LV_HASP_LMETER))
                    strPayload = String(lv_linemeter_get_value(obj)).c_str();
                // if(check_obj_type(list.type[0], LV_HASP_CPICKER)) strPayload =
                // String(lv_cpicker_get_color(obj)).c_str();
                if(check_obj_type(list.type[0], LV_HASP_CHECKBOX))
                    strPayload = String(!lv_checkbox_is_checked(obj) ? 0 : 1).c_str();
                if(check_obj_type(list.type[0], LV_HASP_DDLIST))
                    strPayload = String(lv_dropdown_get_selected(obj)).c_str();
                if(check_obj_type(list.type[0], LV_HASP_ROLLER))
                    strPayload = String(lv_roller_get_selected(obj)).c_str();

                if(check_obj_type(list.type[0], LV_HASP_LED)) strPayload = String(lv_led_get_bright(obj)).c_str();
                if(check_obj_type(list.type[0], LV_HASP_SWITCH)) strPayload = String(lv_switch_get_state(obj)).c_str();
            }
        }
    return false;
}