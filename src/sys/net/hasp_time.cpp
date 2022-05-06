#include <time.h>
#include <sys/time.h>

#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "Preferences.h"
#include "nvs.h"
#include "nvs_flash.h"
#endif

#ifndef MYTZ
#define MYTZ "EST5EDT,M3.2.0/2,M11.1.0"
#endif

#ifndef NTPSERVER1
#define NTPSERVER1 "pool.ntp.org"
#endif

#ifndef NTPSERVER2
#define NTPSERVER2 "time.nist.gov"
#endif

#ifndef NTPSERVER3
#define NTPSERVER3 "time.google.com"
#endif

#if defined(ARDUINO_ARCH_ESP32)
// These strings must be constant and kept in memory
String mytz((char*)0);
String ntp1((char*)0);
String ntp2((char*)0);
String ntp3((char*)0);
#endif

void timeSetup()
{
#if defined(ARDUINO_ARCH_ESP8266)
    LOG_WARNING(TAG_TIME, F("TIMEZONE: %s"), MYTZ);
    configTzTime(MYTZ, NTPSERVER1, NTPSERVER2, NTPSERVER3); // literal string
#endif
#if defined(ARDUINO_ARCH_ESP32)
    Preferences preferences;
    preferences.begin("time", true);

    mytz = preferences.getString("tz", MYTZ);
    ntp1 = preferences.getString("ntp1", NTPSERVER1);
    ntp2 = preferences.getString("ntp2", NTPSERVER2);
    ntp3 = preferences.getString("ntp3", NTPSERVER3);

    LOG_WARNING(TAG_TIME, F("TIMEZONE: %s"), mytz.c_str());
    LOG_WARNING(TAG_TIME, F("NTPSERVER: %s %s %s"), ntp1.c_str(), ntp2.c_str(), ntp3.c_str());

    configTzTime(mytz.c_str(), ntp1.c_str(), ntp2.c_str(), ntp3.c_str());
    preferences.end();
#endif
}

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool timeGetConfig(const JsonObject& settings)
{
    Preferences preferences;
    bool changed = false;

    preferences.begin("time", true);
    settings["tz"]     = preferences.getString("tz", MYTZ);
    settings["ntp"][0] = preferences.getString("ntp1", NTPSERVER1);
    settings["ntp"][1] = preferences.getString("ntp2", NTPSERVER2);
    settings["ntp"][2] = preferences.getString("ntp3", NTPSERVER3);
    preferences.end();

#if ESP_ARDUINO_VERSION_MAJOR >= 2
    nvs_iterator_t it = nvs_entry_find("nvs", "time", NVS_TYPE_ANY);
    while(it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);
        printf("key '%s', type '%d' \n", info.key, info.type);
    };
#endif

    if(changed) configOutput(settings, TAG_TIME);

    return changed;
}

bool timeSetConfig(const JsonObject& settings)
{
    Preferences preferences;
    preferences.begin("time", false);

    configOutput(settings, TAG_TIME);
    bool changed = false;

    char key[16] = "tz";
    changed |= nvsUpdateString(preferences, key, settings[key]);
    changed |= nvsUpdateString(preferences, "ntp1", settings["ntp"][0]);
    changed |= nvsUpdateString(preferences, "ntp2", settings["ntp"][1]);
    changed |= nvsUpdateString(preferences, "ntp3", settings["ntp"][2]);

    preferences.end();
    timeSetup();

    return changed;
}
#endif