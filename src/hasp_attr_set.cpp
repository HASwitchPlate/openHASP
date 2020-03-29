#include "ArduinoLog.h"

#include "lvgl.h"
#include "lv_conf.h"
#include "hasp.h"
#include "hasp_attr_set.h"

#define LVGL7 1

LV_FONT_DECLARE(unscii_8_icon);

static inline bool is_true(const char * s)
{
    return (!strcmp_P(s, PSTR("true")) || !strcmp_P(s, PSTR("TRUE")) || !strcmp_P(s, PSTR("1")));
}

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
            if(!strcmp_P(payload, PSTR("blue"))) return haspLogColor(LV_COLOR_BLUE);
            if(!strcmp_P(payload, PSTR("cyan"))) return haspLogColor(LV_COLOR_CYAN);
            if(!strcmp_P(payload, PSTR("gray"))) return haspLogColor(LV_COLOR_GRAY);
            if(!strcmp_P(payload, PSTR("aqua"))) return haspLogColor(LV_COLOR_AQUA);
            if(!strcmp_P(payload, PSTR("lime"))) return haspLogColor(LV_COLOR_LIME);
            if(!strcmp_P(payload, PSTR("teal"))) return haspLogColor(LV_COLOR_TEAL);
            if(!strcmp_P(payload, PSTR("navy"))) return haspLogColor(LV_COLOR_NAVY);
            break;
        case 5:
            if(!strcmp_P(payload, PSTR("green"))) return haspLogColor(LV_COLOR_GREEN);
            if(!strcmp_P(payload, PSTR("white"))) return haspLogColor(LV_COLOR_WHITE);
            if(!strcmp_P(payload, PSTR("black"))) return haspLogColor(LV_COLOR_BLACK);
            if(!strcmp_P(payload, PSTR("olive"))) return haspLogColor(LV_COLOR_OLIVE);
            break;
        case 6:
            if(!strcmp_P(payload, PSTR("yellow"))) return haspLogColor(LV_COLOR_YELLOW);
            if(!strcmp_P(payload, PSTR("orange"))) return haspLogColor(LV_COLOR_ORANGE);
            if(!strcmp_P(payload, PSTR("purple"))) return haspLogColor(LV_COLOR_PURPLE);
            if(!strcmp_P(payload, PSTR("silver"))) return haspLogColor(LV_COLOR_SILVER);
            if(!strcmp_P(payload, PSTR("maroon"))) return haspLogColor(LV_COLOR_MAROON);
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

static void haspAttributeNotFound(const char * attr)
{
    Log.warning(F("HASP: Unknown property %s"), attr);
}

void set_cpicker_value(lv_obj_t * obj, const char * payload)
{
    lv_color_t color = haspPayloadToColor(payload);
    lv_cpicker_set_color(obj, color);
}

void set_label_long_mode(lv_obj_t * obj, const char * payload)
{
    lv_label_long_mode_t mode;
    if(!strcmp_P(payload, PSTR("expand"))) {
        mode = LV_LABEL_LONG_EXPAND;
    } else if(!strcmp_P(payload, PSTR("break"))) {
        mode = LV_LABEL_LONG_BREAK;
    } else if(!strcmp_P(payload, PSTR("dots"))) {
        mode = LV_LABEL_LONG_DOT;
    } else if(!strcmp_P(payload, PSTR("scroll"))) {
        mode = LV_LABEL_LONG_SROLL;
    } else if(!strcmp_P(payload, PSTR("loop"))) {
        mode = LV_LABEL_LONG_SROLL_CIRC;
    }
    lv_label_set_long_mode(obj, mode);
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

void haspSetLocalStyle(lv_obj_t * obj, const char * attr_p, const char * payload)
{
    uint8_t part  = LV_TABLE_PART_BG;
    uint8_t state = LV_STATE_DEFAULT;
    int16_t var   = atoi(payload);

    int len = strlen(attr_p);

    if(len > 0) {
        // Check Trailing partnumber
        if(attr_p[len - 1] == '1') {
            part = LV_TABLE_PART_CELL1;
        } else if(attr_p[len - 1] == '2') {
            part = LV_TABLE_PART_CELL2;
        } else if(attr_p[len - 1] == '3') {
            part = LV_TABLE_PART_CELL3;
        } else if(attr_p[len - 1] == '4') {
            part = LV_TABLE_PART_CELL4;
            // } else if(attr[len - 1] == '9') {
            // part = LV_PAGE_PART_SCRL;
        }
    }

    // Remove Trailing part digit
    char attr[128];
    if(part != LV_TABLE_PART_BG && len > 0) {
        strncpy(attr, attr_p, len - 1);
    } else {
        strncpy(attr, attr_p, len);
    }

    // debugPrintln(strAttr + "&" + part);

    if(!strcmp_P(attr, PSTR("radius"))) {
        return lv_obj_set_style_local_radius(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("clip_corner"))) {
        return lv_obj_set_style_local_clip_corner(obj, part, state, (bool)var);
    } else if(!strcmp_P(attr, PSTR("size"))) {
        return lv_obj_set_style_local_size(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("transform_width"))) {
        return lv_obj_set_style_local_transform_width(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("transform_height"))) {
        return lv_obj_set_style_local_transform_height(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("opa_scale"))) {
        return lv_obj_set_style_local_opa_scale(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("pad_top"))) {
        return lv_obj_set_style_local_pad_top(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("pad_bottom"))) {
        return lv_obj_set_style_local_pad_bottom(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("pad_left"))) {
        return lv_obj_set_style_local_pad_left(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("pad_right"))) {
        return lv_obj_set_style_local_pad_right(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("pad_inner"))) {
        return lv_obj_set_style_local_pad_inner(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("bg_blend_mode"))) {
        return lv_obj_set_style_local_bg_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("bg_main_stop"))) {
        return lv_obj_set_style_local_bg_main_stop(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("bg_grad_stop"))) {
        return lv_obj_set_style_local_bg_grad_stop(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("bg_grad_dir"))) {
        return lv_obj_set_style_local_bg_grad_dir(obj, part, state, (lv_grad_dir_t)var);
    } else if(!strcmp_P(attr, PSTR("bg_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        if(part != 64)
            return lv_obj_set_style_local_bg_color(obj, part, state, color);
        else
            return lv_obj_set_style_local_bg_color(obj, LV_PAGE_PART_SCRL, LV_STATE_CHECKED, color);
    } else if(!strcmp_P(attr, PSTR("bg_grad_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_bg_grad_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("bg_opa"))) {
        return lv_obj_set_style_local_bg_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("border_width"))) {
        return lv_obj_set_style_local_border_width(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("border_side"))) {
        return lv_obj_set_style_local_border_side(obj, part, state, (lv_border_side_t)var);
    } else if(!strcmp_P(attr, PSTR("border_blend_mode"))) {
        return lv_obj_set_style_local_border_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("border_post"))) {
        return lv_obj_set_style_local_border_post(obj, part, state, (bool)var);
    } else if(!strcmp_P(attr, PSTR("border_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_border_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("border_opa"))) {
        return lv_obj_set_style_local_border_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("outline_width"))) {
        return lv_obj_set_style_local_outline_width(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("outline_pad"))) {
        return lv_obj_set_style_local_outline_pad(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("outline_blend_mode"))) {
        return lv_obj_set_style_local_outline_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("outline_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_outline_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("outline_opa"))) {
        return lv_obj_set_style_local_outline_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("shadow_width"))) {
        return lv_obj_set_style_local_shadow_width(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("shadow_ofs_x"))) {
        return lv_obj_set_style_local_shadow_ofs_x(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("shadow_ofs_y"))) {
        return lv_obj_set_style_local_shadow_ofs_y(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("shadow_spread"))) {
        return lv_obj_set_style_local_shadow_spread(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("shadow_blend_mode"))) {
        return lv_obj_set_style_local_shadow_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("shadow_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_shadow_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("shadow_opa"))) {
        return lv_obj_set_style_local_shadow_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("pattern_repeat"))) {
        return lv_obj_set_style_local_pattern_repeat(obj, part, state, (bool)var);
    } else if(!strcmp_P(attr, PSTR("pattern_blend_mode"))) {
        return lv_obj_set_style_local_pattern_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("pattern_recolor"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_pattern_recolor(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("pattern_opa"))) {
        return lv_obj_set_style_local_pattern_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("pattern_recolor_opa"))) {
        return lv_obj_set_style_local_pattern_recolor_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("pattern_image"))) {
        //   return lv_obj_set_style_local_pattern_image(obj, part, state, (constvoid *)var);
    } else if(!strcmp_P(attr, PSTR("value_letter_space"))) {
        return lv_obj_set_style_local_value_letter_space(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("value_line_space"))) {
        return lv_obj_set_style_local_value_line_space(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("value_blend_mode"))) {
        return lv_obj_set_style_local_value_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("value_ofs_x"))) {
        return lv_obj_set_style_local_value_ofs_x(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("value_ofs_y"))) {
        return lv_obj_set_style_local_value_ofs_y(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("value_align"))) {
        return lv_obj_set_style_local_value_align(obj, part, state, (lv_align_t)var);
    } else if(!strcmp_P(attr, PSTR("value_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_value_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("value_opa"))) {
        return lv_obj_set_style_local_value_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("value_font"))) {
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
    } else if(!strcmp_P(attr, PSTR("value_str"))) {
        return lv_obj_set_style_local_value_str(obj, part, state, (const char *)payload);
    } else if(!strcmp_P(attr, PSTR("text_letter_space"))) {
        return lv_obj_set_style_local_text_letter_space(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("text_line_space"))) {
        return lv_obj_set_style_local_text_line_space(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("text_decor"))) {
        return lv_obj_set_style_local_text_decor(obj, part, state, (lv_text_decor_t)var);
    } else if(!strcmp_P(attr, PSTR("text_blend_mode"))) {
        return lv_obj_set_style_local_text_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("text_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_text_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("text_sel_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_text_sel_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("text_opa"))) {
        return lv_obj_set_style_local_text_opa(obj, part, state, (lv_opa_t)var);
    } else if(!strcmp_P(attr, PSTR("text_font"))) {
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
    } else if(!strcmp_P(attr, PSTR("line_width"))) {
        return lv_obj_set_style_local_line_width(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("line_blend_mode"))) {
        return lv_obj_set_style_local_line_blend_mode(obj, part, state, (lv_blend_mode_t)var);
    } else if(!strcmp_P(attr, PSTR("line_dash_width"))) {
        return lv_obj_set_style_local_line_dash_width(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("line_dash_gap"))) {
        return lv_obj_set_style_local_line_dash_gap(obj, part, state, (lv_style_int_t)var);
    } else if(!strcmp_P(attr, PSTR("line_rounded"))) {
        return lv_obj_set_style_local_line_rounded(obj, part, state, (bool)var);
    } else if(!strcmp_P(attr, PSTR("line_color"))) {
        lv_color_t color = haspPayloadToColor(payload);
        return lv_obj_set_style_local_line_color(obj, part, state, color);
    } else if(!strcmp_P(attr, PSTR("line_opa"))) {
        return lv_obj_set_style_local_line_opa(obj, part, state, (lv_opa_t)var);
    }

    /* Property not found */
    haspAttributeNotFound(attr);
}

void haspSetObjAttribute1(lv_obj_t * obj, const char * attr, const char * payload)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("x"))) {
        lv_obj_set_x(obj, val);
        return;
    } else if(!strcmp_P(attr, PSTR("y"))) {
        lv_obj_set_y(obj, val);
        return;
    } else if(!strcmp_P(attr, PSTR("w"))) {
        lv_obj_set_width(obj, val);
        return;
    } else if(!strcmp_P(attr, PSTR("h"))) {
        lv_obj_set_height(obj, val);
        return;
    }
    haspSetLocalStyle(obj, attr, payload);
}

void haspSetObjAttribute3(lv_obj_t * obj, const char * attr, const char * payload)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("vis"))) {
        lv_obj_set_hidden(obj, val == 0);
        return;
    } else {
        /* .txt and .val depend on objecttype */
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(!strcmp_P(attr, PSTR("txt"))) { // In order of likelihood to occur
            if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                haspSetLabelText(obj, payload);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
                lv_label_set_text(obj, payload);
                return;
            } else if(check_obj_type(list.type[0], LV_HASP_CHECKBOX)) {
#if LVGL7
                lv_checkbox_set_text(obj, payload);
#else
                lv_cb_set_text(obj, payload);
#endif
                return;
            }
        }

        if(!strcmp_P(attr, PSTR("val"))) { // In order of likelihood to occur
            int16_t intval = atoi(payload);

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
                set_cpicker_value(obj, payload);
                return;
            }
        }

        if(!strcmp_P(attr, PSTR("min"))) { // In order of likelihood to occur
            int16_t min = atoi(payload);
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

        if(!strcmp_P(attr, PSTR("max"))) { // In order of likelihood to occur
            int16_t max = atoi(payload);
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
    haspSetLocalStyle(obj, attr, payload);
}

void haspSetObjAttribute4(lv_obj_t * obj, const char * attr, const char * payload)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("rows"))) {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
            lv_roller_set_visible_row_count(obj, (uint8_t)val);
            return;
        }
    } else {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(!strcmp_P(attr, PSTR("rect"))) {
            if(check_obj_type(list.type[0], LV_HASP_CPICKER)) {
                lv_cpicker_set_type(obj, is_true(payload) ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
                return;
            }
        }

        if(!strcmp_P(attr, PSTR("mode"))) {
            if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
                set_label_long_mode(obj, payload);
                return;
            }
        }
    }

    haspSetLocalStyle(obj, attr, payload);
}

void haspSetObjAttribute6(lv_obj_t * obj, const char * attr, const char * payload)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("hidden"))) {
        lv_obj_set_hidden(obj, val == 0);
        return;
    } else {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(!strcmp_P(attr, PSTR("toggle"))) {
            if(check_obj_type(list.type[0], LV_HASP_BUTTON)) {
                haspSetToggle(obj, atoi(payload) > 0);
                return;
            }
        }
    }
    haspSetLocalStyle(obj, attr, payload);
}

void haspSetObjAttribute7(lv_obj_t * obj, const char * attr, const char * payload)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("opacity"))) {
        haspSetOpacity(obj, val);
        return;
    } else if(!strcmp_P(attr, PSTR("enabled"))) {
        lv_obj_set_click(obj, val != 0);
        return;
    } else if(!strcmp_P(attr, PSTR("options"))) {
        /* .options depend on objecttype */
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);

        if(check_obj_type(list.type[0], LV_HASP_DDLIST)) {
#if LVGL7
            lv_dropdown_set_options(obj, payload);
#else
            lv_ddlist_set_options(obj, payload);
#endif
            return;
        } else if(check_obj_type(list.type[0], LV_HASP_ROLLER)) {
            lv_roller_ext_t * ext = (lv_roller_ext_t *)lv_obj_get_ext_attr(obj);
            lv_roller_set_options(obj, payload, ext->mode);
            return;
        }
    }
    haspSetLocalStyle(obj, attr, payload);
}

void hasp_set_obj_attribute(lv_obj_t * obj, const char * attr_p, const char * payload)
{
    if(!obj) {
        Log.warning(F("HASP: Unknown object"));
        return;
    }

    // strip starting '.'
    char * attr = (char *)attr_p;
    if(*attr == '.') attr++;

    switch(strlen(attr)) {
        case 1:
            haspSetObjAttribute1(obj, attr, payload);
            break;
        case 4:
            haspSetObjAttribute4(obj, attr, payload);
            break;
        case 3:
            haspSetObjAttribute3(obj, attr, payload);
            break;
        case 6:
            haspSetObjAttribute6(obj, attr, payload);
            break;
        case 7:
            haspSetObjAttribute7(obj, attr, payload);
            break;
        default:
            haspSetLocalStyle(obj, attr, payload);
    }
}