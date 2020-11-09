#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "hasp_conf.h"
#include "hasp_hal.h"

#if HASP_USE_ETHERNET > 0

IPAddress ip;

void EthernetEvent(WiFiEvent_t event)
{
    switch(event) {
        case SYSTEM_EVENT_ETH_START:
            Log.notice(TAG_ETH, F("Started"));
            // set eth hostname here
            ETH.setHostname("esp32-ethernet");
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            Log.notice(TAG_ETH, F("Connected"));
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            Log.notice(TAG_ETH, F("MAC Address %s"), ETH.macAddress().c_str());
            ip = ETH.localIP();
            Log.notice(TAG_ETH, F("IPv4: %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            if(ETH.fullDuplex()) {
                Log.notice(TAG_ETH, F("FULL_DUPLEX"));
            }
            Log.notice(TAG_ETH, F("LINK_SPEED %d Mbps"), ETH.linkSpeed());
            eth_connected = true;
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            Log.notice(TAG_ETH, F("Disconnected"));
            eth_connected = false;
            break;
        case SYSTEM_EVENT_ETH_STOP:
            Log.notice(TAG_ETH, F("Stopped"));
            eth_connected = false;
            break;
        default:
            break;
    }
}

void ethernetSetup()
{
    WiFi.onEvent(EthernetEvent);
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLKMODE);
}

void ethernetLoop(void)
{
    //
}

bool ethernetEvery5Seconds()
{
    Log.warning(TAG_ETH, eth_connected ? F("ONLINE") : F("OFFLINE"));
    return eth_connected;
}

#endif