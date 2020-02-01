#ifndef HASP_CONFIG_H
#define HASP_CONFIG_H

#include "ArduinoJson.h"

/* json keys used in the configfile */
const char F_CONFIG_STARTPAGE[] PROGMEM = "startpage";
const char F_CONFIG_THEME[] PROGMEM     = "theme";
const char F_CONFIG_HUE[] PROGMEM       = "hue";
const char F_CONFIG_ZIFONT[] PROGMEM    = "font";
const char F_CONFIG_PAGES[] PROGMEM     = "pages";
const char F_CONFIG_ENABLE[] PROGMEM    = "enable";
const char F_CONFIG_HOST[] PROGMEM      = "host";
const char F_CONFIG_PORT[] PROGMEM      = "port";
const char F_CONFIG_USER[] PROGMEM      = "user";
const char F_CONFIG_PASS[] PROGMEM      = "pass";
const char F_CONFIG_SSID[] PROGMEM      = "ssid";
const char F_CONFIG_GROUP[] PROGMEM     = "group";
const char F_GUI_TICKPERIOD[] PROGMEM   = "tickperiod";

void configSetup(JsonDocument & settings);
void configLoop(void);
void configStop(void);

void configSetConfig(JsonObject & settings);
void configGetConfig(JsonDocument & settings);
void configWriteConfig();
bool configChanged(void);

#endif