#ifndef HASP_MDNS_H
#define HASP_MDNS_H

#include "ArduinoJson.h"

void mdnsSetup(const JsonObject & settings);
void mdnsLoop(bool wifiIsConnected);
void mdnsStart(void);
void mdnsStop(void);

bool mdnsGetConfig(const JsonObject & settings);
bool mdnsSetConfig(const JsonObject & settings);

#endif