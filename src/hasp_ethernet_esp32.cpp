#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "hasp_conf.h"
#include "hasp_hal.h"

#if HASP_USE_ETHERNET > 0

IPAddress ip;

void EthernetEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
        Log.notice(F(LOG_ETH_CTR "Started"));
        //set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Log.notice(F(LOG_ETH_CTR "Connected"));
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Log.notice(F(LOG_ETH_CTR "MAC Address %s"), ETH.macAddress().c_str());
        ip = ETH.localIP();
        Log.notice(F(LOG_ETH_CTR "IPv4: %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
        if (ETH.fullDuplex()) {
            Log.notice(F(LOG_ETH_CTR "FULL_DUPLEX"));
        }
        Log.notice(F(LOG_ETH_CTR "LINK_SPEED %d Mbps"), ETH.linkSpeed());
        eth_connected = true;
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Log.notice(F(LOG_ETH_CTR "Disconnected"));
        eth_connected = false;
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Log.notice(F(LOG_ETH_CTR "Stopped"));
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
    Log.warning(F(LOG_ETH_CTR "%s"), eth_connected ? F("ONLINE") : F("OFFLINE"));
    return eth_connected;
}


#endif