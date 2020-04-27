#include "Arduino.h"
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_config.h"
#include "hasp_debug.h"
#include "hasp_gui.h"
#include "hasp_ota.h"
#include "hasp_spiffs.h"
#include "hasp_telnet.h"
//#include "hasp_eeprom.h"
#include "hasp.h"

#include "hasp_conf.h"

#if HASP_USE_SPIFFS>0
#include <FS.h> // Include the SPIFFS library
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#endif
#if HASP_USE_EEPROM>0
#include "EEPROM.h"
#endif

#ifndef FPSTR
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
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
#if HASP_USE_SPIFFS>0
        spiffsInfo();
        spiffsList();
#endif
    }
    Log.notice(F("CONF: Loading %s"), configFile.c_str());
}

void configGetConfig(JsonDocument & settings, bool setupdebug = false)
{
    String configFile((char *)0);
    configFile.reserve(128);
    configFile = String(FPSTR(HASP_CONFIG_FILE));

#if HASP_USE_SPIFFS>0
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

            if(setupdebug) debugSetup();
            return;
        }
    }
#endif

    // File does not exist or error reading file
    if(setupdebug) {
        debugPreSetup(settings[F("debug")]);
    }

    configStartDebug(setupdebug, configFile);
    Log.error(F("CONF: Failed to load %s"), configFile.c_str());
}

void configBackupToEeprom()
{
#if HASP_USE_SPIFFS>0
    String configFile((char *)0);
    configFile.reserve(128);
    configFile = String(FPSTR(HASP_CONFIG_FILE));

    EEPROM.begin(1024);
    uint8_t buffer[128];
    size_t index = 0;

    File file = SPIFFS.open(configFile, "r");
    if(file) {

        while(size_t count = file.read(buffer, sizeof(buffer)) > 0) {
            for(size_t i = 0; i < count; i++) {
                EEPROM.write(index, buffer[i]);
                index++;
            }
        }

        file.close();
        EEPROM.commit();

        Log.verbose(F("CONF: Written %u to EEPROM"), index);
    }
#endif
}

void configWriteConfig()
{
#if HASP_USE_SPIFFS>0
    String configFile((char *)0);
    configFile.reserve(128);
    configFile = String(FPSTR(HASP_CONFIG_FILE));

    /* Read Config File */
    DynamicJsonDocument doc(8 * 256);
    Log.notice(F("CONF: Config LOADING first %s"), configFile.c_str());
    configGetConfig(doc, false);
    Log.trace(F("CONF: Config LOADED first %s"), configFile.c_str());

    // Make sure we have a valid JsonObject to start from
    JsonObject settings;
    if(doc.as<JsonObject>().isNull()) {
        settings = doc.to<JsonObject>(); // Settings are invalid, force creation of an empty JsonObject
    } else {
        settings = doc.as<JsonObject>(); // Settings are valid, cast as a JsonObject
    }

    bool writefile = false;
    bool changed   = false;

#if HASP_USE_WIFI
    if(settings[F("wifi")].as<JsonObject>().isNull()) settings.createNestedObject(F("wifi"));
    changed = wifiGetConfig(settings[F("wifi")]);
    if(changed) {
        Log.verbose(F("WIFI: Settings changed"));
        writefile = true;
    }
#if HASP_USE_MQTT
    if(settings[F("mqtt")].as<JsonObject>().isNull()) settings.createNestedObject(F("mqtt"));
    changed = mqttGetConfig(settings[F("mqtt")]);
    if(changed) {
        Log.verbose(F("MQTT: Settings changed"));
        configOutput(settings[F("mqtt")]);
        writefile = true;
    }
#endif
#if HASP_USE_TELNET
    if(settings[F("telnet")].as<JsonObject>().isNull()) settings.createNestedObject(F("telnet"));
    changed = telnetGetConfig(settings[F("telnet")]);
    if(changed) {
        Log.verbose(F("TELNET: Settings changed"));
        configOutput(settings[F("telnet")]);
        writefile = true;
    }
#endif
#if HASP_USE_MDNS
    if(settings[F("mdns")].as<JsonObject>().isNull()) settings.createNestedObject(F("mdns"));
    changed = mdnsGetConfig(settings[F("mdns")]);
    if(changed) {
        Log.verbose(F("MDNS: Settings changed"));
        writefile = true;
    }
#endif
#if HASP_USE_HTTP
    if(settings[F("http")].as<JsonObject>().isNull()) settings.createNestedObject(F("http"));
    changed = httpGetConfig(settings[F("http")]);
    if(changed) {
        Log.verbose(F("HTTP: Settings changed"));
        configOutput(settings[F("http")]);
        writefile = true;
    }
#endif
#endif

    if(settings[F("debug")].as<JsonObject>().isNull()) settings.createNestedObject(F("debug"));
    changed = debugGetConfig(settings[F("debug")]);
    if(changed) {
        Log.verbose(F("DEBUG: Settings changed"));
        writefile = true;
    }

    if(settings[F("gui")].as<JsonObject>().isNull()) settings.createNestedObject(F("gui"));
    changed = guiGetConfig(settings[F("gui")]);
    if(changed) {
        Log.verbose(F("GUI: Settings changed"));
        writefile = true;
    }

    if(settings[F("hasp")].as<JsonObject>().isNull()) settings.createNestedObject(F("hasp"));
    changed = haspGetConfig(settings[F("hasp")]);
    if(changed) {
        Log.verbose(F("HASP: Settings changed"));
        writefile = true;
    }

    // changed |= otaGetConfig(settings[F("ota")].as<JsonObject>());

    if(writefile) {
        File file = SPIFFS.open(configFile, "w");
        if(file) {
            Log.notice(F("CONF: Writing %s"), configFile.c_str());
            size_t size = serializeJson(doc, file);
            file.close();
            if(size > 0) {
                Log.verbose(F("CONF: [SUCCESS] Saved %s"), configFile.c_str());
                configBackupToEeprom();
                return;
            }
        }

        Log.error(F("CONF: Failed to write %s"), configFile.c_str());

    } else {
        Log.notice(F("CONF: Configuration was not changed"));
    }
#endif
}

void configSetup()
{
#if HASP_USE_SPIFFS>0
    if(!SPIFFS.begin()) {
#endif
    
#if HASP_USE_SPIFFS>0
    } else {
#endif
        DynamicJsonDocument settings(1024 + 128);
  Serial.print(__FILE__);
  Serial.println(__LINE__);

        configGetConfig(settings, true);

            Log.error(F("FILE: SPI flash init failed. Unable to mount FS: Using default settings..."));
#if HASP_USE_SPIFFS>0
        Log.verbose(F("Loading debug settings"));
        debugSetConfig(settings[F("debug")]);
        Log.verbose(F("Loading GUI settings"));
        guiSetConfig(settings[F("gui")]);
        Log.verbose(F("Loading HASP settings"));
        haspSetConfig(settings[F("hasp")]);
        // otaGetConfig(settings[F("ota")]);

#if HASP_USE_WIFI
        Log.verbose(F("Loading WiFi settings"));
        wifiSetConfig(settings[F("wifi")]);
#if HASP_USE_MQTT
        Log.verbose(F("Loading MQTT settings"));
        mqttSetConfig(settings[F("mqtt")]);
#endif
#if HASP_USE_TELNET
        Log.verbose(F("Loading Telnet settings"));
        telnetSetConfig(settings[F("telnet")]);
#endif
#if HASP_USE_MDNS
        Log.verbose(F("Loading MDNS settings"));
        mdnsSetConfig(settings[F("mdns")]);
#endif
#if HASP_USE_HTTP
        Log.verbose(F("Loading HTTP settings"));
        httpSetConfig(settings[F("http")]);
#endif
#endif
    }
    Log.notice(F("User configuration loaded"));
#endif
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