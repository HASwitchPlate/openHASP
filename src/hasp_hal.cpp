#include <ESP.h>
#include "hasp_hal.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <rom/rtc.h> // needed to get the ResetInfo

// Compatibility function for ESP8266 getRestInfo
String esp32ResetReason(uint8_t cpuid)
{
    if(cpuid > 1) {
        return F("Invalid CPU id");
    }
    RESET_REASON reason = rtc_get_reset_reason(cpuid);

    String resetReason((char *)0);
    resetReason.reserve(32);

    resetReason += F("CPU");
    resetReason += cpuid;
    resetReason += F(": ");

    switch(reason) {
        case 1:
            resetReason += F("POWERON");
            break; /**<1, Vbat power on reset*/
        case 3:
            resetReason += F("SW");
            break; /**<3, Software reset digital core*/
        case 4:
            resetReason += F("OWDT");
            break; /**<4, Legacy watch dog reset digital core*/
        case 5:
            resetReason += F("DEEPSLEEP");
            break; /**<5, Deep Sleep reset digital core*/
        case 6:
            resetReason += F("SDIO");
            break; /**<6, Reset by SLC module, reset digital core*/
        case 7:
            resetReason += F("TG0WDT_SYS");
            break; /**<7, Timer Group0 Watch dog reset digital core*/
        case 8:
            resetReason += F("TG1WDT_SYS");
            break; /**<8, Timer Group1 Watch dog reset digital core*/
        case 9:
            resetReason += F("RTCWDT_SYS");
            break; /**<9, RTC Watch dog Reset digital core*/
        case 10:
            resetReason += F("INTRUSION");
            break; /**<10, Instrusion tested to reset CPU*/
        case 11:
            resetReason += F("TGWDT_CPU");
            break; /**<11, Time Group reset CPU*/
        case 12:
            resetReason += F("SW_CPU");
            break; /**<12, Software reset CPU*/
        case 13:
            resetReason += F("RTCWDT_CPU");
            break; /**<13, RTC Watch dog Reset CPU*/
        case 14:
            resetReason += F("EXT_CPU");
            break; /**<14, for APP CPU, reseted by PRO CPU*/
        case 15:
            resetReason += F("RTCWDT_BROWN_OUT");
            break; /**<15, Reset when the vdd voltage is not stable*/
        case 16:
            resetReason += F("RTCWDT_RTC");
            break; /**<16, RTC Watch dog reset digital core and rtc module*/
        default:
            resetReason += F("NO_MEAN");
            return resetReason;
    }
    resetReason += F("_RESET");
    return resetReason;
}
#endif

String halGetResetInfo()
{
#if defined(ARDUINO_ARCH_ESP32)
    String resetReason((char *)0);
    resetReason.reserve(64);

    resetReason += String(esp32ResetReason(0));
    resetReason += F(" / ");
    resetReason += String(esp32ResetReason(1));
    return resetReason;
#else
    return ESP.getResetInfo();
#endif
}

uint8_t halGetHeapFragmentation()
{
#if defined(ARDUINO_ARCH_ESP32)
    return (int8_t)(100.00f - (float)ESP.getMaxAllocHeap() * 100.00f / (float)ESP.getFreeHeap());
#else
    return ESP.getHeapFragmentation();
#endif
}