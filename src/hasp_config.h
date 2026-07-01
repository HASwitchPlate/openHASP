/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_CONFIG > 0

#ifndef HASP_CONFIG_H
#define HASP_CONFIG_H

#include "hasplib.h"

#define MAX_CONFIG_JSON_ALLOC_SIZE (2048)

/* ===== Default Event Processors ===== */
void configSetup(void);
void configLoop(void);
void configEverySecond(void);
void configStart(void);
void configStop(void);

/* ===== Special Event Processors ===== */
DeserializationError configParseFile(String& configFile, JsonDocument& settings);
DeserializationError configRead(JsonDocument& settings, bool setupdebug);
void configWrite(void);
void configOutput(const JsonObject& settings, uint8_t tag);
bool configClearEeprom(void);

/* ===== Getter and Setter Functions ===== */
#if HASP_TARGET_ARDUINO
bool configSet(bool& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
bool configSet(int8_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
bool configSet(uint8_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
bool configSet(uint16_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
bool configSet(int32_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
bool configSet(lv_color_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
bool configSet(char* value, size_t size, const JsonVariant& setting, const __FlashStringHelper* fstr_name);
#endif
bool configSet(bool& value, const JsonVariant& setting, const char* fstr_name);
bool configSet(int8_t& value, const JsonVariant& setting, const char* fstr_name);
bool configSet(uint8_t& value, const JsonVariant& setting, const char* fstr_name);
bool configSet(uint16_t& value, const JsonVariant& setting, const char* fstr_name);
bool configSet(int32_t& value, const JsonVariant& setting, const char* fstr_name);
bool configSet(lv_color_t& value, const JsonVariant& setting, const char* fstr_name);
void configMaskPasswords(JsonDocument& settings);

/* ===== Read/Write Configuration ===== */
#if HASP_TARGET_ARDUINO
void configSetConfig(JsonObject& settings);
void configGetConfig(JsonDocument& settings);
#endif

/* json keys used in the configfile */
constexpr char FP_CONFIG_STARTPAGE[]        = "startpage";
constexpr char FP_CONFIG_STARTDIM[]         = "startdim";
constexpr char FP_CONFIG_THEME[]            = "theme";
constexpr char FP_CONFIG_HUE[]              = "hue";
constexpr char FP_CONFIG_ZIFONT[]           = "font";
constexpr char FP_CONFIG_PAGES[]            = "pages";
constexpr char FP_CONFIG_COLOR1[]           = "color1";
constexpr char FP_CONFIG_COLOR2[]           = "color2";
constexpr char FP_CONFIG_ENABLE[]           = "enable";
constexpr char FP_CONFIG_HOST[]             = "host";
constexpr char FP_CONFIG_PORT[]             = "port";
constexpr char FP_CONFIG_PASV[]             = "pasv";
constexpr char FP_CONFIG_NAME[]             = "name";
constexpr char FP_CONFIG_USER[]             = "user";
constexpr char FP_CONFIG_PASS[]             = "pass";
constexpr char FP_CONFIG_SSID[]             = "ssid";
constexpr char FP_CONFIG_NODE[]             = "node";
constexpr char FP_CONFIG_NODE_TOPIC[]       = "node_t";
constexpr char FP_CONFIG_HASS[]             = "hass";
constexpr char FP_CONFIG_HASS_TOPIC[]       = "hass_t";
constexpr char FP_CONFIG_GROUP[]            = "group";
constexpr char FP_CONFIG_GROUP_TOPIC[]      = "group_t";
constexpr char FP_CONFIG_BROADCAST[]        = "broadcast";
constexpr char FP_CONFIG_BROADCAST_TOPIC[]  = "broadcast_t";
constexpr char FP_CONFIG_BAUD[]             = "baud";
constexpr char FP_CONFIG_LOG[]              = "log";
constexpr char FP_CONFIG_PROTOCOL[]         = "proto";
constexpr char FP_CONFIG_VPN_IP[]           = "vpnip";
constexpr char FP_CONFIG_PRIVATE_KEY[]      = "privkey";
constexpr char FP_CONFIG_PUBLIC_KEY[]       = "pubkey";
constexpr char FP_GUI_ROTATION[]            = "rotate";
constexpr char FP_GUI_INVERT[]              = "invert";
constexpr char FP_GUI_TICKPERIOD[]          = "tick";
constexpr char FP_GUI_IDLEPERIOD1[]         = "idle1";
constexpr char FP_GUI_IDLEPERIOD2[]         = "idle2";
constexpr char FP_GUI_CALIBRATION[]         = "calibration";
constexpr char FP_GUI_BACKLIGHTPIN[]        = "bckl";
constexpr char FP_GUI_BACKLIGHTINVERT[]     = "bcklinv";
constexpr char FP_GUI_POINTER[]             = "cursor";
constexpr char FP_GUI_LONG_TIME[]           = "long";
constexpr char FP_GUI_REPEAT_TIME[]         = "repeat";
constexpr char FP_DEBUG_TELEPERIOD[]        = "tele";
constexpr char FP_DEBUG_ANSI[]              = "ansi";
constexpr char FP_GPIO_CONFIG[]             = "config";

constexpr char FP_HASP_CONFIG_FILE[]  = "/config.json";

constexpr char FP_WIFI[]   = "wifi";
constexpr char FP_WG[]     = "wg";
constexpr char FP_MQTT[]   = "mqtt";
constexpr char FP_HTTP[]   = "http";
constexpr char FP_FTP[]    = "ftp";
constexpr char FP_GPIO[]   = "gpio";
constexpr char FP_MDNS[]   = "mdns";
constexpr char FP_HASP[]   = "hasp";
constexpr char FP_GUI[]    = "gui";
constexpr char FP_DEBUG[]  = "debug";
constexpr char FP_TELNET[] = "telnet";
constexpr char FP_TIME[]   = "time";
constexpr char FP_OTA[]    = "ota";

#endif

#endif // HASP_USE_CONFIG
