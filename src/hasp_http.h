/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_HTTP_H
#define HASP_HTTP_H

#include "hasp_conf.h"

void httpSetup();
void IRAM_ATTR httpLoop(void);
void httpEvery5Seconds(void);
// void httpReconnect(void);
void httpStart(void);
void httpStop(void);

size_t httpClientWrite(const uint8_t * buf, size_t size); // Screenshot Write Data

bool httpGetConfig(const JsonObject & settings);
bool httpSetConfig(const JsonObject & settings);

#endif