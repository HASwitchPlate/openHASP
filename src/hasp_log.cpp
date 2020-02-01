#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#else
#include <Wifi.h>
#endif
#include <WiFiUdp.h>
#include <Syslog.h>

#include "hasp_log.h"
#include "hasp_debug.h"

void debugPrintln(String debugText)
{
    serialPrintln(debugText);
    syslogSend(0, debugText.c_str());
}

void errorPrintln(String debugText)
{
    char buffer[256];
    sprintf_P(buffer, debugText.c_str(), PSTR("[ERROR] "));
    serialPrintln(buffer);
    syslogSend(2, buffer);
}

void warningPrintln(String debugText)
{
    char buffer[256];
    sprintf_P(buffer, debugText.c_str(), PSTR("[WARNING] "));
    serialPrintln(buffer);
    syslogSend(1, buffer);
}