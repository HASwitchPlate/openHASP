#include "hasp_conf.h"

#ifndef HASP_TELNET_H
#define HASP_TELNET_H

#if HASP_USE_TELNET > 0

#include "ArduinoJson.h"

void telnetSetup(const JsonObject & settings);
void telnetLoop(void);
void telnetStop(void);

void telnetPrint(const char * msg);
void telnetPrintln(const char * msg);
void telnetPrint(const __FlashStringHelper * msg);

bool telnetSetConfig(const JsonObject & settings);
bool telnetGetConfig(const JsonObject & settings);

#endif
#endif
