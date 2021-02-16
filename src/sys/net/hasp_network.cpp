/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <time.h>
#include <sys/time.h>
#define MYTZ "EST5EDT,M3.2.0/2,M11.1.0"

#include <Arduino.h>
#include "ArduinoLog.h"

#include "hasp_conf.h"
#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_network.h"

#include "../../hasp/hasp.h"
#include "../../svc/hasp_mdns.h"

#if HASP_USE_ETHERNET > 0 || HASP_USE_WIFI > 0
void networkStart(void)
{
    haspProgressVal(255);                                      // hide
    configTzTime(MYTZ, "pool.ntp.org", "time.nist.gov", NULL); // literal string

    haspReconnect();
    debugStartSyslog();
    // mqttStart();
    httpStart();
    mdnsStart();
}

void networkStop(void)
{
    haspProgressMsg(F("Network Disconnected"));

    debugStopSyslog();
    // mqttStop();
    httpStop();
    mdnsStop();
}

void networkSetup()
{
    #if HASP_USE_ETHERNET > 0
    ethernetSetup();
    #endif

    #if HASP_USE_WIFI > 0
    wifiSetup();
    #endif
}

void networkLoop(void)
{
    #if HASP_USE_ETHERNET > 0
    ethernetLoop();
    #endif

    #if HASP_USE_WIFI > 0
        // wifiLoop();
    #endif
}

bool networkEvery5Seconds(void)
{
    #if HASP_USE_ETHERNET > 0
    return ethernetEvery5Seconds();
    #endif

    #if HASP_USE_WIFI > 0
    return wifiEvery5Seconds();
    #endif

    return false;
}

bool networkEverySecond(void)
{
    #if HASP_USE_ETHERNET > 0
        // return ethernetEverySecond();
    #endif

    #if HASP_USE_WIFI > 0
        // return wifiEverySecond();
    #endif
    return true;
}

void network_get_statusupdate(char * buffer, size_t len)
{
    #if HASP_USE_ETHERNET > 0
    ethernet_get_statusupdate(buffer, len);
    #endif

    #if HASP_USE_WIFI > 0
    wifi_get_statusupdate(buffer, len);
    #endif
}

#endif