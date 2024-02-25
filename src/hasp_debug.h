/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#include "hasp_conf.h"

#include "ArduinoJson.h"
#include "hasp_debug.h"
#include "hasp_macro.h"
#include "lvgl.h"

#include "lang/lang.h"

#if HASP_TARGET_ARDUINO
/* ===== Default Event Processors ===== */
void debugSetup(JsonObject settings);

/* ===== Special Event Processors ===== */

void debugStartSyslog(void);
void debugStopSyslog(void);
// void syslogSend(uint8_t log, const char * debugText);

#else

#define Print void

#include <iostream>
#include <sys/time.h>

#define LOG_LEVEL_SILENT -1

#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ALERT 1
#define LOG_LEVEL_CRITICAL 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_WARNING 4
#define LOG_LEVEL_NOTICE 5
#define LOG_LEVEL_INFO 5
#define LOG_LEVEL_TRACE 6
#define LOG_LEVEL_VERBOSE 7
#define LOG_LEVEL_DEBUG 8
#define LOG_LEVEL_OUTPUT 9

#define LOG_FATAL(x, ...)                                                                                              \
    debugPrintPrefix(x, LOG_LEVEL_FATAL, NULL);                                                                        \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_ERROR(x, ...)                                                                                              \
    debugPrintPrefix(x, LOG_LEVEL_ERROR, NULL);                                                                        \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_WARNING(x, ...)                                                                                            \
    debugPrintPrefix(x, LOG_LEVEL_WARNING, NULL);                                                                      \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_NOTICE(x, ...)                                                                                             \
    debugPrintPrefix(x, LOG_LEVEL_NOTICE, NULL);                                                                       \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_TRACE(x, ...)                                                                                              \
    debugPrintPrefix(x, LOG_LEVEL_TRACE, NULL);                                                                        \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_VERBOSE(x, ...)                                                                                            \
    debugPrintPrefix(x, LOG_LEVEL_VERBOSE, NULL);                                                                      \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_DEBUG(x, ...)                                                                                              \
    debugPrintPrefix(x, LOG_LEVEL_DEBUG, NULL);                                                                        \
    printf(__VA_ARGS__);                                                                                               \
    std::cout << std::endl;                                                                                            \
    fflush(stdout)
#define LOG_INFO(x, ...)                                                                                               \
    debugPrintPrefix(x, LOG_LEVEL_INFO, NULL);                                                                         \
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

#ifdef __cplusplus
extern "C" {
#endif

// Functions used by ANDROID, WINDOWS and POSSIX
void debugLvglLogEvent(lv_log_level_t level, const char* file, uint32_t line, const char* funcname, const char* descr);
IRAM_ATTR void debugLoop(void);
void debugEverySecond(void);
void debugStart(void);
bool debugStartSerial(void);
void debugStop(void);
void debugPrintHaspHeader(Print* output);
void debugPrintTag(uint8_t tag, Print* _logOutput);
void debugPrintPrefix(uint8_t tag, int level, Print* _logOutput);
bool debugSyslogPrefix(uint8_t tag, int level, Print* _logOutput, const char* processname);

#ifdef __cplusplus
}
#endif

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool debugGetConfig(const JsonObject& settings);
bool debugSetConfig(const JsonObject& settings);
#endif

enum {
    TAG_MAIN  = 0,
    TAG_HASP  = 1,
    TAG_ATTR  = 2,
    TAG_MSGR  = 3,
    TAG_OOBE  = 4,
    TAG_HAL   = 5,
    TAG_DRVR  = 6,
    TAG_DEV   = 7,
    TAG_EVENT = 8,

    TAG_DEBG = 10,
    TAG_CONS = 11,
    TAG_TELN = 12,
    TAG_SYSL = 13,
    TAG_TASM = 14,

    TAG_CONF = 20,
    TAG_GUI  = 21,
    TAG_TFT  = 22,

    TAG_EPRM = 30,
    TAG_FILE = 31,
    TAG_NVS  = 32,
    TAG_GPIO = 40,

    TAG_ETH      = 60,
    TAG_WIFI     = 61,
    TAG_HTTP     = 62,
    TAG_OTA      = 63,
    TAG_MDNS     = 64,
    TAG_MQTT     = 65,
    TAG_MQTT_PUB = 66,
    TAG_MQTT_RCV = 67,
    TAG_FTP      = 68,
    TAG_TIME     = 69,
    TAG_NETW     = 70,
    TAG_WG       = 71,

    TAG_LVGL = 90,
    TAG_LVFS = 91,
    TAG_FONT = 92,

    TAG_CUSTOM = 99
};

#define HASP_SERIAL Serial

//#define TERM_COLOR_Black "\u001b[30m"
#define TERM_COLOR_GRAY "\e[37m"
#define TERM_COLOR_RED "\e[91m"
#define TERM_COLOR_GREEN "\e[92m"
#define TERM_COLOR_ORANGE "\e[38;5;214m"
#define TERM_COLOR_YELLOW "\e[93m"
#define TERM_COLOR_BLUE "\e[94m"
#define TERM_COLOR_MAGENTA "\e[35m"
#define TERM_COLOR_CYAN "\e[96m"
#define TERM_COLOR_WHITE "\e[97m"
#define TERM_COLOR_RESET "\e[0m"
#define TERM_CLEAR_LINE "\e[1000D\e[0K"

#endif
