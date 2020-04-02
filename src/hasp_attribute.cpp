#include "ArduinoLog.h"
#include "ArduinoJson.h"

#include "lvgl.h"
#include "lv_conf.h"

#include "hasp.h"
#include "hasp_dispatch.h"
#include "hasp_attribute.h"

LV_FONT_DECLARE(unscii_8_icon);
extern lv_font_t * haspFonts[8];

static inline bool is_true(const char * s);
static inline bool only_digits(const char * s);
static inline void hasp_out_int(lv_obj_t * obj, const char * attr, uint32_t val);
static inline void hasp_out_str(lv_obj_t * obj, const char * attr, const char * data);
static inline void hasp_out_color(lv_obj_t * obj, const char * attr, lv_color_t color);

// this function is missing in lvgl
// OK
static uint8_t lv_roller_get_visible_row_count(lv_obj_t * roller)
{
    const lv_font_t * font    = lv_obj_get_style_text_font(roller, LV_ROLLER_PART_BG);
    lv_style_int_t line_space = lv_obj_get_style_text_line_space(roller, LV_ROLLER_PART_BG);
    lv_coord_t h              = lv_obj_get_height(roller);

    if((lv_font_get_line_height(font) + line_space) != 0)
        return (uint8_t)(h / (lv_font_get_line_height(font) + line_space));
    else
        return 0;
}

// OK
static lv_color_t haspLogColor(lv_color_t color)
{
    uint8_t r = (LV_COLOR_GET_R(color) * 263 + 7) >> 5;
    uint8_t g = (LV_COLOR_GET_G(color) * 259 + 3) >> 6;
    uint8_t b = (LV_COLOR_GET_B(color) * 263 + 7) >> 5;
    Log.trace(F("Color: R%u G%u B%u"), r, g, b);
    return color;
}

// OK
static lv_color_t haspPayloadToColor(const char * payload)
{
    switch(strlen(payload)) {
        case 3:
            if(!strcmp_P(payload, PSTR("red"))) return haspLogColor(LV_COLOR_RED);
            break;
        case 4:
            if(!strcmp_P(payload, PSTR("blue"))) return haspLogColor(LV_COLOR_BLUE);
            if(!strcmp_P(payload, PSTR("cyan"))) return haspLogColor(LV_COLOR_CYAN);
            if(!strcmp_P(payload, PSTR("gray"))) return haspLogColor(LV_COLOR_GRAY);
            /*          if(!strcmp_P(payload, PSTR("aqua"))) return haspLogColor(LV_COLOR_AQUA);
                        if(!strcmp_P(payload, PSTR("lime"))) return haspLogColor(LV_COLOR_LIME);
                        if(!strcmp_P(payload, PSTR("teal"))) return haspLogColor(LV_COLOR_TEAL);
                        if(!strcmp_P(payload, PSTR("navy"))) return haspLogColor(LV_COLOR_NAVY);*/
            break;
        case 5:
            if(!strcmp_P(payload, PSTR("green"))) return haspLogColor(LV_COLOR_GREEN);
            if(!strcmp_P(payload, PSTR("white"))) return haspLogColor(LV_COLOR_WHITE);
            if(!strcmp_P(payload, PSTR("black"))) return haspLogColor(LV_COLOR_BLACK);
            //            if(!strcmp_P(payload, PSTR("olive"))) return haspLogColor(LV_COLOR_OLIVE);
            break;
        case 6:
            if(!strcmp_P(payload, PSTR("yellow"))) return haspLogColor(LV_COLOR_YELLOW);
            if(!strcmp_P(payload, PSTR("orange"))) return haspLogColor(LV_COLOR_ORANGE);
            if(!strcmp_P(payload, PSTR("purple"))) return haspLogColor(LV_COLOR_PURPLE);
            if(!strcmp_P(payload, PSTR("silver"))) return haspLogColor(LV_COLOR_SILVER);
            //            if(!strcmp_P(payload, PSTR("maroon"))) return haspLogColor(LV_COLOR_MAROON);
            break;
        case 7:
            if(!strcmp_P(payload, PSTR("magenta"))) return haspLogColor(LV_COLOR_MAGENTA);

            /* HEX format #rrggbb or #rrggbbaa */
            int r, g, b, a;
            if(*payload == '#' && sscanf(payload + 1, "%2x%2x%2x%2x", &r, &g, &b, &a) == 4) {
                return haspLogColor(LV_COLOR_MAKE(r, g, b));
            } else if(*payload == '#' && sscanf(payload + 1, "%2x%2x%2x", &r, &g, &b) == 3) {
                return haspLogColor(LV_COLOR_MAKE(r, g, b));
            }
        default:
            //            if(!strcmp_P(payload, PSTR("darkblue"))) return haspLogColor(LV_COLOR_MAKE(0, 51, 102));
            //            if(!strcmp_P(payload, PSTR("lightblue"))) return haspLogColor(LV_COLOR_MAKE(46, 203,
            //            203));
            break;
    }

    /* 16-bit RGB565 Color Scheme*/
    if(only_digits(payload)) {
        uint16_t c = atoi(payload);
        /* Initial colors */
        uint8_t R5 = ((c >> 11) & 0b11111);
        uint8_t G6 = ((c >> 5) & 0b111111);
        uint8_t B5 = (c & 0b11111);
        /* Remapped colors */
        uint8_t R8 = (R5 * 527 + 23) >> 6;
        uint8_t G8 = (G6 * 259 + 33) >> 6;
        uint8_t B8 = (B5 * 527 + 23) >> 6;
        return lv_color_make(R8, G8, B8);
    }

    /* Unknown format */
    Log.warning(F("Invalid color %s"), payload);
    return LV_COLOR_BLACK;
}

static void hasp_process_label_long_mode(lv_obj_t * obj, const char * payload, bool update)
{
    if(update) {
        lv_label_long_mode_t mode = LV_LABEL_LONG_EXPAND;
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
        } else {
            return Log.warning(F("Invalid long mode"));
        }
        lv_label_set_long_mode(obj, mode);
    } else {
        // Getter needed
    }
}

// OK
lv_obj_t * FindButtonLabel(lv_obj_t * btn)
{
    if(btn) {
        lv_obj_t * label = lv_obj_get_child_back(btn, NULL);
        if(label) {
            lv_obj_type_t list;
            lv_obj_get_type(label, &list);
            const char * objtype = list.type[0];

            if(check_obj_type(objtype, LV_HASP_LABEL)) {
                return label;
            }

        } else {
            Log.error(F("HASP: FindButtonLabel NULL Pointer encountered"));
        }
    } else {
        Log.warning(F("HASP: Button not defined"));
    }
    return NULL;
}

// OK
static inline void haspSetLabelText(lv_obj_t * obj, const char * value)
{
    lv_obj_t * label = FindButtonLabel(obj);
    if(label) {
        lv_label_set_text(label, value);
    }
}

// OK
static inline bool haspGetLabelText(lv_obj_t * obj, char * text)
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
            text = lv_label_get_text(label);
            return true;
        }

    } else {
        Log.warning(F("HASP: haspGetLabelText NULL Pointer encountered"));
    }

    return false;
}

static void hasp_local_style_attr(lv_obj_t * obj, const char * attr_p, const char * payload, bool update)
{
    uint8_t part  = LV_TABLE_PART_BG;
    uint8_t state = LV_STATE_DEFAULT;
    int16_t var   = atoi(payload);

    int len = strlen(attr_p);
    if(len > 0 && len < 128) {
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

        // Remove Trailing part digit
        char attr[128];
        if(part != LV_TABLE_PART_BG && len > 0) {
            len--;
        }
        strncpy(attr, attr_p, len + 1);
        attr[len] = 0;

        // debugPrintln(strAttr + "&" + part);
        uint8_t t8 = 0;

        /* ***** WARNING ****************************************************
         * when using hasp_out use attr_p for the original attribute name
         * *************************************************************** */

        if(!strcmp_P(attr, PSTR("radius"))) {
            if(update) {
                return lv_obj_set_style_local_radius(obj, part, state, (lv_style_int_t)var);
            } else {
                lv_obj_get_style_local_radius(obj, part, state, &var);
                return hasp_out_int(obj, attr_p, var);
            }

        } else if(!strcmp_P(attr, PSTR("clip_corner"))) {
            if(update) {
                return lv_obj_set_style_local_clip_corner(obj, part, state, (bool)var);
            } else {
                lv_obj_get_style_local_clip_corner(obj, part, state, &t8);
                return hasp_out_int(obj, attr_p, t8);
            }

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
                case 0:
                    lv_obj_set_style_local_text_font(obj, part, state, haspFonts[0]);
                    Log.verbose(F("Changing font to : %s"), (char *)haspFonts[0]->user_data);
                    break;
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
    }

    Log.warning(F("HASP: Unknown property %s"), attr_p);
}

// OK
static void hasp_process_obj_attribute_txt(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type(objtype, LV_HASP_BUTTON)) {
        if(update) {
            haspSetLabelText(obj, payload);
        } else {
            char * text = NULL;
            if(haspGetLabelText(obj, text)) hasp_out_str(obj, attr, text);
        }
        return;
    }
    if(check_obj_type(objtype, LV_HASP_LABEL)) {
        return update ? lv_label_set_text(obj, payload) : hasp_out_str(obj, attr, lv_label_get_text(obj));
    }
    if(check_obj_type(objtype, LV_HASP_CHECKBOX)) {
        return update ? lv_checkbox_set_text(obj, payload) : hasp_out_str(obj, attr, lv_checkbox_get_text(obj));
    }
    if(check_obj_type(objtype, LV_HASP_DDLIST)) {
        char buffer[128];
        lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
        return hasp_out_str(obj, attr, buffer);
    }
    if(check_obj_type(objtype, LV_HASP_ROLLER)) {
        char buffer[128];
        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
        return hasp_out_str(obj, attr, buffer);
    }
}

static void hasp_process_obj_attribute_val(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t intval = atoi(payload);
    uint16_t val   = atoi(payload);

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type(objtype, LV_HASP_BUTTON)) {
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
    }
    if(check_obj_type(objtype, LV_HASP_CHECKBOX)) {
        return update ? lv_checkbox_set_checked(obj, is_true(payload))
                      : hasp_out_int(obj, attr, lv_checkbox_is_checked(obj));
    }
    if(check_obj_type(objtype, LV_HASP_SWITCH)) {
        if(update) {
            return is_true(payload) ? lv_switch_off(obj, LV_ANIM_ON) : lv_switch_on(obj, LV_ANIM_ON);
        } else {
            return hasp_out_int(obj, attr, lv_switch_get_state(obj));
        }
    } else if(check_obj_type(objtype, LV_HASP_DDLIST)) {
        lv_dropdown_set_selected(obj, val);
        return;
    } else if(check_obj_type(objtype, LV_HASP_LMETER)) {
        lv_linemeter_set_value(obj, intval);
        return;
    } else if(check_obj_type(objtype, LV_HASP_SLIDER)) {
        lv_slider_set_value(obj, intval, LV_ANIM_ON);
        return;
    } else if(check_obj_type(objtype, LV_HASP_LED)) {
        lv_led_set_bright(obj, (uint8_t)val);
        return;
    } else if(check_obj_type(objtype, LV_HASP_GAUGE)) {
        lv_gauge_set_value(obj, 0, intval);
        return;
    } else if(check_obj_type(objtype, LV_HASP_ROLLER)) {
        lv_roller_set_selected(obj, val, LV_ANIM_ON);
        return;
    } else if(check_obj_type(objtype, LV_HASP_BAR)) {
        lv_bar_set_value(obj, intval, LV_ANIM_OFF);
        return;
    } else if(check_obj_type(objtype, LV_HASP_CPICKER)) {
        return update ? (void)lv_cpicker_set_color(obj, haspPayloadToColor(payload))
                      : hasp_out_color(obj, attr, lv_cpicker_get_color(obj));
    }
}

// OK
static void hasp_process_obj_attribute_range(lv_obj_t * obj, const char * attr, const char * payload, bool update,
                                             bool set_min, bool set_max)
{
    int16_t val = atoi(payload);

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type(objtype, LV_HASP_SLIDER)) {
        int16_t min = lv_slider_get_min_value(obj);
        int16_t max = lv_slider_get_max_value(obj);
        return update ? lv_slider_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? lv_slider_get_min_value(obj) : lv_slider_get_max_value(obj));
    }
    if(check_obj_type(objtype, LV_HASP_GAUGE)) {
        int16_t min = lv_gauge_get_min_value(obj);
        int16_t max = lv_gauge_get_max_value(obj);
        return update ? lv_gauge_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? lv_gauge_get_min_value(obj) : lv_gauge_get_max_value(obj));
    }
    if(check_obj_type(objtype, LV_HASP_BAR)) {
        int16_t min = lv_bar_get_min_value(obj);
        int16_t max = lv_bar_get_max_value(obj);
        return update ? lv_bar_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? lv_bar_get_min_value(obj) : lv_bar_get_max_value(obj));
    }
    if(check_obj_type(objtype, LV_HASP_LMETER)) {
        int16_t min = lv_linemeter_get_min_value(obj);
        int16_t max = lv_linemeter_get_max_value(obj);
        return update ? lv_linemeter_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr,
                                     set_min ? lv_linemeter_get_min_value(obj) : lv_linemeter_get_max_value(obj));
    }
}

// OK
static void hasp_process_obj_attribute1(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("x"))) {
        return update ? lv_obj_set_x(obj, val) : hasp_out_int(obj, attr, lv_obj_get_x(obj));
    }
    if(!strcmp_P(attr, PSTR("y"))) {
        return update ? lv_obj_set_y(obj, val) : hasp_out_int(obj, attr, lv_obj_get_y(obj));
    }
    if(!strcmp_P(attr, PSTR("w"))) {
        return update ? lv_obj_set_width(obj, val) : hasp_out_int(obj, attr, lv_obj_get_width(obj));
    }
    if(!strcmp_P(attr, PSTR("h"))) {
        return update ? lv_obj_set_height(obj, val) : hasp_out_int(obj, attr, lv_obj_get_height(obj));
    }

    hasp_local_style_attr(obj, attr, payload, update);
}

// OK
static void hasp_process_obj_attribute3(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("vis"))) {
        return update ? lv_obj_set_hidden(obj, !is_true(payload)) : hasp_out_int(obj, attr, !lv_obj_get_hidden(obj));
    }
    if(!strcmp_P(attr, PSTR("txt"))) { // In order of likelihood to occur
        return hasp_process_obj_attribute_txt(obj, attr, payload, update);
    }
    if(!strcmp_P(attr, PSTR("val"))) { // In order of likelihood to occur
        return hasp_process_obj_attribute_val(obj, attr, payload, update);
    }
    if(!strcmp_P(attr, PSTR("min"))) { // In order of likelihood to occur
        return hasp_process_obj_attribute_range(obj, attr, payload, update, true, false);
    }
    if(!strcmp_P(attr, PSTR("max"))) { // In order of likelihood to occur
        return hasp_process_obj_attribute_range(obj, attr, payload, update, false, true);
    }

    hasp_local_style_attr(obj, attr, payload, update);
}

static void hasp_process_obj_attribute4(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("rows"))) {
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);
        const char * objtype = list.type[0];

        if(check_obj_type(objtype, LV_HASP_ROLLER)) {
            return update ? lv_roller_set_visible_row_count(obj, (uint8_t)val)
                          : hasp_out_int(obj, attr, lv_roller_get_visible_row_count(obj));
        }
    }

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(!strcmp_P(attr, PSTR("rect"))) {
        if(check_obj_type(objtype, LV_HASP_CPICKER)) {
            lv_cpicker_set_type(obj, is_true(payload) ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
            return;
        }
    }

    if(!strcmp_P(attr, PSTR("mode"))) {
        if(check_obj_type(objtype, LV_HASP_BUTTON)) {
            lv_obj_t * label = FindButtonLabel(obj);
            if(label) {
                hasp_process_label_long_mode(label, payload, update);
                lv_obj_set_width(label, lv_obj_get_width(obj));
            }
            return;
        }

        if(check_obj_type(objtype, LV_HASP_LABEL)) {
            hasp_process_label_long_mode(obj, payload, update);
            return;
        }
    }

    hasp_local_style_attr(obj, attr, payload, update);
}

// OK
static void hasp_process_obj_attribute6(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("hidden"))) {
        return update ? lv_obj_set_hidden(obj, is_true(payload)) : hasp_out_int(obj, attr, lv_obj_get_hidden(obj));
    }

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(!strcmp_P(attr, PSTR("toggle"))) {
        if(check_obj_type(objtype, LV_HASP_BUTTON)) {
            if(update) {
                bool toggle = is_true(payload);
                lv_btn_set_checkable(obj, toggle);
                lv_obj_set_event_cb(obj, toggle ? toggle_event_handler : btn_event_handler);
            } else {
                hasp_out_int(obj, attr, lv_btn_get_checkable(obj));
            }
        }
    }

    hasp_local_style_attr(obj, attr, payload, update);
}

// OK
static void hasp_process_obj_attribute7(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t val = atoi(payload);

    if(!strcmp_P(attr, PSTR("opacity"))) {
        return update ? lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, val)
                      : hasp_out_int(obj, attr, lv_obj_get_style_opa_scale(obj, LV_OBJ_PART_MAIN));

    } else if(!strcmp_P(attr, PSTR("enabled"))) {
        return update ? lv_obj_set_click(obj, is_true(payload)) : hasp_out_int(obj, attr, lv_obj_get_click(obj));

    } else if(!strcmp_P(attr, PSTR("options"))) {
        /* Attributes depending on objecttype */
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);
        const char * objtype = list.type[0];

        if(check_obj_type(objtype, LV_HASP_DDLIST)) {
            if(update) {
                lv_dropdown_set_options(obj, payload);
            } else {
                hasp_out_str(obj, attr, lv_dropdown_get_options(obj));
            }
            return;

        } else if(check_obj_type(objtype, LV_HASP_ROLLER)) {
            if(update) {
                lv_roller_ext_t * ext = (lv_roller_ext_t *)lv_obj_get_ext_attr(obj);
                lv_roller_set_options(obj, payload, ext->mode);
            } else {
                hasp_out_str(obj, attr, lv_roller_get_options(obj));
            }
            return;
        }
    }

    hasp_local_style_attr(obj, attr, payload, update);
}

// OK
// @param update  bool: change the value if true, dispatch value if false
void hasp_process_obj_attribute(lv_obj_t * obj, const char * attr_p, const char * payload, bool update)
{
    if(!obj) return Log.warning(F("HASP: Unknown object"));

    char * attr = (char *)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(strlen(attr)) {
        case 1:
            hasp_process_obj_attribute1(obj, attr, payload, update);
            break;
        case 4:
            hasp_process_obj_attribute4(obj, attr, payload, update);
            break;
        case 3:
            hasp_process_obj_attribute3(obj, attr, payload, update);
            break;
        case 6:
            hasp_process_obj_attribute6(obj, attr, payload, update);
            break;
        case 7:
            hasp_process_obj_attribute7(obj, attr, payload, update);
            break;
        default:
            hasp_local_style_attr(obj, attr, payload, update); // all other lengths
    }
}

/* **************************
 * Static Inline functions
 * **************************/
static inline bool is_true(const char * s)
{
    return (!strcmp_P(s, PSTR("true")) || !strcmp_P(s, PSTR("TRUE")) || !strcmp_P(s, PSTR("1")) ||
            !strcmp_P(s, PSTR("on")) || !strcmp_P(s, PSTR("ON")) || !strcmp_P(s, PSTR("On")) ||
            !strcmp_P(s, PSTR("yes")) || !strcmp_P(s, PSTR("YES")) || !strcmp_P(s, PSTR("Yes")));
}

static inline bool only_digits(const char * s)
{
    size_t digits = 0;
    while((s + digits) != '\0' && isdigit(*(s + digits))) {
        digits++;
    }
    return strlen(s) == digits;
}

static inline void hasp_out_int(lv_obj_t * obj, const char * attr, uint32_t val)
{
    return hasp_send_attribute(obj, attr, val);
}

static inline void hasp_out_str(lv_obj_t * obj, const char * attr, const char * data)
{
    return hasp_send_attribute_str(obj, attr, data);
}

static inline void hasp_out_color(lv_obj_t * obj, const char * attr, lv_color_t color)
{
    lv_color32_t c32;
    c32.full = lv_color_to32(color);

    DynamicJsonDocument doc(128);
    doc[F("r")] = c32.ch.red;
    doc[F("g")] = c32.ch.green;
    doc[F("b")] = c32.ch.blue;

    char buffer[128];
    serializeJson(doc, buffer, sizeof(buffer));
    hasp_send_attribute_str(obj, attr, buffer);
}
