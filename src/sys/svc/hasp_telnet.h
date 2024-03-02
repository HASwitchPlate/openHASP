/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TELNET_H
#define HASP_TELNET_H

#if HASP_USE_TELNET > 0

#include "hasplib.h"

/* ===== Default Event Processors ===== */
void telnetSetup();
IRAM_ATTR void telnetLoop(void);
void telnetEvery5Seconds(void);
void telnetEverySecond(void);
void telnetStart(void);
void telnetStop(void);

/* ===== Special Event Processors ===== */
void telnet_update_prompt();

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool telnetSetConfig(const JsonObject& settings);
bool telnetGetConfig(const JsonObject& settings);
#endif

#define TELNET_UNAUTHENTICATED 0
#define TELNET_USERNAME_OK 10
#define TELNET_USERNAME_NOK 99
#define TELNET_AUTHENTICATED 255

#endif
#endif
