/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MDNS_H
#define HASP_MDNS_H

#include "ArduinoJson.h"

/* ===== Default Event Processors ===== */
void mdnsSetup();
void IRAM_ATTR mdnsLoop(void);
void mdnsStart(void);
void mdnsStop(void);

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool mdnsGetConfig(const JsonObject & settings);
bool mdnsSetConfig(const JsonObject & settings);
#endif

#endif