/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <Arduino.h>
#include "ArduinoLog.h"

#include "hasp_conf.h"
#include "hasp_hal.h"
#include "hasp_debug.h"
#include "hasp.h"

#if HASP_USE_ETHERNET > 0 || HASP_USE_WIFI > 0

void networkStart(void)
{
    haspProgressVal(255); // hide

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
    mqttStop();
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
    ethernetSetup();
#endif

#if HASP_USE_WIFI > 0
    wifiSetup();
#endif
}

bool networkEvery5Seconds(void)
{
#if HASP_USE_ETHERNET > 0
    ethernetEvery5Seconds();
#endif

#if HASP_USE_WIFI > 0
    wifiEvery5Seconds();
#endif
}

bool networkEverySecond(void)
{
#if HASP_USE_ETHERNET > 0
    // ethernetEverySecond();
#endif

#if HASP_USE_WIFI > 0
    // return wifiEverySecond();
#endif
}

#endif