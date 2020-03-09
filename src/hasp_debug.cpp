#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"
#include "lvgl.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include <Wifi.h>
#endif
#include <WiFiUdp.h>

#include "hasp_log.h"
#include "hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"

#if HASP_USE_TELNET != 0
#include "hasp_telnet.h"
#endif

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

unsigned long debugLastMillis = 0;
uint16_t debugTelePeriod      = 300;

void debugStart()
{
#if defined(ARDUINO_ARCH_ESP32)
    Serial.begin(115200); /* prepare for possible serial debug */
#else
    Serial.begin(74880); /* prepare for possible serial debug */
#endif

    Serial.flush();
    Serial.println();
    Serial.printf_P(PSTR("           _____ _____ _____ _____\r\n          |  |  |  _  |   __|  _  |\r\n"
                         "          |     |     |__   |   __|\r\n          |__|__|__|__|_____|__|\r\n"
                         "        Home Automation Switch Plate\r\n        Open Hardware edition v%u.%u.%u\r\n\r\n"),
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

void serialPrintln(const char * debugText)
{
    String debugTimeText((char *)0);
    debugTimeText.reserve(128);

    debugTimeText = F("[");
    debugTimeText += String(float(millis()) / 1000, 3);
    debugTimeText += F("s] ");
    debugTimeText += halGetMaxFreeBlock();
    debugTimeText += F("/");
    debugTimeText += ESP.getFreeHeap();
    debugTimeText += F(" ");
    debugTimeText += halGetHeapFragmentation();
    debugTimeText += F(" ");

#if LV_MEM_CUSTOM == 0
    /*lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);
    debugTimeText += mem_mon.used_pct;
    debugTimeText += F("% ");
    debugTimeText += mem_mon.free_biggest_size;
    debugTimeText += F("b/");
    debugTimeText += mem_mon.free_size;
    debugTimeText += F("b ");*/
#endif

    Serial.print(debugTimeText);
    Serial.println(debugText);

#if HASP_USE_TELNET != 0
    telnetPrint(debugTimeText.c_str());
    telnetPrintln(debugText);
#endif
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

bool debugGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_DEBUG_TELEPERIOD)] = debugTelePeriod;

    configOutput(settings);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool debugSetConfigLog(const JsonObject & settings, bool silent)
{
    if(!silent) configOutput(settings);
    bool changed = false;

    if(!settings[FPSTR(F_DEBUG_TELEPERIOD)].isNull()) {
        uint16_t period = settings[FPSTR(F_DEBUG_TELEPERIOD)].as<uint16_t>();
        if(debugTelePeriod != period) {
            if(!silent) debugPrintln(F("debugTelePeriod set"));
            debugTelePeriod = period;
            changed         = true;
        }
    }

    return changed;
}

bool debugSetConfig(const JsonObject & settings)
{
    return debugSetConfigLog(settings, false);
}

void debugPreSetup(JsonObject settings)
{
    debugSetConfigLog(settings, true);
}
void debugSetup(JsonObject settings)
{
    debugSetConfigLog(settings, false);
}

void debugLoop()
{
    if(debugTelePeriod > 0 && (millis() - debugLastMillis) >= debugTelePeriod * 1000) {
        dispatchStatusUpdate();
        debugLastMillis = millis();
    }
}
