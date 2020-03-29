#ifndef HASP_WIFI_H
#define HASP_WIFI_H

#include "ArduinoJson.h"

void wifiSetup(JsonObject settings);
bool wifiEvery5Seconds(void);
void wifiStop(void);

bool wifiGetConfig(const JsonObject & settings);
bool wifiSetConfig(const JsonObject & settings);

String wifiGetMacAddress(int start, const char * seperator);

#endif