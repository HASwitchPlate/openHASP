/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MACRO_H
#define HASP_MACRO_H

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#if HASP_TARGET_PC
#define HASP_RANDOM(x) rand() % x
#elif HASP_TARGET_ARDUINO
#define HASP_RANDOM(x) random(x)
#else
#define HASP_RANDOM(x) random() % x
#endif

#if HASP_TARGET_PC
#define LOG_OUTPUT(x, ...) printf(__VA_ARGS__)
#else

#ifndef HASP_LOG_LEVEL
#define HASP_LOG_LEVEL LOG_LEVEL_TRACE
#endif

#define LOG_OUTPUT(...) Log.output(...)

#if HASP_LOG_LEVEL >= LOG_LEVEL_FATAL
#define LOG_FATAL(...)                                                                                                 \
    Log.fatal(__VA_ARGS__);                                                                                            \
    while(true) {                                                                                                      \
    }
#else
#define LOG_FATAL(...)                                                                                                 \
    do {                                                                                                               \
    } while(0)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_ALERT
#define LOG_ALERT(...) Log.alert(__VA_ARGS__)
#else
#define LOG_ALERT(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_CRITICAL
#define LOG_CRITICAL(...) Log.critical(__VA_ARGS__)
#else
#define LOG_CRITICAL(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(...) Log.error(__VA_ARGS__)
#else
#define LOG_ERROR(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARNING(...) Log.warning(__VA_ARGS__)
#else
#define LOG_WARNING(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(...) Log.notice(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_TRACE
#define LOG_TRACE(...) Log.trace(__VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(...) Log.verbose(__VA_ARGS__)
#else
#define LOG_VERBOSE(...)
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...) Log.debug(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#endif

#endif // HASP_MACRO_H
