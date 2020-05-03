#include "hasp_conf.h" // load first
#include <Arduino.h>

#include "hasp_conf.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp.h"
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
    eepromSetup(); // Don't start at boot, only at write
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

#if HASP_USE_GPIO
    guiSetup();
#endif

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

#if HASP_USE_ETHERNET
    ethernetSetup();
#endif

#if HASP_USE_TASMOTA_SLAVE
    slaveSetup();
#endif

    mainLastLoopTime = millis() - 1000; // reset loop counter
}

void loop()
{
    /* Storage Loops */
/*
#if HASP_USE_EEPROM
    // eepromLoop(); // Not used
#endif

#if HASP_USE_SPIFFS
    // spiffsLoop(); // Not used
#endif

#if HASP_USE_SDCARD
    // sdcardLoop(); // Not used
#endif

    // configLoop();  // Not used
*/

    /* Graphics Loops */
    // tftLoop();
    guiLoop();
    /* Application Loops */
    // haspLoop();

#if HASP_USE_GPIO
    gpioLoop();
#endif

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

#if HASP_USE_OTA
    otaLoop();
#endif // OTA

#endif // WIFI

#if HASP_USE_ETHERNET
    ethernetLoop();
#endif

#if HASP_USE_TASMOTA_SLAVE
    slaveLoop();
#endif // TASMOTASLAVE

    // digitalWrite(HASP_OUTPUT_PIN, digitalRead(HASP_INPUT_PIN)); // sets the LED to the button's value

    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        /* Run Every Second */
#if HASP_USE_OTA
        otaEverySecond();
#endif
        debugEverySecond();

        /* Run Every 5 Seconds */
#if HASP_USE_WIFI
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {
            isConnected = wifiEvery5Seconds();
#if HASP_USE_HTTP
            httpEvery5Seconds();
#endif
#if HASP_USE_MQTT
            mqttEvery5Seconds(isConnected);
#endif
        }
#endif // Wifi

        /* Reset loop counter every 10 seconds */
        if(mainLoopCounter >= 9) {
            mainLoopCounter = 0;
        } else {
            mainLoopCounter++;
        }
        mainLastLoopTime += 1000;
    }

    delay(3);
}