#ifndef HASP_WIFI_H
#define HASP_WIFI_H

#include "ArduinoJson.h"

void wifiSetup(JsonObject settings);
bool wifiLoop();
void wifiStop();

bool wifiGetConfig(const JsonObject & settings);
bool wifiSetConfig(const JsonObject & settings);

#endif