/* MIT License - Copyright (c) 2020 Francis Van Roie
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
#elif HASP_USE_LITTLEFS > 0
#include "LITTLEFS.h"
#endif
#elif defined(ARDUINO_ARCH_ESP8266)
// included by default
#endif // ARDUINO_ARCH

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include <FS.h>
#include <Esp.h>

#if HASP_USE_SPIFFS > 0
#define HASP_FS SPIFFS
#elif HASP_USE_LITTLEFS > 0
#define HASP_FS LITTLEFS
#endif // HASP_USE
#endif // ARDUINO_ARCH

#endif // HASP_FILESYSTEM_H