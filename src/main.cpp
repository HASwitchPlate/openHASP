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

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#endif

#if HASP_USE_EEPROM
#include "hasp_eeprom.h"
#endif

#if HASP_USE_WIFI
#include "hasp_wifi.h"
#endif

#if HASP_USE_MQTT
#include "hasp_mqtt.h"
#endif

#if HASP_USE_HTTP
#include "hasp_http.h"
#endif

#if HASP_USE_TELNET
#include "hasp_telnet.h"
#endif

#if HASP_USE_MDNS
#include "hasp_mdns.h"
#endif

#if HASP_USE_BUTTON
#include "hasp_button.h"
#endif

#if HASP_USE_OTA
#include "hasp_ota.h"
#endif

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
    eepromSetup();
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
    wifiSetup();

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

    mainLastLoopTime = millis() - 1000;
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

    // Every Second Loop
    if(millis() - mainLastLoopTime >= 1000) {
        /* Run Every Second */
#if HASP_USE_OTA
        otaEverySecond();
#endif
        debugEverySecond();

        /* Run Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 4) {
            httpEvery5Seconds();
            isConnected = wifiEvery5Seconds();
            mqttEvery5Seconds(isConnected);
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