#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

// #include "lv_conf_v7.h"
// #define LV_THEME_DEFAULT_FLAGS LV_THEME_DEFAULT_FLAG

#include "lv_conf_v8.h"
#include "lv_symbol_mdi_def.h"

#ifndef LV_CONF_STUB_H
#define LV_CONF_STUB_H

#define LV_THEME_DEFAULT_FLAG 0
#define LV_THEME_DEFAULT_FLAGS LV_THEME_DEFAULT_FLAG

//#ifdef LV_THEME_DEFAULT_FLAG

//#define lv_task_handler lv_timer_handler

#define lv_obj_set_click(obj, en)                                                                                      \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE))
#define lv_obj_get_click(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE))

#define lv_obj_set_hidden(obj, en)                                                                                     \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN))
#define lv_obj_get_hidden(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN))

#define lv_btn_set_checkable(obj, en)                                                                                  \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_CHECKABLE))
#define lv_btn_get_checkable(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_CHECKABLE))

#define lv_checkbox_set_checked(obj, en)                                                                               \
    ((en) ? lv_obj_add_state(obj, LV_STATE_CHECKED) : lv_obj_clear_state(obj, LV_STATE_CHECKED))
#define lv_checkbox_is_checked(obj) ((lv_obj_get_state(obj) & LV_STATE_CHECKED) ? true : false)

//#define lv_chart_set_range(chart, ymin, ymax) lv_chart_set_y_range(chart, LV_CHART_AXIS_PRIMARY_Y, ymin, ymax)
#define lv_obj_set_style_local_pad_inner lv_obj_set_style_local_pad_top
#define lv_dropdown_set_draw_arrow(obj, en) lv_dropdown_set_symbol(obj, en ? LV_SYMBOL_DOWN : NULL)

#define lv_cont_set_fit(obj, fit)
#define lv_cont_set_layout(obj, fit)
#define lv_roller_set_auto_fit(obj, fit)
#define lv_obj_set_top(obj, fit)
#define lv_obj_add_protect(a, b)
#define LV_PROTECT_PRESS_LOST 0

#define LV_BTN_STATE_PRESSED LV_STATE_PRESSED
#define LV_BTN_STATE_DISABLED LV_STATE_DISABLED
#define LV_BTN_STATE_CHECKED_RELEASED LV_STATE_CHECKED
#define LV_BTN_STATE_CHECKED_PRESSED LV_STATE_CHECKED + LV_STATE_PRESSED
#define LV_BTN_STATE_CHECKED_DISABLED LV_STATE_CHECKED + LV_STATE_DISABLED
#define LV_BTN_STATE_RELEASED LV_STATE_DEFAULT

#define lv_btn_set_state lv_obj_set_state
#define lv_btn_get_state lv_obj_get_state
#define lv_btn_state_t lv_state_t

#define lv_disp_buf_t lv_disp_draw_buf_t
#define lv_disp_buf_init lv_disp_draw_buf_init

#define lv_switch_get_state lv_obj_get_state

#define LV_CPICKER_PART_MAIN LV_COLORWHEEL_PART_MAIN
#define LV_CPICKER_PART_KNOB LV_COLORWHEEL_PART_KNOB
#define LV_CPICKER_TYPE_RECT 0
#define LV_CPICKER_TYPE_DISC 0
#define lv_cpicker_get_color lv_colorwheel_get_rgb
#define lv_cpicker_set_color lv_colorwheel_set_rgb
#define lv_cpicker_create lv_colorwheel_create
#define lv_cont_create lv_obj_create
#define lv_page_create lv_obj_create

#define LV_BAR_PART_BG LV_BAR_PART_MAIN
#define LV_CHECKBOX_PART_BG LV_CHECKBOX_PART_MAIN
#define LV_PAGE_PART_SCROLLBAR

#define LV_TEXTAREA_PART_BG LV_PART_MAIN
#define LV_BTNMATRIX_PART_BG LV_PART_MAIN
#define LV_KEYBOARD_PART_BG LV_PART_MAIN
#define LV_KEYBOARD_PART_BTN LV_PART
#define LV_SPINBOX_PART_BG LV_PART_MAIN
#define LV_SWITCH_PART_BG LV_PART_MAIN
#define LV_SLIDER_PART_BG LV_PART_MAIN
#define LV_DROPDOWN_PART_SCROLLBAR LV_DROPDOWN_PART_LIST        // ??
#define LV_TEXTAREA_PART_SCROLLBAR LV_TEXTAREA_PART_PLACEHOLDER // ??

#define LV_THEME_DEFAULT_FLAGS LV_THEME_DEFAULT_FLAG

#define lv_textarea_set_cursor_hidden(a, b)

#define lv_img_get_file_name(img)                                                                                      \
    (((lv_img_ext_t*)lv_obj_get_ext_attr(img))->src_type == LV_IMG_SRC_FILE)                                           \
        ? (const char*)((lv_img_ext_t*)lv_obj_get_ext_attr(img))->src                                                  \
        : ""

// For hasp_theme.c
#define lv_obj_refresh_style _lv_obj_refresh_style
#define lv_obj_get_style_list _lv_obj_get_style_list
#define lv_obj_add_style_list _lv_obj_add_style_list
#define lv_obj_report_style_mod lv_obj_report_style_change

#define lv_keyboard_set_cursor_manage(a, b)
#define lv_obj_set_style_value_str(a, b, c)

#define LV_OBJ_PART_MAIN LV_PART_MAIN
#define LV_BTN_PART_MAIN LV_PART_MAIN

#define LV_THEME_DEFAULT_FONT_SMALL LV_FONT_DEFAULT
#define LV_THEME_DEFAULT_FONT_NORMAL LV_FONT_DEFAULT
#define LV_THEME_DEFAULT_FONT_SUBTITLE LV_FONT_DEFAULT
#define LV_THEME_DEFAULT_FONT_TITLE LV_FONT_DEFAULT

#define LV_COLOR_BLACK lv_color_black()
#define LV_COLOR_WHITE lv_color_white()
#define LV_COLOR_SILVER lv_color_make(0xff, 0xff, 0xff)
#define LV_COLOR_RED lv_color_make(0xff, 0xff, 0xff)
#define LV_COLOR_GRAY lv_color_make(0xff, 0xff, 0xff)

#define LV_IMG_CACHE_DEF_SIZE_PSRAM 20

#define _lv_memset_00(p, size) memset(p, 0, size);
#define LV_ASSERT_MEM(x)

//#endif
#endif
