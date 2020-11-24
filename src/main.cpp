/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h" // load first
#include <Arduino.h>

#include "hasp_conf.h"

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp_oobe.h"
#include "hasp_dispatch.h"
#include "hasp_network.h"
#include "hasp.h"

bool isConnected;
uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;

void setup()
{
    /****************************
     * Storage initializations
     ***************************/
#if HASP_USE_EEPROM > 0
    eepromSetup(); // Don't start at boot, only at write
#endif

    // #if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    //     filesystemSetup();  // FS mount is done in configSetup()
    // #endif

    // #if HASP_USE_SDCARD > 0
    //     sdcardSetup();
    // #endif

    /****************************
     * Read & Apply User Configuration
     ***************************/
    configSetup(); // also runs debugPreSetup(), debugSetup() and debugStart()

    dispatchSetup();
    guiSetup();
    debugSetup(); // Init the console
#if HASP_USE_GPIO > 0
    gpioSetup();
#endif

    /****************************
     * Apply User Configuration
     ***************************/

#if HASP_USE_MQTT > 0
    mqttSetup(); // Load Hostname before starting WiFi
#endif

#if HASP_USE_WIFI > 0 || HASP_USE_ETHERNET > 0
    networkSetup();
#endif

    if(!oobeSetup()) {
        haspSetup();
    }

#if HASP_USE_MDNS > 0
    mdnsSetup();
#endif

#if HASP_USE_OTA > 0
    otaSetup();
#endif

#if HASP_USE_HTTP > 0
    httpSetup();
#endif

#if HASP_USE_TELNET > 0
    telnetSetup();
#endif

#if HASP_USE_TASMOTA_SLAVE > 0
    slaveSetup();
#endif

    mainLastLoopTime = millis() - 1000; // reset loop counter
    delay(250);
}

void loop()
{
    /* Storage Loops */
    /*
    #if HASP_USE_EEPROM>0
        // eepromLoop(); // Not used
    #endif

    #if HASP_USE_SPIFFS>0
        // spiffsLoop(); // Not used
    #endif

    #if HASP_USE_SDCARD>0
        // sdcardLoop(); // Not used
    #endif

        // configLoop();  // Not used
    */

    /* Graphics Loops */
    // tftLoop();
    guiLoop();

    /* Application Loops */
    // haspLoop();
    debugLoop();

#if HASP_USE_GPIO > 0
    gpioLoop();
#endif // GPIO

    /* Network Services Loops */
#if HASP_USE_ETHERNET > 0
    ethernetLoop();
#endif // ETHERNET

#if HASP_USE_MQTT > 0
    mqttLoop();
#endif // MQTT

#if HASP_USE_HTTP > 0
    httpLoop();
#endif // HTTP

#if HASP_USE_MDNS > 0
    mdnsLoop();
#endif // MDNS

#if HASP_USE_OTA > 0
    otaLoop();
#endif // OTA

#if HASP_USE_TELNET > 0
    telnetLoop();
#endif // TELNET

#if HASP_USE_TASMOTA_SLAVE > 0
    slaveLoop();
#endif // TASMOTASLAVE

    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        /* Runs Every Second */
        guiEverySecond();   // sleep timer
        debugEverySecond(); // statusupdate
#if HASP_USE_OTA > 0
        otaEverySecond(); // progressbar
#endif

        /* Runs Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {
#if HASP_USE_WIFI > 0
            isConnected = wifiEvery5Seconds();
#endif

#if HASP_USE_ETHERNET > 0
            isConnected = ethernetEvery5Seconds();
#endif

#if HASP_USE_HTTP > 0
            // httpEvery5Seconds();
#endif

#if HASP_USE_MQTT > 0
            mqttEvery5Seconds(isConnected);
#endif
        }

        /* Reset loop counter every 10 seconds */
        if(mainLoopCounter >= 9) {
            mainLoopCounter = 0;
        } else {
            mainLoopCounter++;
        }
        mainLastLoopTime += 1000;
    }

#ifdef ARDUINO_ARCH_ESP8266
    delay(2);
#else
    delay(6);
#endif
}