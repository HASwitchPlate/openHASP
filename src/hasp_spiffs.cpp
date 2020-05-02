#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_conf.h"
#include "hasp_spiffs.h"

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h>
#endif

void spiffsInfo()
{ // Get all information of your SPIFFS
#if 0
    FSInfo fs_info;
    SPIFFS.info(fs_info);

    Serial.println("File sistem info.");

    Serial.print("Total space:      ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");

    Serial.print("Total space used: ");
    Serial.print(fs_info.usedBytes);
    Serial.println("byte");

    Serial.print("Block size:       ");
    Serial.print(fs_info.blockSize);
    Serial.println("byte");

    Serial.print("Page size:        ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");

    Serial.print("Max open files:   ");
    Serial.println(fs_info.maxOpenFiles);

    Serial.print("Max path lenght:  ");
    Serial.println(fs_info.maxPathLength);
    Serial.println("File sistem info.");

    Serial.print("Total space:      ");
    Serial.print(SPIFFS.totalBytes());
    Serial.println("byte");

    Serial.print("Total space used: ");
    Serial.print(SPIFFS.usedBytes());
    Serial.println("byte");

    Serial.print("Block size:       ");
    // Serial.print(SPIFFS);
    Serial.println("byte");

    Serial.print("Page size:        ");
    Serial.print(SPIFFS.totalBytes());
    Serial.println("byte");

    Serial.print("Max open files:   ");
    // Serial.println(SPIFFS.maxOpenFiles());

    Serial.print("Max path lenght:  ");
    // Serial.println(SPIFFS.maxPathLength());
#endif
}

void spiffsList()
{
    Log.verbose(F("FILE: Listing files on the internal flash:"));

#if defined(ARDUINO_ARCH_ESP32)
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
        Log.verbose(F("FILE:    * %s  (%u bytes)"), file.name(), (uint32_t)file.size());
        file = root.openNextFile();
    }
#endif
#if defined(ARDUINO_ARCH_ESP8266)
    Dir dir = SPIFFS.openDir("/");
    while(dir.next()) {
        Log.notice(F("FILE:    * %s  (%u bytes)"), dir.fileName().c_str(), (uint32_t)dir.fileSize());
    }
#endif
}

void spiffsSetup()
{
    // no SPIFFS settings, as settings depend on SPIFFS

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP8266)
    if(!SPIFFS.begin()) {
#else
    if(!SPIFFS.begin(true)) {
#endif
        Log.error(F("FILE: SPI flash init failed. Unable to mount FS."));
    } else {
        Log.verbose(F("FILE: SPI Flash FS mounted"));
    }
#endif
}

String spiffsFormatBytes(size_t bytes)
{
    String output((char *)0);
    output.reserve(128);

    if(bytes < 1024) {
        output += bytes;
    } else if(bytes < (1024 * 1024)) {
        output += bytes / 1024.0;
        output += "K";
    } else if(bytes < (1024 * 1024 * 1024)) {
        output += bytes / 1024.0 / 1024.0;
        output += "M";
    } else {
        output += bytes / 1024.0 / 1024.0 / 1024.0;
        output += "G";
    }
    output += "B";
    return output;
}