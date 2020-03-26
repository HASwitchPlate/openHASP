#include "ArduinoLog.h"

#include "lvgl.h"
#include "lv_conf.h"
#include "hasp.h"
#include "hasp_attr_set.h"

#define LVGL7 1

LV_FONT_DECLARE(unscii_8_icon);

lv_color_t haspLogColor(lv_color_t color)
{
    uint8_t r = (LV_COLOR_GET_R(color) * 263 + 7) >> 5;
    uint8_t g = (LV_COLOR_GET_G(color) * 259 + 3) >> 6;
    uint8_t b = (LV_COLOR_GET_B(color) * 263 + 7) >> 5;
    Log.trace(F("Color: R%u G%u B%u"), r, g, b);
    return color;
}

lv_color_t haspPayloadToColor(const char * payload)
{
    switch(strlen(payload)) {
        case 3:
            if(!strcmp_P(payload, PSTR("red"))) return haspLogColor(LV_COLOR_RED);
            break;
        case 4:
            if(!strcmp_P(payload, PSTR("aqua"))) return haspLogColor(LV_COLOR_AQUA);
            if(!strcmp_P(payload, PSTR("blue"))) return haspLogColor(LV_COLOR_BLUE);
            if(!strcmp_P(payload, PSTR("cyan"))) return haspLogColor(LV_COLOR_CYAN);
            if(!strcmp_P(payload, PSTR("gray"))) return haspLogColor(LV_COLOR_GRAY);
            if(!strcmp_P(payload, PSTR("lime"))) return haspLogColor(LV_COLOR_LIME);
            if(!strcmp_P(payload, PSTR("teal"))) return haspLogColor(LV_COLOR_TEAL);
            if(!strcmp_P(payload, PSTR("navy"))) return haspLogColor(LV_COLOR_NAVY);
            break;
        case 5:
            if(!strcmp_P(payload, PSTR("olive"))) return haspLogColor(LV_COLOR_OLIVE);
            if(!strcmp_P(payload, PSTR("green"))) return haspLogColor(LV_COLOR_GREEN);
            if(!strcmp_P(payload, PSTR("white"))) return haspLogColor(LV_COLOR_WHITE);
            if(!strcmp_P(payload, PSTR("black"))) return haspLogColor(LV_COLOR_BLACK);
            break;
        case 6:
            if(!strcmp_P(payload, PSTR("maroon"))) return haspLogColor(LV_COLOR_MAROON);
            if(!strcmp_P(payload, PSTR("orange"))) return haspLogColor(LV_COLOR_ORANGE);
            if(!strcmp_P(payload, PSTR("purple"))) return haspLogColor(LV_COLOR_PURPLE);
            if(!strcmp_P(payload, PSTR("silver"))) return haspLogColor(LV_COLOR_SILVER);
            if(!strcmp_P(payload, PSTR("yellow"))) return haspLogColor(LV_COLOR_YELLOW);
            break;
        case 7:
            if(!strcmp_P(payload, PSTR("magenta"))) return haspLogColor(LV_COLOR_MAGENTA);

            int r, g, b, a;
            if(*payload == '#' && sscanf(payload + 1, "%2x%2x%2x%2x", &r, &g, &b, &a) == 4) {
                return haspLogColor(LV_COLOR_MAKE(r, g, b));
            } else if(*payload == '#' && sscanf(payload + 1, "%2x%2x%2x", &r, &g, &b) == 3) {
                return haspLogColor(LV_COLOR_MAKE(r, g, b));
            }
        default:
            if(!strcmp_P(payload, PSTR("darkblue"))) return haspLogColor(LV_COLOR_MAKE(0, 51, 102));
            if(!strcmp_P(payload, PSTR("lightblue"))) return haspLogColor(LV_COLOR_MAKE(46, 203, 203));
            break;
    }

    Log.warning(F("Invalid color %s"), payload);
    return LV_COLOR_BLACK;
}

void haspAttributeNotFound(String & strAttr)
{
    Log.warning(F("HASP: Unknown property %s"), strAttr.c_str());
}

void set_cpicker_value(lv_obj_t * obj, const char * payload)
{
    lv_color_t color = haspPayloadToColor(payload);
    lv_cpicker_set_color(obj, color);
    // lv_cpicker_set_color(obj, lv_color_hex(color));
}

void haspSetLabelText(lv_obj_t * obj, const char * value)
{
    if(!obj) {
        Log.warning(F("HASP: Button not defined"));
        return;
    }

    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    if(label) {
        lv_obj_type_t list;
        lv_obj_get_type(label, &list);

        if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
            Log.verbose(F("HASP: Setting value to %s"), value);
            lv_label_set_text(label, value);
        }

    } else {
        Log.error(F("HASP: haspSetLabelText NULL Pointer encountered"));
    }
}

void haspSetOpacity(lv_obj_t * obj, uint8_t val)
{
#if LVGL7
    // Needs LV_USE_OPA_SCALE
    lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, val);
#else
    lv_obj_set_opa_scale_enable(obj, val < 255);
    lv_obj_set_opa_scale(obj, val < 255 ? val : 255);
#endif
}

void haspSetLocalStyle(lv_obj_t * obj, String & strAttr, String & strPayload)
{
    uint8_t part  = LV_TABLE_PART_BG;
    uint8_t state = LV_STATE_DEFAULT;
    int16_t var   = strPayload.toInt();
    // lv_color_t color = lv_color_hex((uint32_t)strPayload.toInt());
    // debugPrintln(strAttr + ":" + strPayload);

    // Check Trailing partnumber
    if(strAttr.endsWith(F("1"))) {
        part = LV_TABLE_PART_CELL1;
    } else if(strAttr.endsWith(F("2"))) {
        part = LV_TABLE_PART_CELL2;
    } else if(strAttr.endsWith(F("3"))) {
        part = LV_TABLE_PART_CELL3;
    } else if(strAttr.endsWith(F("4"))) {
        part = LV_TABLE_PART_CELL4;
        // } else if(strAttr.endsWith(F("9"))) {
        // part = LV_PAGE_PART_SCRL;
    }

    // Remove Trailing part digit
    if(part != LV_TABLE_PART_BG && strAttr.length() > 0) strAttr.remove(strAttr.length() - 1);
    // debugPrintln(strAttr + "&" + part);

    if(strAttr == F("radius")) {
        return lv_obj_set_style_local_radius(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("clip_corner")) {
        return lv_obj_set_style_local_clip_corner(obj, part, state, (bool)var);
    } else if(strAttr == F("size")) {
        return lv_obj_set_style_local_size(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("transform_width")) {
        return lv_obj_set_style_local_transform_width(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("transform_height")) {
        return lv_obj_set_style_local_transform_height(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("opa_scale")) {
        return lv_obj_set_style_local_opa_scale(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("pad_top")) {
        return lv_obj_set_style_local_pad_top(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("pad_bottom")) {
        return lv_obj_set_style_local_pad_bottom(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("pad_left")) {
        return lv_obj_set_style_local_pad_left(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("pad_right")) {
        return lv_obj_set_style_local_pad_right(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("pad_inner")) {
        return lv_obj_set_style_local_pad_inner(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("bg_blend_mode")) {
        return lv_obj_set_style_local_bg_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("bg_main_stop")) {
        return lv_obj_set_style_local_bg_main_stop(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("bg_grad_stop")) {
        return lv_obj_set_style_local_bg_grad_stop(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("bg_grad_dir")) {
        return lv_obj_set_style_local_bg_grad_dir(obj, part, state, (lv_grad_dir_t)var);
    } else if(strAttr == F("bg_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        if(part != 64)
            return lv_obj_set_style_local_bg_color(obj, part, state, color);
        else
            return lv_obj_set_style_local_bg_color(obj, LV_PAGE_PART_SCRL, LV_STATE_CHECKED, color);
    } else if(strAttr == F("bg_grad_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_bg_grad_color(obj, part, state, color);
    } else if(strAttr == F("bg_opa")) {
        return lv_obj_set_style_local_bg_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("border_width")) {
        return lv_obj_set_style_local_border_width(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("border_side")) {
        return lv_obj_set_style_local_border_side(obj, part, state, (lv_border_side_t)var);
    } else if(strAttr == F("border_blend_mode")) {
        return lv_obj_set_style_local_border_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("border_post")) {
        return lv_obj_set_style_local_border_post(obj, part, state, (bool)var);
    } else if(strAttr == F("border_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_border_color(obj, part, state, color);
    } else if(strAttr == F("border_opa")) {
        return lv_obj_set_style_local_border_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("outline_width")) {
        return lv_obj_set_style_local_outline_width(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("outline_pad")) {
        return lv_obj_set_style_local_outline_pad(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("outline_blend_mode")) {
        return lv_obj_set_style_local_outline_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("outline_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_outline_color(obj, part, state, color);
    } else if(strAttr == F("outline_opa")) {
        return lv_obj_set_style_local_outline_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("shadow_width")) {
        return lv_obj_set_style_local_shadow_width(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("shadow_ofs_x")) {
        return lv_obj_set_style_local_shadow_ofs_x(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("shadow_ofs_y")) {
        return lv_obj_set_style_local_shadow_ofs_y(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("shadow_spread")) {
        return lv_obj_set_style_local_shadow_spread(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("shadow_blend_mode")) {
        return lv_obj_set_style_local_shadow_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("shadow_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_shadow_color(obj, part, state, color);
    } else if(strAttr == F("shadow_opa")) {
        return lv_obj_set_style_local_shadow_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("pattern_repeat")) {
        return lv_obj_set_style_local_pattern_repeat(obj, part, state, (bool)var);
    } else if(strAttr == F("pattern_blend_mode")) {
        return lv_obj_set_style_local_pattern_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("pattern_recolor")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_pattern_recolor(obj, part, state, color);
    } else if(strAttr == F("pattern_opa")) {
        return lv_obj_set_style_local_pattern_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("pattern_recolor_opa")) {
        return lv_obj_set_style_local_pattern_recolor_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("pattern_image")) {
        //   return lv_obj_set_style_local_pattern_image(obj, part, state, (constvoid *)var);
    } else if(strAttr == F("value_letter_space")) {
        return lv_obj_set_style_local_value_letter_space(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("value_line_space")) {
        return lv_obj_set_style_local_value_line_space(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("value_blend_mode")) {
        return lv_obj_set_style_local_value_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("value_ofs_x")) {
        return lv_obj_set_style_local_value_ofs_x(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("value_ofs_y")) {
        return lv_obj_set_style_local_value_ofs_y(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("value_align")) {
        return lv_obj_set_style_local_value_align(obj, part, state, (lv_align_t)var);
    } else if(strAttr == F("value_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_value_color(obj, part, state, color);
    } else if(strAttr == F("value_opa")) {
        return lv_obj_set_style_local_value_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("value_font")) {
#if ESP32
        switch(var) {
            case 8:
                lv_obj_set_style_local_value_font(obj, part, state, &unscii_8_icon);
                break;
            case 12:
                lv_obj_set_style_local_value_font(obj, part, state, &lv_font_roboto_12);
                break;
            case 16:
                lv_obj_set_style_local_value_font(obj, part, state, &lv_font_roboto_16);
                break;
            case 22:
                lv_obj_set_style_local_value_font(obj, part, state, &lv_font_roboto_22);
                break;
            case 28:
                lv_obj_set_style_local_value_font(obj, part, state, &lv_font_roboto_28);
                break;
        }
        return;
#endif
        //    return lv_obj_set_style_local_value_font(obj, part, state, (constlv_font_t *)var);
    } else if(strAttr == F("value_str")) {
        return lv_obj_set_style_local_value_str(obj, part, state, (const char *)strPayload.c_str());
    } else if(strAttr == F("text_letter_space")) {
        return lv_obj_set_style_local_text_letter_space(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("text_line_space")) {
        return lv_obj_set_style_local_text_line_space(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("text_decor")) {
        return lv_obj_set_style_local_text_decor(obj, part, state, (lv_text_decor_t)var);
    } else if(strAttr == F("text_blend_mode")) {
        return lv_obj_set_style_local_text_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("text_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_text_color(obj, part, state, color);
    } else if(strAttr == F("text_sel_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_text_sel_color(obj, part, state, color);
    } else if(strAttr == F("text_opa")) {
        return lv_obj_set_style_local_text_opa(obj, part, state, (lv_opa_t)var);
    } else if(strAttr == F("text_font")) {
#if ESP32
        switch(var) {
            case 8:
                lv_obj_set_style_local_text_font(obj, part, state, &unscii_8_icon);
                break;
            case 12:
                lv_obj_set_style_local_text_font(obj, part, state, &lv_font_roboto_12);
                break;
            case 16:
                lv_obj_set_style_local_text_font(obj, part, state, &lv_font_roboto_16);
                break;
            case 22:
                lv_obj_set_style_local_text_font(obj, part, state, &lv_font_roboto_22);
                break;
            case 28:
                lv_obj_set_style_local_text_font(obj, part, state, &lv_font_roboto_28);
                break;
        }
        return;
#endif
        // return lv_obj_set_style_local_text_font(obj, part, state, (constlv_font_t *)var);
    } else if(strAttr == F("line_width")) {
        return lv_obj_set_style_local_line_width(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("line_blend_mode")) {
        return lv_obj_set_style_local_line_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(strAttr == F("line_dash_width")) {
        return lv_obj_set_style_local_line_dash_width(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("line_dash_gap")) {
        return lv_obj_set_style_local_line_dash_gap(obj, part, state, (lv_style_int_t)var);
    } else if(strAttr == F("line_rounded")) {
        return lv_obj_set_style_local_line_rounded(obj, part, state, (bool)var);
    } else if(strAttr == F("line_color")) {
        lv_color_t color = haspPayloadToColor(strPayload.c_str());
        return lv_obj_set_style_local_line_color(obj, part, state, color);
    } else if(strAttr == F("line_opa")) {
        return lv_obj_set_style_local_line_opa(obj, part, state, (lv_opa_t)var);
    }

    /* Property not found */
    haspAttributeNotFound(strAttr);
}

void haspSetObjAttribute1(lv_obj_t * obj, String & strAttr, String & strPayload)
{
    uint16_t val = (uint16_t)strPayload.toInt();

    if(strAttr == F("x")) {
        lv_obj_set_x(obj, val);
        return;
    } else if(strAttr == F("y")) {
        lv_obj_set_y(obj, val);
        return;
    } else if(strAttr == F("w")) {
        lv_obj_set_width(obj, val);
        return;
    } else if(strAttr == F("h")) {
        lv_obj_set_height(obj, val);
        return;
    }
    haspSetLocalStyle(obj, strAttr, strPayload);
}

void haspSetObjAttribute3(lv_obj_t * obj, String & strAttr, String & strPayload)
{
    uint16_t val = (uint16_t)strPayload.toInt();

    if(strAttr == F("vis")) {
        lv_obj_set_hidden(obj, val == 0);
        return;
    } else {
        /* .txt and .val depend on objecttype */
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(strAttr == F("txt")) { // In order of likelihood to occur
            if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                haspSetLabelText(obj, strPayload.c_str());
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
                lv_label_set_text(obj, strPayload.c_str());
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
#if LVGL7
                lv_checkbox_set_text(obj, strPayload.c_str());
#else
                lv_cb_set_text(obj, strPayload.c_str());
#endif
                return;
            }
        }

        if(strAttr == F("val")) { // In order of likelihood to occur
            int16_t intval = (int16_t)strPayload.toInt();

#if LVGL7
            if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                if(lv_btn_get_checkable(obj)) {
                    lv_btn_state_t state;
                    switch(val) {
                        case 0:
                            state = LV_BTN_STATE_RELEASED;
                            break;
                        case 1:
                            state = LV_BTN_STATE_CHECKED_RELEASED;
                            break;
                        default:
                            state = LV_BTN_STATE_DISABLED;
                    }
                    lv_btn_set_state(obj, state);
                    return;
                }
            } else if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
                lv_checkbox_set_checked(obj, val != 0);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_SWITCH)) {
                val == 0 ? lv_switch_off(obj, LV_ANIM_ON) : lv_switch_on(obj, LV_ANIM_ON);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                lv_dropdown_set_selected(obj, val);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LMETER)) {
                lv_linemeter_set_value(obj, intval);
                return;
#else
            if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                if(lv_btn_get_toggle(obj)) {
                    lv_btn_set_state(obj, val == 0 ? LV_BTN_STATE_REL : LV_BTN_STATE_TGL_REL);
                    return;
                }
            } else if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
                lv_cb_set_checked(obj, val != 0);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_SWITCH)) {
                val == 0 ? lv_sw_off(obj, LV_ANIM_ON) : lv_sw_on(obj, LV_ANIM_ON);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
                lv_ddlist_set_selected(obj, val);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LMETER)) {
                lv_lmeter_set_value(obj, intval);
                return;
#endif

            } else if(check_obj_type(list.type[0], LV_HASP_SLIDER)) {
                lv_slider_set_value(obj, intval, LV_ANIM_ON);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LED)) {
                lv_led_set_bright(obj, (uint8_t)val);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_GAUGE)) {
                lv_gauge_set_value(obj, 0, intval);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
                lv_roller_set_selected(obj, val, LV_ANIM_ON);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_BAR)) {
                lv_bar_set_value(obj, intval, LV_ANIM_OFF);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_CPICKER)) {
                set_cpicker_value(obj, strPayload.c_str());
                return;
            }
        }

        if(strAttr == F("min")) { // In order of likelihood to occur
            int16_t min = (int16_t)strPayload.toInt();
            if(check_obj_type(list.type[0], LV_HASP_SLIDER)) {
                int16_t max = lv_slider_get_max_value(obj);
                lv_slider_set_range(obj, min, max);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_GAUGE)) {
                int16_t max = lv_gauge_get_max_value(obj);
                lv_gauge_set_range(obj, min, max);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_BAR)) {
                int16_t max = lv_bar_get_max_value(obj);
                lv_bar_set_range(obj, min, max);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LMETER)) {
                int16_t max = lv_linemeter_get_max_value(obj);
                lv_linemeter_set_range(obj, min, max);
                return;
            }
        }

        if(strAttr == F("max")) { // In order of likelihood to occur
            int16_t max = (int16_t)strPayload.toInt();
            if(check_obj_type(list.type[0], LV_HASP_SLIDER)) {
                int16_t min = lv_slider_get_max_value(obj);
                lv_slider_set_range(obj, min, max);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_GAUGE)) {
                int16_t min = lv_gauge_get_max_value(obj);
                lv_gauge_set_range(obj, min, max);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_BAR)) {
                int16_t min = lv_bar_get_max_value(obj);
                lv_bar_set_range(obj, min, max);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LMETER)) {
                int16_t min = lv_linemeter_get_max_value(obj);
                lv_linemeter_set_range(obj, min, max);
                return;
            }
        }
    }
    haspSetLocalStyle(obj, strAttr, strPayload);
}

void haspSetObjAttribute4(lv_obj_t * obj, String & strAttr, String & strPayload)
{
    uint16_t val = (uint16_t)strPayload.toInt();

    if(strAttr == F("rows")) {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
            lv_roller_set_visible_row_count(obj, (uint8_t)val);
            return;
        }
    } else if(strAttr == F("rect")) {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(check_obj_type(list.type[0], LV_HASP_CPICKER)) {
            strPayload.toLowerCase();
            if(strPayload == F("true")) val = 1;
            lv_cpicker_set_type(obj, val ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
            return;
        }
    }

    haspSetLocalStyle(obj, strAttr, strPayload);
}

void haspSetObjAttribute6(lv_obj_t * obj, String & strAttr, String & strPayload)
{
    uint16_t val = (uint16_t)strPayload.toInt();

    if(strAttr == F("hidden")) {
        lv_obj_set_hidden(obj, val == 0);
        return;
    } else if(strAttr == F("toggle")) {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);
        if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
            haspSetToggle(obj, strPayload.toInt() > 0);
            return;
        }
    }
    haspSetLocalStyle(obj, strAttr, strPayload);
}

void haspSetObjAttribute7(lv_obj_t * obj, String & strAttr, String & strPayload)
{
    uint16_t val = (uint16_t)strPayload.toInt();

    if(strAttr == F("opacity")) {
        haspSetOpacity(obj, val);
        return;
    } else if(strAttr == F("enabled")) {
        lv_obj_set_click(obj, val != 0);
        return;
    } else if(strAttr == F("options")) {
        /* .options depend on objecttype */
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
#if LVGL7
            lv_dropdown_set_options(obj, strPayload.c_str());
#else
            lv_ddlist_set_options(obj, strPayload.c_str());
#endif
            return;
        } else if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
            lv_roller_ext_t * ext = (lv_roller_ext_t *)lv_obj_get_ext_attr(obj);
            lv_roller_set_options(obj, strPayload.c_str(), ext->mode);
            return;
        }
    }
    haspSetLocalStyle(obj, strAttr, strPayload);
}

void haspSetObjAttribute(lv_obj_t * obj, String strAttr, String strPayload)
{
    if(!obj) {
        Log.warning(F("HASP: Unknown object"));
        return;
    }

    if(strAttr.startsWith(".")) {
        strAttr.remove(0, 1);
    } else {
        // return haspAttributeNotFound(strAttr);
    }

    switch(strAttr.length()) {
        case 1:
            haspSetObjAttribute1(obj, strAttr, strPayload);
            break;
        case 4:
            haspSetObjAttribute4(obj, strAttr, strPayload);
            break;
        case 3:
            haspSetObjAttribute3(obj, strAttr, strPayload);
            break;
        case 6:
            haspSetObjAttribute6(obj, strAttr, strPayload);
            break;
        case 7:
            haspSetObjAttribute7(obj, strAttr, strPayload);
            break;
        default:
            haspSetLocalStyle(obj, strAttr, strPayload);
    }
}