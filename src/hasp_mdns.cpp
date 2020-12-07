/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_MDNS > 0

#include "ArduinoJson.h"
#include "ArduinoLog.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266mDNS.h>
// MDNSResponder::hMDNSService hMDNSService;
#endif

#include "hasp_conf.h"

#include "hasp_config.h"
#include "hasp_debug.h"

uint8_t mdnsEnabled = true;

void mdnsSetup()
{
    // mdnsSetConfig(settings);
    // mdnsStart(); // Wifis need to call this at connection time!
}

void mdnsStart()
{
    if(!mdnsEnabled) {
        Log.notice(TAG_MDNS, F("MDNS Responder is disabled"));
        return;
    }

    Log.notice(TAG_MDNS, F("Starting MDNS Responder..."));

#if HASP_USE_MQTT > 0
    String hasp2Node = mqttGetNodename();
#else
    String hasp2Node = "unknown";
#endif

    // Setup mDNS service discovery if enabled
    char buffer[32];
    snprintf_P(buffer, sizeof(buffer), PSTR("%u.%u.%u"), HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);
    uint8_t attempt = 0;

    while(!MDNS.begin(hasp2Node.c_str())) {
        if(attempt++ >= 3) {
            Log.error(TAG_MDNS, F("Responder failed to start %s"), hasp2Node.c_str());
            return;
        }

        // try another hostname
        hasp2Node = mqttGetNodename();
        hasp2Node += F("_");
        hasp2Node += String(attempt);
        Log.verbose(TAG_MDNS, F("Trying hostname %s"), hasp2Node.c_str());
    };

    MDNS.addService(F("http"), F("tcp"), 80);
    MDNS.addServiceTxt(F("http"), F("tcp"), F("app_version"), buffer);
    MDNS.addServiceTxt(F("http"), F("tcp"), F("app_name"), F("HASP-lvgl"));

    // if(debugTelnetEnabled) {
    MDNS.addService(F("telnet"), F("tcp"), 23);
    // }

    Log.trace(TAG_MDNS, F("Responder started"));
}

void IRAM_ATTR mdnsLoop(void)
{
#if defined(ARDUINO_ARCH_ESP8266)
    if(mdnsEnabled) {
        MDNS.update();
    }
#endif
}

void mdnsStop()
{
    return;
#if HASP_USE_MDNS > 0
    MDNS.end();
#endif
}

bool mdnsGetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(mdnsEnabled != settings[FPSTR(F_CONFIG_ENABLE)].as<bool>()) changed = true;
    settings[FPSTR(F_CONFIG_ENABLE)] = mdnsEnabled;

    if(changed) configOutput(settings, TAG_MDNS);
    return changed;
}

/**
 * Reads the settings from json and sets the application variables.
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 * @param[in] settings    JsonObject with the config settings.
 **/
bool mdnsSetConfig(const JsonObject & settings)
{
    configOutput(settings, TAG_MDNS);
    bool changed = false;

    changed |= configSet(mdnsEnabled, settings[FPSTR(F_CONFIG_ENABLE)], F("mdnsEnabled"));

    return changed;
}

#endif // HASP_USE_MDNS
