/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FILESYSTEM_H
#define HASP_FILESYSTEM_H

#include <Arduino.h>

bool filesystemSetup(void);

void filesystemList();
void filesystemInfo();

#if defined(ARDUINO_ARCH_ESP32)
#if HASP_USE_SPIFFS > 0
#include "SPIFFS.h"
#define HASP_FS SPIFFS
#elif HASP_USE_LITTLEFS > 0
#include "LITTLEFS.h"
#define HASP_FS LITTLEFS
#endif
#elif defined(ARDUINO_ARCH_ESP8266)
// included by default
#include <LittleFS.h>
#define HASP_FS LittleFS
#endif // ARDUINO_ARCH

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include <FS.h>
#include <Esp.h>
#endif // ARDUINO_ARCH

#endif // HASP_FILESYSTEM_H