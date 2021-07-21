#ifndef LV_CONF_STUB_H
#define LV_CONF_STUB_H

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#if 0
#include "lv_conf_v7.h"
#define LV_THEME_DEFAULT_FLAGS LV_THEME_DEFAULT_FLAG
#else
#include "lv_conf_v8.h"
#include "lv_compat_v7.h"

#endif

#define LV_THEME_DEFAULT_FLAG 0
#define LV_THEME_DEFAULT_FLAGS LV_THEME_DEFAULT_FLAG

//#ifdef LV_THEME_DEFAULT_FLAG

#define lv_obj_set_top(obj, fit)
#define lv_obj_add_protect(a, b)
#define LV_PROTECT_PRESS_LOST 0

#define LV_CPICKER_TYPE_RECT 0
#define LV_CPICKER_TYPE_DISC 0

#define LV_THEME_DEFAULT_FLAGS LV_THEME_DEFAULT_FLAG

#define LV_THEME_DEFAULT_FONT_SMALL LV_FONT_DEFAULT
#define LV_THEME_DEFAULT_FONT_NORMAL LV_FONT_DEFAULT
#define LV_THEME_DEFAULT_FONT_SUBTITLE LV_FONT_DEFAULT
#define LV_THEME_DEFAULT_FONT_TITLE LV_FONT_DEFAULT

#define LV_IMG_CACHE_DEF_SIZE_PSRAM 20

#define _lv_memset_00(p, size) memset(p, 0, size);
#define LV_ASSERT_MEM(x)

//#endif
#endif
