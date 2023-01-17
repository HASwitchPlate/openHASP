/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"
#include <sys/time.h>

#include "lang/lang.h"
#include "hasp_debug.h"
#include "hasp_macro.h"

#if(!defined(WINDOWS)) && (!defined(POSIX))

#define debug_print(io, ...) io->printf(__VA_ARGS__)
#define debug_newline(io) io->println()

#else
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define debug_print(io, ...) fprintf(stdout, __VA_ARGS__)
#define debug_newline(io) fprintf(stdout, "\n")

#endif

bool debugAnsiCodes = true;

inline void debugSendAnsiCode(const __FlashStringHelper* code, Print* _logOutput)
{
#ifdef ARDUINO
    if(debugAnsiCodes) _logOutput->print(code);
#else
    if(debugAnsiCodes) debug_print(_logOutput, code);
#endif
}

/*
void debug_timestamp()
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    char currentTime[80];
    time_t t = curTime.tv_sec;
    // strftime(currentTime, 80, "%Y-%m-%d %H:%M.%S", localtime(&t));
    strftime(currentTime, 80, "%H:%M:%S", localtime(&t));
    printf("[%s.%03d] ", currentTime, milli);
} */

static void debugPrintTimestamp(int level, Print* _logOutput)
{ /* Print Current Time */

    timeval curTime;
    int rslt     = gettimeofday(&curTime, NULL);
    time_t t     = curTime.tv_sec;
    tm* timeinfo = localtime(&t);
    (void)rslt; // unused

    debugSendAnsiCode(F(TERM_COLOR_CYAN), _logOutput);

    if(timeinfo->tm_year >= 120) {
        unsigned long int milli = curTime.tv_usec / 1000;
        char buffer[24];
        // strftime(buffer, sizeof(buffer), "[%b %d %H:%M:%S", timeinfo); // Literal String
        strftime(buffer, sizeof(buffer), "[" D_TIMESTAMP, timeinfo); // Literal String

#ifdef ARDUINO
        _logOutput->printf(PSTR("%s.%03lu]"), buffer, milli);
#else
        debug_print(_logOutput, PSTR("%s.%03lu]"), buffer, milli);
#endif

    } else {

        uint32_t msecs = millis();
#ifdef ARDUINO
        _logOutput->printf(PSTR("[" D_TIME_MILLIS ".%03d]"), msecs / 1000, msecs % 1000);
#else
        debug_print(_logOutput, PSTR("[" D_TIME_MILLIS ".%03d]"), msecs / 1000, msecs % 1000);
#endif
    }
}

/* ===== Default Event Processors ===== */

static inline void debug_flush()
{
#if defined(ARDUINO)
    HASP_SERIAL.flush();
#endif

#if defined(WINDOWS) || defined(POSIX)
    fflush(stdout);
#endif
}

void debugEverySecond()
{
    // if(debugTelePeriod > 0 && (millis() - debugLastMillis) >= debugTelePeriod * 1000) {
    //     dispatch_statusupdate(NULL, NULL);
    //     debugLastMillis = millis();
    // }
    // printLocalTime();
}

void debugStart(void)
{

#if defined(WINDOWS) || defined(POSIX)
    debug_newline();
    debugPrintHaspHeader(NULL);
    debug_newline();

    LOG_INFO(TAG_DEBG, F("Environment: " PIOENV));
    LOG_INFO(TAG_DEBG, F("Console started"));

    debug_flush();
#else

#if HASP_USE_CONSOLE > 0
    consoleSetup();
#endif

#endif
}

void debugStop()
{}

/* ===== Special Event Processors ===== */

void debugLvglLogEvent(lv_log_level_t level, const char* file, uint32_t line, const char* funcname, const char* descr)
{
#if LV_USE_LOG != 0
    /* used for duplicate detection */
    static const char* last_funcname;
    static uint32_t lastDbgLine;
    // static uint32_t lastDbgFreeMem;

    /* Reduce the number of repeated debug message */
    if(line == lastDbgLine && funcname == last_funcname) return;

    // lv_mem_monitor_t mem_mon;
    // lv_mem_monitor(&mem_mon);

    /* Reduce the number of repeated debug message */
    // if(line != lastDbgLine || mem_mon.free_biggest_size != lastDbgFreeMem) {
    switch(level) {
        case LV_LOG_LEVEL_TRACE:
            LOG_VERBOSE(TAG_LVGL, descr);
            break;
        case LV_LOG_LEVEL_WARN:
            LOG_WARNING(TAG_LVGL, descr);
            break;
        case LV_LOG_LEVEL_ERROR:
            LOG_ERROR(TAG_LVGL, descr);
            break;
        default:
            LOG_TRACE(TAG_LVGL, descr);
    }
    last_funcname = funcname;
    lastDbgLine   = line;
    // lastDbgFreeMem = mem_mon.free_biggest_size;
    //}
#endif
}

// Send the HASP header and version to the output device specified
void debugPrintHaspHeader(Print* output)
{
#ifdef ARDUINO
    if(debugAnsiCodes) output->print(TERM_COLOR_YELLOW);
    output->println();
    output->print(F("\r\n"
                    "        open____ _____ _____ _____\r\n"
                    "          |  |  |  _  |   __|  _  |\r\n"
                    "          |     |     |__   |   __|\r\n"
                    "          |__|__|__|__|_____|__|\r\n"
                    "        Home Automation Switch Plate\r\n"
                    "        Open Hardware edition v"));
    output->println(haspDevice.get_version());
    output->println();
#else
    if(debugAnsiCodes) debug_print(output, TERM_COLOR_YELLOW);
    debug_print(output, F("\r\n"
                          "        open____ _____ _____ _____\r\n"
                          "          |  |  |  _  |   __|  _  |\r\n"
                          "          |     |     |__   |   __|\r\n"
                          "          |__|__|__|__|_____|__|\r\n"
                          "        Home Automation Switch Plate\r\n"
                          "        Open Hardware edition v"));
    debug_print(output, haspDevice.get_version());
    debug_newline(output);
    debug_newline(output);
#endif
}

void debug_get_tag(uint8_t tag, char* buffer)
{
    switch(tag) {
        case TAG_MAIN:
            memcpy_P(buffer, PSTR("MAIN"), 5);
            break;

        case TAG_HASP:
            memcpy_P(buffer, PSTR("HASP"), 5);
            break;

        case TAG_DRVR:
            memcpy_P(buffer, PSTR("DRVR"), 5);
            break;

        case TAG_ATTR:
            memcpy_P(buffer, PSTR("ATTR"), 5);
            break;

        case TAG_MSGR:
            memcpy_P(buffer, PSTR("MSGR"), 5);
            break;

        case TAG_EVENT:
            memcpy_P(buffer, PSTR("EVNT"), 5);
            break;

        case TAG_OOBE:
            memcpy_P(buffer, PSTR("OOBE"), 5);
            break;
        case TAG_HAL:
            memcpy_P(buffer, PSTR("HAL "), 5);
            break;

        case TAG_CONS:
            memcpy_P(buffer, PSTR("UART"), 5);
            break;
        case TAG_DEBG:
            memcpy_P(buffer, PSTR("DBUG"), 5);
            break;
        case TAG_FTP:
            memcpy_P(buffer, PSTR("FTP "), 5);
            break;
        case TAG_TELN:
            memcpy_P(buffer, PSTR("TELN"), 5);
            break;
        case TAG_SYSL:
            memcpy_P(buffer, PSTR("SYSL"), 5);
            break;
        case TAG_TASM:
            memcpy_P(buffer, PSTR("TASM"), 5);
            break;

        case TAG_CONF:
            memcpy_P(buffer, PSTR("CONF"), 5);
            break;
        case TAG_GUI:
            memcpy_P(buffer, PSTR("GUI "), 5);
            break;
        case TAG_TFT:
            memcpy_P(buffer, PSTR("TFT "), 5);
            break;

        case TAG_EPRM:
            memcpy_P(buffer, PSTR("EPRM"), 5);
            break;
        case TAG_FILE:
            memcpy_P(buffer, PSTR("FILE"), 5);
            break;
        case TAG_GPIO:
            memcpy_P(buffer, PSTR("GPIO"), 5);
            break;

        case TAG_TIME:
            memcpy_P(buffer, PSTR("TIME"), 5);
            break;
        case TAG_ETH:
            memcpy_P(buffer, PSTR("ETH "), 5);
            break;
        case TAG_WIFI:
            memcpy_P(buffer, PSTR("WIFI"), 5);
            break;
        case TAG_HTTP:
            memcpy_P(buffer, PSTR("HTTP"), 5);
            break;
        case TAG_MDNS:
            memcpy_P(buffer, PSTR("MDNS"), 5);
            break;
        case TAG_MQTT:
            memcpy_P(buffer, PSTR("MQTT"), 5);
            break;
        case TAG_MQTT_PUB:
            memcpy_P(buffer, PSTR("MQTT PUB"), 9);
            break;
        case TAG_MQTT_RCV:
            memcpy_P(buffer, PSTR("MQTT RCV"), 9);
            break;

        case TAG_OTA:
            memcpy_P(buffer, PSTR("OTA "), 5);
            break;

        case TAG_LVGL:
            memcpy_P(buffer, PSTR("LVGL"), 5);
            break;
        case TAG_LVFS:
            memcpy_P(buffer, PSTR("LVFS"), 5);
            break;
        case TAG_FONT:
            memcpy_P(buffer, PSTR("FONT"), 5);
            break;

        case TAG_CUSTOM:
            memcpy_P(buffer, PSTR("CUST"), 5);
            break;

        default:
            memcpy_P(buffer, PSTR("----"), 5);
            break;
    }
}

static void debugPrintHaspMemory(int level, Print* _logOutput)
{
#ifdef ARDUINO
    size_t maxfree   = haspDevice.get_free_max_block();
    size_t totalfree = haspDevice.get_free_heap();
    uint8_t frag     = haspDevice.get_heap_fragmentation();

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
#endif
}

static void debugPrintLvglMemory(int level, Print* _logOutput)
{
#if LV_MEM_CUSTOM == 0
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

#ifdef ARDUINO
    _logOutput->printf(PSTR("[%5u/%5u%3u]"), mem_mon.free_biggest_size, mem_mon.free_size, mem_mon.frag_pct);
#else
    debug_print(_logOutput, PSTR("[%5u/%5u%3u]"), mem_mon.free_biggest_size, mem_mon.free_size, mem_mon.frag_pct);
#endif // ARDUINO

#endif // LV_MEM_CUSTOM
}

#if defined(ESP32) && defined(HASP_LOG_TASKS)
static void debugPrintTaskName(int level, Print* _logOutput)
{
    debug_print(_logOutput, "[%s%6u]", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
}
#endif

static void debugPrintPriority(int level, Print* _logOutput)
{
    switch(level) {
        case LOG_LEVEL_FATAL ... LOG_LEVEL_ERROR:
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
        case LOG_LEVEL_DEBUG:
            debugSendAnsiCode(F(TERM_COLOR_BLUE), _logOutput);
            break;
        case LOG_LEVEL_OUTPUT:
        default:
            debugSendAnsiCode(F(TERM_COLOR_RESET), _logOutput);
    }
}

void debugPrintPrefix(uint8_t tag, int level, Print* _logOutput)
{
    char buffer[10];
    debug_get_tag(tag, buffer);

#if HASP_USE_SYSLOG > 0
    if(debugSyslogPrefix(tag, level, _logOutput, buffer)) {
        debugPrintHaspMemory(level, _logOutput);
        debugPrintLvglMemory(level, _logOutput);
        return;
    }
#endif // HASP_USE_SYSLOG

    debugSendAnsiCode(F(TERM_CLEAR_LINE), _logOutput);
    debugPrintTimestamp(level, _logOutput);
    debugPrintHaspMemory(level, _logOutput);
    debugPrintLvglMemory(level, _logOutput);
#if defined(ESP32) && defined(HASP_LOG_TASKS)
    debugPrintTaskName(level, _logOutput);
#endif

    if(tag == TAG_MQTT_PUB && level == LOG_LEVEL_NOTICE) {
        debugSendAnsiCode(F(TERM_COLOR_GREEN), _logOutput);
    } else if(tag == TAG_MQTT_RCV && level == LOG_LEVEL_NOTICE) {
        debugSendAnsiCode(F(TERM_COLOR_ORANGE), _logOutput);
    } else {
        debugPrintPriority(level, _logOutput);
    }

#ifdef ARDUINO
    _logOutput->printf(PSTR(" %s: "), buffer);
#else
    debug_print(_logOutput, PSTR(" %s: "), buffer);
#endif
}