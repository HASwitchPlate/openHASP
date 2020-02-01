#include <Arduino.h>
#include "ArduinoJson.h"

#include "hasp_conf.h"
#include "hasp_log.h"
#include "hasp_spiffs.h"

#if LV_USE_HASP_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#endif

void spiffsList()
{
#if defined(ARDUINO_ARCH_ESP32)
    debugPrintln(PSTR("FILE: Listing files on the internal flash:"));
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
        char msg[64];
        sprintf(msg, PSTR("FILE:    * %s  (%u bytes)"), file.name(), (uint32_t)file.size());
        debugPrintln(msg);
        file = root.openNextFile();
    }
#endif
#if defined(ARDUINO_ARCH_ESP8266)
    debugPrintln(PSTR("FILE: Listing files on the internal flash:"));
    Dir dir = SPIFFS.openDir("/");
    while(dir.next()) {
        char msg[64];
        sprintf(msg, PSTR("FILE:    * %s  (%u bytes)"), dir.fileName().c_str(), (uint32_t)dir.fileSize());
        debugPrintln(msg);
    }
#endif
}

void spiffsSetup()
{
    // no SPIFFS settings, as settings depend on SPIFFS

#if LV_USE_HASP_SPIFFS
    char msg[64];
#if defined(ARDUINO_ARCH_ESP8266)
    if(!SPIFFS.begin()) {
#else
    if(!SPIFFS.begin(true)) {
#endif
        sprintf(msg, PSTR("FILE: %sSPI flash init failed. Unable to mount FS."));
        errorPrintln(msg);
    } else {
        sprintf(msg, PSTR("FILE: [SUCCESS] SPI flash FS mounted"));
        debugPrintln(msg);
        // spiffsList(); // Wait on debugSetup()
    }
#endif
}

void spiffsLoop()
{}