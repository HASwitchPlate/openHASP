/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FILESYSTEM_H
#define HASP_FILESYSTEM_H

#include <Arduino.h>

bool filesystemSetup(void);

void filesystemList();
void filesystemInfo();

#if defined(ARDUINO_ARCH_ESP32)
void filesystemUnzip(const char*, const char* filename);
#endif

enum { ZIP_NO_COMPRESSION = 0, ZIP_DEFLTATE = 8 };
typedef uint16_t zip_compression_method_t;

typedef struct
{
    uint16_t dummy_bytes; // total struct needs to be a multiple of 4 bytes
    uint16_t min_version;
    uint16_t flags;
    zip_compression_method_t compression_method;
    uint16_t time_modified;
    uint16_t date_modified;
    uint32_t crc;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length; // OK
    uint16_t extra_length;
} zip_file_header_t;

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