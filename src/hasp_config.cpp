#include "Arduino.h"
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include <FS.h> // Include the SPIFFS library

#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif

#include "hasp_config.h"
//#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_http.h"
#include "hasp_wifi.h"
#include "hasp_mdns.h"
#include "hasp_gui.h"
#include "hasp_ota.h"
#include "hasp_spiffs.h"
#include "hasp_telnet.h"
#include "hasp.h"

#include "hasp_conf.h"
#if HASP_USE_MQTT
#include "hasp_mqtt.h"
#endif

void confDebugSet(const char * name)
{
    /*char buffer[128];
    snprintf(buffer, sizeof(buffer), PSTR("CONF:    * %s set"), name);
    debugPrintln(buffer);*/
    Log.trace(F("CONF:    * %s set"), name);
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

void configStartDebug(bool setupdebug, String & configFile)
{
    if(setupdebug) {
        debugStart(); // Debug started, now we can use it; HASP header sent
        Log.notice(F("FILE: [SUCCESS] SPI flash FS mounted"));
        spiffsList();
    }
    Log.notice(F("CONF: Loading %s"), configFile.c_str());
}

void configGetConfig(JsonDocument & settings, bool setupdebug = false)
{
    String configFile((char *)0);
    configFile.reserve(128);
    configFile = String(FPSTR(HASP_CONFIG_FILE));
    File file  = SPIFFS.open(configFile, "r");

    if(file) {
        size_t size = file.size();
        if(size > 1024) {
            Log.error(F("CONF: Config file size is too large"));
            return;
        }

        DeserializationError error = deserializeJson(settings, file);
        if(!error) {
            file.close();

            /* Load Debug params */
            if(setupdebug) {
                debugPreSetup(settings[F("debug")]);
            }
            configStartDebug(setupdebug, configFile);

            // show settings in log
            String output;
            serializeJson(settings, output);
            String passmask = F("********");
            output.replace(settings[F("http")][F("pass")].as<String>(), passmask);
            output.replace(settings[F("mqtt")][F("pass")].as<String>(), passmask);
            output.replace(settings[F("wifi")][F("pass")].as<String>(), passmask);
            Log.verbose(F("CONF: %s"), output.c_str());
            Log.notice(F("CONF: [SUCCESS] Loaded %s"), configFile.c_str());

            if(setupdebug) debugSetup(settings[F("debug")]);
            return;
        }
    }

    configStartDebug(setupdebug, configFile);
    Log.error(F("CONF: Failed to load %s"), configFile.c_str());
}

void configWriteConfig()
{
    String configFile((char *)0);
    configFile.reserve(128);
    configFile = String(FPSTR(HASP_CONFIG_FILE));

    /* Read Config File */
    DynamicJsonDocument settings(1024 * 2);
    Log.notice(F("CONF: Config LOADING first %s"), configFile.c_str());
    configGetConfig(settings, false);
    Log.trace(F("CONF: Config LOADED first %s"), configFile.c_str());

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

    if(changed) {
        File file = SPIFFS.open(configFile, "w");
        if(file) {
            Log.notice(F("CONF: Writing %s"), configFile.c_str());
            size_t size = serializeJson(settings, file);
            file.close();
            if(size > 0) {
                Log.verbose(F("CONF: [SUCCESS] Saved %s"), configFile.c_str());
                return;
            }
        }

        Log.error(F("CONF: Failed to write %s"), configFile.c_str());
    } else {
        Log.verbose(F("CONF: Configuration was not changed"));
    }
}

void configSetup(JsonDocument & settings)
{
    if(!SPIFFS.begin()) {
        Log.error(F("FILE: SPI flash init failed. Unable to mount FS."));
    } else {
        configGetConfig(settings, true);
    }
}

void configOutput(const JsonObject & settings)
{
    String output((char *)0);
    output.reserve(128);
    serializeJson(settings, output);

    String passmask((char *)0);
    passmask.reserve(128);
    passmask = F("\"pass\":\"********\"");

    String password((char *)0);
    password.reserve(128);
    password = F("\"pass\":\"");
    password += settings[F("pass")].as<String>();
    password += F("\"");

    if(password.length() > 2) output.replace(password, passmask);
    Log.trace(F("CONF: %s"), output.c_str());
}