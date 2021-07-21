#ifndef LV_COMPAT_V7_H
#define LV_COMPAT_V7_H

#if 1

/*********************
 *     v7 COLORS
 *********************/
#define LV_COLOR_WHITE lv_color_white()
#define LV_COLOR_BLACK lv_color_black()
#define LV_COLOR_SILVER lv_color_make(0xC0, 0xC0, 0xC0)
#define LV_COLOR_GRAY lv_color_make(0x80, 0x80, 0x80)
#define LV_COLOR_RED lv_color_make(0xFF, 0x00, 0x00)
#define LV_COLOR_MAROON lv_color_make(0x80, 0x00, 0x00)
#define LV_COLOR_YELLOW lv_color_make(0xFF, 0xFF, 0x00)
#define LV_COLOR_OLIVE lv_color_make(0x80, 0x80, 0x00)
#define LV_COLOR_LIME lv_color_make(0x00, 0xFF, 0x00)
#define LV_COLOR_GREEN lv_color_make(0x00, 0x80, 0x00)
#define LV_COLOR_CYAN lv_color_make(0x00, 0xFF, 0xFF)
#define LV_COLOR_AQUA LV_COLOR_CYAN
#define LV_COLOR_TEAL lv_color_make(0x00, 0x80, 0x80)
#define LV_COLOR_BLUE lv_color_make(0x00, 0x00, 0xFF)
#define LV_COLOR_NAVY lv_color_make(0x00, 0x00, 0x80)
#define LV_COLOR_MAGENTA lv_color_make(0xFF, 0x00, 0xFF)
#define LV_COLOR_PURPLE lv_color_make(0x80, 0x00, 0x80)
#define LV_COLOR_ORANGE lv_color_make(0xFF, 0xA5, 0x00)

/*********************
 *     v7 STATES
 *********************/
#define LV_INDEV_STATE_PR LV_INDEV_STATE_PRESSED
#define LV_INDEV_STATE_REL LV_INDEV_STATE_RELEASED

#define LV_BTN_STATE_PRESSED LV_STATE_PRESSED
#define LV_BTN_STATE_DISABLED LV_STATE_DISABLED
#define LV_BTN_STATE_CHECKED_RELEASED LV_STATE_CHECKED
#define LV_BTN_STATE_CHECKED_PRESSED LV_STATE_CHECKED + LV_STATE_PRESSED
#define LV_BTN_STATE_CHECKED_DISABLED LV_STATE_CHECKED + LV_STATE_DISABLED
#define LV_BTN_STATE_RELEASED LV_STATE_DEFAULT

// State types
// #define lv_indev_state_t lv_indev_state_t
// #define lv_get_state_info_t ?
#define lv_style_state_t lv_state_t
#define lv_btn_state_t lv_state_t

#define lv_btn_get_state lv_obj_get_state
#define lv_btn_set_state lv_obj_set_state
#define lv_btn_get_checkable(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_CHECKABLE))
#define lv_btn_set_checkable(obj, en)                                                                                  \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_CHECKABLE))

// lv_checkbox_is_inactive (C++ function, in Checkbox (lv_cb)) // Todo
#define lv_checkbox_get_state lv_obj_get_state
#define lv_checkbox_set_state lv_obj_set_state
#define lv_checkbox_is_checked(obj) ((lv_obj_get_state(obj) & LV_STATE_CHECKED) ? true : false)
#define lv_checkbox_set_checked(obj, en)                                                                               \
    ((en) ? lv_obj_add_state(obj, LV_STATE_CHECKED) : lv_obj_clear_state(obj, LV_STATE_CHECKED))

#define lv_imgbtn_get_state lv_obj_get_state
#define lv_imgbtn_set_state lv_obj_set_state
#define lv_imgbtn_get_checkable(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_CHECKABLE))
#define lv_imgbtn_set_checkable(obj, en)                                                                               \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_CHECKABLE))

#define lv_switch_get_state lv_obj_get_state
#define lv_switch_set_state lv_obj_set_state

#define lv_obj_get_click(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE))
#define lv_obj_set_click(obj, en)                                                                                      \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE))

#define lv_obj_get_hidden(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN))
#define lv_obj_set_hidden(obj, en)                                                                                     \
    ((en) ? lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN) : lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN))

#define lv_obj_is_focused(obj) (lv_obj_has_flag(obj, LV_OBJ_FLAG_FOCUSED))
// lv_obj_is_protected (C++ function, in Base object (lv_obj)) // Todo

/*********************
 *     v7 CORE WIDGETS PARTS
 *********************/
#define LV_ARC_PART_BG LV_PART_MAIN
#define LV_ARC_PART_KNOB LV_PART_KNOB
#define LV_ARC_PART_INDIC LV_PART_INDICATOR

#define LV_BAR_PART_BG LV_PART_MAIN
#define LV_BAR_PART_INDIC LV_PART_INDICATOR

#define LV_BTN_PART_MAIN LV_PART_MAIN

#define LV_BTNMATRIX_PART_BG LV_PART_MAIN
#define LV_BTNMATRIX_PART_BTN LV_PART_ITEMS

#define LV_CANVAS_PART_MAIN LV_PART_MAIN

#define LV_CHECKBOX_PART_BG LV_PART_MAIN
#define LV_CHECKBOX_PART_BULLET_PART_INDIC LV_PART_INDICATOR

#define LV_DROPDOWN_PART_MAIN LV_PART_MAIN
#define LV_DROPDOWN_PART_LIST LV_PART_MAIN           // Use lv_dropdown_get_list() to get the list object
#define LV_DROPDOWN_PART_SCROLLBAR LV_PART_SCROLLBAR // Use lv_dropdown_get_list() to get the list object
#define LV_DROPDOWN_PART_SELECTED LV_PART_SELECTED   // Use lv_dropdown_get_list() to get the list object

#define LV_IMG_PART_MAIN LV_PART_MAIN

#define LV_LABEL_PART_MAIN LV_PART_MAIN

#define LV_LINE_PART_MAIN LV_PART_MAIN

#define LV_OBJ_PART_MAIN LV_PART_MAIN

#define LV_ROLLER_PART_BG LV_PART_MAIN
#define LV_ROLLER_PART_SELECTED LV_PART_SELECTED

#define LV_SLIDER_PART_BG LV_PART_MAIN
#define LV_SLIDER_PART_KNOB LV_PART_KNOB
#define LV_SLIDER_PART_INDIC LV_PART_INDICATOR

#define LV_SWITCH_PART_BG LV_PART_MAIN
#define LV_SWITCH_PART_KNOB LV_PART_KNOB
#define LV_SWITCH_PART_INDIC LV_PART_INDICATOR

#define LV_TABLE_PART_BG LV_PART_MAIN
#define LV_TABLE_PART_CELL1 LV_PART_ITEMS
#define LV_TABLE_PART_CELL2 LV_PART_ITEMS
#define LV_TABLE_PART_CELL3 LV_PART_ITEMS
#define LV_TABLE_PART_CELL4 LV_PART_ITEMS

#define LV_TEXTAREA_PART_BG LV_PART_MAIN
#define LV_TEXTAREA_PART_SCROLLBAR LV_PART_SCROLLBAR
#define LV_TEXTAREA_PART_EDGE_FLASH /* edge flash is removed */
#define LV_TEXTAREA_PART_CURSOR LV_PART_CURSOR
#define LV_TEXTAREA_PART_PLACEHOLDER LV_PART_TEXTAREA_PLACEHOLDER

/*********************
 *     v7 OBSOLETE WIDGETS PARTS
 *********************/
#define LV_CONT_PART_MAIN LV_PART_MAIN // Use obj instead

#define LV_GAUGE_PART_MAIN LV_PART_MAIN // Use meter instead
#define LV_GAUGE_PART_MAJOR             // Use meter instead
#define LV_GAUGE_PART_NEEDLE            // Use meter instead

#define LV_LINEMETER_PART_MAIN LV_PART_MAIN // Use meter instead

#define LV_OBJMASK_PART_MAIN LV_PART_MAIN

/* Use obj instead of page */
#define LV_PAGE_PART_BG LV_PART_MAIN
#define LV_PAGE_PART_SCROLLBAR LV_PART_SCROLLBAR
#define LV_PAGE_PART_EDGE_FLASH /* edge flash is removed */
#define LV_PAGE_PART_SCROLLABLE LV_PART_MAIN

/*********************
 *     v7 EXTRA WIDGETS PARTS
 *********************/
#define LV_CALENDAR_PART_BG LV_PART_MAIN
#define LV_CALENDAR_PART_HEADER LV_PART_MAIN
#define LV_CALENDAR_PART_DAY_NAMES LV_PART_ITEMS
#define LV_CALENDAR_PART_DATE LV_PART_ITEMS

#define LV_CHART_PART_BG LV_PART_MAIN
#define LV_CHART_PART_SERIES_BG LV_PART_ITEMS
#define LV_CHART_PART_SERIES LV_PART_ITEMS
#define LV_CHART_PART_CURSOR LV_PART_CURSOR

#define LV_COLORWHEEL_PART_MAIN LV_PART_MAIN
#define LV_COLORWHEEL_PART_KNOB LV_PART_KNOB
#define LV_CPICKER_PART_MAIN LV_COLORWHEEL_PART_MAIN
#define LV_CPICKER_PART_KNOB LV_COLORWHEEL_PART_KNOB

#define LV_IMGBTN_PART_MAIN LV_PART_MAIN

#define LV_KEYBOARD_PART_BG LV_PART_MAIN
#define LV_KEYBOARD_PART_BTN LV_PART_ITEMS

#define LV_LED_PART_MAIN LV_PART_MAIN

#define LV_LIST_PART_BG LV_PART_MAIN
#define LV_LIST_PART_SCROLLBAR LV_PART_SCROLLBAR
#define LV_LIST_PART_EDGE_FLASH /* edge flash is removed */
#define LV_LIST_PART_SCROLLABLE LV_PART_MAIN

/* The mesasge box is built from other widgets */
// #define LV_MSGBOX_PART_BG LV_PART_MAIN
// #define LV_MSGBOX_PART_BTN_BG
// #define LV_MSGBOX_PART_BTN

#define LV_SPINBOX_PART_BG LV_PART_MAIN
#define LV_SPINBOX_PART_CURSOR LV_PART_CURSOR

#define LV_SPINNER_PART_BG LV_PART_MAIN
#define LV_SPINNER_PART_INDIC LV_PART_INDICATOR

/* There are no special parts on the Tab view but the lv_obj and lv_btnnmatrix widgets are used to create the Tab view.
 */
// #define LV_TABVIEW_PART_BG
// #define LV_TABVIEW_PART_BG_SCROLLABLE
// #define LV_TABVIEW_PART_TAB_BG
// #define LV_TABVIEW_PART_TAB_BTN
// #define LV_TABVIEW_PART_INDIC LV_PART_INDICATOR

/* The Tile view is built from an lv_obj container and lv_obj tiles. */
// #define LV_TILEVIEW_PART_BG LV_PART_MAIN
// #define LV_TILEVIEW_PART_SCROLLBAR LV_PART_SCROLLBAR
// #define LV_TILEVIEW_PART_EDGE_FLASH /* edge flash is removed */

/* The Window is built from other widgets so you can check their documentation */
// #define LV_WIN_PART_BG LV_PART_MAIN
// #define LV_WIN_PART_HEADER
// #define LV_WIN_PART_CONTENT_SCROLLABLE
// #define LV_WIN_PART_SCROLLBAR LV_PART_SCROLLBAR

/*********************
 *     v7 COMPATIBILITY TYPES
 *********************/
#define lv_style_int_t int32_t // lv_style_value_t::num
#define lv_anim_value_t lv_coord_t
#define lv_disp_buf_t lv_disp_draw_buf_t

/*********************
 *     v7 COMPATIBILITY APIS
 *********************/
#define lv_cont_create lv_obj_create // use base object instead
#define lv_page_create lv_obj_create // use base object instead
#define lv_cpicker_create lv_colorwheel_create
#define lv_cpicker_get_color lv_colorwheel_get_rgb
#define lv_cpicker_set_color lv_colorwheel_set_rgb
#define lv_dropdown_set_draw_arrow(obj, en) lv_dropdown_set_symbol(obj, en ? LV_SYMBOL_DOWN : NULL)
//#define lv_chart_set_range(chart, ymin, ymax) lv_chart_set_y_range(chart, LV_CHART_AXIS_PRIMARY_Y, ymin, ymax)
#define lv_img_get_file_name(img)                                                                                      \
    (((lv_img_ext_t*)lv_obj_get_ext_attr(img))->src_type == LV_IMG_SRC_FILE)                                           \
        ? (const char*)((lv_img_ext_t*)lv_obj_get_ext_attr(img))->src                                                  \
        : ""
#define lv_disp_buf_init lv_disp_draw_buf_init
#define lv_obj_refresh_style _lv_obj_refresh_style
#define lv_obj_get_style_list _lv_obj_get_style_list
#define lv_obj_add_style_list _lv_obj_add_style_list
#define lv_obj_report_style_mod lv_obj_report_style_change

/*********************
 *     v7 OBSOLETE APIS
 *********************/
#define lv_btn_set_fit(obj, fit)
#define lv_roller_set_auto_fit(obj, fit)
#define lv_keyboard_set_cursor_manage(a, b)
#define lv_cont_set_fit(obj, fit)
#define lv_cont_set_fit2(obj, fit)
#define lv_cont_set_fit4(obj, fit)
#define lv_cont_set_layout(obj, layout) // Use: lv_obj_set_layout(obj, type) // with LV_LAYOUT_FLEX or LV_LAYOUT_GRID
#define lv_obj_set_style_value_str(a, b, c)
#define lv_textarea_set_cursor_hidden(a, b)

#endif // LV_VERSION_CHECK(8, 0, 0)

#endif // LV_COMPAT_V7_H