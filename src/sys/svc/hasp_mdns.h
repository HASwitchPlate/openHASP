/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MDNS_H
#define HASP_MDNS_H

#include "ArduinoJson.h"

struct hasp_mdns_config_t
{
    uint8_t enable = true;
};

/* ===== Default Event Processors ===== */
void mdnsSetup();
IRAM_ATTR void mdnsLoop(void);
void mdnsStart(void);
void mdnsStop(void);

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool mdnsGetConfig(const JsonObject& settings);
bool mdnsSetConfig(const JsonObject& settings);
#endif

#endif