#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "hasp_conf.h"

#if HASP_USE_ETHERNET > 0

#if defined(W5500_MOSI) && defined(W5500_MISO) && defined(W5500_SCLK)
#define W5500_LAN
#include <SPI.h>
#include <Ethernet3.h>
#else
#include <LwIP.h>
#include <STM32Ethernet.h>
#include <EthernetUdp.h>
#endif

EthernetClient EthClient;
IPAddress ip;

void ethernetSetup()
{
#ifdef W5500_LAN
    byte mac[6];
    uint32_t baseUID = (uint32_t)UID_BASE;
    mac[0]           = 0x00;
    mac[1]           = 0x80;
    mac[2]           = 0xE1;
    mac[3]           = (baseUID & 0x00FF0000) >> 16;
    mac[4]           = (baseUID & 0x0000FF00) >> 8;
    mac[5]           = (baseUID & 0x000000FF);

    Ethernet.setCsPin(W5500_CS);
    Ethernet.setRstPin(W5500_RST);
    Ethernet.setHostname("HASP");
    Log.notice(F("ETH: Begin Ethernet W5500"));
    if(Ethernet.begin(mac) == 0) {
        Log.notice(F("ETH: Failed to configure Ethernet using DHCP"));
    } else {
        ip = Ethernet.localIP();
        Log.notice(F("ETH: DHCP Success got IP=%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    }

#else
    // start Ethernet and UDP
    Log.notice(F("ETH: Begin Ethernet LAN8720"));
    if(Ethernet.begin() == 0) {
        Log.notice(F("ETH: Failed to configure Ethernet using DHCP"));
    } else {
        ip = Ethernet.localIP();
        Log.notice(F("ETH: DHCP Success got IP=%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    }

    uint8_t * mac;
    mac = Ethernet.MACAddress();
    Log.notice(F("ETH: MAC Address %x:%x:%x:%x:%x:%x"), *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4),
               *(mac + 5));
#endif
}

void ethernetLoop(void)
{
    switch(Ethernet.maintain()) {
        case 1:
            // renewed fail
            Log.notice(F("ETH: Error: renewed fail"));
            break;

        case 2:
            // renewed success
            ip = Ethernet.localIP();
            Log.notice(F("ETH: DHCP Renew Success got IP=%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            break;

        case 3:
            // rebind fail
            Log.notice(F("Error: rebind fail"));
            break;

        case 4:
            // rebind success
            ip = Ethernet.localIP();
            Log.notice(F("ETH: DHCP Rebind Success got IP=%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
            break;

        default:
            // nothing happened
            break;
    }
}

#endif