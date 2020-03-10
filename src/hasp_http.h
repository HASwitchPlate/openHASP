#ifndef HASP_HTTP_H
#define HASP_HTTP_H

#include <Arduino.h>
#include "ArduinoJson.h"

void httpSetup(const JsonObject & settings);
void httpLoop(void);
void httpEverySecond(void);
void httpReconnect(void);

bool httpGetConfig(const JsonObject & settings);
bool httpSetConfig(const JsonObject & settings);

#endif