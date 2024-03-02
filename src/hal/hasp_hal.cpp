/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_hal.h"
#include "hasp_conf.h"

#if defined(ESP8266)
#include <Esp.h>
#include <ESP8266WiFi.h>
#endif

#if defined(ESP32)
#include <Esp.h>
#include <WiFi.h>
#include "esp_system.h"
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <rom/rtc.h> // needed to get the ResetInfo

// Compatibility function for ESP8266 getRestInfo
// String esp32ResetReason(uint8_t cpuid)
// {
//     if(cpuid > 1) {
//         return F("Invalid CPU id");
//     }
//     RESET_REASON reason = rtc_get_reset_reason(cpuid);

//     String resetReason((char*)0);
//     resetReason.reserve(128);

//     resetReason += F("CPU");
//     resetReason += cpuid;
//     resetReason += F(": ");

//     switch(reason) {
//         case 1:
//             resetReason += F("POWERON");
//             break; /**<1, Vbat power on reset*/
//         case 3:
//             resetReason += F("SW");
//             break; /**<3, Software reset digital core*/
//         case 4:
//             resetReason += F("OWDT");
//             break; /**<4, Legacy watch dog reset digital core*/
//         case 5:
//             resetReason += F("DEEPSLEEP");
//             break; /**<5, Deep Sleep reset digital core*/
//         case 6:
//             resetReason += F("SDIO");
//             break; /**<6, Reset by SLC module, reset digital core*/
//         case 7:
//             resetReason += F("TG0WDT_SYS");
//             break; /**<7, Timer Group0 Watch dog reset digital core*/
//         case 8:
//             resetReason += F("TG1WDT_SYS");
//             break; /**<8, Timer Group1 Watch dog reset digital core*/
//         case 9:
//             resetReason += F("RTCWDT_SYS");
//             break; /**<9, RTC Watch dog Reset digital core*/
//         case 10:
//             resetReason += F("INTRUSION");
//             break; /**<10, Instrusion tested to reset CPU*/
//         case 11:
//             resetReason += F("TGWDT_CPU");
//             break; /**<11, Time Group reset CPU*/
//         case 12:
//             resetReason += F("SW_CPU");
//             break; /**<12, Software reset CPU*/
//         case 13:
//             resetReason += F("RTCWDT_CPU");
//             break; /**<13, RTC Watch dog Reset CPU*/
//         case 14:
//             resetReason += F("EXT_CPU");
//             break; /**<14, for APP CPU, reset by PRO CPU*/
//         case 15:
//             resetReason += F("RTCWDT_BROWN_OUT");
//             break; /**<15, Reset when the vdd voltage is not stable*/
//         case 16:
//             resetReason += F("RTCWDT_RTC");
//             break; /**<16, RTC Watch dog reset digital core and rtc module*/
//         default:
//             resetReason += F("NO_MEAN");
//             return resetReason;
//     }
//     resetReason += F("_RESET");
//     return resetReason;
// }
#endif

// void halRestartMcu(void)
// {
// #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
//     ESP.restart();
// #else
//     NVIC_SystemReset();
// #endif
//     for(;;) {
//     } // halt
// }

// String halGetResetInfo()
// {
// #if defined(ARDUINO_ARCH_ESP32)
//     String resetReason((char*)0);
//     resetReason.reserve(128);

//     resetReason += String(esp32ResetReason(0));
//     resetReason += F(" / ");
//     resetReason += String(esp32ResetReason(1));
//     return resetReason;
// #elif defined(ARDUINO_ARCH_ESP8266)
//     return ESP.getResetInfo();
// #else
//     return "";
// #endif
// }

// String halGetCoreVersion()
// {
// #if defined(ARDUINO_ARCH_ESP32)
//     return String(ESP.getSdkVersion());
// #elif defined(ARDUINO_ARCH_ESP8266)
//     return String(ESP.getCoreVersion());
// #else
//     return String(STM32_CORE_VERSION_MAJOR) + "." + STM32_CORE_VERSION_MINOR + "." + STM32_CORE_VERSION_PATCH;
// #endif
// }

// String halGetChipModel()
// {
//     String model((char*)0);
//     model.reserve(128);

// #if defined(STM32F4xx)
//     model = F("STM32");
// #endif

// #if defined(STM32F4xx)
//     model = F("STM32F4xx");

// #elif defined(ARDUINO_ARCH_ESP8266)
//     model = F("ESP8266");

// #elif defined(ARDUINO_ARCH_ESP32)
//     esp_chip_info_t chip_info;
//     esp_chip_info(&chip_info);

//     model = chip_info.cores;
//     model += F("core ");
//     switch(chip_info.model) {
//         case CHIP_ESP32:
//             model += F("ESP32");
//             break;
// #ifdef CHIP_ESP32S2
//         case CHIP_ESP32S2:
//             model += F("ESP32-S2");
//             break;
// #endif
//         default:
//             model = F("Unknown ESP32");
//     }
//     model += F(" rev");
//     model += chip_info.revision;

// #else
//     model = F("Unknown");
// #endif

//     return model;
// }

/*******************************/
/* Memory Management Functions */

#if defined(STM32F4xx)
#include <malloc.h> // for mallinfo()
#include <unistd.h> // for sbrk()

int freeHighMemory()
{
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}
#endif

/*
extern char *fake_heap_end;   // current heap start
extern char *fake_heap_start;   // current heap end

char* getHeapStart() {
   return fake_heap_start;
}

char* getHeapEnd() {
   return (char*)sbrk(0);
}

char* getHeapLimit() {
   return fake_heap_end;
}

int getMemUsed() { // returns the amount of used memory in bytes
   struct mallinfo mi = mallinfo();
   return mi.uordblks;
}

int getMemFree() { // returns the amount of free memory in bytes
   struct mallinfo mi = mallinfo();
   return mi.fordblks + freeHighMemory();
} */

// size_t halGetMaxFreeBlock()
// {
// #if defined(ARDUINO_ARCH_ESP32)
//     return ESP.getMaxAllocHeap();
// #elif defined(ARDUINO_ARCH_ESP8266)
//     return ESP.getMaxFreeBlockSize();
// #else
//     return freeHighMemory();
// #endif
// }

// size_t halGetFreeHeap(void)
// {
// #if defined(ARDUINO_ARCH_ESP32)
//     return ESP.getFreeHeap();
// #elif defined(ARDUINO_ARCH_ESP8266)
//     return ESP.getFreeHeap();
// #else
//     struct mallinfo chunks = mallinfo();

//     // fordblks
//     //    This is the total size of memory occupied by free (not in use) chunks.

//     return chunks.fordblks + freeHighMemory();
// #endif
// }

// uint8_t halGetHeapFragmentation()
// {
// #if defined(ARDUINO_ARCH_ESP32)
//     return (int8_t)(100.00f - (float)ESP.getMaxAllocHeap() * 100.00f / (float)ESP.getFreeHeap());
// #elif defined(ARDUINO_ARCH_ESP8266)
//     return ESP.getHeapFragmentation();
// #else
//     return (int8_t)(100.00f - (float)freeHighMemory() * 100.00f / (float)halGetFreeHeap());
// #endif
// }

String halGetMacAddress(int start, const char* separator)
{
    byte mac[6];

#if defined(STM32F4xx) || defined(STM32F7xx) || defined(STM32H7xx)
    uint8_t* mac_p = nullptr;
#if HASP_USE_ETHERNET > 0
#if USE_BUILTIN_ETHERNET > 0
    mac_p = Ethernet.MACAddress();
    for(int i = 0; i < 6; i++) mac[i] = *(mac_p + i);
#else
    Ethernet.macAddress(mac);
#endif
#endif
#else
    WiFi.macAddress(mac);
#endif

    String cMac((char*)0);
    cMac.reserve(32);

    for(int i = start; i < 6; ++i) {
        if(mac[i] < 0x10) cMac += "0";
        cMac += String(mac[i], HEX);
        if(i < 5) cMac += separator;
    }
    cMac.toUpperCase();
    return cMac;
}

// uint16_t halGetCpuFreqMHz()
// {
// #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_32)
//     return ESP.getCpuFreqMHz();
// #else
//     return (F_CPU / 1000 / 1000);
// #endif
// }

String halDisplayDriverName()
{
#if defined(ILI9341_DRIVER)
    return F("ILI9341");
#elif defined(ST7735_DRIVER)
    return F("ST7735");
#elif defined(ILI9163_DRIVER)
    return F("ILI9163");
#elif defined(S6D02A1_DRIVER)
    return F("S6D02A1");
#elif defined(ST7796_DRIVER)
    return F("ST7796");
#elif defined(ILI9486_DRIVER)
    return F("ILI9486");
#elif defined(ILI9481_DRIVER)
    return F("ILI9481");
#elif defined(ILI9488_DRIVER)
    return F("ILI9488");
#elif defined(HX8357D_DRIVER)
    return F("HX8357D");
#elif defined(EPD_DRIVER)
    return F("EPD");
#elif defined(ST7789_DRIVER)
    return F("ST7789");
#elif defined(R61581_DRIVER)
    return F("R61581");
#elif defined(ST7789_2_DRIVER)
    return F("ST7789_2");
#elif defined(RM68140_DRIVER)
    return F("RM68140");
#endif
    return F("Unknown");
}

String halGpioName(uint8_t gpio)
{
#if defined(STM32F4xx)
    String ioName;
    uint16_t name = digitalPin[gpio];
    uint8_t num   = name % 16;
    switch(name / 16) {
        case PortName::PortA:
            ioName = F("PA");
            break;
        case PortName::PortB:
            ioName = F("PB");
            break;
#if defined GPIOC_BASE
        case PortName::PortC:
            ioName = F("PC");
            break;
#endif
#if defined GPIOD_BASE
        case PortName::PortD:
            ioName = F("PD");
            break;
#endif
#if defined GPIOE_BASE
        case PortName::PortE:
            ioName = F("PE");
            break;
#endif
#if defined GPIOF_BASE
        case PortName::PortF:
            ioName = F("PF");
            break;
#endif
#if defined GPIOG_BASE
        case PortName::PortG:
            ioName = F("PG");
            break;
#endif
#if defined GPIOH_BASE
        case PortName::PortH:
            ioName = F("PH");
            break;
#endif
#if defined GPIOI_BASE
        case PortName::PortI:
            ioName = F("PI");
            break;
#endif
#if defined GPIOJ_BASE
        case PortName::PortJ:
            ioName = F("PJ");
            break;
#endif
#if defined GPIOK_BASE
        case PortName::PortK:
            ioName = F("PK");
            break;
#endif
#if defined GPIOZ_BASE
        case PortName::PortZ:
            ioName = F("PZ");
            break;
#endif
        default:
            ioName = F("P?");
    }
    ioName += num;
    ioName += F(" (io");
    ioName += gpio;
    ioName += F(")");
    return ioName;
#endif

// For ESP32 pin labels on boards use the GPIO number
#ifdef ARDUINO_ARCH_ESP32
    return /*String(F("gpio")) +*/ String(gpio);
#endif

#ifdef ARDUINO_ARCH_ESP8266
    // For ESP8266 the pin labels are not the same as the GPIO number
    // These are for the NodeMCU pin definitions:
    //        GPIO       Dxx
    switch(gpio) {
        case 16:
            return F("D0");
        case 5:
            return F("D1");
        case 4:
            return F("D2");
        case 0:
            return F("D3");
        case 2:
            return F("D4");
        case 14:
            return F("D5");
        case 12:
            return F("D6");
        case 13:
            return F("D7");
        case 15:
            return F("D8");
        case 3:
            return F("TX");
        case 1:
            return F("RX");
        // case 9:
        //     return F("D11");
        // case 10:
        //     return F("D12");
        default:
            return F("D?"); // Invalid pin
    }
#endif
}