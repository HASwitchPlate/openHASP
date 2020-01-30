#include "Arduino.h"
#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library

#include "hasp_config.h"
#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_http.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_mdns.h"
#include "hasp_gui.h"
#include "hasp_tft.h"
#include "hasp_ota.h"
#include "hasp.h"

//#define HASP_CONFIG_FILE F("/config.json")
static const char HASP_CONFIG_FILE[] PROGMEM = "/config.json";

void spiffsList()
{
#if defined(ARDUINO_ARCH_ESP32)
    debugPrintln(F("FILE: Listing files on the internal flash:"));
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
        char msg[64];
        sprintf_P(msg, PSTR("FILE:    * %s  (%u bytes)"), file.name(), (uint32_t)file.size());
        debugPrintln(msg);
        file = root.openNextFile();
    }
#endif
#if defined(ARDUINO_ARCH_ESP8266)
    debugPrintln(F("FILE: Listing files on the internal flash:"));
    Dir dir = SPIFFS.openDir("/");
    while(dir.next()) {
        char msg[64];
        sprintf_P(msg, PSTR("FILE:    * %s  (%u bytes)"), dir.fileName().c_str(), (uint32_t)dir.fileSize());
        debugPrintln(msg);
    }
#endif
}

bool configChanged()
{
    return false;
}

void configLoop()
{
    if(configChanged()) {
        // configSetConfig();
    }
}

void configGetConfig(JsonDocument & settings, bool setupdebug = false)
{
    File file = SPIFFS.open(FPSTR(HASP_CONFIG_FILE), "r");

    if(file) {
        size_t size = file.size();
        if(size > 1024) {
            errorPrintln(F("CONF: %sConfig file size is too large"));
            return;
        }

        DeserializationError error = deserializeJson(settings, file);
        if(!error) {
            file.close();

            if(setupdebug) {
                debugSetup(); // Debug started, now we can use it; HASP header sent
                debugPrintln(F("FILE: [SUCCESS] SPI flash FS mounted"));
                spiffsList();
            }
            debugPrintln(String(F("CONF: Loading ")) + String(FPSTR(HASP_CONFIG_FILE)));

            // show settings in log
            String output;
            serializeJson(settings, output);
            debugPrintln(String(F("CONF: ")) + output);

            debugPrintln(String(F("CONF: [SUCCESS] Loaded ")) + String(FPSTR(HASP_CONFIG_FILE)));
            return;
        }
    }

    if(setupdebug) {
        // setup debugging defaults
        debugSetup(); // Debug started, now we can use it; HASP header sent
        debugPrintln(F("FILE: [SUCCESS] SPI flash FS mounted"));
        spiffsList();
    }
    debugPrintln(String(F("CONF: Loading ")) + String(FPSTR(HASP_CONFIG_FILE)));
    errorPrintln(String(F("CONF: %sFailed to load ")) + String(FPSTR(HASP_CONFIG_FILE)));
}

void configWriteConfig()
{
    /* Read Config File */
    DynamicJsonDocument settings(1024);
    debugPrintln(String(F("CONF: Config LOADING first ")) + String(FPSTR(HASP_CONFIG_FILE)));
    configGetConfig(settings, false);
    debugPrintln(String(F("CONF: Config LOADED first ")) + String(FPSTR(HASP_CONFIG_FILE)));

    bool changed = true;
    // changed |= debugGetConfig(settings[F("debug")].to<JsonObject>());
    // changed |= guiGetConfig(settings[F("gui")].to<JsonObject>());
    changed |= haspGetConfig(settings[F("hasp")].to<JsonObject>());
    changed |= httpGetConfig(settings[F("http")].to<JsonObject>());
    // changed |= mdnsGetConfig(settings[F("mdns")].to<JsonObject>());
    changed |= mqttGetConfig(settings[F("mqtt")].to<JsonObject>());
    // changed |= otaGetConfig(settings[F("ota")].to<JsonObject>());
    // changed |= tftGetConfig(settings[F("tft")].to<JsonObject>());
    changed |= wifiGetConfig(settings[F("wifi")].to<JsonObject>());

    if(changed) {
        File file = SPIFFS.open(FPSTR(HASP_CONFIG_FILE), "w");
        if(file) {
            debugPrintln(F("CONF: Writing /config.json"));
            size_t size = serializeJson(settings, file);
            file.close();
            if(size > 0) {
                debugPrintln(F("CONF: [SUCCESS] /config.json saved"));
                return;
            }
        }

        errorPrintln(F("CONF: %sFailed to write /config.json"));
    } else {
        debugPrintln(F("CONF: Configuration was not changed"));
    }
}

void configSetup(JsonDocument & settings)
{
    if(!SPIFFS.begin()) {
        errorPrintln(F("FILE: %sSPI flash init failed. Unable to mount FS."));
    } else {
        configGetConfig(settings, true);
    }
}