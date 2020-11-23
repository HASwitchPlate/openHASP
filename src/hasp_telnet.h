/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TELNET_H
#define HASP_TELNET_H

#if HASP_USE_TELNET > 0

#include "hasp_conf.h"
#include "ArduinoJson.h"

/* ===== Default Event Processors ===== */
void telnetSetup();
void IRAM_ATTR telnetLoop(void);
void telnetEvery5Seconds(void);
void telnetEverySecond(void);
void telnetStart(void);
void telnetStop(void);

/* ===== Special Event Processors ===== */

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */

bool telnetSetConfig(const JsonObject & settings);
bool telnetGetConfig(const JsonObject & settings);

#define TELNET_UNAUTHENTICATED 0
#define TELNET_USERNAME_OK 10
#define TELNET_USERNAME_NOK 99
#define TELNET_AUTHENTICATED 255

#endif
#endif
