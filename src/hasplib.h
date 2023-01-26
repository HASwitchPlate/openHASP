/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef ARDUINO
#include <Arduino.h>
#include "ArduinoLog.h"
#endif

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#include <cstdlib>
#include <cctype>
#include <string>
#include <stdint.h>

#include "ArduinoJson.h"

#include "hasp_conf.h"
#include "hasp_mem.h"

#include "lv_conf.h"
#include "lvgl.h"
#include "lv_fs_if.h"

#include "hasp/hasp.h"
#include "hasp/hasp_attribute.h"
#include "hasp/hasp_dispatch.h"
#include "hasp/hasp_event.h"
#include "hasp/hasp_font.h"
#include "hasp/hasp_object.h"
#include "hasp/hasp_page.h"
#include "hasp/hasp_parser.h"
#include "hasp/hasp_lvfs.h"

#include "hasp/lv_theme_hasp.h"

#ifdef ESP32
#include "hasp/hasp_nvs.h"
#endif

#if HASP_USE_FREETYPE > 0
#include "lv_freetype.h"
#endif

#if HASP_USE_CONFIG > 0
#include "hasp_config.h"
#endif

#if HASP_USE_PNGDECODE > 0
#include "lv_png.h"
#endif

#if HASP_USE_BMPDECODE > 0
#include "lv_bmp.h"
#endif

#if HASP_USE_GIFDECODE > 0
#include "lv_gif.h"
#endif

#if HASP_USE_JPGDECODE > 0
#include "lv_sjpg.h"
#endif

#if defined(HASP_USE_CUSTOM)
#include "custom/my_custom.h"
#endif