/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* ===========================================================================

- LOG_FATAL() - A fatal exception is caught, the program should halt with while(1){}
- LOG_ERROR() - An important but non-fatal error occured, this error should be checked and not ignored
- LOG_WARNING() - Send at the end of a function to indicate failure of the sub process, can be ignored

- LOG_TRACE() - Information at the START of an action to notify another function is now running
    - LOG_INFO()   - Send at the END of a function to indicate successful completion of the sub process
    - LOG_VERBOSE() - Send DEBUG information DURING a subprocess

=========================================================================== */

#include "hasp_conf.h"
#include "ConsoleInput.h"
#include "lvgl.h"
// #include "time.h"
#include <StreamUtils.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <sntp.h> // sntp_servermode_dhcp()
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#if HASP_USE_SYSLOG > 0
#include <WiFiUdp.h>
#endif
#elif defined(STM32F4xx) || defined(STM32F7xx)
#include <time.h>
#endif

#include "hasp_conf.h"
#include "dev/device.h"

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"

#include "hasp/hasp_dispatch.h"
#include "hasp/hasp.h"

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200
#endif

#if HASP_USE_SYSLOG > 0

#ifndef SYSLOG_SERVER
#define SYSLOG_SERVER ""
#endif

#ifndef SYSLOG_PORT
#define SYSLOG_PORT 514
#endif

#ifndef APP_NAME
#define APP_NAME "HASP"
#endif

// variables for debug stream writer
// static String debugOutput((char *)0);
// static StringStream debugStream((String &)debugOutput);

// const char* syslogAppName   = APP_NAME;
char debugSyslogHost[32]    = SYSLOG_SERVER;
uint16_t debugSyslogPort    = SYSLOG_PORT;
uint8_t debugSyslogFacility = 0;
uint8_t debugSyslogProtocol = 0;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP* syslogClient                      = NULL;
WriteBufferingStream* bufferedSyslogClient = NULL;
#define SYSLOG_PROTO_IETF 0

// Create a new syslog instance with LOG_KERN facility
// Syslog syslog(syslogClient, SYSLOG_SERVER, SYSLOG_PORT, MQTT_CLIENT, APP_NAME, LOG_KERN);
// Create a new empty syslog instance
// Syslog * syslog;

#endif // USE_SYSLOG

// Serial Settings
// uint16_t serialInputIndex   = 0; // Empty buffer
// char serialInputBuffer[220] = "";
// uint16_t historyIndex       = sizeof(serialInputBuffer) - 1; // Empty buffer
int32_t debugSerialBaud = SERIAL_SPEED;
extern bool debugAnsiCodes;

extern dispatch_conf_t dispatch_setings;

// #if HASP_USE_SYSLOG > 0
// void syslogSend(uint8_t priority, const char * debugText)
// {
//     if(strlen(debugSyslogHost) != 0 && WiFi.isConnected()) {
//         syslog->log(priority, debugText);
//     }
// }
// #endif

// TODO: Remove old compatibility options
static int32_t multiply_legacy_baudrate(int32_t baud)
{
    switch(baud) {
        case 960:
        case 1920:
        case 3840:
        case 5760:
        case 7488:
        case 11520:
            baud *= 10; // multiply old values
            break;
    }
    return baud;
}

void debugStartSyslog()
{

#if HASP_USE_SYSLOG > 0
    // syslog = new Syslog(syslogClient, debugSyslogProtocol == 0 ? SYSLOG_PROTO_IETF : SYSLOG_PROTO_BSD);
    // syslog->server(debugSyslogHost, debugSyslogPort);
    // syslog->deviceHostname(mqttNodeName);
    // syslog->appName(syslogAppName);
    // uint16_t priority = (uint16_t)(debugSyslogFacility + 16) << 3; // localx facility, x = 0-7
    // syslog->defaultPriority(priority);

    if(strlen(debugSyslogHost) > 0) {
        if(!syslogClient) syslogClient = new WiFiUDP();

        if(syslogClient) {
            if(syslogClient->beginPacket(debugSyslogHost, debugSyslogPort)) {
                if(!bufferedSyslogClient) bufferedSyslogClient = new WriteBufferingStream(*syslogClient, 256);
                if(!bufferedSyslogClient)
                    Log.registerOutput(2, bufferedSyslogClient, HASP_LOG_LEVEL, true);
                else
                    Log.registerOutput(2, syslogClient, HASP_LOG_LEVEL, true);
                LOG_INFO(TAG_SYSL, F(D_SERVICE_STARTED));
            }
        } else {
            LOG_ERROR(TAG_SYSL, F(D_SERVICE_START_FAILED));
        }
    }
#endif
}

void debugStopSyslog()
{
#if HASP_USE_SYSLOG > 0
    if(strlen(debugSyslogHost) > 0) {
        LOG_WARNING(TAG_SYSL, F(D_SERVICE_STOPPED));
        Log.unregisterOutput(2);
    }
#endif
}

#if HASP_USE_CONFIG > 0
bool debugGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(debugAnsiCodes != settings[FPSTR(FP_DEBUG_ANSI)]) changed = true;
    settings[FPSTR(FP_DEBUG_ANSI)] = (uint8_t)debugAnsiCodes;

    if(debugSerialBaud != settings[FPSTR(FP_CONFIG_BAUD)]) changed = true;
    settings[FPSTR(FP_CONFIG_BAUD)] = debugSerialBaud;

    if(dispatch_setings.teleperiod != settings[FPSTR(FP_DEBUG_TELEPERIOD)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_DEBUG_TELEPERIOD)] = dispatch_setings.teleperiod;

#if HASP_USE_SYSLOG > 0
    if(strcmp(debugSyslogHost, settings[FPSTR(FP_CONFIG_HOST)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_HOST)] = debugSyslogHost;

    if(debugSyslogPort != settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PORT)] = debugSyslogPort;

    if(debugSyslogProtocol != settings[FPSTR(FP_CONFIG_PROTOCOL)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PROTOCOL)] = debugSyslogProtocol;

    if(debugSyslogFacility != settings[FPSTR(FP_CONFIG_LOG)].as<uint8_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_LOG)] = debugSyslogFacility;
#endif

    if(changed) configOutput(settings, TAG_DEBG);
    return changed;
}

/** Set DEBUG Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool debugSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_DEBG);
    bool changed = false;

    /* Serial Settings */
    changed |= configSet(debugSerialBaud, settings[FPSTR(FP_CONFIG_BAUD)], F("debugSerialBaud"));
    if(changed) { // baudrate was changed
        debugSerialBaud = multiply_legacy_baudrate(debugSerialBaud);
    }

    /* Ansi Code Settings */
    changed |= configSet(debugAnsiCodes, settings[FPSTR(FP_DEBUG_ANSI)], F("debugAnsi"));

    /* Teleperiod Settings */
    changed |= configSet(dispatch_setings.teleperiod, settings[FPSTR(FP_DEBUG_TELEPERIOD)], F("debugTelePeriod"));

/* Syslog Settings */
#if HASP_USE_SYSLOG > 0
    if(!settings[FPSTR(FP_CONFIG_HOST)].isNull()) {
        changed |= strcmp(debugSyslogHost, settings[FPSTR(FP_CONFIG_HOST)]) != 0;
        strncpy(debugSyslogHost, settings[FPSTR(FP_CONFIG_HOST)], sizeof(debugSyslogHost));
    }
    changed |= configSet(debugSyslogPort, settings[FPSTR(FP_CONFIG_PORT)], F("debugSyslogPort"));
    changed |= configSet(debugSyslogProtocol, settings[FPSTR(FP_CONFIG_PROTOCOL)], F("debugSyslogProtocol"));
    changed |= configSet(debugSyslogFacility, settings[FPSTR(FP_CONFIG_LOG)], F("debugSyslogFacility"));
#endif

    return changed;
}
#endif // HASP_USE_CONFIG

/*
size_t debugHistorycount()
{
    size_t count = 0;
    for(size_t i = 1; i < sizeof(serialInputBuffer); i++) {
        if(serialInputBuffer[i] == 0 && serialInputBuffer[i - 1] != 0) count++;
    }
    return count;
}

size_t debugHistoryIndex(size_t num)
{
    size_t pos = 0;
    while(num > 0 && pos < sizeof(serialInputBuffer) - 2) {
        if(serialInputBuffer[pos] == 0) {
            num--;
            // skip extra \0s
            while(serialInputBuffer[pos] == 0) {
                pos++;
            }
        } else {
            pos++;
        }
    }

    return pos;
}

void debugShowHistory()
{
    size_t num = debugHistorycount();
    HASP_SERIAL.println();
    for(int i = 0; i <= num; i++) {
        HASP_SERIAL.print("[");
        HASP_SERIAL.print(i);
        HASP_SERIAL.print("] ");
        size_t pos = debugHistoryIndex(i);
        if(pos < sizeof(serialInputBuffer)) HASP_SERIAL.println((char *)(serialInputBuffer + pos));
    }
}

void debugGetHistoryLine(size_t num)
{
    size_t pos    = debugHistoryIndex(num);
    size_t len    = strlen(serialInputBuffer);
    char * dst    = serialInputBuffer;
    char * src    = serialInputBuffer + pos;
    size_t newlen = strlen(src);
    if(len < newlen) {
        // make room, shift whole buffer right
        dst = serialInputBuffer + newlen - len;
        src = serialInputBuffer;
        memmove(dst, src, sizeof(serialInputBuffer) - newlen + len);

        dst = serialInputBuffer;
        memset(dst, 0, newlen);
    } else {
        memset(dst, 0, len);
    }
    dst = serialInputBuffer;
    src = serialInputBuffer + pos + newlen - len;
    memmove(dst, src, newlen);
}
*/

bool debugSyslogPrefix(uint8_t tag, int level, Print* _logOutput, const char* processname)
{

#if HASP_USE_SYSLOG > 0

    if(syslogClient && _logOutput == syslogClient) {
        if(syslogClient->beginPacket(debugSyslogHost, debugSyslogPort)) {

            // IETF Doc: https://tools.ietf.org/html/rfc5424 - The Syslog Protocol
            // BSD Doc: https://tools.ietf.org/html/rfc3164 - The BSD syslog Protocol
            char buffer[32 + STR_LEN_HOSTNAME];
            int len;
            uint priority = (16 + debugSyslogFacility) * 8 + level;

            if(debugSyslogProtocol == SYSLOG_PROTO_IETF) {
                len = snprintf_P(buffer, sizeof(buffer), PSTR("<%d>1 - %s %s - - \xEF\xBB\xBF"), priority,
                                 haspDevice.get_hostname(), processname);
            } else {
                len = snprintf_P(buffer, sizeof(buffer), PSTR("<%d>%s %s: "), priority, haspDevice.get_hostname(),
                                 processname);
            }

            if(len > 0) syslogClient->write((uint8_t*)buffer, len);

            // syslogClient->print(F("<"));
            // syslogClient->print((16 + debugSyslogFacility) * 8 + level);
            // syslogClient->print(F(">"));

            // if(debugSyslogProtocol == SYSLOG_PROTO_IETF) {
            //     syslogClient->print(F("1 - "));
            // }

            // // debug_get_tag(tag, buffer);
            // char buffer[10];
            // syslogClient->print(F("%s %s"), haspDevice.get_hostname(), buffer);

            // if(debugSyslogProtocol == SYSLOG_PROTO_IETF) {
            //     syslogClient->print(F(" - - - \xEF\xBB\xBF")); // include UTF-8 BOM
            // } else {
            //     syslogClient->print(F(": "));
            // }

            // debugPrintHaspMemory(level, _logOutput);
            // debugPrintLvglMemory(level, _logOutput);
        }
        return true;
    }
#endif // HASP_USE_SYSLOG

    return false;
}

void debugPrintSuffix(uint8_t tag, int level, Print* _logOutput)
{
#if HASP_USE_SYSLOG > 0
    if(syslogClient && _logOutput == syslogClient) {
        syslogClient->endPacket();
        return;
    } else if(bufferedSyslogClient && _logOutput == bufferedSyslogClient) {
        bufferedSyslogClient->flush();
        syslogClient->endPacket();
        return;
    }
#endif

    if(debugAnsiCodes)
        _logOutput->println(F(TERM_COLOR_RESET));
    else
        _logOutput->println();

    if(_logOutput == &Serial) {
        console_update_prompt();
#if HASP_USE_TELNET > 0
    } else {
        telnet_update_prompt();
#endif
    }
}

// Start Serial Port at correct
bool debugStartSerial()
{
    if(debugSerialBaud < 0) {
        LOG_WARNING(TAG_DEBG, F(D_SERVICE_DISABLED " (%u Bps)"), debugSerialBaud);
        return false;
    }

    uint32_t baudrate = debugSerialBaud;
    if(baudrate < 9600) baudrate = SERIAL_SPEED; // 9600 baud minimum

#if defined(STM32F4xx) || defined(STM32F7xx)
#ifndef STM32_SERIAL1       // Define what Serial port to use for log output
    HASP_SERIAL.setRx(PA3); // User Serial2
    HASP_SERIAL.setTx(PA2);
#endif
#endif

    HASP_SERIAL.begin(baudrate); /* prepare for possible serial debug */
    delay(10);
    Log.registerOutput(0, &HASP_SERIAL, HASP_LOG_LEVEL, true); // LOG_LEVEL_VERBOSE

    HASP_SERIAL.println();
    debugPrintHaspHeader(&HASP_SERIAL);
    HASP_SERIAL.flush();

    LOG_INFO(TAG_DEBG, F(D_SERVICE_STARTED " @ %u bps"), debugSerialBaud);
    LOG_INFO(TAG_DEBG, F(D_INFO_ENVIRONMENT ": " PIOENV));
    return true;
}

// Do NOT call Log function before debugSetup is called
void debugSetup(JsonObject settings)
{
    Log.begin(LOG_LEVEL_WARNING, true);
    Log.setPrefix(debugPrintPrefix); // Uncomment to get timestamps as prefix
    Log.setSuffix(debugPrintSuffix); // Uncomment to get newline as suffix
    Log.unregisterOutput(0);
    Log.unregisterOutput(1);
    Log.unregisterOutput(3);

#if HASP_USE_CONFIG > 0
    if(!settings[FPSTR(FP_CONFIG_BAUD)].isNull()) {
        debugSerialBaud = multiply_legacy_baudrate(settings[FPSTR(FP_CONFIG_BAUD)].as<int32_t>());
    }
#endif
}

IRAM_ATTR void debugLoop(void)
{}

void printLocalTime()
{
    char buffer[128];
    time_t rawtime;
    struct tm* timeinfo;

    // if(!time(nullptr)) return;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S.", timeinfo);
    HASP_SERIAL.println(buffer);
    // struct tm timeinfo;
    // time_t now = time(nullptr);

    // Serial-.print(ctime(&now));
    // HASP_SERIAL.print(&timeinfo, " %d %B %Y %H:%M:%S ");

#if LWIP_VERSION_MAJOR > 1

    // LwIP v2 is able to list more details about the currently configured SNTP servers
    for(int i = 0; i < SNTP_MAX_SERVERS; i++) {
        IPAddress sntp   = *sntp_getserver(i);
        const char* name = sntp_getservername(i);
        if(sntp.isSet()) {
            HASP_SERIAL.printf("sntp%d:     ", i);
            if(name) {
                HASP_SERIAL.printf("%s (%s) ", name, sntp.toString().c_str());
            } else {
                HASP_SERIAL.printf("%s ", sntp.toString().c_str());
            }
            HASP_SERIAL.printf("IPv6: %s Reachability: %o\n", sntp.isV6() ? D_YES : D_NO, sntp_getreachability(i));
        }
    }
#endif
}
