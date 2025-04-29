/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FILESYSTEM_H
#define HASP_FILESYSTEM_H

#include <Arduino.h>
#include <FS.h>
#include "hasp_debug.h"

#include <functional>
#include <memory>
#include <Stream.h>

bool filesystemSetup(void);
void filesystemList();
void filesystemInfo();
void filesystemSetupFiles();

#if defined(ARDUINO_ARCH_ESP32)
#if HASP_USE_SPIFFS > 0
#include "SPIFFS.h"
#define HASP_FS SPIFFS
#elif HASP_USE_LITTLEFS > 0

#ifndef ESP_ARDUINO_VERSION_VAL
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0)
#include <LittleFS.h>
#define HASP_FS LittleFS
#else
#include "LITTLEFS.h"
#include "esp_littlefs.h"
#define HASP_FS LITTLEFS
#endif // ESP_ARDUINO_VERSION

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

#if defined(ARDUINO_ARCH_ESP32)
void filesystemUnzip(const char*, const char* filename, uint8_t source);
String filesystem_list(fs::FS& fs, const char* dirname, uint8_t levels);
#endif

#endif // HASP_FILESYSTEM_H