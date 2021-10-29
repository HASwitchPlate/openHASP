/*
    _   ___ ___  _   _ ___ _  _  ___  _    ___   ___
   /_\ | _ \   \| | | |_ _| \| |/ _ \| |  / _ \ / __|
  / _ \|   / |) | |_| || || .` | (_) | |_| (_) | (_ |
 /_/ \_\_|_\___/ \___/|___|_|\_|\___/|____\___/ \___|

  Log library for Arduino
  version 1.0.3
  https://github.com/thijse/Arduino-Log

Licensed under the MIT License <http://opensource.org/licenses/MIT>.

*/

#ifndef LOGGING_H
#define LOGGING_H
#include <inttypes.h>
#include <stdarg.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif
//#include "StringStream.h"
typedef void (*printfunction)(uint8_t tag, int level, Print*);

//#include <stdint.h>
//#include <stddef.h>
// *************************************************************************
//  Uncomment line below to fully disable logging, and reduce project size
// ************************************************************************
//#define DISABLE_LOGGING

#define LOG_LEVEL_SILENT -1

#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ALERT 1
#define LOG_LEVEL_CRITICAL 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_WARNING 4
#define LOG_LEVEL_NOTICE 5
#define LOG_LEVEL_INFO 5
#define LOG_LEVEL_TRACE 6
#define LOG_LEVEL_VERBOSE 7
#define LOG_LEVEL_DEBUG 8
#define LOG_LEVEL_OUTPUT 9

//#define CR "\n"
#define LOGGING_VERSION 1_0_3

/**
 * Logging is a helper class to output informations over
 * RS232. If you know log4j or log4net, this logging class
 * is more or less similar ;-) <br>
 * Different loglevels can be used to extend or reduce output
 * All methods are able to handle any number of output parameters.
 * All methods print out a formated string (like printf).<br>
 * To reduce output and program size, reduce loglevel.
 *
 * Output format string can contain below wildcards. Every wildcard
 * must be start with percent sign (\%)
 *
 * ---- Wildcards
 *
 * %s	replace with an string (char*)
 * %c	replace with an character
 * %d	replace with an integer value
 * %l	replace with an long value
 * %x	replace and convert integer value into hex
 * %X	like %x but combine with 0x123AB
 * %b	replace and convert integer value into binary
 * %B	like %x but combine with 0b10100011
 * %t	replace and convert boolean value into "t" or "f"
 * %T	like %t but convert into "true" or "false"
 *
 * ---- Loglevels
 *
 * 0 - LOG_LEVEL_SILENT     no output
 * 1 - LOG_LEVEL_FATAL      fatal errors
 * 2 - LOG_LEVEL_ERROR      all errors
 * 3 - LOG_LEVEL_WARNING    errors and warnings
 * 4 - LOG_LEVEL_NOTICE     errors, warnings and notices
 * 5 - LOG_LEVEL_TRACE      errors, warnings, notices, traces
 * 6 - LOG_LEVEL_VERBOSE    subprocesses and steps
 * 7 - LOG_LEVEL_DEBUG      all
 */

class Logging {
  public:
    /**
     * default Constructor
     */
    Logging()
#ifndef DISABLE_LOGGING
    //   : _level(LOG_LEVEL_SILENT), _showLevel(true)
#endif
    {}

    /**
     * Initializing, must be called as first. Note that if you use
     * this variant of Init, you need to initialize the baud rate
     * yourself, if printer happens to be a serial port.
     *
     * \param level - logging levels <= this will be logged.
     * \param printer - place that logging output will be sent to.
     * \return void
     *
     */
    void begin(int level, bool showLevel = true);

    /**
     * Register up to 3 printers to a certain slot
     *
     * \param slot - index of the printer to register.
     * \param printer - place that logging output will be sent to.
     * \return void
     *
     */
    void registerOutput(uint8_t slot, Print* logOutput, int level, bool showLevel);

    /**
     * Unregister the printer in a certain slot
     *
     * \param slot - index of the printer to register.
     * \return void
     *
     */
    void unregisterOutput(uint8_t slot);

    /**
     * Set the log level.
     *
     * \param level - The new log level.
     * \return void
     */
    void setLevel(uint8_t slot, int level);

    /**
     * Get the log level.
     *
     * \return The current log level.
     */
    int getLevel(uint8_t slot) const;

    /**
     * Set whether to show the log level.
     *
     * \param showLevel - true if the log level should be shown for each log
     *                    false otherwise.
     * \return void
     */
    void setShowLevel(uint8_t slot, bool showLevel);

    /**
     * Get whether the log level is shown during logging
     *
     * \return true if the log level is be shown for each log
     *         false otherwise.
     */
    bool getShowLevel(uint8_t slot) const;

    /**
     * Sets a function to be called before each log command.
     *
     * \param f - The function to be called
     * \return void
     */
    void setPrefix(printfunction f);

    /**
     * Sets a function to be called after each log command.
     *
     * \param f - The function to be called
     * \return void
     */
    void setSuffix(printfunction f);

    /**
     * Output a fatal error message. Output message contains
     * F: followed by original message
     * Fatal error messages are printed out at
     * loglevels >= LOG_LEVEL_FATAL
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void fatal(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_FATAL, msg, args...);
#endif
    }

    /**
     * Output an error message. Output message contains
     * E: followed by original message
     * Error messages are printed out at
     * loglevels >= LOG_LEVEL_ERROR
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void error(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_ERROR, msg, args...);
#endif
    }

    /**
     * Output a warning message. Output message contains
     * W: followed by original message
     * Warning messages are printed out at
     * loglevels >= LOG_LEVEL_WARNING
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void warning(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_WARNING, msg, args...);
#endif
    }

    /**
     * Output a notice message. Output message contains
     * N: followed by original message
     * Notice messages are printed out at
     * loglevels >= LOG_LEVEL_NOTICE
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void notice(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_NOTICE, msg, args...);
#endif
    }

    /**
     * Output a trace message. Output message contains
     * N: followed by original message
     * Trace messages are printed out at
     * loglevels >= LOG_LEVEL_TRACE
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void trace(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_TRACE, msg, args...);
#endif
    }

    /**
     * Output a verbose message. Output message contains
     * V: followed by original message
     * Verbose messages are printed out at
     * loglevels >= LOG_LEVEL_VERBOSE
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void verbose(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_VERBOSE, msg, args...);
#endif
    }

    /**
     * Output a debug message. Output message contains
     * V: followed by original message
     * Debug messages are printed out at
     * loglevels >= LOG_LEVEL_DEBUG
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void debug(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_DEBUG, msg, args...);
#endif
    }

    /**
     * Output a normal message. Output message contains
     * V: followed by original message
     * Output messages are always printed out
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
    template <class T, typename... Args> void output(uint8_t tag, T msg, Args... args)
    {
#ifndef DISABLE_LOGGING
        printLevel(tag, LOG_LEVEL_OUTPUT, msg, args...);
#endif
    }

  private:
    void print(Print* logOutput, const char* format, va_list args);

    void print(Print* logOutput, const __FlashStringHelper* format, va_list args);

    void printFormat(Print* logOutput, const char format, va_list* args);

    template <class T> void printLevel(uint8_t tag, int level, T msg, ...)
    {
#ifndef DISABLE_LOGGING

        for(int i = 0; i < 3; i++) {
            if(_logOutput[i] == NULL || level > _level[i]) continue;

            if(_prefix != NULL) {
                _prefix(tag, level, _logOutput[i]);
            }

            va_list args;
            va_start(args, msg);
            print(_logOutput[i], msg, args);

            if(_suffix != NULL) {
                _suffix(tag, level, _logOutput[i]);
            }
        }

#endif
    }

#ifndef DISABLE_LOGGING
    int _level[3];
    bool _showLevel[3];
    Print* _logOutput[3] = {NULL,NULL,NULL};

    printfunction _prefix = NULL;
    printfunction _suffix = NULL;
#endif
};

extern Logging Log;
#endif