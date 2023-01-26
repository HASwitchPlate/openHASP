/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_WIFI_H
#define HASP_WIFI_H

#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#endif

void wifiSetup();
bool wifiShowAP();
bool wifiShowAP(char* ssid, char* pass);
bool wifiEvery5Seconds(void);
void wifiStop(void);

bool wifiValidateSsid(const char* ssid, const char* pass);
void wifi_get_statusupdate(char* buffer, size_t len);

void wifi_get_info(JsonDocument& doc);
const char* wifi_get_ssid();
const char* wifi_get_ip_address();

#if HASP_USE_CONFIG > 0
bool wifiGetConfig(const JsonObject& settings);
bool wifiSetConfig(const JsonObject& settings);
#endif

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#ifndef WIFI_PASSW
#define WIFI_PASSWORD ""
#else
#define WIFI_PASSWORD WIFI_PASSW
#endif
#endif

#endif