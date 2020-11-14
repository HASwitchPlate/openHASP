#ifndef HASP_FILESYSTEM_H
#define HASP_FILESYSTEM_H

#include <Arduino.h>

bool filesystemSetup(void);

void filesystemList();
void filesystemInfo();


#if defined(ARDUINO_ARCH_ESP32)
#include <FS.h>
#include <ESP.h>

#if HASP_USE_SPIFFS > 0
#include "SPIFFS.h"
extern FS * HASP_FS = &SPIFFS;
#elif HASP_USE_LITTLEFS > 0
#include "LittleFS.h"
#define HASP_FS LITTLEFS
#endif

#elif defined(ARDUINO_ARCH_ESP8266)
#include <FS.h>
#include <ESP.h>
#define HASP_FS SPIFFS
#endif

#endif
