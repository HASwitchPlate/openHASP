/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_attribute.h" /*To see all the hashes*/

static void set_style_attribute(lv_style_t * style, lv_state_t state, int16_t val)
{
    lv_color_t color;

    switch(1) {
        case ATTR_RADIUS:
            lv_style_set_radius(style, state, val);
            break;
        case ATTR_BG_COLOR:
            lv_style_set_bg_color(style, state, color);
            break;

            // break; case ATTR_CLIP_CORNER:lv_style_set_clip_corner(style, state, val!=0);
        case ATTR_SIZE:
            lv_style_set_size(style, state, val);
            break;
        case ATTR_TRANSFORM_WIDTH:
            lv_style_set_transform_width(style, state, val);
            break;
        case ATTR_TRANSFORM_HEIGHT:
            lv_style_set_transform_height(style, state, val);
            // break; case ATTR_TRANSFORM_ANGLE:lv_style_set_transform_angle(style, state, val);
            // break; case ATTR_TRANSFORM_ZOOM:lv_style_set_transform_zoom(style, state, val);
            // break; case ATTR_OPA_SCALE:lv_style_set_opa_scale(style, state, (lv_opa_t)val);
            break;
        case ATTR_PAD_TOP:
            lv_style_set_pad_top(style, state, val);
            break;
        case ATTR_PAD_BOTTOM:
            lv_style_set_pad_bottom(style, state, val);
            break;
        case ATTR_PAD_LEFT:
            lv_style_set_pad_left(style, state, val);
            break;
        case ATTR_PAD_RIGHT:
            lv_style_set_pad_right(style, state, val);
            break;
        case ATTR_PAD_INNER:
            lv_style_set_pad_inner(style, state, val);
            // break; case ATTR_MARGIN_TOP:lv_style_set_margin_top(style, state, val);
            // break; case ATTR_MARGIN_BOTTOM:lv_style_set_margin_bottom(style, state, val);
            // break; case ATTR_MARGIN_LEFT:lv_style_set_margin_left(style, state, val);
            // break; case ATTR_MARGIN_RIGHT:lv_style_set_margin_right(style, state, val);
            // break; case ATTR_BG_BLEND_MODE:lv_style_set_bg_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_BG_MAIN_STOP:
            lv_style_set_bg_main_stop(style, state, val);
            break;
        case ATTR_BG_GRAD_STOP:
            lv_style_set_bg_grad_stop(style, state, val);
            // break; case ATTR_BG_GRAD_DIR:lv_style_set_bg_grad_dir, lv_grad_dir_t, _int, scalar)
            break;
        case ATTR_BG_GRAD_COLOR:
            lv_style_set_bg_grad_color(style, state, color);
            break;
        case ATTR_BG_OPA:
            lv_style_set_bg_opa(style, state, (lv_opa_t)val);
            break;
        case ATTR_BORDER_WIDTH:
            lv_style_set_border_width(style, state, val);
            // break; case ATTR_BORDER_SIDE:lv_style_set_border_side, lv_border_side_t, _int, scalar)
            // break; case ATTR_BORDER_BLEND_MODE:lv_style_set_border_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_BORDER_POST:
            lv_style_set_border_post(style, state, val != 0);
            break;
        case ATTR_BORDER_COLOR:
            lv_style_set_border_color(style, state, color);
            break;
        case ATTR_BORDER_OPA:
            lv_style_set_border_opa(style, state, (lv_opa_t)val);
            break;
        case ATTR_OUTLINE_WIDTH:
            lv_style_set_outline_width(style, state, val);
            break;
        case ATTR_OUTLINE_PAD:
            lv_style_set_outline_pad(style, state, val);
            // break; case ATTR_OUTLINE_BLEND_MODE:lv_style_set_outline_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_OUTLINE_COLOR:
            lv_style_set_outline_color(style, state, color);
            break;
        case ATTR_OUTLINE_OPA:
            lv_style_set_outline_opa(style, state, (lv_opa_t)val);
            break;
        case ATTR_SHADOW_WIDTH:
            lv_style_set_shadow_width(style, state, val);
            break;
        case ATTR_SHADOW_OFS_X:
            lv_style_set_shadow_ofs_x(style, state, val);
            break;
        case ATTR_SHADOW_OFS_Y:
            lv_style_set_shadow_ofs_y(style, state, val);
            break;
        case ATTR_SHADOW_SPREAD:
            lv_style_set_shadow_spread(style, state, val);
            // break; case ATTR_SHADOW_BLEND_MODE:lv_style_set_shadow_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_SHADOW_COLOR:
            lv_style_set_shadow_color(style, state, color);
            break;
        case ATTR_SHADOW_OPA:
            lv_style_set_shadow_opa(style, state, (lv_opa_t)val);
            break;
        case ATTR_PATTERN_REPEAT:
            lv_style_set_pattern_repeat(style, state, val != 0);
            // break; case ATTR_PATTERN_BLEND_MODE:lv_style_set_pattern_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_PATTERN_RECOLOR:
            lv_style_set_pattern_recolor(style, state, color);
            break;
        case ATTR_PATTERN_OPA:
            lv_style_set_pattern_opa(style, state, (lv_opa_t)val);
            break;
        case ATTR_PATTERN_RECOLOR_OPA:
            lv_style_set_pattern_recolor_opa(style, state, (lv_opa_t)val);
            // break; case ATTR_PATTERN_IMAGE:lv_style_set_pattern_image, const void*, _ptr, scalar)
            break;
        case ATTR_VALUE_LETTER_SPACE:
            lv_style_set_value_letter_space(style, state, val);
            break;
        case ATTR_VALUE_LINE_SPACE:
            lv_style_set_value_line_space(style, state, val);
            // break; case ATTR_VALUE_BLEND_MODE:lv_style_set_value_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_VALUE_OFS_X:
            lv_style_set_value_ofs_x(style, state, val);
            break;
        case ATTR_VALUE_OFS_Y:
            lv_style_set_value_ofs_y(style, state, val);
            break;
        case ATTR_VALUE_ALIGN:
            lv_style_set_value_align(style, state, (lv_align_t)val);
            break;
        case ATTR_VALUE_COLOR:
            lv_style_set_value_color(style, state, color);
            break;
        case ATTR_VALUE_OPA:
            lv_style_set_value_opa(style, state, (lv_opa_t)val);
            // break; case ATTR_VALUE_FONT:lv_style_set_value_font, const lv_font_t*, _ptr, scalar)
            // break; case ATTR_VALUE_STR:lv_style_set_value_str, const char*, _ptr, scalar)
            break;
        case ATTR_TEXT_LETTER_SPACE:
            lv_style_set_text_letter_space(style, state, val);
            break;
        case ATTR_TEXT_LINE_SPACE:
            lv_style_set_text_line_space(style, state, val);
            // break; case ATTR_TEXT_DECOR:lv_style_set_text_decor, lv_text_decor_t, _int, scalar)
            // break; case ATTR_TEXT_BLEND_MODE:lv_style_set_text_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_TEXT_COLOR:
            lv_style_set_text_color(style, state, color);
            break;
        case ATTR_TEXT_SEL_COLOR:
            lv_style_set_text_sel_color(style, state, color);
            // break; case ATTR_TEXT_SEL_BG_COLOR:lv_style_set_text_sel_bg_color(style, state, color);
            break;
        case ATTR_TEXT_OPA:
            lv_style_set_text_opa(style, state, (lv_opa_t)val);
            // break; case ATTR_TEXT_FONT:lv_style_set_text_font, const lv_font_t*, _ptr, scalar)
            break;
        case ATTR_LINE_WIDTH:
            lv_style_set_line_width(style, state, val);
            // break; case ATTR_LINE_BLEND_MODE:lv_style_set_line_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_LINE_DASH_WIDTH:
            lv_style_set_line_dash_width(style, state, val);
            break;
        case ATTR_LINE_DASH_GAP:
            lv_style_set_line_dash_gap(style, state, val);
            break;
        case ATTR_LINE_ROUNDED:
            lv_style_set_line_rounded(style, state, val != 0);
            break;
        case ATTR_LINE_COLOR:
            lv_style_set_line_color(style, state, color);
            break;
        case ATTR_LINE_OPA:
            lv_style_set_line_opa(style, state, (lv_opa_t)val);
            // break; case ATTR_IMAGE_BLEND_MODE:lv_style_set_image_blend_mode, lv_blend_mode_t, _int, scalar)
            break;
        case ATTR_IMAGE_RECOLOR:
            lv_style_set_image_recolor(style, state, color);
            break;
        case ATTR_IMAGE_OPA:
            lv_style_set_image_opa(style, state, (lv_opa_t)val);
            break;
        case ATTR_IMAGE_RECOLOR_OPA:
            lv_style_set_image_recolor_opa(style, state, (lv_opa_t)val);
            // break; case ATTR_TRANSITION_TIME:lv_style_set_transition_time(style, state, val);
            // break; case ATTR_TRANSITION_DELAY:lv_style_set_transition_delay(style, state, val);
            // break; case ATTR_TRANSITION_PROP_1:lv_style_set_ transition_prop_1(style, state, val);
            // break; case ATTR_TRANSITION_PROP_2:lv_style_set_ transition_prop_2(style, state, val);
            // break; case ATTR_TRANSITION_PROP_3:lv_style_set_ transition_prop_3(style, state, val);
            // break; case ATTR_TRANSITION_PROP_4:lv_style_set_ transition_prop_4(style, state, val);
            // break; case ATTR_TRANSITION_PROP_5:lv_style_set_ transition_prop_5(style, state, val);
            // break; case ATTR_TRANSITION_PROP_6:lv_style_set_ transition_prop_6(style, state, val);
            //#if LV_USE_ANIMATION
            // break; case ATTR_TRANSITION_PATH:lv_style_set_transition_path, lv_anim_path_t*, _ptr, scalar)
            //#else
            //        /*For compatibility*/
            // break; case ATTR_TRANSITION_PATH:lv_style_set_transition_path, const void*, _ptr, scalar)
            //#endif
            break;
        case ATTR_SCALE_WIDTH:
            lv_style_set_scale_width(style, state, val);
            break;
        case ATTR_SCALE_BORDER_WIDTH:
            lv_style_set_scale_border_width(style, state, val);
            break;
        case ATTR_SCALE_END_BORDER_WIDTH:
            lv_style_set_scale_end_border_width(style, state, val);
            break;
        case ATTR_SCALE_END_LINE_WIDTH:
            lv_style_set_scale_end_line_width(style, state, val);
            break;
        case ATTR_SCALE_GRAD_COLOR:
            lv_style_set_scale_grad_color(style, state, color);
            break;
        case ATTR_SCALE_END_COLOR:
            lv_style_set_scale_end_color(style, state, color);
        default:;
    }
}