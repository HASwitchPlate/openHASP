/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */


#include "hasp_conf.h"
#include "hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_network.h"

#if HASP_USE_ETHERNET > 0 && defined(ARDUINO_ARCH_ESP32)

IPAddress ip;

void EthernetEvent(WiFiEvent_t event)
{
    switch(event) {
        case SYSTEM_EVENT_ETH_START:
            Log.notice(TAG_ETH, F("Started"));
            // set eth hostname here
            ETH.setHostname(mqttGetNodename().c_str());
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            Log.notice(TAG_ETH, F("Connected"));
            eth_connected = true;
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
            networkStart(); // Start network services
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            Log.notice(TAG_ETH, F("Disconnected"));
            eth_connected = false;
            networkStop(); // Stop network services
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

void IRAM_ATTR ethernetLoop(void)
{}

bool ethernetEvery5Seconds()
{
    // Log.warning(TAG_ETH, eth_connected ? F("ONLINE") : F("OFFLINE"));
    return eth_connected;
}

void ethernet_get_statusupdate(char * buffer, size_t len)
{
    snprintf_P(buffer, len, PSTR("\"eth\":\"%s\",\"link\":\"%d Mbps\",\"ip\":\"%s\","), eth_connected ? F("ON") : F("OFF"), ETH.linkSpeed(),
               ETH.localIP().toString().c_str());
}

#endif