#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

#include "ArduinoJson.h"

void debugPreSetup(JsonObject settings);
void debugSetup(JsonObject settings);
void debugLoop(void);
void debugStart(void);
void debugStop(void);

void serialPrintln(String & debugText);
void serialPrintln(const char * debugText);

void syslogSend(uint8_t log, const char * debugText);

bool debugGetConfig(const JsonObject & settings);
bool debugSetConfig(const JsonObject & settings);

#endif