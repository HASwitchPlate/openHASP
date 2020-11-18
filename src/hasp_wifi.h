/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_WIFI_H
#define HASP_WIFI_H

#include "ArduinoJson.h"

void wifiSetup();
bool wifiShowAP();
bool wifiShowAP(char * ssid, char * pass);
bool wifiEvery5Seconds(void);
void wifiStop(void);
bool wifiTestConnection();

bool wifiGetConfig(const JsonObject & settings);
bool wifiSetConfig(const JsonObject & settings);

#endif