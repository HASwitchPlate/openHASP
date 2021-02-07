/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"
#if HASP_USE_MDNS > 0

    #if defined(ARDUINO_ARCH_ESP32)
        #include <ESPmDNS.h>
    #elif defined(ARDUINO_ARCH_ESP8266)
        #include <ESP8266mDNS.h>
    // MDNSResponder::hMDNSService hMDNSService;
    #endif

    #include "hasp/hasp.h"
    #include "hasp_config.h"
    #include "hasp_debug.h"

// uint8_t mdnsEnabled = true;
hasp_mdns_config_t mdns_config;

void mdnsSetup()
{
    // mdnsSetConfig(settings);
    // mdnsStart(); // Wifis need to call this at connection time!
}

void mdnsStart()
{
    if(!mdns_config.enable) {
        Log.notice(TAG_MDNS, F(D_SERVICE_DISABLED));
        return;
    }

    Log.notice(TAG_MDNS, F(D_SERVICE_STARTING));

    #if HASP_USE_MQTT > 0
    String hasp2Node = mqttGetNodename();
    #else
    String hasp2Node = "unknown";
    #endif

    // Setup mDNS service discovery if enabled
    char buffer[32];
    haspGetVersion(buffer, sizeof(buffer));

    uint8_t attempt = 0;
    while(!MDNS.begin(hasp2Node.c_str())) {
        if(attempt++ >= 3) {
            Log.error(TAG_MDNS, F(D_SERVICE_START_FAILED ": %s"), hasp2Node.c_str());
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

    Log.trace(TAG_MDNS, F(D_SERVICE_STARTED));
}

void IRAM_ATTR mdnsLoop(void)
{
    #if defined(ARDUINO_ARCH_ESP8266)
    if(mdns_config.enable) {
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

    #if HASP_USE_CONFIG > 0
bool mdnsGetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(mdns_config.enable != settings[FPSTR(F_CONFIG_ENABLE)].as<bool>()) changed = true;
    settings[FPSTR(F_CONFIG_ENABLE)] = mdns_config.enable;

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

    changed |= configSet(mdns_config.enable, settings[FPSTR(F_CONFIG_ENABLE)], F("mdnsEnabled"));

    return changed;
}
    #endif // HASP_USE_CONFIG

#endif // HASP_USE_MDNS
