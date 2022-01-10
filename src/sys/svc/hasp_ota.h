/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#ifndef HASP_OTA_H
#define HASP_OTA_H

#include "hasp_conf.h"

#include "ArduinoJson.h"
#include <ArduinoOTA.h>

/* ===== Default Event Processors ===== */
#if HASP_USE_OTA > 0
void otaSetup(void);
IRAM_ATTR void otaLoop(void);
void otaEverySecond(void);
#endif

/* ===== Special Event Processors ===== */
void otaHttpUpdate(const char* espOtaUrl);

#endif
#endif