/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"
#if HASP_USE_MDNS > 0

#if defined(ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#include <mdns.h>
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
        LOG_TRACE(TAG_MDNS, F(D_SERVICE_DISABLED));
        return;
    }

    LOG_TRACE(TAG_MDNS, F(D_SERVICE_STARTING));

    // Setup mDNS service discovery if enabled

    /* uint8_t attempt = 0;
    while(!MDNS.begin(hasp2Node.c_str())) {
        if(attempt++ >= 3) {
            LOG_ERROR(TAG_MDNS, F(D_SERVICE_START_FAILED ": %s"), hasp2Node.c_str());
            return;
        }

        // try another hostname
        hasp2Node = mqttGetNodename();
        hasp2Node += F("_");
        hasp2Node += String(attempt);
        LOG_VERBOSE(TAG_MDNS, F("Trying hostname %s"), hasp2Node.c_str());
    };*/

    if(MDNS.begin(haspDevice.get_hostname())) {
        char value[1024]; // 32
        char service[12];
        char key[12];
        char proto[4];
        sprintf_P(proto, PSTR("tcp"));

        // strcpy_P(service, PSTR("http"));
        // MDNS.addService(service, proto, 80);

        // strcpy_P(key, PSTR("app_version"));
        // MDNS.addServiceTxt(service, proto, key, haspDevice.get_version());

        // strcpy_P(key, PSTR("app_name"));
        // strcpy_P(value, PSTR(D_MANUFACTURER));
        // MDNS.addServiceTxt(service, proto, key, value);

        strcpy_P(service, PSTR("openhasp"));
        MDNS.addService(service, proto, 80);

        StaticJsonDocument<1024> doc;
        dispatch_get_discovery_data(doc);

        JsonObject data = doc.as<JsonObject>();
        for(JsonPair i : data) {
            MDNS.addServiceTxt(service, proto, i.key().c_str(), i.value().as<String>());
        }

        // if(debugTelnetEnabled) {
        strcpy_P(service, PSTR("telnet"));
        MDNS.addService(service, proto, 23);
        // }

        LOG_INFO(TAG_MDNS, F(D_SERVICE_STARTED));
    } else {
        LOG_ERROR(TAG_MDNS, F(D_SERVICE_START_FAILED));
    }
}

bool mdns_remove_service(char* service, char* proto)
{
#if ESP32
    return mdns_service_remove("_arduino", "_tcp") == ESP_OK;
#endif

#if ESP8266
    return MDNS.removeService(haspDevice.get_hostname(), "_arduino", "_tcp");
#endif
}

IRAM_ATTR void mdnsLoop(void)
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
bool mdnsGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(mdns_config.enable != settings[FPSTR(FP_CONFIG_ENABLE)].as<bool>()) changed = true;
    settings[FPSTR(FP_CONFIG_ENABLE)] = mdns_config.enable;

    if(changed) configOutput(settings, TAG_MDNS);
    return changed;
}

/**
 * Reads the settings from json and sets the application variables.
 * @note: data pixel should be formatted to uint32_t RGBA. Imagemagick requirements.
 * @param[in] settings    JsonObject with the config settings.
 **/
bool mdnsSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_MDNS);
    bool changed = false;

    changed |= configSet(mdns_config.enable, settings[FPSTR(FP_CONFIG_ENABLE)], F("mdnsEnabled"));

    return changed;
}
#endif // HASP_USE_CONFIG

#endif // HASP_USE_MDNS
