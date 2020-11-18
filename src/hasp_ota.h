/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

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