/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

#include "ArduinoJson.h"
#include "lvgl.h"

/* ===== Default Event Processors ===== */
void debugPreSetup(JsonObject settings);
void debugSetup();
void debugLoop(void);
void debugEverySecond(void);
void debugStart(void);
void debugStop(void);

/* ===== Special Event Processors ===== */
void debugLvglLogEvent(lv_log_level_t level, const char * file, uint32_t line, const char * funcname, const char * descr);
void debugPrintHaspHeader(Print * output);
void debugStartSyslog(void);
void debugStopSyslog(void);
// void syslogSend(uint8_t log, const char * debugText);

/* ===== Read/Write Configuration ===== */
bool debugGetConfig(const JsonObject & settings);
bool debugSetConfig(const JsonObject & settings);

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