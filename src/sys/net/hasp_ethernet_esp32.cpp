/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"
#include "hasp_debug.h"
#include "hasp_network.h"

#include "hal/hasp_hal.h"
#include "dev/device.h"

#if HASP_USE_ETHERNET > 0 && defined(ARDUINO_ARCH_ESP32) && defined(HASP_ETHERNET)

bool eth_connected = false;
IPAddress ip;

void ethernet_get_ipaddress(char* buffer, size_t len)
{
    IPAddress ip = HASP_ETHERNET.localIP();
    snprintf_P(buffer, len, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
}

void EthernetEvent(WiFiEvent_t event)
{
    switch(event) {
        case ARDUINO_EVENT_ETH_START:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_STARTED));
            // set eth hostname here
            HASP_ETHERNET.setHostname(haspDevice.get_hostname());
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_CONNECTED));
            eth_connected = true;
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            LOG_TRACE(TAG_ETH, F(D_INFO_MAC_ADDRESS " %s"), HASP_ETHERNET.macAddress().c_str());
            ip = HASP_ETHERNET.localIP();
            LOG_TRACE(TAG_ETH, F("IPv4: %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            if(HASP_ETHERNET.fullDuplex()) {
                LOG_TRACE(TAG_ETH, F(D_INFO_FULL_DUPLEX));
            }
            LOG_TRACE(TAG_ETH, F(D_INFO_LINK_SPEED " %d Mbps"), HASP_ETHERNET.linkSpeed());
            eth_connected = true;
            network_connected(); // Start network services
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            LOG_TRACE(TAG_ETH, F(D_SERVICE_DISCONNECTED));
            eth_connected = false;
            network_disconnected(); // Stop network services
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

    bool started = false;
    WiFi.onEvent(EthernetEvent);
#if HASP_USE_ETHSPI > 0
    started = HASP_ETHERNET.begin(ETHSPI_MOSI_GPIO, ETHSPI_MISO_GPIO, ETHSPI_SCLK_GPIO, ETHSPI_CS_GPIO, ETHSPI_INT_GPIO,
                                  ETHSPI_HOST);
#else
    started = HASP_ETHERNET.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLKMODE);
#endif

    if(started) LOG_TRACE(TAG_ETH, F("ETHSPI Started "));
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
               eth_connected ? F("on") : F("off"), HASP_ETHERNET.linkSpeed(),
               HASP_ETHERNET.localIP().toString().c_str(), HASP_ETHERNET.macAddress().c_str());
}

void ethernet_get_info(JsonDocument& doc)
{
    char size_buf[32];
    String buffer((char*)0);
    buffer.reserve(64);

    JsonObject info = doc.createNestedObject(F(D_INFO_ETHERNET));

    buffer = HASP_ETHERNET.linkSpeed();
    buffer += F(" Mbps");
    if(HASP_ETHERNET.fullDuplex()) {
        buffer += F(" " D_INFO_FULL_DUPLEX);
    }

    info[F(D_INFO_LINK_SPEED)]  = buffer;
    info[F(D_INFO_IP_ADDRESS)]  = HASP_ETHERNET.localIP().toString();
    info[F(D_INFO_GATEWAY)]     = HASP_ETHERNET.gatewayIP().toString();
    info[F(D_INFO_DNS_SERVER)]  = HASP_ETHERNET.dnsIP().toString();
    info[F(D_INFO_MAC_ADDRESS)] = HASP_ETHERNET.macAddress();
}

#endif
