/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TIME_H
#define HASP_TIME_H

#include "hasplib.h"

/* ===== Default Event Processors ===== */
void timeSetup();

/* ===== Special Event Processors ===== */

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool timeGetConfig(const JsonObject& settings);
bool timeSetConfig(const JsonObject& settings);
#endif

#endif