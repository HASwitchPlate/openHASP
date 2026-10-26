/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"

#include "hasp_debug.h"
#include "hal/hasp_hal.h"

#if HASP_USE_ETHERNET > 0 && (defined(STM32F4xx) || defined(STM32F7xx))

EthernetClient EthClient;
IPAddress ip;

void ethernetSetup()
{
#if USE_BUILTIN_ETHERNET > 0
    // start Ethernet and UDP
    LOG_TRACE(TAG_ETH, F("LAN8720 " D_SERVICE_STARTING));
    if(Ethernet.begin() == 0) {
        LOG_TRACE(TAG_ETH, F("Failed to configure Ethernet using DHCP"));
        eth_connected = false;
    } else {
        ip = Ethernet.localIP();
        LOG_TRACE(TAG_ETH, F("DHCP Success got IP %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
        eth_connected = true;
    }

    LOG_TRACE(TAG_ETH, F("MAC Address %s"), Ethernet.macAddress().c_str());

#else
    byte mac[6];
    uint32_t baseUID = (uint32_t)UID_BASE;
    mac[0]           = 0x00;
    mac[1]           = 0x80;
    mac[2]           = 0xE1;
    mac[3]           = (baseUID & 0x00FF0000) >> 16;
    mac[4]           = (baseUID & 0x0000FF00) >> 8;
    mac[5]           = (baseUID & 0x000000FF);

    char ethHostname[12];
    memset(ethHostname, 0, sizeof(ethHostname));
    snprintf_P(ethHostname, sizeof(ethHostname), PSTR("HASP-%02x%02x%02x"), mac[3], mac[4], mac[5]);

    Ethernet.setCsPin(W5500_CS);
    Ethernet.setRstPin(W5500_RST);
    Ethernet.setHostname(ethHostname);
    LOG_TRACE(TAG_ETH, F("W5500 " D_SERVICE_STARTING));
    if(Ethernet.begin(mac) == 0) {
        LOG_TRACE(TAG_ETH, F("Failed to configure Ethernet using DHCP"));
    } else {
        ip = Ethernet.localIP();
        LOG_TRACE(TAG_ETH, F("DHCP Success got IP %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    }
#endif
}

void ethernetLoop(void)
{
    switch(Ethernet.maintain()) {
        case 1:
            // renewed fail
            LOG_ERROR(TAG_ETH, F("Error: renewed fail"));
            break;

        case 2:
            // renewed success
            ip = Ethernet.localIP();
            LOG_TRACE(TAG_ETH, F("DHCP Renew Success got IP=%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            break;

        case 3:
            // rebind fail
            LOG_ERROR(TAG_ETH, F("Error: rebind fail"));
            break;

        case 4:
            // rebind success
            ip = Ethernet.localIP();
            LOG_TRACE(TAG_ETH, F("DHCP Rebind Success got IP=%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            break;

        default:
            // nothing happened
            break;
    }
}

bool ethernetEvery5Seconds()
{
    bool state;
#if USE_BUILTIN_ETHERNET > 0
    state = Ethernet.linkStatus() == LinkON;
#else
    state      = Ethernet.link() == 1;
#endif
    LOG_WARNING(TAG_ETH, state ? F(D_NETWORK_ONLINE) : F(D_NETWORK_OFFLINE));
    return state;
}

void ethernet_get_statusupdate(char* buffer, size_t len)
{
#if USE_BUILTIN_ETHERNET > 0
    bool state = Ethernet.linkStatus() == LinkON;
#else
    bool state = Ethernet.link() == 1;
#endif

    IPAddress ip = Ethernet.localIP();
    snprintf_P(buffer, len, PSTR("\"eth\":\"%s\",\"link\":%d,\"ip\":\"%d.%d.%d.%d\","), state ? F("on") : F("off"), 10,
               ip[0], ip[1], ip[2], ip[3]);
}

void ethernet_get_info(JsonDocument& doc)
{
    char size_buf[32];
    String buffer((char*)0);
    buffer.reserve(64);

    JsonObject info = doc.createNestedObject(F(D_INFO_ETHERNET));

    // buffer = Ethernet.linkSpeed();
    // buffer += F(" Mbps");
    // if(Ethernet.fullDuplex()) {
    //     buffer += F(" " D_INFO_FULL_DUPLEX);
    // }

    // info[F(D_INFO_LINK_SPEED)] = buffer;

    IPAddress ip = Ethernet.localIP();
    snprintf_P(size_buf, sizeof(size_buf), PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    info[F(D_INFO_IP_ADDRESS)] = size_buf;
    // info[F(D_INFO_GATEWAY)]     = Ethernet.gatewayIP().toString();
    // info[F(D_INFO_DNS_SERVER)]  = Ethernet.dnsIP().toString();
    // info[F(D_INFO_MAC_ADDRESS)] = Ethernet.macAddress();
}

#endif