/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/***************************************************
// This file contains the default settings that are
// burned into the compiled firmware.
//
// These default settings can be changed at runtime
//
// To use: Save a copy as user_config_override.h
***************************************************/
#ifndef HASP_USER_CONFIG_OVERRIDE_H
#define HASP_USER_CONFIG_OVERRIDE_H

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
 *        Timezone Settings
 **************************************************/
#define MYTZ "CET-1CEST,M3.5.0,M10.5.0/3"            // A full list with possible timezones can be found here https://gist.github.com/alwynallan/24d96091655391107939

/***************************************************
 *        Interface Language Settings
 **************************************************/
#define HASP_LANGUAGE en_US                          // English
// #define HASP_LANGUAGE nl_NL                       // Dutch
// #define HASP_LANGUAGE hu_HU                       // Hungarian
// #define HASP_LANGUAGE ro_RO                       // Romanian

/***************************************************
 *        Web interface coloring
 **************************************************/
// Light theme (default)
#define D_HTTP_COLOR_TEXT               "#000"       // Global text color - Black
#define D_HTTP_COLOR_BACKGROUND         "#fff"       // Global background color - White
#define D_HTTP_COLOR_INPUT_TEXT         "#000"       // Input text color - Black
#define D_HTTP_COLOR_INPUT              "#fff"       // Input background color - White
#define D_HTTP_COLOR_INPUT_WARNING      "#f00"       // Input warning border color - Red
#define D_HTTP_COLOR_BUTTON_TEXT        "#fff"       // Button text color - White
#define D_HTTP_COLOR_BUTTON             "#1fa3ec"    // Button color - Vivid blue
#define D_HTTP_COLOR_BUTTON_RESET       "#f00"       // Restart/Reset button color - red

/*
// Dark theme
#define D_HTTP_COLOR_TEXT               "#eaeaea"    // Global text color - Very light gray
#define D_HTTP_COLOR_BACKGROUND         "#252525"    // Global background color - Very dark gray (mostly black)
#define D_HTTP_COLOR_INPUT_TEXT         "#000"       // Input text color - Black
#define D_HTTP_COLOR_INPUT              "#ddd"       // Input background color - Very light gray
#define D_HTTP_COLOR_INPUT_WARNING      "#ff5661"    // Warning text color - Brick Red
#define D_HTTP_COLOR_BUTTON_TEXT        "#faffff"    // Button text color - Very pale (mostly white) cyan
#define D_HTTP_COLOR_BUTTON             "#1fa3ec"    // Button color - Vivid blue
#define D_HTTP_COLOR_BUTTON_RESET       "#d43535"    // Restart/Reset/Delete button color - Strong red
*/

/***************************************************
 *        Other Settings
 **************************************************/
//#define HASP_USE_HA                                 // Enable Home Assistant auto-discovery

#endif