#ifndef HASP_HTTP_H
#define HASP_HTTP_H

#include <Arduino.h>
#include "ArduinoJson.h"

void httpSetup();
void httpLoop(void);
void httpEvery5Seconds(void);
void httpReconnect(void);

bool httpGetConfig(const JsonObject & settings);
bool httpSetConfig(const JsonObject & settings);

#endif