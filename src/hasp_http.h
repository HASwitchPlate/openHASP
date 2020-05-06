#ifndef HASP_HTTP_H
#define HASP_HTTP_H

#include <Arduino.h>
#include "ArduinoJson.h"

void httpSetup();
void httpLoop(void);
void httpEvery5Seconds(void);
void httpReconnect(void);

size_t httpClientWrite(const uint8_t *buf, size_t size); // Screenshot Write Data

bool httpGetConfig(const JsonObject & settings);
bool httpSetConfig(const JsonObject & settings);

#endif