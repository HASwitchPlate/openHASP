/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"
#include "hasp_debug.h"
#include "hasp_network.h"

#include "hal/hasp_hal.h"
#include "dev/device.h"

#if HASP_USE_ETHERNET > 0 && defined(ARDUINO_ARCH_ESP32)

IPAddress ip;

void EthernetEvent(WiFiEvent_t event)
{
    switch(event) {
        case SYSTEM_EVENT_ETH_START:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_STARTED));
            // set eth hostname here
            ETH.setHostname(haspDevice.get_hostname());
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_CONNECTED));
            eth_connected = true;
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            LOG_TRACE(TAG_ETH, F(D_INFO_MAC_ADDRESS " %s"), ETH.macAddress().c_str());
            ip = ETH.localIP();
            LOG_TRACE(TAG_ETH, F("IPv4: %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            if(ETH.fullDuplex()) {
                LOG_TRACE(TAG_ETH, F(D_INFO_FULL_DUPLEX));
            }
            LOG_TRACE(TAG_ETH, F(D_INFO_LINK_SPEED " %d Mbps"), ETH.linkSpeed());
            eth_connected = true;
            networkStart(); // Start network services
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_DISCONNECTED));
            eth_connected = false;
            networkStop(); // Stop network services
            break;
        case SYSTEM_EVENT_ETH_STOP:
            LOG_WARNING(TAG_ETH, F(D_SERVICE_STOPPED));
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

IRAM_ATTR void ethernetLoop(void)
{}

bool ethernetEvery5Seconds()
{
    // LOG_WARNING(TAG_ETH, eth_connected ? F(D_NETWORK_ONLINE) : F(D_NETWORK_OFFLINE));
    return eth_connected;
}

void ethernet_get_statusupdate(char* buffer, size_t len)
{
    snprintf_P(buffer, len, PSTR("\"eth\":\"%s\",\"link\":\"%d Mbps\",\"ip\":\"%s\",\"mac\":\"%s\","),
               eth_connected ? F("on") : F("off"), ETH.linkSpeed(), ETH.localIP().toString().c_str(),
               ETH.macAddress().c_str());
}

void ethernet_get_info(JsonDocument& doc)
{
    char size_buf[32];
    String buffer((char*)0);
    buffer.reserve(64);

    JsonObject info = doc.createNestedObject(F(D_INFO_ETHERNET));

    buffer = ETH.linkSpeed();
    buffer += F(" Mbps");
    if(ETH.fullDuplex()) {
        buffer += F(" " D_INFO_FULL_DUPLEX);
    }

    info[F(D_INFO_LINK_SPEED)]  = buffer;
    info[F(D_INFO_IP_ADDRESS)]  = ETH.localIP().toString();
    info[F(D_INFO_GATEWAY)]     = ETH.gatewayIP().toString();
    info[F(D_INFO_DNS_SERVER)]  = ETH.dnsIP().toString();
    info[F(D_INFO_MAC_ADDRESS)] = ETH.macAddress();
}

#endif