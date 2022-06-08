/* MIT License - Copyright (c) 2022 Ben Suffolk, ben@vanilla.net
   For full license information read the LICENSE file in the project folder */


#include "hasp_conf.h"
#include "hasp_debug.h"
#include "hasp_network.h"

#include "hal/hasp_hal.h"
#include "dev/device.h"

#if HASP_USE_ETHERNET > 0 && defined(ARDUINO_ARCH_ESP32) && HASP_USE_SPI_ETHERNET > 0

static bool eth_connected = false;

void EthernetEvent(WiFiEvent_t event)
{
   IPAddress ip;
   switch(event) {
        case ARDUINO_EVENT_ETH_START:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_STARTED));
            // set eth hostname here
            ETHSPI.setHostname(haspDevice.get_hostname());
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_CONNECTED));
            eth_connected = true;
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            LOG_TRACE(TAG_ETH, F(D_INFO_MAC_ADDRESS " %s"), ETHSPI.macAddress().c_str());
            ip = ETHSPI.localIP();
            LOG_TRACE(TAG_ETH, F("IPv4: %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            if(ETHSPI.fullDuplex()) {
                LOG_TRACE(TAG_ETH, F(D_INFO_FULL_DUPLEX));
            }
            LOG_TRACE(TAG_ETH, F(D_INFO_LINK_SPEED " %d Mbps"), ETHSPI.linkSpeed());
            eth_connected = true;
            networkStart(); // Start network services
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_DISCONNECTED));
            eth_connected = false;
            networkStop(); // Stop network services
            break;
        case ARDUINO_EVENT_ETH_STOP:
            LOG_WARNING(TAG_ETH, F(D_SERVICE_STOPPED));
            eth_connected = false;
            break;
        default:
            break;
    }
}

void ethernetSetup()
{
#if HASP_USE_WIFI == 0
  // Need to make sure we get the Ethernet Events
  WiFi.begin();
  WiFi.mode(WIFI_OFF);
#endif

  WiFi.onEvent(EthernetEvent);
  ETHSPI.begin();
}

IRAM_ATTR void ethernetLoop(void)
{}

bool ethernetEvery5Seconds()
{
    return eth_connected;
}

void ethernet_get_statusupdate(char* buffer, size_t len)
{
    snprintf_P(buffer, len, PSTR("\"eth\":\"%s\",\"link\":\"%d Mbps\",\"ip\":\"%s\",\"mac\":\"%s\","),
               eth_connected ? F("on") : F("off"), ETHSPI.linkSpeed(), ETHSPI.localIP().toString().c_str(),
               ETHSPI.macAddress().c_str());
}

void ethernet_get_info(JsonDocument& doc)
{
    char size_buf[32];
    String buffer((char*)0);
    buffer.reserve(64);

    JsonObject info = doc.createNestedObject(F(D_INFO_ETHERNET));

    buffer = ETHSPI.linkSpeed();
    buffer += F(" Mbps");
    if(ETHSPI.fullDuplex()) {
        buffer += F(" " D_INFO_FULL_DUPLEX);
    }

    info[F(D_INFO_LINK_SPEED)]  = buffer;
    info[F(D_INFO_IP_ADDRESS)]  = ETHSPI.localIP().toString();
    info[F(D_INFO_GATEWAY)]     = ETHSPI.gatewayIP().toString();
    info[F(D_INFO_DNS_SERVER)]  = ETHSPI.dnsIP().toString();
    info[F(D_INFO_MAC_ADDRESS)] = ETHSPI.macAddress();
}

#endif

