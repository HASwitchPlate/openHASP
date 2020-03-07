#include "Arduino.h"
#include "ArduinoJson.h"
#include <FS.h> // Include the SPIFFS library

#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif

#include "hasp_config.h"
#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_http.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_mdns.h"
#include "hasp_gui.h"
// #include "hasp_tft.h"
#include "hasp_ota.h"
#include "hasp_spiffs.h"
#include "hasp_telnet.h"
#include "hasp.h"

void confDebugSet(const char * name)
{
    char buffer[127];
    snprintf(buffer, sizeof(buffer), PSTR("CONF:    * %s set"), name);
    debugPrintln(buffer);
}

bool configSet(int8_t & value, const JsonVariant & setting, const char * name)
{
    if(!setting.isNull()) {
        int8_t val = setting.as<int8_t>();
        if(value != val) {
            confDebugSet(name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(uint8_t & value, const JsonVariant & setting, const char * name)
{
    if(!setting.isNull()) {
        uint8_t val = setting.as<uint8_t>();
        if(value != val) {
            confDebugSet(name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(uint16_t & value, const JsonVariant & setting, const char * name)
{
    if(!setting.isNull()) {
        uint16_t val = setting.as<uint16_t>();
        if(value != val) {
            confDebugSet(name);
            value = val;
            return true;
        }
    }
    return false;
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

void configStartDebug(bool setupdebug, String & configFile)
{
    if(setupdebug) {
        debugStart(); // Debug started, now we can use it; HASP header sent
        debugPrintln(F("FILE: [SUCCESS] SPI flash FS mounted"));
        spiffsList();
    }
    debugPrintln(String(F("CONF: Loading ")) + configFile);
}

void configGetConfig(JsonDocument & settings, bool setupdebug = false)
{
    String configFile((char *)0);
    configFile.reserve(127);
    configFile = String(FPSTR(HASP_CONFIG_FILE));
    File file  = SPIFFS.open(configFile, "r");

    if(file) {
        size_t size = file.size();
        if(size > 1024) {
            errorPrintln(F("CONF: %sConfig file size is too large"));
            return;
        }

        DeserializationError error = deserializeJson(settings, file);
        if(!error) {
            file.close();

            /* Load Debug params */
            debugPreSetup(settings[F("debug")]);
            configStartDebug(setupdebug, configFile);

            // show settings in log
            String output;
            serializeJson(settings, output);
            String passmask = F("********");
            output.replace(settings[F("http")][F("pass")].as<String>(), passmask);
            output.replace(settings[F("mqtt")][F("pass")].as<String>(), passmask);
            output.replace(settings[F("wifi")][F("pass")].as<String>(), passmask);
            debugPrintln(String(F("CONF: ")) + output);

            debugPrintln(String(F("CONF: [SUCCESS] Loaded ")) + configFile);
            debugSetup(settings[F("debug")]);
            return;
        }
    }

    configStartDebug(setupdebug, configFile);
    errorPrintln(String(F("CONF: %sFailed to load ")) + configFile);
}

void configWriteConfig()
{
    String configFile((char *)0);
    configFile.reserve(127);
    configFile = String(FPSTR(HASP_CONFIG_FILE));

    /* Read Config File */
    DynamicJsonDocument settings(2 * 1024);
    debugPrintln(String(F("CONF: Config LOADING first ")) + configFile);
    configGetConfig(settings, false);
    debugPrintln(String(F("CONF: Config LOADED first ")) + configFile);

    bool changed = true;

#if HASP_USE_WIFI
    changed |= wifiGetConfig(settings[F("wifi")].to<JsonObject>());

#if HASP_USE_MQTT
    changed |= mqttGetConfig(settings[F("mqtt")].to<JsonObject>());
#endif

#if HASP_USE_TELNET
    changed |= telnetGetConfig(settings[F("telnet")].to<JsonObject>());
#endif

#if HASP_USE_MDNS
    changed |= mdnsGetConfig(settings[F("mdns")].to<JsonObject>());
#endif

#if HASP_USE_HTTP
    changed |= httpGetConfig(settings[F("http")].to<JsonObject>());
#endif

#endif

    changed |= debugGetConfig(settings[F("debug")].to<JsonObject>());
    changed |= guiGetConfig(settings[F("gui")].to<JsonObject>());
    changed |= haspGetConfig(settings[F("hasp")].to<JsonObject>());
    // changed |= otaGetConfig(settings[F("ota")].to<JsonObject>());
    // changed |= tftGetConfig(settings[F("tft")].to<JsonObject>());

    if(changed) {
        File file = SPIFFS.open(configFile, "w");
        if(file) {
            debugPrintln(String(F("CONF: Writing ")) + configFile);
            size_t size = serializeJson(settings, file);
            file.close();
            if(size > 0) {
                debugPrintln(String(F("CONF: [SUCCESS] Saved ")) + configFile);
                return;
            }
        }

        errorPrintln(String(F("CONF: %sFailed to write ")) + configFile);
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

void configOutput(const JsonObject & settings)
{
    String output((char *)0);
    output.reserve(127);
    serializeJson(settings, output);

    String passmask((char *)0);
    passmask.reserve(127);
    passmask = F("\"pass\":\"********\"");

    String password((char *)0);
    password.reserve(127);
    password = F("\"pass\":\"");
    password += settings[F("pass")].as<String>();
    password += F("\"");

    if(password.length() > 2) output.replace(password, passmask);
    debugPrintln(String(F("CONF: ")) + output);
}