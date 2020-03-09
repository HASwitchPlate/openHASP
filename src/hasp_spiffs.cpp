#include <Arduino.h>
#include "ArduinoJson.h"

#include "hasp_conf.h"
#include "hasp_log.h"
#include "hasp_spiffs.h"

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h>
#endif

void spiffsList()
{
    char buffer[128];
    debugPrintln(PSTR("FILE: Listing files on the internal flash:"));

#if defined(ARDUINO_ARCH_ESP32)
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
        snprintf(buffer, sizeof(buffer), PSTR("FILE:    * %s  (%u bytes)"), file.name(), (uint32_t)file.size());
        debugPrintln(buffer);
        file = root.openNextFile();
    }
#endif
#if defined(ARDUINO_ARCH_ESP8266)
    Dir dir = SPIFFS.openDir("/");
    while(dir.next()) {
        snprintf(buffer, sizeof(buffer), PSTR("FILE:    * %s  (%u bytes)"), dir.fileName().c_str(),
                 (uint32_t)dir.fileSize());
        debugPrintln(buffer);
    }
#endif
}

void spiffsSetup()
{
    // no SPIFFS settings, as settings depend on SPIFFS

#if HASP_USE_SPIFFS
    char buffer[128];
#if defined(ARDUINO_ARCH_ESP8266)
    if(!SPIFFS.begin()) {
#else
    if(!SPIFFS.begin(true)) {
#endif
        snprintf(buffer, sizeof(buffer), PSTR("FILE: %%sSPI flash init failed. Unable to mount FS."));
        errorPrintln(buffer);
    } else {
        snprintf(buffer, sizeof(buffer), PSTR("FILE: SPI Flash FS mounted"));
        debugPrintln(buffer);
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