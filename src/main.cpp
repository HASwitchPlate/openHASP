/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <Arduino.h>
#include "hasp_conf.h" // load first

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp_oobe.h"

#include "hasp/hasp_dispatch.h"
#include "hasp/hasp.h"

#include "net/hasp_network.h"

#include "dev/device.h"

bool isConnected;
uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;

void setup()
{
    haspDevice.pre_setup();

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
#if HASP_USE_CONFIG > 0
    configSetup(); // also runs debugPreSetup(), debugSetup() and debugStart()
#endif

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

#if HASP_USE_CONFIG > 0
    if(!oobeSetup())
#endif
    {
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
    guiStart();
}

void loop()
{
    networkLoop();
    guiLoop();
    haspLoop();

#if HASP_USE_MQTT > 0
    mqttLoop();
#endif // MQTT

#if HASP_USE_TASMOTA_SLAVE > 0
    slaveLoop();
#endif // TASMOTASLAVE

#if HASP_USE_HTTP > 0
    httpLoop();
#endif // HTTP

#if HASP_USE_GPIO > 0
    gpioLoop();
#endif // GPIO

#if HASP_USE_OTA > 0
    otaLoop();
#endif // OTA

#if HASP_USE_MDNS > 0
    mdnsLoop();
#endif // MDNS

#if HASP_USE_TELNET > 0
    telnetLoop(); // Console
#endif            // TELNET

    debugLoop(); // Console
    haspDevice.loop();

    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        /* Runs Every Second */
        haspEverySecond();
        debugEverySecond(); // statusupdate

#if HASP_USE_OTA > 0
        otaEverySecond(); // progressbar
#endif

        /* Runs Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {
            isConnected = networkEvery5Seconds(); // Check connection

#if HASP_USE_HTTP > 0
            // httpEvery5Seconds();
#endif

#if HASP_USE_MQTT > 0
            mqttEvery5Seconds(isConnected);
#endif

#if HASP_USE_GPIO > 0
            //   gpioEvery5Seconds();
#endif

            haspDevice.loop_5s();
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