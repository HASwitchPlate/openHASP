/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_CONFIG > 0

#ifndef HASP_CONFIG_H
#define HASP_CONFIG_H

#include "hasp_conf.h"
#include "ArduinoJson.h"
#include "hasp_debug.h" // for TAG_CONF

/* ===== Default Event Processors ===== */
void configSetup(void);
void IRAM_ATTR configLoop(void);
void configEverySecond(void);
void configStart(void);
void configStop(void);

/* ===== Special Event Processors ===== */
void configWriteConfig(void);
void configOutput(const JsonObject & settings, uint8_t tag = TAG_CONF);
bool configClearEeprom(void);

/* ===== Getter and Setter Functions ===== */
bool configSet(int8_t & value, const JsonVariant & setting, const __FlashStringHelper * fstr_name);
bool configSet(uint8_t & value, const JsonVariant & setting, const __FlashStringHelper * fstr_name);
bool configSet(uint16_t & value, const JsonVariant & setting, const __FlashStringHelper * fstr_name);

/* ===== Read/Write Configuration ===== */
void configSetConfig(JsonObject & settings);
void configGetConfig(JsonDocument & settings);

/* json keys used in the configfile */
const char F_CONFIG_STARTPAGE[] PROGMEM = "startpage";
const char F_CONFIG_STARTDIM[] PROGMEM  = "startdim";
const char F_CONFIG_THEME[] PROGMEM     = "theme";
const char F_CONFIG_HUE[] PROGMEM       = "hue";
const char F_CONFIG_ZIFONT[] PROGMEM    = "font";
const char F_CONFIG_PAGES[] PROGMEM     = "pages";
const char F_CONFIG_ENABLE[] PROGMEM    = "enable";
const char F_CONFIG_HOST[] PROGMEM      = "host";
const char F_CONFIG_PORT[] PROGMEM      = "port";
const char F_CONFIG_NAME[] PROGMEM      = "name";
const char F_CONFIG_USER[] PROGMEM      = "user";
const char F_CONFIG_PASS[] PROGMEM      = "pass";
const char F_CONFIG_SSID[] PROGMEM      = "ssid";
const char F_CONFIG_GROUP[] PROGMEM     = "group";
const char F_CONFIG_BAUD[] PROGMEM      = "baud";
const char F_CONFIG_LOG[] PROGMEM       = "log";
const char F_CONFIG_PROTOCOL[] PROGMEM  = "proto";
const char F_GUI_ROTATION[] PROGMEM     = "rotation";
const char F_GUI_TICKPERIOD[] PROGMEM   = "tickperiod";
const char F_GUI_IDLEPERIOD1[] PROGMEM  = "idle1";
const char F_GUI_IDLEPERIOD2[] PROGMEM  = "idle2";
const char F_GUI_CALIBRATION[] PROGMEM  = "calibration";
const char F_GUI_BACKLIGHTPIN[] PROGMEM = "bcklpin";
const char F_GUI_POINTER[] PROGMEM      = "pointer";
const char F_DEBUG_TELEPERIOD[] PROGMEM = "teleperiod";
const char F_GPIO_CONFIG[] PROGMEM      = "config";

const char HASP_CONFIG_FILE[] PROGMEM = "/config.json";

#endif

#endif // HASP_USE_CONFIG