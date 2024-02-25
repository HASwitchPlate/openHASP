/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_OTA_H
#define HASP_OTA_H

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "hasp_conf.h"

#include "ArduinoJson.h"
#include <ArduinoOTA.h>

/* ===== Default Event Processors ===== */
void otaSetup(void);
#if HASP_USE_ARDUINOOTA > 0
IRAM_ATTR void otaLoop(void);
void otaEverySecond(void);
#endif // HASP_USE_ARDUINOOTA

/* ===== Special Event Processors ===== */
#if HASP_USE_HTTP_UPDATE > 0
void ota_http_update(const char* espOtaUrl);
#endif // HASP_USE_HTTP_UPDATE

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool otaGetConfig(const JsonObject& settings);
bool otaSetConfig(const JsonObject& settings);
#endif

#endif
#endif