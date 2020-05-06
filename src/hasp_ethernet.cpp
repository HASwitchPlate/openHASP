#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "hasp_conf.h"

#if HASP_USE_ETHERNET>0

#include <LwIP.h>
#include <STM32Ethernet.h>
#include <EthernetUdp.h>

EthernetClient EthClient;

void ethernetSetup()
{
    // start Ethernet and UDP
    Log.notice(F("ETH: Begin Ethernet"));

    if (Ethernet.begin() == 0) {
        Log.notice(F("ETH: Failed to configure Ethernet using DHCP"));
    } else {
        IPAddress ip = Ethernet.localIP();
        Log.notice(F("ETH: DHCP Success got IP=%d.%d.%d.%d"),ip[0], ip[1], ip[2], ip[3]);
    }

        uint8_t *mac;
        mac = Ethernet.MACAddress();
        Log.notice(F("ETH: MAC Address %x:%x:%x:%x:%x:%x"),*mac,*(mac+1),*(mac+2),*(mac+3),*(mac+4),*(mac+5));

}

void ethernetLoop(void)
{
     Ethernet.maintain();
}


#endif