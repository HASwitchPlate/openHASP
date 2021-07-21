/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_ATTR_SET_H
#define HASP_ATTR_SET_H

#include "hasplib.h"

static inline void lv_obj_set_event_cb(lv_obj_t* obj, lv_event_cb_t cb)
{
    lv_obj_add_event_cb(obj, cb, LV_EVENT_ALL, NULL);
}

void attr_out_str(lv_obj_t* obj, const char* attribute, const char* data);
void attr_out_int(lv_obj_t* obj, const char* attribute, int32_t val);
void attr_out_color(lv_obj_t* obj, const char* attribute, lv_color_t color);

#if LVGL_VERSION_MAJOR >= 7

#ifdef __cplusplus
extern "C" {
#endif

lv_chart_series_t* my_chart_get_series(lv_obj_t* chart, uint8_t ser_num);
void my_obj_set_value_str_text(lv_obj_t* obj, uint8_t part, lv_state_t state, const char* text);

void my_btnmatrix_map_clear(lv_obj_t* obj);
void my_msgbox_map_clear(lv_obj_t* obj);
void line_clear_points(lv_obj_t* obj);

void hasp_process_obj_attribute(lv_obj_t* obj, const char* attr_p, const char* payload, bool update);

bool attribute_set_normalized_value(lv_obj_t* obj, hasp_update_value_t& value);

#ifdef __cplusplus
} /* extern "C" */
#endif

typedef enum {
    HASP_ATTR_TYPE_LONG_MODE_INVALID       = -10,
    HASP_ATTR_TYPE_RANGE_ERROR             = -9,
    HASP_ATTR_TYPE_METHOD_INVALID_FOR_PAGE = -8,
    HASP_ATTR_TYPE_ALIGN_INVALID           = -5,
    HASP_ATTR_TYPE_COLOR_INVALID           = -4,
    HASP_ATTR_TYPE_STR_READONLY            = -3,
    HASP_ATTR_TYPE_BOOL_READONLY           = -2,
    HASP_ATTR_TYPE_INT_READONLY            = -1,
    HASP_ATTR_TYPE_NOT_FOUND               = 0,
    HASP_ATTR_TYPE_INT,
    HASP_ATTR_TYPE_BOOL,
    HASP_ATTR_TYPE_STR,
    HASP_ATTR_TYPE_COLOR,
    HASP_ATTR_TYPE_ALIGN,
    HASP_ATTR_TYPE_DIRECTION_XY,
    HASP_ATTR_TYPE_DIRECTION_CLOCK,
    HASP_ATTR_TYPE_METHOD_OK,
} hasp_attribute_type_t;

struct hasp_attr_local_opa_t
{
    uint16_t hash;
    void (*set)(lv_obj_t*, uint8_t, lv_state_t, lv_opa_t);
    lv_opa_t (*get)(const lv_obj_t*, uint8_t);
};

struct hasp_attr_local_int_t
{
    uint16_t hash;
    void (*set)(lv_obj_t*, uint8_t, lv_state_t, lv_style_int_t);
    lv_style_int_t (*get)(const lv_obj_t*, uint8_t);
};

struct hasp_attr_update_bool_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, bool);
    bool (*get)(const lv_obj_t*);
};

struct hasp_attr_update_uint16_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, uint16_t);
    uint16_t (*get)(const lv_obj_t*);
};

struct hasp_attr_update_lv_anim_value_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, lv_anim_value_t);
    lv_anim_value_t (*get)(const lv_obj_t*);
};

struct hasp_attr_update_int16_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, int16_t);
    int16_t (*get)(const lv_obj_t*);
};

struct hasp_attr_update_uint8_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, uint8_t);
    uint8_t (*get)(const lv_obj_t*);
};

struct hasp_attr_update8_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, uint8_t);
    uint8_t (*get)(const lv_obj_t*);
};

struct hasp_attr_update_uint16_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, uint16_t);
    uint16_t (*get)(lv_obj_t*);
};

struct hasp_attr_update_bool_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, bool);
    bool (*get)(lv_obj_t*);
};

struct hasp_attr_update_lv_coord_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, lv_coord_t);
    lv_coord_t (*get)(lv_obj_t*);
};

struct hasp_attr_update_char_const_t
{
    lv_hasp_obj_type_t obj_type;
    uint16_t hash;
    void (*set)(lv_obj_t*, const char*);
    const char* (*get)(const lv_obj_t*);
};

#define _HASP_ATTRIBUTE_V7(prop_name, func_name, value_type)                                                           \
    static inline hasp_attribute_type_t attribute_##func_name(lv_obj_t* obj, uint8_t part, lv_state_t state,           \
                                                              bool update, value_type val, int32_t& res)               \
    {                                                                                                                  \
        if(update) lv_obj_set_style_##func_name(obj, part, state, (value_type)val);                                    \
        res = (int32_t)lv_obj_get_style_##func_name(obj, part);                                                        \
        return HASP_ATTR_TYPE_INT;                                                                                     \
    }

#define _HASP_ATTRIBUTE(prop_name, func_name, value_type)                                                              \
    static inline hasp_attribute_type_t attribute_##func_name(lv_obj_t* obj, uint8_t part, lv_state_t state,           \
                                                              bool update, value_type val, int32_t& res)               \
    {                                                                                                                  \
        if(update) lv_obj_set_style_##func_name(obj, (value_type)val, part);                                           \
        res = (int32_t)lv_obj_get_style_##func_name(obj, part);                                                        \
        return HASP_ATTR_TYPE_INT;                                                                                     \
    }

_HASP_ATTRIBUTE(ALIGN, align, lv_align_t)
_HASP_ATTRIBUTE(ANIM_SPEED, anim_speed, uint32_t)
_HASP_ATTRIBUTE(ANIM_TIME, anim_time, uint32_t)
// _HASP_ATTRIBUTE(ARC_COLOR, arc_color, lv_color_t)
// _HASP_ATTRIBUTE(ARC_COLOR_FILTERED, arc_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(ARC_IMG_SRC, arc_img_src, const void*)
_HASP_ATTRIBUTE(ARC_OPA, arc_opa, lv_opa_t)
_HASP_ATTRIBUTE(ARC_ROUNDED, arc_rounded, lv_coord_t)
_HASP_ATTRIBUTE(ARC_WIDTH, arc_width, lv_coord_t)
_HASP_ATTRIBUTE(BASE_DIR, base_dir, lv_base_dir_t)
// _HASP_ATTRIBUTE(BG_COLOR, bg_color, lv_color_t)
// _HASP_ATTRIBUTE(BG_COLOR_FILTERED, bg_color_filtered, lv_color_t)
// _HASP_ATTRIBUTE(BG_GRAD_COLOR, bg_grad_color, lv_color_t)
// _HASP_ATTRIBUTE(BG_GRAD_COLOR_FILTERED, bg_grad_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(BG_GRAD_DIR, bg_grad_dir, lv_grad_dir_t)
_HASP_ATTRIBUTE(BG_GRAD_STOP, bg_grad_stop, lv_coord_t)
_HASP_ATTRIBUTE(BG_IMG_OPA, bg_img_opa, lv_opa_t)
// _HASP_ATTRIBUTE(BG_IMG_RECOLOR, bg_img_recolor, lv_color_t)
// _HASP_ATTRIBUTE(BG_IMG_RECOLOR_FILTERED, bg_img_recolor_filtered, lv_color_t)
_HASP_ATTRIBUTE(BG_IMG_RECOLOR_OPA, bg_img_recolor_opa, lv_opa_t)
_HASP_ATTRIBUTE(BG_IMG_SRC, bg_img_src, const void*)
_HASP_ATTRIBUTE(BG_IMG_TILED, bg_img_tiled, bool)
_HASP_ATTRIBUTE(BG_MAIN_STOP, bg_main_stop, lv_coord_t)
_HASP_ATTRIBUTE(BG_OPA, bg_opa, lv_opa_t)
_HASP_ATTRIBUTE(BLEND_MODE, blend_mode, lv_blend_mode_t)
// _HASP_ATTRIBUTE(BORDER_COLOR, border_color, lv_color_t)
// _HASP_ATTRIBUTE(BORDER_COLOR_FILTERED, border_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(BORDER_OPA, border_opa, lv_opa_t)
_HASP_ATTRIBUTE(BORDER_POST, border_post, bool)
_HASP_ATTRIBUTE(BORDER_SIDE, border_side, lv_border_side_t)
_HASP_ATTRIBUTE(BORDER_WIDTH, border_width, lv_coord_t)
_HASP_ATTRIBUTE(CLIP_CORNER, clip_corner, bool)
_HASP_ATTRIBUTE(COLOR_FILTER_DSC, color_filter_dsc, const lv_color_filter_dsc_t*)
_HASP_ATTRIBUTE(COLOR_FILTER_OPA, color_filter_opa, lv_opa_t)
_HASP_ATTRIBUTE(HEIGHT, height, lv_coord_t)
_HASP_ATTRIBUTE(IMG_OPA, img_opa, lv_opa_t)
// _HASP_ATTRIBUTE(IMG_RECOLOR, img_recolor, lv_color_t)
// _HASP_ATTRIBUTE(IMG_RECOLOR_FILTERED, img_recolor_filtered, lv_color_t)
_HASP_ATTRIBUTE(IMG_RECOLOR_OPA, img_recolor_opa, lv_opa_t)
_HASP_ATTRIBUTE(LAYOUT, layout, uint16_t)
// _HASP_ATTRIBUTE(LINE_COLOR, line_color, lv_color_t)
// _HASP_ATTRIBUTE(LINE_COLOR_FILTERED, line_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(LINE_DASH_GAP, line_dash_gap, lv_coord_t)
_HASP_ATTRIBUTE(LINE_DASH_WIDTH, line_dash_width, lv_coord_t)
_HASP_ATTRIBUTE(LINE_OPA, line_opa, lv_opa_t)
_HASP_ATTRIBUTE(LINE_ROUNDED, line_rounded, lv_coord_t)
_HASP_ATTRIBUTE(LINE_WIDTH, line_width, lv_coord_t)
_HASP_ATTRIBUTE(MAX_HEIGHT, max_height, lv_coord_t)
_HASP_ATTRIBUTE(MAX_WIDTH, max_width, lv_coord_t)
_HASP_ATTRIBUTE(MIN_HEIGHT, min_height, lv_coord_t)
_HASP_ATTRIBUTE(MIN_WIDTH, min_width, lv_coord_t)
_HASP_ATTRIBUTE(OPA, opa, lv_opa_t)
// _HASP_ATTRIBUTE(OUTLINE_COLOR, outline_color, lv_color_t)
// _HASP_ATTRIBUTE(OUTLINE_COLOR_FILTERED, outline_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(OUTLINE_OPA, outline_opa, lv_opa_t)
_HASP_ATTRIBUTE(OUTLINE_PAD, outline_pad, lv_coord_t)
_HASP_ATTRIBUTE(OUTLINE_WIDTH, outline_width, lv_coord_t)
_HASP_ATTRIBUTE(PAD_BOTTOM, pad_bottom, lv_coord_t)
_HASP_ATTRIBUTE(PAD_COLUMN, pad_column, lv_coord_t)
_HASP_ATTRIBUTE(PAD_LEFT, pad_left, lv_coord_t)
_HASP_ATTRIBUTE(PAD_RIGHT, pad_right, lv_coord_t)
_HASP_ATTRIBUTE(PAD_ROW, pad_row, lv_coord_t)
_HASP_ATTRIBUTE(PAD_TOP, pad_top, lv_coord_t)
_HASP_ATTRIBUTE(RADIUS, radius, lv_coord_t)
// _HASP_ATTRIBUTE(SHADOW_COLOR, shadow_color, lv_color_t)
// _HASP_ATTRIBUTE(SHADOW_COLOR_FILTERED, shadow_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(SHADOW_OFS_X, shadow_ofs_x, lv_coord_t)
_HASP_ATTRIBUTE(SHADOW_OFS_Y, shadow_ofs_y, lv_coord_t)
_HASP_ATTRIBUTE(SHADOW_OPA, shadow_opa, lv_opa_t)
_HASP_ATTRIBUTE(SHADOW_SPREAD, shadow_spread, lv_coord_t)
_HASP_ATTRIBUTE(SHADOW_WIDTH, shadow_width, lv_coord_t)
_HASP_ATTRIBUTE(TEXT_ALIGN, text_align, lv_text_align_t)
// _HASP_ATTRIBUTE(TEXT_COLOR, text_color, lv_color_t)
// _HASP_ATTRIBUTE(TEXT_COLOR_FILTERED, text_color_filtered, lv_color_t)
_HASP_ATTRIBUTE(TEXT_DECOR, text_decor, lv_text_decor_t)
_HASP_ATTRIBUTE(TEXT_FONT, text_font, const lv_font_t*)
_HASP_ATTRIBUTE(TEXT_LETTER_SPACE, text_letter_space, lv_coord_t)
_HASP_ATTRIBUTE(TEXT_LINE_SPACE, text_line_space, lv_coord_t)
_HASP_ATTRIBUTE(TEXT_OPA, text_opa, lv_opa_t)
_HASP_ATTRIBUTE(TRANSFORM_ANGLE, transform_angle, lv_coord_t)
_HASP_ATTRIBUTE(TRANSFORM_HEIGHT, transform_height, lv_coord_t)
_HASP_ATTRIBUTE(TRANSFORM_WIDTH, transform_width, lv_coord_t)
_HASP_ATTRIBUTE(TRANSFORM_ZOOM, transform_zoom, lv_coord_t)
_HASP_ATTRIBUTE(TRANSITION, transition, const lv_style_transition_dsc_t*)
_HASP_ATTRIBUTE(TRANSLATE_X, translate_x, lv_coord_t)
_HASP_ATTRIBUTE(TRANSLATE_Y, translate_y, lv_coord_t)
_HASP_ATTRIBUTE(WIDTH, width, lv_coord_t)
_HASP_ATTRIBUTE(X, x, lv_coord_t)
_HASP_ATTRIBUTE(Y, y, lv_coord_t)

#if LVGL_VERSION_MAJOR == 7
_HASP_ATTRIBUTE(PAD_INNER, pad_inner, lv_style_int_t)
#endif

#if LV_USE_BLEND_MODES
_HASP_ATTRIBUTE(BG_BLEND_MODE, bg_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(BORDER_BLEND_MODE, border_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(OUTLINE_BLEND_MODE, outline_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(SHADOW_BLEND_MODE, shadow_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(PATTERN_BLEND_MODE, pattern_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(VALUE_BLEND_MODE, value_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(TEXT_BLEND_MODE, text_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(LINE_BLEND_MODE, line_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(IMAGE_BLEND_MODE, image_blend_mode, lv_blend_mode_t)
#endif

#if LV_USE_SHADOW
_HASP_ATTRIBUTE(SHADOW_WIDTH, shadow_width, lv_style_int_t)
_HASP_ATTRIBUTE(SHADOW_OFS_X, shadow_ofs_x, lv_style_int_t)
_HASP_ATTRIBUTE(SHADOW_OFS_Y, shadow_ofs_y, lv_style_int_t)
_HASP_ATTRIBUTE(SHADOW_SPREAD, shadow_spread, lv_style_int_t)
//_HASP_ATTRIBUTE(SHADOW_COLOR, shadow_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(SHADOW_OPA, shadow_opa, lv_opa_t)
#endif

/* attribute hashes */
/* Object Part Attributes */
#define ATTR_SIZE 16417
#define ATTR_RADIUS 20786
#define ATTR_CLIP_CORNER 9188
#define ATTR_OPA_SCALE 64875
#define ATTR_TRANSFORM_HEIGHT 55994
#define ATTR_TRANSFORM_WIDTH 48627

/* Background Attributes */
#define ATTR_BG_OPA 48966
#define ATTR_BG_COLOR 64969
#define ATTR_BG_GRAD_DIR 41782
#define ATTR_BG_GRAD_STOP 4025
#define ATTR_BG_MAIN_STOP 63118
#define ATTR_BG_BLEND_MODE 31147
#define ATTR_BG_GRAD_COLOR 44140

/* Margin Attributes */
#define ATTR_MARGIN_TOP 7812
#define ATTR_MARGIN_LEFT 24440
#define ATTR_MARGIN_BOTTOM 37692
#define ATTR_MARGIN_RIGHT 2187

/* Padding Attributes */
#define ATTR_PAD_TOP 59081
#define ATTR_PAD_LEFT 43123
#define ATTR_PAD_INNER 9930
#define ATTR_PAD_RIGHT 65104
#define ATTR_PAD_BOTTOM 3767

/* Text Attributes */
#define ATTR_TEXT_OPA 37166
#define ATTR_TEXT_FONT 22465
#define ATTR_TEXT_COLOR 23473
#define ATTR_TEXT_DECOR 1971
#define ATTR_TEXT_LETTER_SPACE 62079
#define ATTR_TEXT_SEL_COLOR 32076
#define ATTR_TEXT_LINE_SPACE 54829
#define ATTR_TEXT_BLEND_MODE 32195

/* Border Attributes */
#define ATTR_BORDER_OPA 2061
#define ATTR_BORDER_SIDE 53962
#define ATTR_BORDER_POST 49491
#define ATTR_BORDER_BLEND_MODE 23844
#define ATTR_BORDER_WIDTH 24531
#define ATTR_BORDER_COLOR 21264

/* Outline Attributes */
#define ATTR_OUTLINE_OPA 23011
#define ATTR_OUTLINE_PAD 26038
#define ATTR_OUTLINE_COLOR 6630
#define ATTR_OUTLINE_BLEND_MODE 25038
#define ATTR_OUTLINE_WIDTH 9897

/* Shadow Attributes */
#define ATTR_SHADOW_OPA 38401
#define ATTR_SHADOW_WIDTH 13255
#define ATTR_SHADOW_OFS_X 44278
#define ATTR_SHADOW_OFS_Y 44279
#define ATTR_SHADOW_SPREAD 21138
#define ATTR_SHADOW_BLEND_MODE 64048
#define ATTR_SHADOW_COLOR 9988

/* Line Attributes */
#define ATTR_LINE_OPA 24501
#define ATTR_LINE_WIDTH 25467
#define ATTR_LINE_COLOR 22200
#define ATTR_LINE_DASH_WIDTH 32676
#define ATTR_LINE_ROUNDED 15042
#define ATTR_LINE_DASH_GAP 49332
#define ATTR_LINE_BLEND_MODE 60284

/* Value Attributes */
#define ATTR_VALUE_OPA 50482
#define ATTR_VALUE_STR 1091
#define ATTR_VALUE_FONT 9405
#define ATTR_VALUE_ALIGN 27895
#define ATTR_VALUE_COLOR 52661
#define ATTR_VALUE_OFS_X 21415
#define ATTR_VALUE_OFS_Y 21416
#define ATTR_VALUE_LINE_SPACE 26921
#define ATTR_VALUE_BLEND_MODE 4287
#define ATTR_VALUE_LETTER_SPACE 51067

/* Pattern attributes */
#define ATTR_PATTERN_BLEND_MODE 43456
#define ATTR_PATTERN_RECOLOR_OPA 35074
#define ATTR_PATTERN_RECOLOR 7745
#define ATTR_PATTERN_REPEAT 31338
#define ATTR_PATTERN_OPA 43633
#define ATTR_PATTERN_IMAGE 61292

#define ATTR_TRANSITION_PROP_1 49343
#define ATTR_TRANSITION_PROP_2 49344
#define ATTR_TRANSITION_PROP_3 49345
#define ATTR_TRANSITION_PROP_4 49346
#define ATTR_TRANSITION_PROP_5 49347
#define ATTR_TRANSITION_PROP_6 49348
#define ATTR_TRANSITION_TIME 26263
#define ATTR_TRANSITION_PATH 43343
#define ATTR_TRANSITION_DELAY 64537

#define ATTR_IMAGE_OPA 58140
#define ATTR_IMAGE_RECOLOR 52204
#define ATTR_IMAGE_BLEND_MODE 11349
#define ATTR_IMAGE_RECOLOR_OPA 43949

#define ATTR_SCALE_END_LINE_WIDTH 30324
#define ATTR_SCALE_END_BORDER_WIDTH 34380
#define ATTR_SCALE_BORDER_WIDTH 2440
#define ATTR_SCALE_GRAD_COLOR 47239
#define ATTR_SCALE_WIDTH 36017
#define ATTR_SCALE_END_COLOR 44074

/* Page Attributes */
#define ATTR_NEXT 60915
#define ATTR_PREV 21587
#define ATTR_BACK 57799

/* Object Attributes */
#define ATTR_X 120
#define ATTR_Y 121
#define ATTR_W 119
#define ATTR_H 104
#define ATTR_OPTIONS 29886
#define ATTR_ENABLED 28193
#define ATTR_CLICK 17064
#define ATTR_OPACITY 10155
#define ATTR_TOGGLE 38580
#define ATTR_HIDDEN 11082
#define ATTR_VIS 16320
#define ATTR_SWIPE 11802
#define ATTR_MODE 45891
// #define ATTR_RECT 11204
#define ATTR_ALIGN 34213
#define ATTR_ROWS 52153
#define ATTR_COLS 36307
#define ATTR_MIN 46130
#define ATTR_MAX 45636
#define ATTR_VAL 15809
#define ATTR_COLOR 58979
#define ATTR_TXT 9328
#define ATTR_TEXT 53869
#define ATTR_SRC 4964
#define ATTR_ID 6715
#define ATTR_EXT_CLICK_H 46643
#define ATTR_EXT_CLICK_V 46657
#define ATTR_ANIM_TIME 59451
#define ATTR_ANIM_SPEED 281
#define ATTR_START_VALUE 11828
#define ATTR_COMMENT 62559

// methods
#define ATTR_DELETE 50027
#define ATTR_CLEAR 1069
#define ATTR_TO_FRONT 44741
#define ATTR_TO_BACK 24555

// Gauge
#define ATTR_CRITICAL_VALUE 39281
#define ATTR_ANGLE 2387
#define ATTR_LABEL_COUNT 20356
#define ATTR_LINE_COUNT 57860
#define ATTR_FORMAT 38871

// Arc
#define ATTR_TYPE 1658
#define ATTR_ROTATION 44830
#define ATTR_ADJUSTABLE 19145
#define ATTR_START_ANGLE 44310
#define ATTR_END_ANGLE 41103
#define ATTR_START_ANGLE1 39067
#define ATTR_END_ANGLE1 33634

// Dropdown
#define ATTR_DIRECTION 32415
#define ATTR_SYMBOL 33592
#define ATTR_OPEN 25738
#define ATTR_CLOSE 41880
#define ATTR_MAX_HEIGHT 30946
#define ATTR_SHOW_SELECTED 56029

// Buttonmatrix
#define ATTR_ONE_CHECK 45935

// Tabview
#define ATTR_BTN_POS 35697
#define ATTR_COUNT 29103

// Msgbox
#define ATTR_MODAL 7405
#define ATTR_AUTO_CLOSE 7880

// Image
#define ATTR_OFFSET_X 65388
#define ATTR_OFFSET_Y 65389
#define ATTR_AUTO_SIZE 63729

// Spinner
#define ATTR_SPEED 14375
#define ATTR_THICKNESS 24180
//#define ATTR_ARC_LENGTH 755 - use ATTR_ANGLE
// #define ATTR_DIRECTION 32415 - see Dropdown

// Line
#define ATTR_POINTS 8643
#define ATTR_Y_INVERT 44252

/* hasp user data */
#define ATTR_ACTION 42102
#define ATTR_TRANSITION 10933
#define ATTR_GROUPID 48986
#define ATTR_OBJID 41010
#define ATTR_OBJ 53623

#define ATTR_TEXT_MAC 38107
#define ATTR_TEXT_IP 41785
#define ATTR_TEXT_HOSTNAME 10125
#define ATTR_TEXT_MODEL 54561
#define ATTR_TEXT_VERSION 60178
#define ATTR_TEXT_SSID 62981

#endif

#endif