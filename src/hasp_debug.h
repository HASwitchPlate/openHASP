#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

#include "ArduinoJson.h"
#include "lvgl.h"

String debugHaspHeader(void);

void debugPreSetup(JsonObject settings);
void debugSetup();
void debugLoop(void);
void debugEverySecond(void);
void debugStart(void);
void debugStop(void);

void serialPrintln(String & debugText, uint8_t level);
void serialPrintln(const char * debugText, uint8_t level);

void syslogSend(uint8_t log, const char * debugText);

bool debugGetConfig(const JsonObject & settings);
bool debugSetConfig(const JsonObject & settings);

// void debugPrintPrefix(int level, Print * _logOutput);
// void debugPrintSuffix(int level, Print * _logOutput);
// void debugSendOuput(const char * buffer);
void debugLvgl(lv_log_level_t level, const char * file, uint32_t line, const char * funcname, const char * descr);

#endif