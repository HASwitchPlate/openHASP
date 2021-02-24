/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
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

#include "hasp/hasp.h"
#include "sys/svc/hasp_mdns.h"

#if HASP_USE_ETHERNET > 0 || HASP_USE_WIFI > 0
void networkStart(void)
{
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    configTzTime(MYTZ, "pool.ntp.org", "time.nist.gov", NULL); // literal string
#endif

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

#if HASP_USE_TASMOTA_CLIENT > 0
    tasmotaclientLoop();
#endif // HASP_USE_TASMOTA_CLIENT

#if HASP_USE_HTTP > 0
    httpLoop();
#endif // HTTP

#if HASP_USE_GPIO > 0
    gpioLoop();
#endif // GPIO

#if HASP_USE_OTA > 0
    otaLoop();
#endif // OTA

#if HASP_USE_MDNS > 0
    mdnsLoop();
#endif // MDNS

#if HASP_USE_TELNET > 0
    telnetLoop(); // Console
#endif            // TELNET
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
    bool connected = false;

#if HASP_USE_ETHERNET > 0
    connected |= ethernetEverySecond();
#endif

#if HASP_USE_WIFI > 0
    // connected |= wifiEverySecond();
#endif

#if HASP_USE_OTA > 0
    otaEverySecond(); // progressbar
#endif

    return true;
}

void network_get_statusupdate(char* buffer, size_t len)
{
#if HASP_USE_ETHERNET > 0
    ethernet_get_statusupdate(buffer, len);
#endif

#if HASP_USE_WIFI > 0
    wifi_get_statusupdate(buffer, len);
#endif
}

#endif