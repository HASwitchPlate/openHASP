#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include <Wifi.h>
#endif
#include <WiFiUdp.h>

#include "hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#if HASP_USE_SYSLOG != 0
#include <Syslog.h>

#ifndef SYSLOG_SERVER
#define SYSLOG_SERVER ""
#endif
#ifndef SYSLOG_PORT
#define SYSLOG_PORT 514
#endif
#ifndef APP_NAME
#define APP_NAME "HASP"
#endif

std::string debugAppName    = APP_NAME;
std::string debugSyslogHost = SYSLOG_SERVER;
uint16_t debugSyslogPort    = SYSLOG_PORT;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP syslogClient;

// Create a new syslog instance with LOG_KERN facility
// Syslog syslog(syslogClient, SYSLOG_SERVER, SYSLOG_PORT, MQTT_CLIENT, APP_NAME, LOG_KERN);
// Create a new empty syslog instance
Syslog syslog(syslogClient, debugSyslogHost.c_str(), debugSyslogPort, debugAppName.c_str(), debugAppName.c_str(),
              LOG_LOCAL0);
#endif

void debugSetup()
{
#if defined(ARDUINO_ARCH_ESP32)
    Serial.begin(115200); /* prepare for possible serial debug */
#else
    Serial.begin(74880); /* prepare for possible serial debug */
#endif

    Serial.flush();
    Serial.println();
    Serial.printf_P(PSTR("           _____ _____ _____ _____\n          |  |  |  _  |   __|  _  |\n"
                         "          |     |     |__   |   __|\n          |__|__|__|__|_____|__|\n"
                         "        Home Automation Switch Plate\n        Open Hardware edition v%u.%u.%u\n\n"),
                    HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);
    Serial.flush();

    // prepare syslog configuration here (can be anywhere before first call of
    // log/logf method)

#if HASP_USE_SYSLOG != 0
    syslog.server(debugSyslogHost.c_str(), debugSyslogPort);
    syslog.deviceHostname(debugAppName.c_str());
    syslog.appName(debugAppName.c_str());
    syslog.defaultPriority(LOG_LOCAL0);
#endif
}

void debugLoop()
{}

void serialPrintln(const char * debugText)
{
    String debugTimeText((char *)0);
    debugTimeText.reserve(127);

    debugTimeText = F("[");
    debugTimeText += String(float(millis()) / 1000, 3);
    debugTimeText += F("s] ");
    debugTimeText += ESP.getMaxFreeBlockSize();
    debugTimeText += F("/");
    debugTimeText += ESP.getFreeHeap();
    debugTimeText += F(" ");
    debugTimeText += halGetHeapFragmentation();
    debugTimeText += F(" ");

    Serial.print(debugTimeText);
    Serial.println(debugText);
}

void serialPrintln(String & debugText)
{
    serialPrintln(debugText.c_str());
}

#if HASP_USE_SYSLOG != 0
void syslogSend(uint8_t log, const char * debugText)
{
    if(WiFi.isConnected() && debugSyslogHost != "") {
        switch(log) {
            case 1:
                syslog.log(LOG_WARNING, debugText);
                break;
            case2:
                syslog.log(LOG_ERR, debugText);
                break;
            default:
                syslog.log(LOG_INFO, debugText);
        }
    }
}
#endif

void debugStop()
{
    Serial.flush();
}