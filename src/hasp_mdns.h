#ifndef HASP_MDNS_H
#define HASP_MDNS_H

#include "ArduinoJson.h"

void mdnsSetup();
void mdnsLoop(void);
void mdnsStart(void);
void mdnsStop(void);

bool mdnsGetConfig(const JsonObject & settings);
bool mdnsSetConfig(const JsonObject & settings);

#endif