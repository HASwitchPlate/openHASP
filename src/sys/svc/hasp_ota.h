/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#ifndef HASP_OTA_H
#define HASP_OTA_H

#include "ArduinoJson.h"

/* ===== Default Event Processors ===== */
void otaSetup(void);
void otaLoop(void);
void otaEverySecond(void);

/* ===== Special Event Processors ===== */
void otaHttpUpdate(const char* espOtaUrl);

#endif
#endif