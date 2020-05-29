#ifndef HASP_CONFIG_H
#define HASP_CONFIG_H

#include "ArduinoJson.h"

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

const char HASP_CONFIG_FILE[] PROGMEM = "/config.json";

void configSetup();
void configStop(void);

void configSetConfig(JsonObject & settings);
void configGetConfig(JsonDocument & settings);
void configWriteConfig();
void configOutput(const JsonObject & settings);

bool configSet(int8_t & value, const JsonVariant & setting, const char * name);
bool configSet(uint8_t & value, const JsonVariant & setting, const char * name);
bool configSet(uint16_t & value, const JsonVariant & setting, const char * name);
bool configClear();

#endif