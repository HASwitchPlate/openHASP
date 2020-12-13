/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* ===========================================================================

- Log.fatal() - A fatal exception is caught, the program should halt with while(1){}
- Log.error() - An important but non-fatal error occured, this error should be checked and not ignored
- Log.warning() - Send at the end of a function to indicate failure of the sub process, can be ignored

- Log.notice() - Information at the START of an action to notify another function is now running
    - Log.trace()   - Send at the END of a function to indicate successful completion of the sub process
    - Log.verbose() - Send DEBUG information DURING a subprocess

=========================================================================== */

#include "hasp_conf.h"
#include "ConsoleInput.h"
#include "lvgl.h"
//#include "time.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <sntp.h> // sntp_servermode_dhcp()
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#endif

#include "hasp.h"
#include "hasp_hal.h"
#include "hasp_conf.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200
#endif

#if HASP_USE_SYSLOG > 0
#include <WiFiUdp.h>

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

// extern char mqttNodeName[16];
const char * syslogAppName  = APP_NAME;
char debugSyslogHost[32]    = SYSLOG_SERVER;
uint16_t debugSyslogPort    = SYSLOG_PORT;
uint8_t debugSyslogFacility = 0;
uint8_t debugSyslogProtocol = 0;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP * syslogClient;
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
uint16_t debugSerialBaud = SERIAL_SPEED / 10; // Multiplied by 10
bool debugSerialStarted  = false;
bool debugAnsiCodes      = true;

ConsoleInput debugConsole(&Serial, 220);

//#define TERM_COLOR_Black "\u001b[30m"
#define TERM_COLOR_GRAY "\e[37m"
#define TERM_COLOR_RED "\e[91m"
#define TERM_COLOR_GREEN "\e[92m"
#define TERM_COLOR_ORANGE "\e[38;5;214m"
#define TERM_COLOR_YELLOW "\e[93m"
#define TERM_COLOR_BLUE "\e[94m"
#define TERM_COLOR_MAGENTA "\e[35m"
#define TERM_COLOR_CYAN "\e[96m"
#define TERM_COLOR_WHITE "\e[97m"
#define TERM_COLOR_RESET "\e[0m"
#define TERM_CLEAR_LINE "\e[1000D\e[0K"

unsigned long debugLastMillis = 0;
uint16_t debugTelePeriod      = 300;

// Send the HASP header and version to the output device specified
void debugPrintHaspHeader(Print * output)
{
    if(debugAnsiCodes) output->print(TERM_COLOR_YELLOW);
    output->println();
    output->print(F(""
                    "           _____ _____ _____ _____\r\n"
                    "          |  |  |  _  |   __|  _  |\r\n"
                    "          |     |     |__   |   __|\r\n"
                    "          |__|__|__|__|_____|__|\r\n"
                    "        Home Automation Switch Plate\r\n"
                    "        Open Hardware edition v"));
    char buffer[32];
    snprintf(buffer, sizeof(buffer), PSTR("%u.%u.%u"), HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);
    output->println(buffer);
    output->println();
}

void debugStart()
{
    if(debugSerialStarted) {
        Serial.flush();
        // Serial.println();
        // Serial.println(debugHaspHeader());
        // Serial.flush();
    }

    // prepare syslog configuration here (can be anywhere before first call of
    // log/logf method)
}

// #if HASP_USE_SYSLOG > 0
// void syslogSend(uint8_t priority, const char * debugText)
// {
//     if(strlen(debugSyslogHost) != 0 && WiFi.isConnected()) {
//         syslog->log(priority, debugText);
//     }
// }
// #endif

void debugSetup()
{
    // memset(serialInputBuffer, 0, sizeof(serialInputBuffer));
    // serialInputIndex = 0;
    Log.notice(TAG_DEBG, F("Setting the console parser"));
    debugConsole.setLineCallback(dispatch_text_line);
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
                Log.registerOutput(2, syslogClient, LOG_LEVEL_VERBOSE, true);
                Log.trace(TAG_DEBG, F("Syslog client started"));
            }
        } else {
            Log.error(TAG_DEBG, F("Failed to start syslog client"));
        }
    }
#endif
}

void debugStopSyslog()
{
#if HASP_USE_SYSLOG > 0
    if(strlen(debugSyslogHost) > 0) {
        Log.unregisterOutput(2);
    }
#endif
}

void debugStop()
{
    if(debugSerialStarted) Serial.flush();
}

bool debugGetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(debugSerialBaud != settings[FPSTR(F_CONFIG_BAUD)].as<uint16_t>()) changed = true;
    settings[FPSTR(F_CONFIG_BAUD)] = debugSerialBaud;

    if(debugTelePeriod != settings[FPSTR(F_DEBUG_TELEPERIOD)].as<uint16_t>()) changed = true;
    settings[FPSTR(F_DEBUG_TELEPERIOD)] = debugTelePeriod;

#if HASP_USE_SYSLOG > 0
    if(strcmp(debugSyslogHost, settings[FPSTR(F_CONFIG_HOST)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_HOST)] = debugSyslogHost;

    if(debugSyslogPort != settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(F_CONFIG_PORT)] = debugSyslogPort;

    if(debugSyslogProtocol != settings[FPSTR(F_CONFIG_PROTOCOL)].as<uint8_t>()) changed = true;
    settings[FPSTR(F_CONFIG_PROTOCOL)] = debugSyslogProtocol;

    if(debugSyslogFacility != settings[FPSTR(F_CONFIG_LOG)].as<uint8_t>()) changed = true;
    settings[FPSTR(F_CONFIG_LOG)] = debugSyslogFacility;
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
bool debugSetConfig(const JsonObject & settings)
{
    configOutput(settings, TAG_DEBG);
    bool changed = false;

    /* Serial Settings*/
    changed |= configSet(debugSerialBaud, settings[FPSTR(F_CONFIG_BAUD)], F("debugSerialBaud"));

    /* Teleperiod Settings*/
    changed |= configSet(debugTelePeriod, settings[FPSTR(F_DEBUG_TELEPERIOD)], F("debugTelePeriod"));

    /* Syslog Settings*/
#if HASP_USE_SYSLOG > 0
    if(!settings[FPSTR(F_CONFIG_HOST)].isNull()) {
        changed |= strcmp(debugSyslogHost, settings[FPSTR(F_CONFIG_HOST)]) != 0;
        strncpy(debugSyslogHost, settings[FPSTR(F_CONFIG_HOST)], sizeof(debugSyslogHost));
    }
    changed |= configSet(debugSyslogPort, settings[FPSTR(F_CONFIG_PORT)], F("debugSyslogPort"));
    changed |= configSet(debugSyslogProtocol, settings[FPSTR(F_CONFIG_PROTOCOL)], F("debugSyslogProtocol"));
    changed |= configSet(debugSyslogFacility, settings[FPSTR(F_CONFIG_LOG)], F("debugSyslogFacility"));
#endif

    return changed;
}

inline void debugSendAnsiCode(const __FlashStringHelper * code, Print * _logOutput)
{
    if(debugAnsiCodes) _logOutput->print(code);
}
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
    Serial.println();
    for(int i = 0; i <= num; i++) {
        Serial.print("[");
        Serial.print(i);
        Serial.print("] ");
        size_t pos = debugHistoryIndex(i);
        if(pos < sizeof(serialInputBuffer)) Serial.println((char *)(serialInputBuffer + pos));
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

static void debugPrintTimestamp(int level, Print * _logOutput)
{ /* Print Current Time */
    time_t rawtime;
    struct tm * timeinfo;

    // time(&rawtime);
    // timeinfo = localtime(&rawtime);

    // strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S.", timeinfo);
    // Serial.println(buffer);

    debugSendAnsiCode(F(TERM_COLOR_CYAN), _logOutput);

    /* if(timeinfo->tm_year >= 120) {
         char buffer[64];
         strftime(buffer, sizeof(buffer), "[%b %d %H:%M:%S.", timeinfo); // Literal String
         _logOutput->print(buffer);
         _logOutput->printf(PSTR("%03lu]"), millis() % 1000);
     } else */
    {
        uint32_t msecs = millis();
        _logOutput->printf(PSTR("[%16d.%03d]"), msecs / 1000, msecs % 1000);
    }
}

static void debugPrintHaspMemory(int level, Print * _logOutput)
{
    size_t maxfree     = halGetMaxFreeBlock();
    uint32_t totalfree = halGetFreeHeap();
    uint8_t frag       = halGetHeapFragmentation();

    /* Print HASP Memory Info */
    if(debugAnsiCodes) {
        if(maxfree > (1024u * 5) && (totalfree > 1024u * 6) && (frag <= 10))
            debugSendAnsiCode(F(TERM_COLOR_GREEN), _logOutput);
        else if(maxfree > (1024u * 3) && (totalfree > 1024u * 5) && (frag <= 20))
            debugSendAnsiCode(F(TERM_COLOR_ORANGE), _logOutput);
        else
            debugSendAnsiCode(F(TERM_COLOR_RED), _logOutput);
    }
    _logOutput->printf(PSTR("[%5u/%5u%3u]"), maxfree, totalfree, frag);
}

#if LV_MEM_CUSTOM == 0
static void debugPrintLvglMemory(int level, Print * _logOutput)
{
    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);

    /* Print LVGL Memory Info */
    if(debugAnsiCodes) {
        if(mem_mon.free_biggest_size > (1024u * 2) && (mem_mon.free_size > 1024u * 2.5) && (mem_mon.frag_pct <= 10))
            debugSendAnsiCode(F(TERM_COLOR_GREEN), _logOutput);
        else if(mem_mon.free_biggest_size > (1024u * 1) && (mem_mon.free_size > 1024u * 1.5) &&
                (mem_mon.frag_pct <= 25))
            debugSendAnsiCode(F(TERM_COLOR_ORANGE), _logOutput);
        else
            debugSendAnsiCode(F(TERM_COLOR_RED), _logOutput);
    }
    _logOutput->printf(PSTR("[%5u/%5u%3u]"), mem_mon.free_biggest_size, mem_mon.free_size, mem_mon.frag_pct);
}
#endif

static void debugPrintPriority(int level, Print * _logOutput)
{
    // if(_logOutput == &syslogClient) {
    // }

    switch(level) {
        case LOG_LEVEL_FATAL:
        case LOG_LEVEL_ERROR:
            debugSendAnsiCode(F(TERM_COLOR_RED), _logOutput);
            break;
        case LOG_LEVEL_WARNING:
            debugSendAnsiCode(F(TERM_COLOR_YELLOW), _logOutput);
            break;
        case LOG_LEVEL_NOTICE:
            debugSendAnsiCode(F(TERM_COLOR_WHITE), _logOutput);
            break;
        case LOG_LEVEL_TRACE:
            debugSendAnsiCode(F(TERM_COLOR_GRAY), _logOutput);
            break;
        case LOG_LEVEL_VERBOSE:
            debugSendAnsiCode(F(TERM_COLOR_CYAN), _logOutput);
            break;
        default:
            debugSendAnsiCode(F(TERM_COLOR_RESET), _logOutput);
    }
}

static void debugPrintTag(uint8_t tag, Print * _logOutput)
{
    switch(tag) {
        case TAG_MAIN:
            _logOutput->print(F("MAIN"));
            break;

        case TAG_HASP:
            _logOutput->print(F("HASP"));
            break;

        case TAG_ATTR:
            _logOutput->print(F("ATTR"));
            break;

        case TAG_MSGR:
            _logOutput->print(F("MSGR"));
            break;

        case TAG_OOBE:
            _logOutput->print(F("OOBE"));
            break;
        case TAG_HAL:
            _logOutput->print(F("HAL "));
            break;

        case TAG_DEBG:
            _logOutput->print(F("DEBG"));
            break;
        case TAG_TELN:
            _logOutput->print(F("TELN"));
            break;
        case TAG_SYSL:
            _logOutput->print(F("SYSL"));
            break;
        case TAG_TASM:
            _logOutput->print(F("TASM"));
            break;

        case TAG_CONF:
            _logOutput->print(F("CONF"));
            break;
        case TAG_GUI:
            _logOutput->print(F("GUI "));
            break;
        case TAG_TFT:
            _logOutput->print(F("TFT "));
            break;

        case TAG_EPRM:
            _logOutput->print(F("EPRM"));
            break;
        case TAG_FILE:
            _logOutput->print(F("FILE"));
            break;
        case TAG_GPIO:
            _logOutput->print(F("GPIO"));
            break;

        case TAG_ETH:
            _logOutput->print(F("ETH "));
            break;
        case TAG_WIFI:
            _logOutput->print(F("WIFI"));
            break;
        case TAG_HTTP:
            _logOutput->print(F("HTTP"));
            break;
        case TAG_MDNS:
            _logOutput->print(F("MDNS"));
            break;
        case TAG_MQTT:
            _logOutput->print(F("MQTT"));
            break;
        case TAG_MQTT_PUB:
            _logOutput->print(F("MQTT PUB"));
            break;
        case TAG_MQTT_RCV:
            _logOutput->print(F("MQTT RCV"));
            break;

        case TAG_OTA:
            _logOutput->print(F("OTA"));
            break;
        case TAG_FWUP:
            _logOutput->print(F("FWUP"));
            break;

        case TAG_LVGL:
            _logOutput->print(F("LVGL"));
            break;
        case TAG_LVFS:
            _logOutput->print(F("LVFS"));
            break;
        case TAG_FONT:
            _logOutput->print(F("FONT"));
            break;

        default:
            _logOutput->print(F("----"));
            break;
    }
}

void debugPrintPrefix(uint8_t tag, int level, Print * _logOutput)
{
#if HASP_USE_SYSLOG > 0

    if(_logOutput == syslogClient && syslogClient) {
        if(syslogClient->beginPacket(debugSyslogHost, debugSyslogPort)) {

            // IETF Doc: https://tools.ietf.org/html/rfc5424 - The Syslog Protocol
            // BSD Doc: https://tools.ietf.org/html/rfc3164 - The BSD syslog Protocol

            syslogClient->print(F("<"));
            syslogClient->print((16 + debugSyslogFacility) * 8 + level);
            syslogClient->print(F(">"));

            if(debugSyslogProtocol == SYSLOG_PROTO_IETF) {
                syslogClient->print(F("1 - "));
            }

            syslogClient->print(mqttGetNodename());
            syslogClient->print(F(" "));
            debugPrintTag(tag, _logOutput);

            if(debugSyslogProtocol == SYSLOG_PROTO_IETF) {
                syslogClient->print(F(" - - - \xEF\xBB\xBF")); // include UTF-8 BOM
            } else {
                syslogClient->print(F(": "));
            }

            debugPrintHaspMemory(level, _logOutput);
#if LV_MEM_CUSTOM == 0
            debugPrintLvglMemory(level, _logOutput);
#endif
        }
        return;
    }
#endif // HASP_USE_SYSLOG

    debugSendAnsiCode(F(TERM_CLEAR_LINE), _logOutput);
    debugPrintTimestamp(level, _logOutput);
    debugPrintHaspMemory(level, _logOutput);

#if LV_MEM_CUSTOM == 0
    debugPrintLvglMemory(level, _logOutput);
#endif

    if(tag == TAG_MQTT_PUB && level == LOG_LEVEL_NOTICE) {
        debugSendAnsiCode(F(TERM_COLOR_GREEN), _logOutput);
    } else if(tag == TAG_MQTT_RCV && level == LOG_LEVEL_NOTICE) {
        debugSendAnsiCode(F(TERM_COLOR_ORANGE), _logOutput);
    } else {
        debugPrintPriority(level, _logOutput);
    }

    _logOutput->print(F(" "));
    debugPrintTag(tag, _logOutput);
    _logOutput->print(F(": "));
}

void debugPrintSuffix(uint8_t tag, int level, Print * _logOutput)
{
#if HASP_USE_SYSLOG > 0
    if(_logOutput == syslogClient && syslogClient) {
        syslogClient->endPacket();
        return;
    }
#endif

    if(debugAnsiCodes)
        _logOutput->println(F(TERM_COLOR_RESET));
    else
        _logOutput->println();

    if(_logOutput == &Serial) {
        debugConsole.update();
    } else {
        _logOutput->print("hasp > ");
    }
}

void debugPreSetup(JsonObject settings)
{
    // Link stream to debugOutput
    // debugOutput.reserve(512);

    Log.begin(LOG_LEVEL_WARNING, true);
    Log.setPrefix(debugPrintPrefix); // Uncomment to get timestamps as prefix
    Log.setSuffix(debugPrintSuffix); // Uncomment to get newline as suffix

    uint32_t baudrate = settings[FPSTR(F_CONFIG_BAUD)].as<uint32_t>() * 10;
    if(baudrate == 0) baudrate = SERIAL_SPEED;
    if(baudrate >= 9600u) { /* the baudrates are stored divided by 10 */

#if defined(STM32F4xx)
#ifndef STM32_SERIAL1      // Define what Serial port to use for log output
        Serial.setRx(PA3); // User Serial2
        Serial.setTx(PA2);
#endif
#endif
        Serial.begin(baudrate); /* prepare for possible serial debug */
        delay(10);
        Log.registerOutput(0, &Serial, LOG_LEVEL_VERBOSE, true);
        debugSerialStarted = true;

        // Print Header
        Serial.println();
        debugPrintHaspHeader(&Serial);
        // Serial.println(debugHaspHeader());
        // Serial.println();
        Serial.flush();

        Log.trace(TAG_DEBG, ("Serial started at %u baud"), baudrate);
    }
}

#if LV_USE_LOG != 0
void debugLvglLogEvent(lv_log_level_t level, const char * file, uint32_t line, const char * funcname,
                       const char * descr)
{
    /* used for duplicate detection */
    static uint32_t lastDbgLine;
    static uint16_t lastDbgFreeMem;

    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);

    /* Reduce the number of reepeated debug message */
    if(line != lastDbgLine || mem_mon.free_biggest_size != lastDbgFreeMem) {
        switch(level) {
            case LV_LOG_LEVEL_TRACE:
                Log.verbose(TAG_LVGL, descr);
                break;
            case LV_LOG_LEVEL_WARN:
                Log.warning(TAG_LVGL, descr);
                break;
            case LV_LOG_LEVEL_ERROR:
                Log.error(TAG_LVGL, descr);
                break;
            default:
                Log.notice(TAG_LVGL, descr);
        }
        lastDbgLine    = line;
        lastDbgFreeMem = mem_mon.free_biggest_size;
    }
}
#endif

void IRAM_ATTR debugLoop(void)
{
    int16_t keypress = debugConsole.readKey();

    switch(keypress) {

        case ConsoleInput::KEY_PAGE_UP:
            dispatch_page_next();
            break;

        case ConsoleInput::KEY_PAGE_DOWN:
            dispatch_page_prev();
            break;

        case(ConsoleInput::KEY_FN)...(ConsoleInput::KEY_FN + 12):
            haspSetPage(keypress - ConsoleInput::KEY_FN - 1);
            break;
    }

    // while(Serial.available()) {
    //     char ch = Serial.read();
    //     // Serial.println((byte)ch);
    //     switch(ch) {
    //         case 1: // ^A = goto begin
    //             serialInputIndex = 0;
    //             historyIndex     = 0;
    //             break;
    //         case 3: // ^C
    //             serialInputIndex = 0;
    //             historyIndex     = 0;
    //             break;
    //         case 5: // ^E = goto end
    //             serialInputIndex = strlen(serialInputBuffer);
    //             historyIndex     = 0;
    //             break;
    //         case 8: // Backspace
    //         {
    //             if(serialInputIndex > strlen(serialInputBuffer)) {
    //                 serialInputIndex = strlen(serialInputBuffer);
    //             }

    //             if(serialInputIndex > 0) {
    //                 serialInputIndex--;
    //                 size_t len      = strlen(serialInputBuffer);
    //                 char * currchar = serialInputBuffer + serialInputIndex;
    //                 memmove(currchar, currchar + 1, len - serialInputIndex);
    //             }
    //             historyIndex = 0;
    //         } break;
    //         case 9: // Delete
    //         {
    //             size_t len            = strlen(serialInputBuffer);
    //             char * nextchar       = serialInputBuffer + serialInputIndex;
    //             char * remainingchars = serialInputBuffer + serialInputIndex + 1;
    //             memmove(nextchar, remainingchars, len - serialInputIndex);
    //             historyIndex = 0;
    //         } break;
    //         case 10 ... 13: // LF, VT, FF, CR
    //             if(serialInputBuffer[0] != 0) {
    //                 Serial.println();
    //                 dispatchTextLine(serialInputBuffer);

    //                 size_t numchars = 1;
    //                 memmove(serialInputBuffer + numchars, serialInputBuffer,
    //                         sizeof(serialInputBuffer) - numchars); // Shift chars right
    //             }
    //             serialInputIndex     = 0;
    //             serialInputBuffer[0] = 0;
    //             historyIndex         = 0;
    //             debugShowHistory();
    //             break;

    //         case 27:
    //             /*if(Serial.peek() >= 0)*/ {
    //                 char nextchar = Serial.read();
    //                 if(nextchar == 91 /*&& Serial.peek() >= 0*/) {
    //                     nextchar = Serial.read();
    //                     switch(nextchar) {
    //                         case 51: // Del
    //                             /*if(Serial.peek() >= 0)*/ {
    //                                 nextchar = Serial.read();
    //                             }
    //                             if(nextchar == 126) {
    //                                 size_t len            = strlen(serialInputBuffer);
    //                                 char * nextchar       = serialInputBuffer + serialInputIndex;
    //                                 char * remainingchars = serialInputBuffer + serialInputIndex + 1;
    //                                 memmove(nextchar, remainingchars, len - serialInputIndex);
    //                             }
    //                             break;
    //                         case 53: // Page Up
    //                             /*if(Serial.peek() >= 0)*/ {
    //                                 nextchar = Serial.read();
    //                             }
    //                             if(nextchar == 126) {
    //                                 dispatchPageNext();
    //                             }
    //                             historyIndex = 0;
    //                             break;
    //                         case 54: // Page Down
    //                             /*if(Serial.peek() >= 0)*/ {
    //                                 nextchar = Serial.read();
    //                                 if(nextchar == 126) {
    //                                     dispatchPagePrev();
    //                                 }
    //                             }
    //                             historyIndex = 0;
    //                             break;
    //                         case 65: {
    //                             size_t count = debugHistorycount();
    //                             if(historyIndex < count) {
    //                                 historyIndex++;
    //                                 debugGetHistoryLine(historyIndex);
    //                             }
    //                             break;
    //                         }
    //                         case 66:
    //                             if(historyIndex > 0) {
    //                                 historyIndex--;
    //                                 debugGetHistoryLine(historyIndex);
    //                             }
    //                             break;
    //                         case 68: // Left
    //                             if(serialInputIndex > 0) {
    //                                 serialInputIndex--;
    //                             }
    //                             historyIndex = 0;
    //                             break;
    //                         case 67: // Right
    //                             if(serialInputIndex < strlen(serialInputBuffer)) {
    //                                 serialInputIndex++;
    //                             }
    //                             historyIndex = 0;
    //                             break;
    //                             //  default:
    //                             //      Serial.println((byte)nextchar);
    //                     }
    //                 }
    //                 /* } else { // ESC, clear buffer
    //                      serialInputIndex                    = 0;
    //                      serialInputBuffer[serialInputIndex] = 0;*/
    //             }
    //             break;

    //         case 32 ... 126:
    //         case 128 ... 254: {
    //             Serial.print(ch);
    //             size_t len = strlen(serialInputBuffer);
    //             if(serialInputIndex > len) serialInputIndex = len;

    //             if(serialInputIndex == len && serialInputIndex < sizeof(serialInputBuffer) - 2) {
    //                 // expand needed
    //                 if(serialInputBuffer[serialInputIndex + 1] != 0) {
    //                     // shift right needed
    //                     char * dst = serialInputBuffer + len + 1;
    //                     char * src = serialInputBuffer + len;
    //                     memmove(dst, src, sizeof(serialInputBuffer) - len - 1);
    //                 }
    //             }

    //             if(serialInputIndex < sizeof(serialInputBuffer) - 2) {
    //                 if((size_t)1 + serialInputIndex >= strlen(serialInputBuffer))
    //                     serialInputBuffer[serialInputIndex + 1] = 0;
    //                 serialInputBuffer[serialInputIndex++] = ch;
    //             }
    //         } break;
}
/*void printLocalTime()
{
    char buffer[128];
    time_t rawtime;
    struct tm * timeinfo;

    // if(!time(nullptr)) return;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S.", timeinfo);
    Serial.println(buffer);
    // struct tm timeinfo;
    // time_t now = time(nullptr);

    // Serial-.print(ctime(&now));
    // Serial.print(&timeinfo, " %d %B %Y %H:%M:%S ");

#if LWIP_VERSION_MAJOR > 1

    // LwIP v2 is able to list more details about the currently configured SNTP servers
    for(int i = 0; i < SNTP_MAX_SERVERS; i++) {
        IPAddress sntp    = *sntp_getserver(i);
        const char * name = sntp_getservername(i);
        if(sntp.isSet()) {
            Serial.printf("sntp%d:     ", i);
            if(name) {
                Serial.printf("%s (%s) ", name, sntp.toString().c_str());
            } else {
                Serial.printf("%s ", sntp.toString().c_str());
            }
            Serial.printf("IPv6: %s Reachability: %o\n", sntp.isV6() ? "Yes" : "No", sntp_getreachability(i));
        }
    }
#endif
}*/

void debugEverySecond()
{
    if(debugTelePeriod > 0 && (millis() - debugLastMillis) >= debugTelePeriod * 1000) {
        dispatch_output_statusupdate();
        debugLastMillis = millis();
    }
    // printLocalTime();
}
