#ifndef HASP_MDNS_H
#define HASP_MDNS_H

#include "ArduinoJson.h"

void mdnsSetup(const JsonObject & settings);
void mdnsLoop(bool wifiIsConnected);
void mdnsStop();
bool mdnsGetConfig(const JsonObject & settings);

#endif