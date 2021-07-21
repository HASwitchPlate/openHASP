#ifndef LV_ATTR_COMPAT_V7_H
#define LV_ATTR_COMPAT_V7_H

#include "lvgl.h"

#if LVGL_VERSION_MAJOR == 8

#ifdef __cplusplus
extern "C" {
#endif

// Usage:      Search & Replace all `lv_obj_get_style_` to `lv_obj_get_v7_style_` in your source code
// Optionally: Search & Replace all `lv_obj_set_style_local_` to `lv_obj_set_v7_style_` in your source code

#define _LV_ATTRIBUTE_V7(prop_name, func_name, value_type)                                                             \
    static inline value_type lv_obj_get_v7_style_##func_name(const lv_obj_t* obj, lv_part_t part, lv_state_t state)    \
    {                                                                                                                  \
        return lv_obj_get_style_##func_name(obj, part | state);                                                        \
    }                                                                                                                  \
                                                                                                                       \
    static inline void lv_obj_set_v7_style_##func_name(lv_obj_t* obj, lv_part_t part, lv_state_t state,                \
                                                       value_type value)                                               \
    {                                                                                                                  \
        lv_obj_set_style_##func_name(obj, value, part | state);                                                        \
    }                                                                                                                  \
                                                                                                                       \
    inline void lv_obj_set_style_local_##func_name(lv_obj_t* obj, lv_part_t part, lv_state_t state, value_type value)  \
    {                                                                                                                  \
        lv_obj_set_style_##func_name(obj, value, part | state);                                                        \
    }

_LV_ATTRIBUTE_V7(ALIGN, align, lv_align_t)
_LV_ATTRIBUTE_V7(ANIM_SPEED, anim_speed, uint32_t)
_LV_ATTRIBUTE_V7(ANIM_TIME, anim_time, uint32_t)
_LV_ATTRIBUTE_V7(ARC_COLOR, arc_color, lv_color_t)
_LV_ATTRIBUTE_V7(ARC_COLOR_FILTERED, arc_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(ARC_IMG_SRC, arc_img_src, const void*)
_LV_ATTRIBUTE_V7(ARC_OPA, arc_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(ARC_ROUNDED, arc_rounded, lv_coord_t)
_LV_ATTRIBUTE_V7(ARC_WIDTH, arc_width, lv_coord_t)
_LV_ATTRIBUTE_V7(BASE_DIR, base_dir, lv_base_dir_t)
_LV_ATTRIBUTE_V7(BG_COLOR, bg_color, lv_color_t)
_LV_ATTRIBUTE_V7(BG_COLOR_FILTERED, bg_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(BG_GRAD_COLOR, bg_grad_color, lv_color_t)
_LV_ATTRIBUTE_V7(BG_GRAD_COLOR_FILTERED, bg_grad_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(BG_GRAD_DIR, bg_grad_dir, lv_grad_dir_t)
_LV_ATTRIBUTE_V7(BG_GRAD_STOP, bg_grad_stop, lv_coord_t)
_LV_ATTRIBUTE_V7(BG_IMG_OPA, bg_img_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(BG_IMG_RECOLOR, bg_img_recolor, lv_color_t)
_LV_ATTRIBUTE_V7(BG_IMG_RECOLOR_FILTERED, bg_img_recolor_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(BG_IMG_RECOLOR_OPA, bg_img_recolor_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(BG_IMG_SRC, bg_img_src, const void*)
_LV_ATTRIBUTE_V7(BG_IMG_TILED, bg_img_tiled, bool)
_LV_ATTRIBUTE_V7(BG_MAIN_STOP, bg_main_stop, lv_coord_t)
_LV_ATTRIBUTE_V7(BG_OPA, bg_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(BLEND_MODE, blend_mode, lv_blend_mode_t)
_LV_ATTRIBUTE_V7(BORDER_COLOR, border_color, lv_color_t)
_LV_ATTRIBUTE_V7(BORDER_COLOR_FILTERED, border_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(BORDER_OPA, border_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(BORDER_POST, border_post, bool)
_LV_ATTRIBUTE_V7(BORDER_SIDE, border_side, lv_border_side_t)
_LV_ATTRIBUTE_V7(BORDER_WIDTH, border_width, lv_coord_t)
_LV_ATTRIBUTE_V7(CLIP_CORNER, clip_corner, bool)
_LV_ATTRIBUTE_V7(COLOR_FILTER_DSC, color_filter_dsc, const lv_color_filter_dsc_t*)
_LV_ATTRIBUTE_V7(COLOR_FILTER_OPA, color_filter_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(HEIGHT, height, lv_coord_t)
_LV_ATTRIBUTE_V7(IMG_OPA, img_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(IMG_RECOLOR, img_recolor, lv_color_t)
_LV_ATTRIBUTE_V7(IMG_RECOLOR_FILTERED, img_recolor_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(IMG_RECOLOR_OPA, img_recolor_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(LAYOUT, layout, uint16_t)
_LV_ATTRIBUTE_V7(LINE_COLOR, line_color, lv_color_t)
_LV_ATTRIBUTE_V7(LINE_COLOR_FILTERED, line_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(LINE_DASH_GAP, line_dash_gap, lv_coord_t)
_LV_ATTRIBUTE_V7(LINE_DASH_WIDTH, line_dash_width, lv_coord_t)
_LV_ATTRIBUTE_V7(LINE_OPA, line_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(LINE_ROUNDED, line_rounded, lv_coord_t)
_LV_ATTRIBUTE_V7(LINE_WIDTH, line_width, lv_coord_t)
_LV_ATTRIBUTE_V7(MAX_HEIGHT, max_height, lv_coord_t)
_LV_ATTRIBUTE_V7(MAX_WIDTH, max_width, lv_coord_t)
_LV_ATTRIBUTE_V7(MIN_HEIGHT, min_height, lv_coord_t)
_LV_ATTRIBUTE_V7(MIN_WIDTH, min_width, lv_coord_t)
_LV_ATTRIBUTE_V7(OPA, opa, lv_opa_t)
_LV_ATTRIBUTE_V7(OUTLINE_COLOR, outline_color, lv_color_t)
_LV_ATTRIBUTE_V7(OUTLINE_COLOR_FILTERED, outline_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(OUTLINE_OPA, outline_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(OUTLINE_PAD, outline_pad, lv_coord_t)
_LV_ATTRIBUTE_V7(OUTLINE_WIDTH, outline_width, lv_coord_t)
_LV_ATTRIBUTE_V7(PAD_BOTTOM, pad_bottom, lv_coord_t)
_LV_ATTRIBUTE_V7(PAD_COLUMN, pad_column, lv_coord_t)
_LV_ATTRIBUTE_V7(PAD_LEFT, pad_left, lv_coord_t)
_LV_ATTRIBUTE_V7(PAD_RIGHT, pad_right, lv_coord_t)
_LV_ATTRIBUTE_V7(PAD_ROW, pad_row, lv_coord_t)
_LV_ATTRIBUTE_V7(PAD_TOP, pad_top, lv_coord_t)
_LV_ATTRIBUTE_V7(RADIUS, radius, lv_coord_t)
_LV_ATTRIBUTE_V7(SHADOW_COLOR, shadow_color, lv_color_t)
_LV_ATTRIBUTE_V7(SHADOW_COLOR_FILTERED, shadow_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(SHADOW_OFS_X, shadow_ofs_x, lv_coord_t)
_LV_ATTRIBUTE_V7(SHADOW_OFS_Y, shadow_ofs_y, lv_coord_t)
_LV_ATTRIBUTE_V7(SHADOW_OPA, shadow_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(SHADOW_SPREAD, shadow_spread, lv_coord_t)
_LV_ATTRIBUTE_V7(SHADOW_WIDTH, shadow_width, lv_coord_t)
_LV_ATTRIBUTE_V7(TEXT_ALIGN, text_align, lv_text_align_t)
_LV_ATTRIBUTE_V7(TEXT_COLOR, text_color, lv_color_t)
_LV_ATTRIBUTE_V7(TEXT_COLOR_FILTERED, text_color_filtered, lv_color_t)
_LV_ATTRIBUTE_V7(TEXT_DECOR, text_decor, lv_text_decor_t)
_LV_ATTRIBUTE_V7(TEXT_FONT, text_font, const lv_font_t*)
_LV_ATTRIBUTE_V7(TEXT_LETTER_SPACE, text_letter_space, lv_coord_t)
_LV_ATTRIBUTE_V7(TEXT_LINE_SPACE, text_line_space, lv_coord_t)
_LV_ATTRIBUTE_V7(TEXT_OPA, text_opa, lv_opa_t)
_LV_ATTRIBUTE_V7(TRANSFORM_ANGLE, transform_angle, lv_coord_t)
_LV_ATTRIBUTE_V7(TRANSFORM_HEIGHT, transform_height, lv_coord_t)
_LV_ATTRIBUTE_V7(TRANSFORM_WIDTH, transform_width, lv_coord_t)
_LV_ATTRIBUTE_V7(TRANSFORM_ZOOM, transform_zoom, lv_coord_t)
_LV_ATTRIBUTE_V7(TRANSITION, transition, const lv_style_transition_dsc_t*)
_LV_ATTRIBUTE_V7(TRANSLATE_X, translate_x, lv_coord_t)
_LV_ATTRIBUTE_V7(TRANSLATE_Y, translate_y, lv_coord_t)
_LV_ATTRIBUTE_V7(WIDTH, width, lv_coord_t)
_LV_ATTRIBUTE_V7(X, x, lv_coord_t)
_LV_ATTRIBUTE_V7(Y, y, lv_coord_t)

#ifdef __cplusplus
}
#endif

#endif // LVGL_VERSION_MAJOR

#endif // LV_ATTR_COMPAT_V7_H