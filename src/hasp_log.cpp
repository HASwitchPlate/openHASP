#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoLog.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include <Wifi.h>
#endif
#include <WiFiUdp.h>

#if HASP_USE_SYSLOG != 0
#include <Syslog.h>
#endif

#include "hasp_log.h"
#include "hasp_debug.h"

void debugPrintln(String & debugText)
{
    serialPrintln(debugText, LOG_LEVEL_NOTICE);

#if HASP_USE_SYSLOG != 0
    syslogSend(LOG_INFO, debugText.c_str());
#endif
}

void debugPrintln(const __FlashStringHelper * debugText)
{
    String buffer((char *)0);
    buffer.reserve(128);
    buffer = debugText;
    debugPrintln(buffer);
}

void debugPrintln(const char * debugText)
{
    serialPrintln(debugText, LOG_LEVEL_NOTICE);

#if HASP_USE_SYSLOG != 0
    syslogSend(LOG_INFO, debugText);
#endif
}

void errorPrintln(String debugText)
{
    char buffer[128];
    sprintf_P(buffer, debugText.c_str(), PSTR("[ERROR] "));
    serialPrintln(buffer, LOG_LEVEL_ERROR);

#if HASP_USE_SYSLOG != 0
    syslogSend(LOG_ERR, buffer);
#endif
}

void warningPrintln(String debugText)
{
    char buffer[128];
    sprintf_P(buffer, debugText.c_str(), PSTR("[WARNING] "));
    serialPrintln(buffer, LOG_LEVEL_WARNING);

#if HASP_USE_SYSLOG != 0
    syslogSend(LOG_WARNING, buffer);
#endif
}