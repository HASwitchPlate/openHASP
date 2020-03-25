#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

#include "ArduinoJson.h"

String debugHaspHeader(void);

void debugPreSetup(JsonObject settings);
void debugSetup(JsonObject settings);
void debugLoop(void);
void debugEverySecond(void);
void debugStart(void);
void debugStop(void);

void serialPrintln(String & debugText, uint8_t level);
void serialPrintln(const char * debugText, uint8_t level);

void syslogSend(uint8_t log, const char * debugText);

bool debugGetConfig(const JsonObject & settings);
bool debugSetConfig(const JsonObject & settings);

#endif