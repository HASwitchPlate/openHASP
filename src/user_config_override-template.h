/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/***************************************************
// This file contains the default settings that are
// burned into the compiled firmware.
//
// These default settings can be changed at runtime
//
// To use: Save a copy as user_config_override.h
***************************************************/

#define SERIAL_SPEED 115200

/***************************************************
          WiFi Settings
 **************************************************/
#define WIFI_SSID ""
#define WIFI_PASSW ""

/***************************************************
          MQTT Settings
 **************************************************/
#define MQTT_HOST ""
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSW ""
#define MQTT_PREFIX "hasp"
#define MQTT_NODENAME "plate01"
#define MQTT_GROUPNAME "plates"

#define MQTT_TELEPERIOD 60000
#define MQTT_STATEPERIOD 300000

/***************************************************
 *        Server Settings
 **************************************************/
#define OTA_HOSTNAME ""
#define OTA_SERVER ""
#define OTA_PORT 80
#define OTA_URL ""
#define OTA_PASSWORD ""

/***************************************************
 *        Syslog Settings
 **************************************************/
#define SYSLOG_SERVER ""
#define SYSLOG_PORT 514
#define APP_NAME "HASP"

/***************************************************
 *        Font Settings
 **************************************************/
//#define USE_BASIC_FONTS  //*_basic.c
//#define USE_BASIC_HU //or USE_BASIC_RO //*_basic_XX.c (needs USE_BASIC_FONTS)
//#define USE_FONTAWESOME_SEPARATE //fontawesome_26.c
//#define USE_EXTRA_BIGNUMBERS //roboto_condensed_bignumbers_44.c