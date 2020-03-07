#include "hasp_conf.h"
#include <Arduino.h>
#include "ArduinoJson.h"

#include "TFT_eSPI.h"

#include "hasp_debug.h"
#include "hasp_spiffs.h"
#include "hasp_config.h"
#include "hasp_tft.h"
#include "hasp_gui.h"
#include "hasp_ota.h"
//#include "hasp_ota.h"
#include "hasp.h"

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

bool isConnected;

void setup()
{
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

    /* Read Config File */
    DynamicJsonDocument settings(1024);
    configSetup(settings);

#if HASP_USE_SDCARD
    sdcardSetup();
#endif

    // debugSetup(settings[F("debug")]);

    /* Init Graphics */
    TFT_eSPI screen = TFT_eSPI();
    guiSetup(screen, settings[F("gui")]);
    tftSetup(screen, settings[F("tft")]);

    /* Init GUI Application */
    haspSetup(settings[F("hasp")]);

    /* Init Network Services */
#if HASP_USE_WIFI
    wifiSetup(settings[F("wifi")]);

#if HASP_USE_MQTT
    mqttSetup(settings[F("mqtt")]);
#endif

#if HASP_USE_TELNET
    telnetSetup(settings[F("telnet")]);
#endif

#if HASP_USE_MDNS
    mdnsSetup(settings[F("mdns")]);
#endif

#if HASP_USE_HTTP
    httpSetup(settings[F("http")]);
#endif

#if HASP_USE_BUTTON
    buttonSetup();
#endif

    otaSetup(settings[F("ota")]);
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
    isConnected = wifiLoop();

#if HASP_USE_MQTT
    mqttLoop(isConnected);
#endif

#if HASP_USE_HTTP
    httpLoop(isConnected);
#endif

#if HASP_USE_TELNET
    telnetLoop(isConnected);
#endif

#if HASP_USE_MDNS
    mdnsLoop(isConnected);
#endif

#if HASP_USE_BUTTON
    buttonLoop();
#endif

    otaLoop(isConnected);
    debugLoop();
#endif

    // delay(1);
}