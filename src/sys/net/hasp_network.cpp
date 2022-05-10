/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_network.h"
#include "sys/svc/hasp_mdns.h"

#if HASP_USE_ETHERNET > 0 || HASP_USE_WIFI > 0
void networkStart(void)
{
    // haspProgressVal(255); // hide
    haspReconnect();
    debugStartSyslog();
// mqttStart();
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
#if HASP_START_HTTP
    httpStart();
#endif
#endif

#if HASP_USE_MDNS > 0
    mdnsStart();
#endif // HASP_USE_MDNS
}

void networkStop(void)
{
    haspProgressMsg(F("Network Disconnected"));

    debugStopSyslog();
    // mqttStop();
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
    httpStop();
#endif

#if HASP_USE_MDNS > 0
    mdnsStop();
#endif
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

IRAM_ATTR void networkLoop(void)
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

#if HASP_USE_ARDUINOOTA > 0
    otaLoop();
#endif // OTA

#if HASP_USE_MDNS > 0
    mdnsLoop();
#endif // MDNS

#if HASP_USE_FTP > 0
    ftpLoop();
#endif // FTP

#if HASP_USE_TELNET > 0
    telnetLoop();
#endif // TELNET
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

/* bool networkEverySecond(void)
{
    bool connected = false;

#if HASP_USE_ETHERNET > 0
    connected |= ethernetEverySecond();
#endif

#if HASP_USE_WIFI > 0
    // connected |= wifiEverySecond();
#endif

#if HASP_USE_ARDUINOOTA > 0
    otaEverySecond(); // progressbar
#endif

    return true;
} */

void network_get_statusupdate(char* buffer, size_t len)
{
#if HASP_USE_ETHERNET > 0
    ethernet_get_statusupdate(buffer, len);
#endif

#if HASP_USE_WIFI > 0
    wifi_get_statusupdate(buffer, len);
#endif
}

void network_get_ipaddress(char* buffer, size_t len)
{
#if HASP_USE_ETHERNET > 0
    IPAddress ip = Ethernet.localIP();
    snprintf_P(buffer, len, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    return;
#endif

#if HASP_USE_WIFI > 0

#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    snprintf_P(buffer, len, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
#else
    strncpy(buffer, WiFi.localIP().toString().c_str(), len);
#endif

    return;
#endif // HASP_USE_WIFI

    snprintf(buffer, len, "");
}

void network_get_info(JsonDocument& doc)
{
#if HASP_USE_ETHERNET > 0
    ethernet_get_info(doc);
#endif

#if HASP_USE_WIFI > 0
    wifi_get_info(doc);
#endif
}

#endif