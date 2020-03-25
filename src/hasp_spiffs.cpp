#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_conf.h"
//#include "hasp_log.h"
#include "hasp_spiffs.h"

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h>
#endif

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
        Log.notice(F("FILE: SPI Flash FS mounted"));
    }
#endif
}

void spiffsLoop()
{}

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