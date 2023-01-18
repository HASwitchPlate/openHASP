/**
 * @file lv_theme_hasp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"          /*To see all the widgets*/
#include "lv_conf.h"       /*To see all the widgets*/
#include "lv_theme_hasp.h" /*To see all the widgets*/

//#if LV_USE_THEME_HASP

#include "src/lv_misc/lv_gc.h"

#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif /* LV_ENABLE_GC */

/*********************
 *      DEFINES
 *********************/

#define HASP_DPX(n) hasp_dpx(n)

/*SCREEN*/

#define COLOR_SCR (IS_LIGHT ? LV_COLOR_WHITE : lv_color_hex(0x545b6a))
#define COLOR_SCR_GRAD (IS_LIGHT ? LV_COLOR_SILVER : lv_color_hex(0x222b3a))
#define COLOR_SCR_TEXT (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0xefefef))
//#define HUE_COLOR lv_color_hex(0x2F8CD8)
#define HUE_COLOR theme.color_primary

/*BUTTON*/
#define COLOR_BTN (lv_color_lighten(HUE_COLOR, LV_OPA_30))
#define COLOR_BTN_GRAD (lv_color_darken(HUE_COLOR, LV_OPA_30))
#define COLOR_BTN_PR (lv_color_darken(COLOR_BTN_CHK, LV_OPA_20))
#define COLOR_BTN_PR_GRAD lv_color_darken(COLOR_BTN_GRAD, LV_OPA_20)
#define COLOR_BTN_CHK lv_color_darken(COLOR_BTN_GRAD, LV_OPA_20)
#define COLOR_BTN_CHK_GRAD lv_color_darken(COLOR_BTN_GRAD, LV_OPA_10)
#define COLOR_BTN_CHK_PR (lv_color_darken(COLOR_BTN_CHK, LV_OPA_20))
#define COLOR_BTN_CHK_PR_GRAD (lv_color_darken(COLOR_BTN_CHK_GRAD, LV_OPA_30))
#define COLOR_BTN_DIS (IS_LIGHT ? lv_color_hex3(0xccc) : lv_color_hex3(0x888))

#define COLOR_BTN_BORDER (IS_LIGHT ? (lv_color_darken(HUE_COLOR, LV_OPA_50)) : (lv_color_lighten(HUE_COLOR, LV_OPA_40)))
#define COLOR_BTN_BORDER_PR (IS_LIGHT ? lv_color_hex3(0xccc) : lv_color_hex(0x5f656e))
#define COLOR_BTN_BORDER_CHK (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0x5f656e))
#define COLOR_BTN_BORDER_CHK_PR (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0x5f656e))
#define COLOR_BTN_BORDER_INA (IS_LIGHT ? lv_color_hex3(0x888) : lv_color_hex(0x404040))

/*BACKGROUND*/
#define COLOR_BG COLOR_BTN
#define COLOR_BG_PR COLOR_BTN_PR
#define COLOR_BG_CHK COLOR_BTN_CHK
#define COLOR_BG_PR_CHK COLOR_BTN_CHK_PR
#define COLOR_BG_DIS COLOR_BG

#define COLOR_BG_BORDER (IS_LIGHT ? lv_color_hex(0xd6dde3) : lv_color_hex(0x808a97)) /*dfe7ed*/
#define COLOR_BG_BORDER_PR (IS_LIGHT ? lv_color_hex3(0xccc) : lv_color_hex(0x5f656e))
#define COLOR_BG_BORDER_CHK (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0x5f656e))
#define COLOR_BG_BORDER_CHK_PR (IS_LIGHT ? lv_color_hex(0x3b3e42) : lv_color_hex(0x5f656e))
#define COLOR_BG_BORDER_DIS (IS_LIGHT ? lv_color_hex(0xd6dde3) : lv_color_hex(0x5f656e))

#define COLOR_BG_TEXT lv_color_hex(0xeeeeee)
#define COLOR_BG_TEXT_PR LV_COLOR_WHITE
#define COLOR_BG_TEXT_CHK LV_COLOR_WHITE
#define COLOR_BG_TEXT_CHK_PR LV_COLOR_WHITE
#define COLOR_BG_TEXT_DIS (IS_LIGHT ? lv_color_hex3(0xaaa) : lv_color_hex3(0x999))

/*SECONDARY BACKGROUND*/
#define COLOR_BG_SEC lv_color_lighten(theme.color_primary, LV_OPA_30)
#define COLOR_BG_GRAD COLOR_BTN_GRAD
#define COLOR_BG_SEC_BORDER (IS_LIGHT ? lv_color_hex(0xdfe7ed) : lv_color_hex(0x404040))
#define COLOR_BG_SEC_TEXT (IS_LIGHT ? lv_color_hex(0x31404f) : lv_color_hex(0xa5a8ad))
#define COLOR_BG_SEC_TEXT_DIS (IS_LIGHT ? lv_color_hex(0xaaaaaa) : lv_color_hex(0xa5a8ad))

#define TRANSITION_TIME 0 /*((theme.flags & LV_THEME_HASP_FLAG_NO_TRANSITION) ? 0 : 150)*/
#define BORDER_WIDTH HASP_DPX(2)
#define BORDER_COLOR ((theme.flags & IS_LIGHT) ? lv_color_make(0x40, 0x40, 0x40) : lv_color_make(0xb0, 0xb0, 0xb0))
#define OUTLINE_WIDTH ((theme.flags & LV_THEME_HASP_FLAG_NO_FOCUS) ? 0 : HASP_DPX(3))
#define OUTLINE_COLOR ((theme.flags & LV_THEME_HASP_FLAG_NO_FOCUS) ? BORDER_COLOR : theme.color_secondary)

#define IS_LIGHT (theme.flags & LV_THEME_HASP_FLAG_LIGHT)
#define NO_FOCUS (theme.flags & LV_THEME_HASP_FLAG_NO_FOCUS)

#define PAD_DEF (lv_disp_get_size_category(NULL) <= LV_DISP_SIZE_MEDIUM ? HASP_DPX(15) : (HASP_DPX(30)))

/*SCROLLBAR*/
#define SCROLLBAR_COLOR                                                                                                \
    (IS_LIGHT ? lv_color_mix(theme.color_primary, lv_color_hex3(0x999), LV_OPA_10)                                     \
              : lv_color_mix(theme.color_primary, lv_color_hex3(0x999), LV_OPA_10))

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{

    lv_style_t transparent;  // Objects with transparent background, like Checkbox, container
    lv_style_t screen;       // Background pages
    lv_style_t pretty;       // Background gradient
    lv_style_t pretty_color; // Primary Colored gradient
    // lv_style_t bg_click;
    // lv_style_t bg_sec;
    lv_style_t btn;
    lv_style_t pad_inner; // Space inner objects
    lv_style_t pad_small; // Inner margin and space inner objects

#if LV_USE_DROPDOWN || LV_USE_ROLLER
    lv_style_t line_space;
    lv_style_t selected;
#endif

#if LV_USE_ARC
    lv_style_t arc_indic;
    lv_style_t arc_bg;
    lv_style_t arc_knob;
#endif

#if LV_USE_BAR
    //    lv_style_t bar_bg;
    //    lv_style_t bar_indic;
#endif

#if LV_USE_CALENDAR
    lv_style_t calendar_date_nums;
#endif

#if LV_USE_CPICKER
    lv_style_t cpicker_bg, cpicker_indic;
#endif

#if LV_USE_CHART
    lv_style_t chart_bg, chart_series_bg, chart_series;
#endif

#if LV_USE_CHECKBOX
    lv_style_t cb_bullet;
#endif

#if LV_USE_GAUGE
    lv_style_t gauge_main, gauge_strong, gauge_needle;
#endif

#if LV_USE_KEYBOARD
    lv_style_t kb_bg;
#endif

#if LV_USE_LED
    lv_style_t led;
#endif

#if LV_USE_LINEMETER
    lv_style_t lmeter;
#endif

#if LV_USE_LIST
    lv_style_t list_btn;
#endif

#if LV_USE_MSGBOX
    lv_style_t mbox_bg;
#endif

#if LV_USE_PAGE
    lv_style_t sb;
#if LV_USE_ANIMATION
    lv_style_t edge_flash;
#endif
#endif // LV_USE_PAGE

#if LV_USE_SLIDER
    lv_style_t slider_knob, slider_bg;
#endif

#if LV_USE_SPINBOX
    lv_style_t spinbox_cursor;
#endif

#if LV_USE_SWITCH
    lv_style_t sw_knob;
#endif

#if LV_USE_TABLE
    lv_style_t table_cell;
#endif

#if LV_USE_TABVIEW || LV_USE_WIN
    lv_style_t tabview_btns, tabview_btns_bg, tabview_indic, tabview_page_scrl;
#endif

#if LV_USE_TEXTAREA
    lv_style_t ta_cursor, ta_placeholder;
#endif

} theme_styles_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void theme_apply(lv_theme_t* th, lv_obj_t* obj, lv_theme_style_t name);
static void style_init_reset(lv_style_t* style);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;
static theme_styles_t* styles;

static bool inited;

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_style_int_t hasp_dpx(lv_style_int_t n)
{
    return (n == 0 ? 0 : LV_MATH_MAX(((LV_DPI * (n) + 80) / 160), 1)); /*+80 for rounding*/
}

static void my_style_set_fg_color(lv_style_t* style, lv_state_t state, lv_color_t color)
{
    lv_style_set_text_color(style, state, color);
    lv_style_set_image_recolor(style, state, color);
    lv_style_set_value_color(style, state, color);
}

static void basic_init(lv_style_int_t border_width, lv_style_int_t outline_width, lv_color_t value_color,
                       lv_color_t color_scr_text)
{
    // Objects with transparent background, like Checkbox, container
    style_init_reset(&styles->transparent);
    lv_style_set_bg_opa(&styles->transparent, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_opa(&styles->transparent, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&styles->transparent, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_value_color(&styles->transparent, LV_STATE_DEFAULT, color_scr_text);
    lv_style_set_value_font(&styles->transparent, LV_STATE_DEFAULT, theme.font_normal);

    if(!NO_FOCUS) lv_style_set_outline_width(&styles->transparent, LV_STATE_DEFAULT, outline_width);

    // Background gradient
    style_init_reset(&styles->pretty);

    lv_style_set_radius(&styles->pretty, LV_STATE_DEFAULT, LV_DPI / 15);

    lv_style_set_bg_color(&styles->pretty, LV_STATE_DEFAULT, COLOR_SCR);
    lv_style_set_bg_grad_color(&styles->pretty, LV_STATE_DEFAULT, COLOR_SCR_GRAD);
    lv_style_set_bg_grad_dir(&styles->pretty, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&styles->pretty, LV_STATE_DEFAULT, LV_OPA_COVER);

    lv_style_set_line_width(&styles->pretty, LV_STATE_DEFAULT, 1);
    lv_style_set_line_color(&styles->pretty, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

    lv_style_set_scale_end_line_width(&styles->pretty, LV_STATE_DEFAULT, 1);
    lv_style_set_scale_end_color(&styles->pretty, LV_STATE_DEFAULT, theme.color_primary);

    my_style_set_fg_color(&styles->pretty, LV_STATE_DEFAULT, color_scr_text);
    // lv_style_set_text_font(&styles->pretty, LV_STATE_DEFAULT, theme.font_normal);
    // lv_style_set_text_color(&styles->pretty, LV_STATE_DEFAULT, color_scr_text);

    // lv_style_set_value_color(&styles->pretty, LV_STATE_DEFAULT, color_scr_text);
    lv_style_set_value_font(&styles->pretty, LV_STATE_DEFAULT, theme.font_normal);

    lv_style_set_border_opa(&styles->pretty, LV_STATE_DEFAULT, LV_OPA_30);
    lv_style_set_border_width(&styles->pretty, LV_STATE_DEFAULT, border_width);
    lv_style_set_border_color(&styles->pretty, LV_STATE_DEFAULT, BORDER_COLOR);
    lv_style_set_border_color(&styles->pretty, LV_STATE_EDITED, lv_color_darken(theme.color_secondary, LV_OPA_30));

    lv_style_set_outline_width(&styles->pretty, LV_STATE_DEFAULT, outline_width);

    if(!NO_FOCUS) {
        lv_style_set_border_color(&styles->pretty, LV_STATE_FOCUSED, theme.color_secondary);
        lv_style_set_border_width(&styles->pretty, LV_STATE_FOCUSED, outline_width);
    }

    style_init_reset(&styles->screen);
    lv_style_copy(&styles->screen, &styles->pretty);
    lv_style_set_radius(&styles->screen, LV_STATE_DEFAULT, 0);

    // Primary Colored gradient
    style_init_reset(&styles->pretty_color);
    lv_style_set_radius(&styles->pretty_color, LV_STATE_DEFAULT, LV_DPI / 15);
    lv_style_set_bg_color(&styles->pretty_color, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_bg_grad_color(&styles->pretty_color, LV_STATE_DEFAULT, COLOR_BG_GRAD);
    lv_style_set_bg_grad_dir(&styles->pretty_color, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);

    lv_style_set_bg_opa(&styles->pretty_color, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_line_width(&styles->pretty_color, LV_STATE_DEFAULT, 1);
    lv_style_set_scale_end_line_width(&styles->pretty_color, LV_STATE_DEFAULT, 1);
    lv_style_set_scale_end_color(&styles->pretty_color, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_line_color(&styles->pretty_color, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
    // lv_style_set_text_font(&styles->pretty_color, LV_STATE_DEFAULT, theme.font_normal);
    // lv_style_set_text_color(&styles->pretty_color, LV_STATE_DEFAULT, color_scr_text);

    my_style_set_fg_color(&styles->pretty_color, LV_STATE_DEFAULT, color_scr_text);
    // lv_style_set_value_color(&styles->pretty_color, LV_STATE_DEFAULT, color_scr_text);
    lv_style_set_value_font(&styles->pretty_color, LV_STATE_DEFAULT, theme.font_normal);

    lv_style_set_border_opa(&styles->pretty_color, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_border_width(&styles->pretty_color, LV_STATE_DEFAULT, border_width);
    lv_style_set_border_color(&styles->pretty_color, LV_STATE_DEFAULT, BORDER_COLOR);
    lv_style_set_border_color(&styles->pretty_color, LV_STATE_EDITED,
                              lv_color_darken(theme.color_secondary, LV_OPA_30));

    lv_style_set_outline_width(&styles->pretty_color, LV_STATE_DEFAULT, outline_width);
    if(!NO_FOCUS) {
        lv_style_set_border_color(&styles->pretty_color, LV_STATE_FOCUSED, theme.color_secondary);
        lv_style_set_border_width(&styles->pretty_color, LV_STATE_FOCUSED, outline_width);
    }

    /*    style_init_reset(&styles->bg_sec);
        lv_style_copy(&styles->bg_sec, &styles->pretty_color);
        lv_style_set_bg_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC);
        lv_style_set_bg_grad_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_BORDER);
        lv_style_set_bg_grad_dir(&styles->bg_sec, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);

        lv_style_set_border_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_BORDER);
        lv_style_set_text_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
        lv_style_set_value_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
        lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
        lv_style_set_line_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
    */

    /*
    style_init_reset(&styles->bg_click);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_PR);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_CHK);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_PR_CHK);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_DIS);
    lv_style_set_border_width(&styles->bg_click, LV_STATE_CHECKED, 0);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_FOCUSED | LV_STATE_PRESSED,
        lv_color_darken(theme.color_primary, LV_OPA_20));
    lv_style_set_border_color(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_BORDER_PR);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_BORDER_CHK);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_BORDER_CHK_PR);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_BORDER_DIS);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_TEXT_CHK_PR);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_TEXT_CHK_PR);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);
    lv_style_set_transition_prop_5(&styles->bg_click, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);*/

    style_init_reset(&styles->btn);
    lv_style_set_radius(&styles->btn, LV_STATE_DEFAULT, LV_DPI / 12);
    lv_style_set_bg_opa(&styles->btn, LV_STATE_DEFAULT, LV_OPA_COVER);

    lv_style_set_bg_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BTN);
    lv_style_set_bg_color(&styles->btn, LV_STATE_PRESSED, COLOR_BTN_PR);

    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BTN_GRAD);
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_PRESSED, COLOR_BTN_CHK_GRAD);

    lv_style_set_bg_grad_dir(&styles->btn, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    //    lv_style_set_bg_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BTN);
    //    lv_style_set_bg_color(&styles->btn, LV_STATE_PRESSED, COLOR_BTN_PR);

    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED, COLOR_BTN_CHK);
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_CHECKED, COLOR_BTN_CHK_GRAD);
    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BTN_CHK_PR);
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BTN_CHK_PR_GRAD);

    lv_style_set_bg_color(&styles->btn, LV_STATE_DISABLED, COLOR_BTN);
    lv_style_set_bg_color(&styles->btn, LV_STATE_DISABLED | LV_STATE_CHECKED, COLOR_BTN_DIS);

    lv_style_set_border_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BTN_BORDER);
    lv_style_set_border_color(&styles->btn, LV_STATE_CHECKED, COLOR_BTN_BORDER_CHK);
    lv_style_set_border_color(&styles->btn, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
    lv_style_set_border_width(&styles->btn, LV_STATE_DEFAULT, border_width);
    lv_style_set_border_opa(&styles->btn, LV_STATE_DEFAULT, LV_OPA_50);
    // lv_style_set_border_opa(&styles->btn, LV_STATE_CHECKED, LV_OPA_40);

    my_style_set_fg_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    my_style_set_fg_color(&styles->btn, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
    my_style_set_fg_color(&styles->btn, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    my_style_set_fg_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BG_TEXT_CHK_PR);
    my_style_set_fg_color(&styles->btn, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);

    // lv_style_set_text_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    // lv_style_set_text_color(&styles->btn, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
    // lv_style_set_text_color(&styles->btn, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    // lv_style_set_text_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BG_TEXT_CHK_PR);
    // lv_style_set_text_color(&styles->btn, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);

    // lv_style_set_image_recolor(&styles->btn, LV_STATE_DEFAULT, value_color);
    // lv_style_set_image_recolor(&styles->btn, LV_STATE_PRESSED, value_color);
    // lv_style_set_image_recolor(&styles->btn, LV_STATE_PRESSED, LV_COLOR_WHITE);
    // lv_style_set_image_recolor(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, LV_COLOR_WHITE);
    // lv_style_set_image_recolor(&styles->btn, LV_STATE_DISABLED,
    //                            IS_LIGHT ? lv_color_hex(0x888888) : lv_color_hex(0x888888));

    // lv_style_set_value_color(&styles->btn, LV_STATE_DEFAULT, value_color);
    // lv_style_set_value_color(&styles->btn, LV_STATE_PRESSED, value_color);
    // lv_style_set_value_color(&styles->btn, LV_STATE_CHECKED, LV_COLOR_WHITE);
    // lv_style_set_value_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, LV_COLOR_WHITE);
    // lv_style_set_value_color(&styles->btn, LV_STATE_DISABLED,
    //                          IS_LIGHT ? lv_color_hex(0x888888) : lv_color_hex(0x888888));

    lv_style_set_pad_hor(&styles->btn, LV_STATE_DEFAULT, HASP_DPX(40));
    lv_style_set_pad_ver(&styles->btn, LV_STATE_DEFAULT, HASP_DPX(15));
    lv_style_set_pad_inner(&styles->btn, LV_STATE_DEFAULT, HASP_DPX(20));

    lv_style_set_outline_width(&styles->btn, LV_STATE_DEFAULT, outline_width);
    if(!NO_FOCUS) {
        lv_style_set_outline_opa(&styles->btn, LV_STATE_DEFAULT, LV_OPA_0);
        lv_style_set_outline_opa(&styles->btn, LV_STATE_FOCUSED, LV_OPA_50);
        lv_style_set_outline_color(&styles->btn, LV_STATE_DEFAULT, theme.color_primary);
        lv_style_set_outline_color(&styles->btn, LV_STATE_EDITED, theme.color_secondary);
    }

    lv_style_set_transition_time(&styles->btn, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_4(&styles->btn, LV_STATE_DEFAULT, LV_STYLE_BORDER_OPA);
    lv_style_set_transition_prop_5(&styles->btn, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
    lv_style_set_transition_prop_6(&styles->btn, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);
    lv_style_set_transition_delay(&styles->btn, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_delay(&styles->btn, LV_STATE_PRESSED, 0);

    style_init_reset(&styles->pad_inner);
    lv_style_set_pad_inner(&styles->pad_inner, LV_STATE_DEFAULT,
                           lv_disp_get_size_category(NULL) <= LV_DISP_MEDIUM_LIMIT ? HASP_DPX(20) : HASP_DPX(40));

    style_init_reset(&styles->pad_small);
    lv_style_int_t pad_small_value =
        lv_disp_get_size_category(NULL) <= LV_DISP_MEDIUM_LIMIT ? HASP_DPX(10) : HASP_DPX(20);
    lv_style_set_pad_all(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_inner(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);

#if LV_USE_DROPDOWN || LV_USE_ROLLER
    style_init_reset(&styles->line_space);
    lv_style_set_text_line_space(&styles->line_space, LV_STATE_DEFAULT, HASP_DPX(20));
    lv_style_set_clip_corner(&styles->line_space, LV_STATE_DEFAULT, true);

    style_init_reset(&styles->selected);
    lv_style_set_radius(&styles->selected, LV_STATE_DEFAULT, 0);
    lv_style_set_text_color(&styles->selected, LV_STATE_DEFAULT, IS_LIGHT ? LV_COLOR_WHITE : LV_COLOR_WHITE);
    lv_style_set_bg_color(&styles->selected, LV_STATE_PRESSED, COLOR_BG_PR);
    lv_style_set_text_color(&styles->selected, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
#endif
}

static void cont_init(void)
{
#if LV_USE_CONT != 0

#endif
}

static void btn_init(void)
{
#if LV_USE_BTN != 0

#endif
}

static void label_init(void)
{
#if LV_USE_LABEL != 0

#endif
}

static void bar_init(void)
{
#if LV_USE_BAR
    /*    style_init_reset(&styles->bar_bg);
        lv_style_set_radius(&styles->bar_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
        lv_style_set_bg_opa(&styles->bar_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_style_set_bg_color(&styles->bar_bg, LV_STATE_DEFAULT, COLOR_BG_SEC);
        lv_style_set_value_color(&styles->bar_bg, LV_STATE_DEFAULT, value_color);
        lv_style_set_outline_color(&styles->bar_bg, LV_STATE_DEFAULT, theme.color_primary);
        lv_style_set_outline_color(&styles->bar_bg, LV_STATE_EDITED, theme.color_secondary);
        lv_style_set_outline_opa(&styles->bar_bg, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_style_set_outline_width(&styles->bar_bg, LV_STATE_DEFAULT, outline_width);
        lv_style_set_transition_time(&styles->bar_bg, LV_STATE_DEFAULT, TRANSITION_TIME);
        lv_style_set_transition_prop_6(&styles->bar_bg, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);

        style_init_reset(&styles->bar_indic);
        lv_style_set_bg_opa(&styles->bar_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_style_set_radius(&styles->bar_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
        lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DEFAULT, theme.color_primary);
        lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DISABLED, lv_color_hex3(0x888));
        lv_style_set_value_color(&styles->bar_indic, LV_STATE_DEFAULT, IS_LIGHT ? lv_color_hex(0x41404f) :
       LV_COLOR_WHITE);*/
#endif
}

static void img_init(void)
{
#if LV_USE_IMG != 0

#endif
}

static void line_init(void)
{
#if LV_USE_LINE != 0

#endif
}

static void led_init(void)
{
#if LV_USE_LED != 0
    style_init_reset(&styles->led);
    lv_style_set_bg_opa(&styles->led, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->led, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_border_width(&styles->led, LV_STATE_DEFAULT, 2);
    lv_style_set_border_opa(&styles->led, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_border_color(&styles->led, LV_STATE_DEFAULT, lv_color_lighten(theme.color_primary, LV_OPA_30));
    lv_style_set_radius(&styles->led, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_shadow_width(&styles->led, LV_STATE_DEFAULT, HASP_DPX(15));
    lv_style_set_shadow_color(&styles->led, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_shadow_spread(&styles->led, LV_STATE_DEFAULT, HASP_DPX(5));
    lv_style_set_margin_all(&styles->led, LV_STATE_DEFAULT, HASP_DPX(5));
#endif
}

static void slider_init(void)
{
#if LV_USE_SLIDER != 0
    style_init_reset(&styles->slider_knob);
    lv_style_set_bg_opa(&styles->slider_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DEFAULT, IS_LIGHT ? LV_COLOR_GRAY : LV_COLOR_WHITE);
    // lv_style_set_value_color(&styles->slider_knob, LV_STATE_DEFAULT,
    //                          value_color);
    // lv_style_set_radius(&styles->slider_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&styles->slider_knob, LV_STATE_DEFAULT, HASP_DPX(7));

    style_init_reset(&styles->slider_bg);
    lv_style_set_margin_all(&styles->slider_bg, LV_STATE_DEFAULT, HASP_DPX(10));

#endif
}

static void switch_init(void)
{
#if LV_USE_SWITCH != 0
    style_init_reset(&styles->sw_knob);
    lv_style_set_bg_opa(&styles->sw_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->sw_knob, LV_STATE_DEFAULT, IS_LIGHT ? LV_COLOR_GRAY : LV_COLOR_WHITE);
    // lv_style_set_bg_opa(&styles->sw_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
    // lv_style_set_bg_color(&styles->sw_knob, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    // lv_style_set_radius(&styles->sw_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&styles->sw_knob, LV_STATE_DEFAULT, HASP_DPX(2));
#endif
}

static void linemeter_init(void)
{
#if LV_USE_LINEMETER != 0
    style_init_reset(&styles->lmeter);
    lv_style_set_radius(&styles->lmeter, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_hor(&styles->lmeter, LV_STATE_DEFAULT, HASP_DPX(20));
    lv_style_set_pad_top(&styles->lmeter, LV_STATE_DEFAULT, HASP_DPX(20));
    lv_style_set_pad_inner(&styles->lmeter, LV_STATE_DEFAULT, HASP_DPX(30));
    lv_style_set_scale_width(&styles->lmeter, LV_STATE_DEFAULT, HASP_DPX(25));

    lv_style_set_line_color(&styles->lmeter, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_scale_grad_color(&styles->lmeter, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_scale_end_color(&styles->lmeter, LV_STATE_DEFAULT, lv_color_hex3(0x888));
    lv_style_set_line_width(&styles->lmeter, LV_STATE_DEFAULT, HASP_DPX(10));
    lv_style_set_scale_end_line_width(&styles->lmeter, LV_STATE_DEFAULT, HASP_DPX(7));
#endif
}

static void gauge_init(void)
{
#if LV_USE_GAUGE != 0
    style_init_reset(&styles->gauge_main);
    lv_style_set_line_color(&styles->gauge_main, LV_STATE_DEFAULT, lv_color_hex3(0x888));
    lv_style_set_scale_grad_color(&styles->gauge_main, LV_STATE_DEFAULT, lv_color_hex3(0x888));
    lv_style_set_scale_end_color(&styles->gauge_main, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_line_width(&styles->gauge_main, LV_STATE_DEFAULT, HASP_DPX(2)); // normal scale  minor thick width
    lv_style_set_scale_end_line_width(&styles->gauge_main, LV_STATE_DEFAULT,
                                      HASP_DPX(2)); // strong scale minor tick width
    lv_style_set_scale_end_border_width(&styles->gauge_main, LV_STATE_DEFAULT, HASP_DPX(3));
    lv_style_set_pad_hor(&styles->gauge_main, LV_STATE_DEFAULT, HASP_DPX(20));     // left margin
    lv_style_set_pad_top(&styles->gauge_main, LV_STATE_DEFAULT, HASP_DPX(20));     // top margin
    lv_style_set_pad_inner(&styles->gauge_main, LV_STATE_DEFAULT, HASP_DPX(15));   // position of the labels
    lv_style_set_scale_width(&styles->gauge_main, LV_STATE_DEFAULT, HASP_DPX(10)); // minor thick length
    lv_style_set_radius(&styles->gauge_main, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

    style_init_reset(&styles->gauge_strong);
    lv_style_set_line_color(&styles->gauge_strong, LV_STATE_DEFAULT, lv_color_hex3(0x888));
    lv_style_set_scale_grad_color(&styles->gauge_strong, LV_STATE_DEFAULT, lv_color_hex3(0x888));
    lv_style_set_scale_end_color(&styles->gauge_strong, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_line_width(&styles->gauge_strong, LV_STATE_DEFAULT, HASP_DPX(8)); // strong scale arc width
    lv_style_set_scale_end_line_width(&styles->gauge_strong, LV_STATE_DEFAULT, HASP_DPX(8));
    lv_style_set_scale_width(&styles->gauge_strong, LV_STATE_DEFAULT, HASP_DPX(20)); // major thick length

    style_init_reset(&styles->gauge_needle);
    lv_style_set_line_color(&styles->gauge_needle, LV_STATE_DEFAULT,
                            IS_LIGHT ? lv_color_hex(0x464b5b) : LV_COLOR_WHITE);
    lv_style_set_line_width(&styles->gauge_needle, LV_STATE_DEFAULT, HASP_DPX(8));
    lv_style_set_bg_opa(&styles->gauge_needle, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->gauge_needle, LV_STATE_DEFAULT, IS_LIGHT ? lv_color_hex(0x464b5b) : LV_COLOR_WHITE);
    lv_style_set_radius(&styles->gauge_needle, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_size(&styles->gauge_needle, LV_STATE_DEFAULT, HASP_DPX(30));
    lv_style_set_pad_inner(&styles->gauge_needle, LV_STATE_DEFAULT, HASP_DPX(10));
#endif
}

static void arc_init(void)
{
#if LV_USE_ARC != 0
    style_init_reset(&styles->arc_indic);
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_line_width(&styles->arc_indic, LV_STATE_DEFAULT, HASP_DPX(25));
    lv_style_set_line_rounded(&styles->arc_indic, LV_STATE_DEFAULT, true);

    style_init_reset(&styles->arc_bg);
    lv_style_set_line_color(&styles->arc_bg, LV_STATE_DEFAULT, lv_color_mix(BORDER_COLOR, COLOR_SCR_GRAD, 128));
    lv_style_set_line_width(&styles->arc_bg, LV_STATE_DEFAULT, HASP_DPX(25));
    lv_style_set_line_rounded(&styles->arc_bg, LV_STATE_DEFAULT, true);
    lv_style_set_radius(&styles->arc_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&styles->arc_bg, LV_STATE_DEFAULT, HASP_DPX(25));
    /*
        lv_style_set_border_opa(&styles->pretty_color, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_style_set_border_opa(&styles->pretty_color, LV_STATE_FOCUSED, LV_OPA_30);
        lv_style_set_border_width(&styles->pretty_color, LV_STATE_DEFAULT, LV_DPI / 50 >= 1 ? LV_DPI / 50 : 1);
        lv_style_set_border_width(&styles->pretty_color, LV_STATE_FOCUSED, LV_DPI / 50 >= 1 ? LV_DPI / 25 : 2);
        lv_style_set_pad_all(&styles->pretty_color, LV_STATE_DEFAULT, LV_DPI / 50 >= 1 ? LV_DPI / 25 : 2);
    */
    style_init_reset(&styles->arc_knob);
    lv_style_set_bg_opa(&styles->arc_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->arc_knob, LV_STATE_DEFAULT, IS_LIGHT ? LV_COLOR_GRAY : LV_COLOR_WHITE);
    lv_style_set_radius(&styles->arc_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&styles->arc_knob, LV_STATE_DEFAULT, HASP_DPX(1));

#endif
}

static void spinner_init(void)
{
#if LV_USE_SPINNER != 0
#endif
}

static void chart_init(void)
{
#if LV_USE_CHART
    style_init_reset(&styles->chart_bg);
    lv_style_set_text_color(&styles->chart_bg, LV_STATE_DEFAULT, IS_LIGHT ? COLOR_BG_TEXT_DIS : lv_color_hex(0xa1adbd));

    style_init_reset(&styles->chart_series_bg);
    lv_style_set_line_width(&styles->chart_series_bg, LV_STATE_DEFAULT, HASP_DPX(1));
    lv_style_set_line_dash_width(&styles->chart_series_bg, LV_STATE_DEFAULT, HASP_DPX(10));
    lv_style_set_line_dash_gap(&styles->chart_series_bg, LV_STATE_DEFAULT, HASP_DPX(10));
    lv_style_set_line_color(&styles->chart_series_bg, LV_STATE_DEFAULT, COLOR_BG_BORDER);

    style_init_reset(&styles->chart_series);
    lv_style_set_line_width(&styles->chart_series, LV_STATE_DEFAULT, HASP_DPX(3));
    lv_style_set_size(&styles->chart_series, LV_STATE_DEFAULT, HASP_DPX(4));
    lv_style_set_pad_inner(&styles->chart_series, LV_STATE_DEFAULT, HASP_DPX(2)); /*Space between columns*/
    lv_style_set_radius(&styles->chart_series, LV_STATE_DEFAULT, HASP_DPX(1));

#endif
}

static void calendar_init(lv_style_int_t pad_def, lv_color_t value_color)
{
#if LV_USE_CALENDAR
    style_init_reset(&styles->calendar_date_nums);
    lv_style_set_radius(&styles->calendar_date_nums, LV_STATE_DEFAULT, HASP_DPX(4));
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_CHECKED, value_color);
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_DISABLED, LV_COLOR_GRAY);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_CHECKED, IS_LIGHT ? LV_OPA_20 : LV_OPA_40);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_PRESSED, LV_OPA_20);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_FOCUSED, theme.color_primary);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_DEFAULT,
                          IS_LIGHT ? lv_color_hex(0x666666) : LV_COLOR_WHITE);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_CHECKED, theme.color_primary);
    lv_style_set_border_width(&styles->calendar_date_nums, LV_STATE_CHECKED, 2);
    lv_style_set_border_side(&styles->calendar_date_nums, LV_STATE_CHECKED, LV_BORDER_SIDE_LEFT);
    lv_style_set_border_color(&styles->calendar_date_nums, LV_STATE_CHECKED, theme.color_primary);
    lv_style_set_pad_inner(&styles->calendar_date_nums, LV_STATE_DEFAULT, HASP_DPX(3));
    lv_style_set_pad_hor(&styles->calendar_date_nums, LV_STATE_DEFAULT, pad_def);
    lv_style_set_pad_bottom(&styles->calendar_date_nums, LV_STATE_DEFAULT, pad_def);
#endif
}

static void cpicker_init(void)
{
#if LV_USE_CPICKER
    style_init_reset(&styles->cpicker_bg);
    lv_style_set_scale_width(&styles->cpicker_bg, LV_STATE_DEFAULT, HASP_DPX(30));
    lv_style_set_bg_opa(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->cpicker_bg, LV_STATE_DEFAULT, COLOR_SCR);
    lv_style_set_pad_inner(&styles->cpicker_bg, LV_STATE_DEFAULT, HASP_DPX(20));
    lv_style_set_radius(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

    style_init_reset(&styles->cpicker_indic);
    lv_style_set_radius(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_bg_opa(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_width(&styles->cpicker_indic, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_FOCUSED, theme.color_primary);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_EDITED, theme.color_secondary);
    lv_style_set_pad_all(&styles->cpicker_indic, LV_STATE_DEFAULT, HASP_DPX(13));
#endif
}

static void checkbox_init(void)
{
#if LV_USE_CHECKBOX != 0
    /*    style_init_reset(&styles->cb_bg);
        lv_style_set_radius(&styles->cb_bg, LV_STATE_DEFAULT, HASP_DPX(4));
        lv_style_set_pad_inner(&styles->cb_bg, LV_STATE_DEFAULT, HASP_DPX(10));
        lv_style_set_outline_color(&styles->cb_bg, LV_STATE_DEFAULT, theme.color_primary);
        lv_style_set_outline_opa(&styles->cb_bg, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_style_set_outline_opa(&styles->cb_bg, LV_STATE_FOCUSED, LV_OPA_50);
        lv_style_set_outline_width(&styles->cb_bg, LV_STATE_DEFAULT, outline_width);
        lv_style_set_outline_pad(&styles->cb_bg, LV_STATE_DEFAULT, HASP_DPX(10));
        lv_style_set_transition_time(&styles->cb_bg, LV_STATE_DEFAULT, TRANSITION_TIME);
        lv_style_set_transition_prop_6(&styles->cb_bg, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);
    */
    style_init_reset(&styles->cb_bullet);
    lv_style_set_outline_opa(&styles->cb_bullet, LV_STATE_FOCUSED, LV_OPA_TRANSP);
    lv_style_set_radius(&styles->cb_bullet, LV_STATE_DEFAULT, HASP_DPX(4));
    lv_style_set_pattern_image(&styles->cb_bullet, LV_STATE_CHECKED, LV_SYMBOL_OK);
    lv_style_set_pattern_recolor(&styles->cb_bullet, LV_STATE_CHECKED, LV_COLOR_WHITE);
    lv_style_set_pattern_opa(&styles->cb_bullet, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_pattern_opa(&styles->cb_bullet, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_style_set_transition_prop_3(&styles->cb_bullet, LV_STATE_DEFAULT, LV_STYLE_PATTERN_OPA);
    lv_style_set_text_font(&styles->cb_bullet, LV_STATE_CHECKED, theme.font_small);
    lv_style_set_pad_all(&styles->cb_bullet, LV_STATE_DEFAULT, HASP_DPX(3));

    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_PRESSED, COLOR_BTN_PR);
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_PRESSED, COLOR_BTN_CHK_GRAD);
    lv_style_set_text_color(&styles->cb_bullet, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);

    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_CHECKED, COLOR_BTN);
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_CHECKED, COLOR_BTN_GRAD);
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BTN_CHK_PR);
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BTN_CHK_PR_GRAD);

    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DISABLED, COLOR_BTN);
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DISABLED | LV_STATE_CHECKED, COLOR_BTN_DIS);

    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_CHECKED, COLOR_BTN_BORDER_CHK);
    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
#endif
}

static void btnmatrix_init(void)
{}

static void keyboard_init(void)
{
#if LV_USE_KEYBOARD
    style_init_reset(&styles->kb_bg);
    lv_style_set_radius(&styles->kb_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->kb_bg, LV_STATE_DEFAULT, HASP_DPX(4));
    lv_style_set_border_side(&styles->kb_bg, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP);
    lv_style_set_border_color(&styles->kb_bg, LV_STATE_DEFAULT, IS_LIGHT ? COLOR_BG_TEXT : LV_COLOR_BLACK);
    lv_style_set_border_color(&styles->kb_bg, LV_STATE_EDITED, theme.color_secondary);
    lv_style_set_pad_all(&styles->kb_bg, LV_STATE_DEFAULT, HASP_DPX(5));
    lv_style_set_pad_inner(&styles->kb_bg, LV_STATE_DEFAULT, HASP_DPX(3));
#endif
}

static void msgbox_init(void)
{
#if LV_USE_MSGBOX
    style_init_reset(&styles->mbox_bg);
    lv_style_set_shadow_width(&styles->mbox_bg, LV_STATE_DEFAULT, HASP_DPX(50));
    lv_style_set_shadow_color(&styles->mbox_bg, LV_STATE_DEFAULT, IS_LIGHT ? LV_COLOR_SILVER : lv_color_hex3(0x777));

#endif
}

static void page_init(lv_style_int_t dpx7)
{
#if LV_USE_PAGE
    style_init_reset(&styles->sb);
    lv_style_set_bg_opa(&styles->sb, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->sb, LV_STATE_DEFAULT, SCROLLBAR_COLOR);
    lv_style_set_radius(&styles->sb, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_size(&styles->sb, LV_STATE_DEFAULT, dpx7);
    lv_style_set_pad_right(&styles->sb, LV_STATE_DEFAULT, dpx7);
    lv_style_set_pad_bottom(&styles->sb, LV_STATE_DEFAULT, dpx7);

#if LV_USE_ANIMATION
    style_init_reset(&styles->edge_flash);
    lv_style_set_bg_opa(&styles->edge_flash, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->edge_flash, LV_STATE_DEFAULT, lv_color_hex3(0x888));
#endif
#endif
}

static void textarea_init(lv_style_int_t border_width)
{
#if LV_USE_TEXTAREA
    style_init_reset(&styles->ta_cursor);
    lv_style_set_border_color(&styles->ta_cursor, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
    lv_style_set_border_width(&styles->ta_cursor, LV_STATE_DEFAULT, border_width);
    lv_style_set_pad_left(&styles->ta_cursor, LV_STATE_DEFAULT, HASP_DPX(1));
    lv_style_set_border_side(&styles->ta_cursor, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT);

    style_init_reset(&styles->ta_placeholder);
    lv_style_set_text_color(&styles->ta_placeholder, LV_STATE_DEFAULT,
                            IS_LIGHT ? COLOR_BG_TEXT_DIS : lv_color_hex(0xa1adbd));
#endif
}

static void spinbox_init(void)
{
#if LV_USE_SPINBOX

    style_init_reset(&styles->spinbox_cursor);
    lv_style_set_bg_opa(&styles->spinbox_cursor, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->spinbox_cursor, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_text_color(&styles->spinbox_cursor, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_pad_ver(&styles->spinbox_cursor, LV_STATE_DEFAULT, HASP_DPX(100));

#endif
}

static void list_init(lv_style_int_t pad_def, lv_style_int_t border_width)
{
#if LV_USE_LIST != 0
    /*    style_init_reset(&styles->list_bg);
        lv_style_set_clip_corner(&styles->list_bg, LV_STATE_DEFAULT, true);
        lv_style_set_pad_left(&styles->list_bg, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_right(&styles->list_bg, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_top(&styles->list_bg, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_bottom(&styles->list_bg, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_inner(&styles->list_bg, LV_STATE_DEFAULT, 0);
    */

    style_init_reset(&styles->list_btn);
    lv_style_set_bg_opa(&styles->list_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_PRESSED, COLOR_BG_PR);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_DISABLED, COLOR_BG_DIS);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_CHECKED, COLOR_BG_CHK);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BG_PR_CHK);

    lv_style_set_text_color(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    lv_style_set_text_color(&styles->list_btn, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_text_color(&styles->list_btn, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);

    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);

    lv_style_set_border_side(&styles->list_btn, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    lv_style_set_border_color(&styles->list_btn, LV_STATE_FOCUSED, theme.color_primary);
    lv_style_set_border_width(&styles->list_btn, LV_STATE_DEFAULT, border_width);

    lv_style_set_outline_color(&styles->list_btn, LV_STATE_FOCUSED, theme.color_secondary);
    // lv_style_set_outline_width(&styles->list_btn, LV_STATE_FOCUSED, outline_width);
    lv_style_set_outline_pad(&styles->list_btn, LV_STATE_FOCUSED, -border_width);

    lv_style_set_pad_all(&styles->list_btn, LV_STATE_DEFAULT, pad_def);
    lv_style_set_pad_inner(&styles->list_btn, LV_STATE_DEFAULT, pad_def);

    lv_style_set_transform_width(&styles->list_btn, LV_STATE_DEFAULT, -pad_def);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_PRESSED, -border_width);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_CHECKED, -border_width);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_DISABLED, -border_width);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_FOCUSED, -border_width);

    lv_style_set_transition_time(&styles->list_btn, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_6(&styles->list_btn, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
    lv_style_set_transition_prop_5(&styles->list_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
#endif
}

static void ddlist_init(void)
{
#if LV_USE_DROPDOWN != 0

    /*    style_init_reset(&styles->ddlist_page);
        lv_style_set_text_line_space(&styles->ddlist_page, LV_STATE_DEFAULT, HASP_DPX(20));
        lv_style_set_clip_corner(&styles->ddlist_page, LV_STATE_DEFAULT, true);

        style_init_reset(&styles->ddlist_sel);
        lv_style_set_radius(&styles->ddlist_sel, LV_STATE_DEFAULT, 0);
       // lv_style_set_bg_opa(&styles->ddlist_sel, LV_STATE_DEFAULT, LV_OPA_COVER);
       // lv_style_set_bg_color(&styles->ddlist_sel, LV_STATE_DEFAULT, theme.color_primary);
        lv_style_set_text_color(&styles->ddlist_sel, LV_STATE_DEFAULT,
                                IS_LIGHT ?LV_COLOR_WHITE :LV_COLOR_WHITE);
        lv_style_set_bg_color(&styles->ddlist_sel, LV_STATE_PRESSED, COLOR_BG_PR);
        lv_style_set_text_color(&styles->ddlist_sel, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
        */
#endif
}

static void roller_init(void)
{
#if LV_USE_ROLLER != 0
    /*   style_init_reset(&styles->roller_bg);
       lv_style_set_text_line_space(&styles->roller_bg, LV_STATE_DEFAULT, HASP_DPX(25));

       style_init_reset(&styles->roller_sel);
       lv_style_set_bg_opa(&styles->roller_sel, LV_STATE_DEFAULT, LV_OPA_COVER);
       lv_style_set_bg_color(&styles->roller_sel, LV_STATE_DEFAULT, theme.color_primary);
       lv_style_set_text_color(&styles->roller_sel, LV_STATE_DEFAULT, LV_COLOR_WHITE);*/
#endif
}

static void tabview_init(void)
{
#if LV_USE_TABVIEW != 0
#endif
}

static void tileview_init(void)
{
#if LV_USE_TILEVIEW != 0
#endif
}

static void table_init(lv_style_int_t pad_def)
{
#if LV_USE_TABLE != 0
    style_init_reset(&styles->table_cell);
    lv_style_set_border_color(&styles->table_cell, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    lv_style_set_border_width(&styles->table_cell, LV_STATE_DEFAULT, 1);
    lv_style_set_border_side(&styles->table_cell, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM);
    lv_style_set_pad_all(&styles->table_cell, LV_STATE_DEFAULT, pad_def);
#endif
}

static void win_init(void)
{
#if LV_USE_WIN != 0
#endif
}

static void tabview_win_shared_init(lv_style_int_t pad_def, lv_style_int_t dpx7, lv_color_t color_scr_text)
{
#if LV_USE_TABVIEW || LV_USE_WIN
    style_init_reset(&styles->tabview_btns_bg);
    lv_style_set_bg_opa(&styles->tabview_btns_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->tabview_btns_bg, LV_STATE_DEFAULT, COLOR_SCR_GRAD);
    lv_style_set_text_color(&styles->tabview_btns_bg, LV_STATE_DEFAULT, color_scr_text);
    lv_style_set_image_recolor(&styles->tabview_btns_bg, LV_STATE_DEFAULT, lv_color_hex(0x979a9f));
    lv_style_set_pad_top(&styles->tabview_btns_bg, LV_STATE_DEFAULT, dpx7);
    lv_style_set_pad_hor(&styles->tabview_btns_bg, LV_STATE_DEFAULT, dpx7);

    style_init_reset(&styles->tabview_btns);
    lv_style_set_bg_opa(&styles->tabview_btns, LV_STATE_PRESSED, LV_OPA_50);
    lv_style_set_bg_color(&styles->tabview_btns, LV_STATE_PRESSED, lv_color_hex3(0x888));
    lv_style_set_text_color(&styles->tabview_btns, LV_STATE_CHECKED, color_scr_text);
    lv_style_set_pad_ver(&styles->tabview_btns, LV_STATE_DEFAULT, HASP_DPX(20));
    lv_style_set_text_color(&styles->tabview_btns, LV_STATE_FOCUSED, theme.color_primary);
    lv_style_set_text_color(&styles->tabview_btns, LV_STATE_EDITED, theme.color_secondary);

    style_init_reset(&styles->tabview_indic);
    lv_style_set_bg_opa(&styles->tabview_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->tabview_indic, LV_STATE_DEFAULT, theme.color_primary);
    lv_style_set_bg_color(&styles->tabview_indic, LV_STATE_EDITED, theme.color_secondary);
    lv_style_set_size(&styles->tabview_indic, LV_STATE_DEFAULT, HASP_DPX(5));
    lv_style_set_radius(&styles->tabview_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

    style_init_reset(&styles->tabview_page_scrl);
    lv_style_set_pad_all(&styles->tabview_page_scrl, LV_STATE_DEFAULT, pad_def);
    lv_style_set_pad_inner(&styles->tabview_page_scrl, LV_STATE_DEFAULT, pad_def);
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_theme_hasp_init(lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                               const lv_font_t* font_small, const lv_font_t* font_normal,
                               const lv_font_t* font_subtitle, const lv_font_t* font_title)
{

    /* This trick is required only to avoid the garbage collection of
     * styles' data if LVGL is used in a binding (e.g. Micropython)
     * In a general case styles could be simple `static lv_style_t my style` variables*/
    if(!inited) {
        LV_GC_ROOT(_lv_theme_material_styles) = lv_mem_alloc(sizeof(theme_styles_t));
        styles                                = (theme_styles_t*)LV_GC_ROOT(_lv_theme_material_styles);
    }

    theme.color_primary   = color_primary;
    theme.color_secondary = color_secondary;
    theme.font_small      = font_small;
    theme.font_normal     = font_normal;
    theme.font_subtitle   = font_subtitle;
    theme.font_title      = font_title;
    theme.flags           = flags;

    lv_style_int_t border_width  = BORDER_WIDTH;
    lv_style_int_t outline_width = OUTLINE_WIDTH;
    lv_style_int_t pad_def       = PAD_DEF;
    lv_style_int_t dpx7          = HASP_DPX(7);
    lv_color_t value_color       = IS_LIGHT ? lv_color_hex(0x31404f) : LV_COLOR_WHITE;
    lv_color_t color_scr_text    = COLOR_SCR_TEXT;

    basic_init(border_width, outline_width, value_color, color_scr_text);
    cont_init();
    btn_init();
    label_init();
    bar_init();
    img_init();
    line_init();
    led_init();
    slider_init();
    switch_init();
    linemeter_init();
    gauge_init();
    arc_init();
    spinner_init();
    chart_init();
    calendar_init(pad_def, value_color);
    cpicker_init();
    checkbox_init();
    btnmatrix_init();
    keyboard_init();
    msgbox_init();
    page_init(dpx7);
    textarea_init(border_width);
    spinbox_init();
    list_init(pad_def, border_width);
    ddlist_init();
    roller_init();
    tabview_init();
    tileview_init();
    table_init(pad_def);
    win_init();
    tabview_win_shared_init(pad_def, dpx7, color_scr_text);

    theme.apply_xcb = NULL;
    theme.apply_cb  = theme_apply;

    inited = true;

    lv_obj_report_style_mod(NULL);

    return &theme;
}

static void theme_apply(lv_theme_t* th, lv_obj_t* obj, lv_theme_style_t name)
{
    LV_UNUSED(th);

    lv_style_list_t* list;

    switch(name) {
        case LV_THEME_NONE:
            break;

        case LV_THEME_SCR:
            list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
            _lv_style_list_add_style(list, &styles->screen);
            break;

        case LV_THEME_OBJ:
            list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
            _lv_style_list_add_style(list, &styles->pretty);
            break;

#if LV_USE_CONT
        case LV_THEME_CONT:
            list = lv_obj_get_style_list(obj, LV_CONT_PART_MAIN);
            _lv_style_list_add_style(list, &styles->transparent);
            _lv_style_list_add_style(list, &styles->pad_small);
            break;
#endif

#if LV_USE_BTN
        case LV_THEME_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->btn);
            //_lv_style_list_add_style(list, &styles->bg_sec);
            break;
#endif

#if LV_USE_BTNMATRIX
        case LV_THEME_BTNMATRIX:
            list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->pad_inner);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BTN);
            _lv_style_list_add_style(list, &styles->btn);
            //_lv_style_list_add_style(list, &styles->bg_click);
            break;
#endif

#if LV_USE_KEYBOARD
        case LV_THEME_KEYBOARD:
            list = lv_obj_get_style_list(obj, LV_KEYBOARD_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->kb_bg);

            list = lv_obj_get_style_list(obj, LV_KEYBOARD_PART_BTN);
            _lv_style_list_add_style(list, &styles->btn);
            _lv_style_list_add_style(list, &styles->pretty);
            // _lv_style_list_add_style(list, &styles->bg_click);
            break;
#endif

#if LV_USE_BAR
        case LV_THEME_BAR:
            list = lv_obj_get_style_list(obj, LV_BAR_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_BAR_PART_INDIC);
            _lv_style_list_add_style(list, &styles->pretty_color);
            break;
#endif

#if LV_USE_SWITCH
        case LV_THEME_SWITCH:
            list = lv_obj_get_style_list(obj, LV_SWITCH_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_SWITCH_PART_INDIC);
            _lv_style_list_add_style(list, &styles->pretty_color);

            list = lv_obj_get_style_list(obj, LV_SWITCH_PART_KNOB);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->sw_knob);
            // _lv_style_list_add_style(list, &styles->pad_small);

            break;
#endif

#if LV_USE_CANVAS
        case LV_THEME_CANVAS:
            break;
#endif

#if LV_USE_IMG
        case LV_THEME_IMAGE:
            break;
#endif

#if LV_USE_IMGBTN
        case LV_THEME_IMGBTN:
            break;
#endif

#if LV_USE_LABEL
        case LV_THEME_LABEL:
            break;
#endif

#if LV_USE_LINE
        case LV_THEME_LINE:
            break;
#endif

#if LV_USE_ARC
        case LV_THEME_ARC:
            list = lv_obj_get_style_list(obj, LV_ARC_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            //_lv_style_list_add_style(list, &styles->pad_small);
            _lv_style_list_add_style(list, &styles->arc_bg);

            list = lv_obj_get_style_list(obj, LV_ARC_PART_INDIC);
            _lv_style_list_add_style(list, &styles->arc_indic);

            list = lv_obj_get_style_list(obj, LV_ARC_PART_KNOB);
            _lv_style_list_add_style(list, &styles->pretty);
            //_lv_style_list_add_style(list, &styles->bg_click);
            _lv_style_list_add_style(list, &styles->arc_knob);
            break;
#endif

#if LV_USE_SPINNER
        case LV_THEME_SPINNER:
            list = lv_obj_get_style_list(obj, LV_SPINNER_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->transparent);

            list = lv_obj_get_style_list(obj, LV_SPINNER_PART_INDIC);
            _lv_style_list_add_style(list, &styles->arc_indic);
            break;
#endif

#if LV_USE_SLIDER
        case LV_THEME_SLIDER:
            list = lv_obj_get_style_list(obj, LV_SLIDER_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_SLIDER_PART_INDIC);
            _lv_style_list_add_style(list, &styles->pretty_color);

            list = lv_obj_get_style_list(obj, LV_SLIDER_PART_KNOB);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->slider_knob);
            break;
#endif

#if LV_USE_CHECKBOX
        case LV_THEME_CHECKBOX:
            list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BG);
            _lv_style_list_add_style(list, &styles->transparent);

            list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BULLET);
            //_lv_style_list_add_style(list, &styles->btn);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->cb_bullet);
            break;
#endif

#if LV_USE_MSGBOX
        case LV_THEME_MSGBOX:
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->pad_small);
            _lv_style_list_add_style(list, &styles->mbox_bg); // shadow
            break;

        case LV_THEME_MSGBOX_BTNS:
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN_BG);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN);
            _lv_style_list_add_style(list, &styles->btn);
            break;

#endif
#if LV_USE_LED
        case LV_THEME_LED:
            list = lv_obj_get_style_list(obj, LV_LED_PART_MAIN);
            _lv_style_list_add_style(list, &styles->led);
            break;
#endif
#if LV_USE_PAGE
        case LV_THEME_PAGE:
            list = lv_obj_get_style_list(obj, LV_PAGE_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLABLE);
            _lv_style_list_add_style(list, &styles->pad_inner);

            list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->sb);

#if LV_USE_ANIMATION
            list = lv_obj_get_style_list(obj, LV_PAGE_PART_EDGE_FLASH);
            _lv_style_list_add_style(list, &styles->edge_flash);
#endif
            break;
#endif
#if LV_USE_TABVIEW
        case LV_THEME_TABVIEW:
            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_TAB_BG);
            _lv_style_list_add_style(list, &styles->tabview_btns_bg);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_INDIC);
            _lv_style_list_add_style(list, &styles->pretty_color);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_TAB_BTN);
            _lv_style_list_add_style(list, &styles->tabview_btns);
            break;

        case LV_THEME_TABVIEW_PAGE:
            list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLABLE);
            _lv_style_list_add_style(list, &styles->tabview_page_scrl);

            break;
#endif

#if LV_USE_TILEVIEW
        case LV_THEME_TILEVIEW:
            list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->sb);

#if LV_USE_ANIMATION
            list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_EDGE_FLASH);
            _lv_style_list_add_style(list, &styles->edge_flash);
#endif
            break;
#endif

#if LV_USE_ROLLER
        case LV_THEME_ROLLER:
            list = lv_obj_get_style_list(obj, LV_ROLLER_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->line_space); // clip and linespace

            list = lv_obj_get_style_list(obj, LV_ROLLER_PART_SELECTED);
            _lv_style_list_add_style(list, &styles->pretty_color);
            //_lv_style_list_add_style(list, &styles->btn);
            _lv_style_list_add_style(list, &styles->selected);
            break;
#endif

#if LV_USE_OBJMASK
        case LV_THEME_OBJMASK:
            list = lv_obj_get_style_list(obj, LV_OBJMASK_PART_MAIN);
            break;
#endif

#if LV_USE_LIST
        case LV_THEME_LIST:
            list = lv_obj_get_style_list(obj, LV_LIST_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_LIST_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->sb);
            break;

        case LV_THEME_LIST_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->list_btn);
            break;
#endif

#if LV_USE_DROPDOWN
        case LV_THEME_DROPDOWN:
            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->cb_bullet);
            //_lv_style_list_add_style(list, &styles->bg_click);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_LIST);
            _lv_style_list_add_style(list, &styles->pretty);
            //_lv_style_list_add_style(list, &styles->cb_bullet);
            _lv_style_list_add_style(list, &styles->pad_small);
            _lv_style_list_add_style(list, &styles->line_space); // clip and linespace

            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->sb);

            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SELECTED);
            _lv_style_list_add_style(list, &styles->pretty_color);
            _lv_style_list_add_style(list, &styles->pad_small);
            _lv_style_list_add_style(list, &styles->selected);
            break;
#endif

#if LV_USE_CHART
        case LV_THEME_CHART:
            list = lv_obj_get_style_list(obj, LV_CHART_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->chart_bg);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES_BG);
            _lv_style_list_add_style(list, &styles->pad_small);
            _lv_style_list_add_style(list, &styles->chart_series_bg);

            list = lv_obj_get_style_list(obj, LV_CHART_PART_CURSOR);
            _lv_style_list_add_style(list, &styles->chart_series_bg);

            list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES);
            _lv_style_list_add_style(list, &styles->chart_series);
            break;
#endif
#if LV_USE_TABLE
        case LV_THEME_TABLE: {
            list = lv_obj_get_style_list(obj, LV_TABLE_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty_color);

            int idx = 1; /* start value should be 1, not zero, since cell styles
                        start at 1 due to presence of LV_TABLE_PART_BG=0
                        in the enum (lv_table.h) */
                         /* declaring idx outside loop to work with older compilers */
            for(; idx <= LV_TABLE_CELL_STYLE_CNT; idx++) {
                list = lv_obj_get_style_list(obj, idx);
                _lv_style_list_add_style(list, &styles->table_cell);
            }
            break;
        }
#endif

#if LV_USE_WIN
        case LV_THEME_WIN:
            list = lv_obj_get_style_list(obj, LV_WIN_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_WIN_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->sb);

            list = lv_obj_get_style_list(obj, LV_WIN_PART_CONTENT_SCROLLABLE);
            _lv_style_list_add_style(list, &styles->tabview_page_scrl);

            list = lv_obj_get_style_list(obj, LV_WIN_PART_HEADER);
            _lv_style_list_add_style(list, &styles->tabview_btns_bg);
            break;

        case LV_THEME_WIN_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->tabview_btns);
            break;
#endif

#if LV_USE_TEXTAREA
        case LV_THEME_TEXTAREA:
            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_PLACEHOLDER);
            _lv_style_list_add_style(list, &styles->ta_placeholder);

            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_CURSOR);
            _lv_style_list_add_style(list, &styles->ta_cursor);

            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->sb);
            break;

#endif

#if LV_USE_SPINBOX
        case LV_THEME_SPINBOX:
            list = lv_obj_get_style_list(obj, LV_SPINBOX_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_SPINBOX_PART_CURSOR);
            _lv_style_list_add_style(list, &styles->spinbox_cursor);
            break;

        case LV_THEME_SPINBOX_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->pretty_color);
            // _lv_style_list_add_style(list, &styles->bg_click);
            break;
#endif

#if LV_USE_CALENDAR
        case LV_THEME_CALENDAR:
            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_BG);
            _lv_style_list_add_style(list, &styles->pretty);

            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_DATE);
            _lv_style_list_add_style(list, &styles->calendar_date_nums);

            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_HEADER);
            _lv_style_list_add_style(list, &styles->transparent);
            _lv_style_list_add_style(list, &styles->pad_small);

            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_DAY_NAMES);
            _lv_style_list_add_style(list, &styles->transparent);
            _lv_style_list_add_style(list, &styles->pad_small);
            break;
#endif
#if LV_USE_CPICKER
        case LV_THEME_CPICKER:
            list = lv_obj_get_style_list(obj, LV_CPICKER_PART_MAIN);
            _lv_style_list_add_style(list, &styles->cpicker_bg);

            list = lv_obj_get_style_list(obj, LV_CPICKER_PART_KNOB);
            _lv_style_list_add_style(list, &styles->cpicker_indic);
            break;
#endif

#if LV_USE_LINEMETER
        case LV_THEME_LINEMETER:
            list = lv_obj_get_style_list(obj, LV_LINEMETER_PART_MAIN);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->lmeter);
            break;
#endif

#if LV_USE_GAUGE
        case LV_THEME_GAUGE:
            list = lv_obj_get_style_list(obj, LV_GAUGE_PART_MAIN);
            _lv_style_list_add_style(list, &styles->pretty);
            _lv_style_list_add_style(list, &styles->gauge_main);

            list = lv_obj_get_style_list(obj, LV_GAUGE_PART_MAJOR);
            _lv_style_list_add_style(list, &styles->gauge_strong);

            list = lv_obj_get_style_list(obj, LV_GAUGE_PART_NEEDLE);
            _lv_style_list_add_style(list, &styles->gauge_needle);
            break;
#endif
        default:
            break;
    }

    lv_obj_refresh_style(obj, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init_reset(lv_style_t* style)
{
    if(inited)
        lv_style_reset(style);
    else
        lv_style_init(style);
}

//#endif
