#include "Arduino.h"
#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#else
#include <ESP8266mDNS.h>
// MDNSResponder::hMDNSService hMDNSService;
#endif

#include "hasp_conf.h"

#include "hasp_log.h"
#include "hasp_mdns.h"
#include "hasp_mqtt.h"
#include "hasp_config.h"

uint8_t mdnsEnabled     = true;
const float haspVersion = 0.38;

void mdnsSetup(const JsonObject & settings)
{
    mdnsSetConfig(settings);
    debugPrintln(F("MDNS: Setup Complete"));
}

void mdnsStart()
{
    if(mdnsEnabled) {
        String hasp2Node = mqttGetNodename();
        // Setup mDNS service discovery if enabled
        /*if(debugTelnetEnabled) {
        }
        return;
        char buffer[128];
        snprintf_P(buffer, sizeof(buffer), PSTR("%u.%u.%u"), HASP_VERSION_MAJOR, HASP_VERSION_MINOR,
                   HASP_VERSION_REVISION);
        MDNS.addServiceTxt(hasp2Node, "tcp", "app_version", buffer); */
        if(MDNS.begin(hasp2Node.c_str())) {
            debugPrintln(F("MDNS: Responder started"));
            MDNS.addService(F("http"), F("tcp"), 80);
            MDNS.addService(F("telnet"), F("tcp"), 23);
            MDNS.addServiceTxt(hasp2Node, F("tcp"), F("app_name"), F("HASP-lvgl"));
            /*
                        addService("arduino", "tcp", port);
                        addServiceTxt("arduino", "tcp", "tcp_check", "no");
                        addServiceTxt("arduino", "tcp", "ssh_upload", "no");
                        addServiceTxt("arduino", "tcp", "board", ARDUINO_BOARD);
                        addServiceTxt("arduino", "tcp", "auth_upload", (auth) ? "yes" : "no");
                        */
        } else {
            errorPrintln(String(F("MDNS: %sResponder failed to start ")) + hasp2Node);
        };
    }
}

void mdnsLoop()
{
#if defined(ARDUINO_ARCH_ESP8266)
    if(mdnsEnabled) {
        MDNS.update();
    }
#endif
}

void mdnsStop()
{
    MDNS.end();
}

bool mdnsGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_ENABLE)] = mdnsEnabled;

    configOutput(settings);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool mdnsSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    changed |= configSet(mdnsEnabled, settings[FPSTR(F_CONFIG_ENABLE)], PSTR("mdnsEnabled"));

    return changed;
}