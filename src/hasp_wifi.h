#ifndef HASP_WIFI_H
#define HASP_WIFI_H

#include "ArduinoJson.h"

void wifiSetup(JsonObject settings);
bool wifiLoop();
bool wifiGetConfig(const JsonObject & settings);

#endif