#include "hasp_conf.h" // load first
#include <Arduino.h>

#include "hasp_debug.h"
#include "hasp_spiffs.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp.h"
#include "hasp_conf.h"
#include "hasp_oobe.h"
#include "hasp_gpio.h"

bool isConnected;
uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;

void setup()
{

    /****************************
     * Constant initialzations
     ***************************/

    /* Init Storage */
#if HASP_USE_EEPROM
//    eepromSetup(); // Don't start at boot, only at write
#endif

#if HASP_USE_SPIFFS
    spiffsSetup();
#endif

#if HASP_USE_SDCARD
    sdcardSetup();
#endif

    /****************************
     * Read & Apply User Configuration
     ***************************/
    configSetup();

    /****************************
     * Apply User Configuration
     ***************************/
    debugSetup();
    gpioSetup();
    guiSetup();

#if HASP_USE_WIFI
    wifiSetup();
#endif
    oobeSetup();
    haspSetup();

#if HASP_USE_WIFI

#if HASP_USE_HTTP
    httpSetup();
#endif

#if HASP_USE_MQTT
    mqttSetup();
#endif

#if HASP_USE_TELNET
    telnetSetup();
#endif

#if HASP_USE_MDNS
    mdnsSetup();
#endif

#if HASP_USE_OTA
    otaSetup();
#endif

#endif // WIFI

#if HASP_USE_BUTTON
    buttonSetup();
#endif

#if HASP_USE_TASMOTA_SLAVE
    slaveSetup();
#endif

    mainLastLoopTime = millis() - 1000; // reset loop counter
}

void loop()
{
    /* Storage Loops */
#if HASP_USE_EEPROM
    eepromLoop();
#endif
    // spiffsLoop();
#if HASP_USE_SDCARD
    // sdcardLoop();
#endif

    // configLoop();

    /* Graphics Loops */
    // tftLoop();
    guiLoop();

    /* Application Loops */
    // haspLoop();

    /* Network Services Loops */
#if HASP_USE_WIFI

#if HASP_USE_MQTT
    mqttLoop();
#endif // MQTT

#if HASP_USE_HTTP
    httpLoop();
#endif // HTTP

#if HASP_USE_TELNET
    telnetLoop();
#endif // TELNET

#if HASP_USE_MDNS
    mdnsLoop();
#endif // MDNS

#if HASP_USE_BUTTON
    buttonLoop();
#endif // BUTTON

#if HASP_USE_OTA
    otaLoop();
#endif // OTA

#endif // WIFI

#if HASP_USE_TASMOTA_SLAVE
    slaveLoop();
#endif // TASMOTASLAVE

    // Every Second Loop
    if(millis() - mainLastLoopTime >= 1000) {
        /* Run Every Second */
#if HASP_USE_OTA
        otaEverySecond();
#endif
        debugEverySecond();

        /* Run Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 4) {
#if HASP_USE_HTTP
            httpEvery5Seconds();
#endif

#if HASP_USE_WIFI
            isConnected = wifiEvery5Seconds();

#if HASP_USE_MQTT
            mqttEvery5Seconds(isConnected);
#endif

#endif
        }

        /* Update counters */
        mainLastLoopTime += 1000;
        mainLoopCounter++;
        if(mainLoopCounter >= 10) {
                    mainLoopCounter = 0;
        }
    }

    delay(3);
}