#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#ifndef HASP_OTA_H
#define HASP_OTA_H

#include "ArduinoJson.h"

void otaSetup();
void otaLoop(void);
void otaEverySecond(void);
void otaHttpUpdate(const char * espOtaUrl);

#endif
#endif