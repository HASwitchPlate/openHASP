#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"

#include "hasp_debug.h"
#include "hasp_spiffs.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp.h"
#include "hasp_conf.h"

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
#if defined(ARDUINO_ARCH_ESP8266)
    pinMode(D1, OUTPUT);
    pinMode(D2, INPUT_PULLUP);
#endif

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
    DynamicJsonDocument settings(128);
    configSetup(settings[F("debug")]);

    /****************************
     * Apply User Configuration
     ***************************/
    debugSetup(settings[F("debug")]);

    /* Init Graphics */
    // TFT_eSPI screen = TFT_eSPI();
    guiSetup(settings[F("gui")]);

#if HASP_USE_WIFI
    wifiSetup(settings[F("wifi")]);
#endif

    /* Init GUI Application */
    haspSetup(settings[F("hasp")]);

    /* Init Network Services */
#if HASP_USE_WIFI
    wifiSetup(settings[F("wifi")]);

#if HASP_USE_HTTP
    httpSetup(settings[F("http")]);
#endif

#if HASP_USE_MQTT
    mqttSetup(settings[F("mqtt")]);
#endif

#if HASP_USE_TELNET
    telnetSetup(settings[F("telnet")]);
#endif

#if HASP_USE_MDNS
    mdnsSetup(settings[F("mdns")]);
#endif

#if HASP_USE_OTA
    otaSetup(settings[F("ota")]);
#endif

#endif // WIFI

#if HASP_USE_BUTTON
    buttonSetup();
#endif
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
        /* Update counters */
        mainLastLoopTime += 1000;
        mainLoopCounter++;
        if(mainLoopCounter >= 10) {
            mainLoopCounter = 0;
        }

        /* Run Every Second */
#if HASP_USE_OTA
        otaEverySecond();
#endif
        debugEverySecond();

        /* Run Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {
            httpEvery5Seconds();
            isConnected = wifiEvery5Seconds();
            mqttEvery5Seconds(isConnected);
        }
    }

    delay(3);
}