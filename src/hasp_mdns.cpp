#include "Arduino.h"
#include "ArduinoJson.h"

#ifdef ESP32
#include <ESPmDNS.h>
#else
#include <ESP8266mDNS.h>
MDNSResponder::hMDNSService hMDNSService;
#endif

#include "hasp_mdns.h"

const char F_CONFIG_ENABLE[] PROGMEM = "enable";

bool mdnsEnabled        = true;
String hasp2Node        = "plate01";
const float haspVersion = 0.38;

void mdnsSetup(const JsonObject & settings)
{
    if(mdnsEnabled) {
        // Setup mDNS service discovery if enabled
        //     MDNS.addService(String(hasp2Node), String("tcp"), 80);
        /*if(debugTelnetEnabled) {
            MDNS.addService(haspNode, "telnet", "tcp", 23);
        }*/
        //      MDNS.addServiceTxt(hasp2Node, "tcp", "app_name", "HASwitchPlate");
        //      MDNS.addServiceTxt(hasp2Node, "tcp", "app_version", String(haspVersion).c_str());
        MDNS.begin(hasp2Node.c_str());
    }
}

void mdnsLoop(bool wifiIsConnected)
{
    // if(mdnsEnabled) {
    //     MDNS();
    // }s
}

void mdnsStop()
{
    MDNS.end();
}

bool mdnsGetConfig(const JsonObject & settings)
{
    if(!settings.isNull() && settings[F_CONFIG_ENABLE] == mdnsEnabled) return false;

    settings[F_CONFIG_ENABLE] = mdnsEnabled;

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}