#include "hasp_conf.h"
#include <Arduino.h>

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

void debugPrintln(String debugText)
{
    serialPrintln(debugText);

#if HASP_USE_SYSLOG != 0
    syslogSend(0, debugText.c_str());
#endif
}

void errorPrintln(String debugText)
{
    char buffer[256];
    sprintf_P(buffer, debugText.c_str(), PSTR("[ERROR] "));
    serialPrintln(buffer);

#if HASP_USE_SYSLOG != 0
    syslogSend(2, buffer);
#endif
}

void warningPrintln(String debugText)
{
    char buffer[256];
    sprintf_P(buffer, debugText.c_str(), PSTR("[WARNING] "));
    serialPrintln(buffer);

#if HASP_USE_SYSLOG != 0
    syslogSend(1, buffer);
#endif
}