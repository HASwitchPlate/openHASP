/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

#include "ArduinoJson.h"
#include "hasp_macro.h"
#include "lvgl.h"

#include "lang/lang.h"

#ifndef WINDOWS
#include "ArduinoLog.h"

/* ===== Default Event Processors ===== */
void debugPreSetup(JsonObject settings);
void debugSetup();
void debugLoop(void);
void debugEverySecond(void);
void debugStart(void);
void debugStop(void);

/* ===== Special Event Processors ===== */
void debugLvglLogEvent(lv_log_level_t level, const char* file, uint32_t line, const char* funcname, const char* descr);
void debugPrintHaspHeader(Print* output);
void debugStartSyslog(void);
void debugStopSyslog(void);
// void syslogSend(uint8_t log, const char * debugText);
#else
#include <iostream>

#define LOG_FATAL(x, ...)                                                                                              \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_ERROR(x, ...)                                                                                              \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_WARNING(x, ...)                                                                                            \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_NOTICE(x, ...)                                                                                             \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_TRACE(x, ...)                                                                                              \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_VERBOSE(x, ...)                                                                                            \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_DEBUG(x, ...)                                                                                              \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_INFO(x, ...)                                                                                               \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)

/* json keys used in the configfile */
// const char FP_CONFIG_STARTPAGE[] PROGMEM = "startpage";
// const char FP_CONFIG_STARTDIM[] PROGMEM = "startdim";
// const char FP_CONFIG_THEME[] PROGMEM = "theme";
// const char FP_CONFIG_HUE[] PROGMEM = "hue";
// const char FP_CONFIG_ZIFONT[] PROGMEM = "font";
// const char FP_CONFIG_PAGES[] PROGMEM = "pages";
// const char FP_CONFIG_ENABLE[] PROGMEM = "enable";
// const char FP_CONFIG_HOST[] PROGMEM = "host";
// const char FP_CONFIG_PORT[] PROGMEM = "port";
// const char FP_CONFIG_NAME[] PROGMEM = "name";
// const char FP_CONFIG_USER[] PROGMEM = "user";
// const char FP_CONFIG_PASS[] PROGMEM = "pass";
// const char FP_CONFIG_SSID[] PROGMEM = "ssid";
// const char FP_CONFIG_GROUP[] PROGMEM = "group";
// const char FP_CONFIG_BAUD[] PROGMEM = "baud";
// const char FP_CONFIG_LOG[] PROGMEM = "log";
// const char FP_CONFIG_PROTOCOL[] PROGMEM = "proto";
// const char FP_GUI_ROTATION[] PROGMEM = "rotate";
// const char FP_GUI_INVERT[] PROGMEM = "invert";
// const char FP_GUI_TICKPERIOD[] PROGMEM = "tick";
// const char FP_GUI_IDLEPERIOD1[] PROGMEM = "idle1";
// const char FP_GUI_IDLEPERIOD2[] PROGMEM = "idle2";
// const char FP_GUI_CALIBRATION[] PROGMEM = "calibration";
// const char FP_GUI_BACKLIGHTPIN[] PROGMEM = "bckl";
// const char FP_GUI_POINTER[] PROGMEM = "cursor";
// const char FP_DEBUG_TELEPERIOD[] PROGMEM = "tele";
// const char FP_GPIO_CONFIG[] PROGMEM = "config";

// const char FP_HASP_CONFIG_FILE[] PROGMEM = "/config.json";

// const char FP_WIFI[] PROGMEM = "wifi";
// const char FP_MQTT[] PROGMEM = "mqtt";
// const char FP_HTTP[] PROGMEM = "http";
// const char FP_GPIO[] PROGMEM = "gpio";
// const char FP_MDNS[] PROGMEM = "mdns";
// const char FP_HASP[] PROGMEM = "hasp";
// const char FP_GUI[] PROGMEM = "gui";
// const char FP_DEBUG[] PROGMEM = "debug";

#endif

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool debugGetConfig(const JsonObject& settings);
bool debugSetConfig(const JsonObject& settings);
#endif

// void debugPrintPrefix(int level, Print * _logOutput);
// void debugPrintSuffix(int level, Print * _logOutput);
// void debugSendOuput(const char * buffer);

enum {
    TAG_MAIN = 0,
    TAG_HASP = 1,
    TAG_ATTR = 2,
    TAG_MSGR = 3,
    TAG_OOBE = 4,
    TAG_HAL  = 5,
    TAG_DRVR = 6,
    TAG_DEV  = 7,

    TAG_DEBG = 10,
    TAG_TELN = 11,
    TAG_SYSL = 12,
    TAG_TASM = 13,

    TAG_CONF = 20,
    TAG_GUI  = 21,
    TAG_TFT  = 22,

    TAG_EPRM = 30,
    TAG_FILE = 31,
    TAG_GPIO = 40,

    TAG_FWUP = 50,

    TAG_ETH      = 60,
    TAG_WIFI     = 61,
    TAG_HTTP     = 62,
    TAG_OTA      = 63,
    TAG_MDNS     = 64,
    TAG_MQTT     = 65,
    TAG_MQTT_PUB = 66,
    TAG_MQTT_RCV = 67,

    TAG_LVGL = 90,
    TAG_LVFS = 91,
    TAG_FONT = 92
};

#endif
