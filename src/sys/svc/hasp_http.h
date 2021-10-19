/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_HTTP_H
#define HASP_HTTP_H

#include "hasp_conf.h"

struct hasp_http_config_t
{
    bool enable   = true;
    uint16_t port = 80;

    char user[32]     = "";
    char password[32] = "";
};

void httpSetup();
IRAM_ATTR void httpLoop(void);
// void httpReconnect(void);
void httpStart(void);
void httpStop(void);

size_t httpClientWrite(const uint8_t* buf, size_t size); // Screenshot Write Data

#if HASP_USE_CONFIG > 0
bool httpGetConfig(const JsonObject& settings);
bool httpSetConfig(const JsonObject& settings);
#endif // HASP_USE_CONFIG

#endif