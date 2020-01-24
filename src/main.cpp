
#include "ArduinoJson.h"
#include "TFT_eSPI.h"

#include "hasp_conf.h"

#include "hasp_debug.h"
#include "hasp_eeprom.h"
#include "hasp_spiffs.h"
#include "hasp_config.h"
#include "hasp_tft.h"
#include "hasp_gui.h"
//#include "hasp_ota.h"
#include "hasp.h"

#if LV_USE_HASP_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#endif

#if LV_USE_HASP_WIFI
#include "hasp_wifi.h"
#endif

#if LV_USE_HASP_MQTT
#include "hasp_mqtt.h"
#endif

#if LV_USE_HASP_HTTP
#include "hasp_http.h"
#endif

bool isConnected;

void setup()
{
    /* Init Storage */
    eepromSetup();
#if LV_USE_HASP_SPIFFS
    // spiffsSetup();
#endif

    /* Read Config File */
    DynamicJsonDocument settings(1024);
    // configGetConfig(doc);
    // JsonObject settings = doc.as<JsonObject>();
    configSetup(settings);

    if(!settings[F("pins")][F("TFT_BCKL")].isNull()) {
        int8_t pin = settings[F("pins")][F("TFT_BCKL")].as<int8_t>();
#if defined(ARDUINO_ARCH_ESP32)
        if(pin >= 0)
            // configure LED PWM functionalitites
            ledcSetup(0, 5000, 10);
        // attach the channel to the GPIO to be controlled
        ledcAttachPin(pin, 0);
#else
        pinMode(pin, OUTPUT);
#endif
    }

#if LV_USE_HASP_SDCARD
    sdcardSetup();
#endif

    /* Init Graphics */
    TFT_eSPI screen = TFT_eSPI();
    tftSetup(screen, settings[F("tft")]);
    guiSetup(screen, settings[F("gui")]);

    /* Init GUI Application */
    haspSetup(settings[F("hasp")]);

    /* Init Network Services */
#if LV_USE_HASP_WIFI
    wifiSetup(settings[F("wifi")]);

#if LV_USE_HASP_MQTT
    mqttSetup(settings[F("mqtt")]);
#endif

#if LV_USE_HASP_MDNS
    mdnsSetup(settings[F("mdns")]);
#endif

#if LV_USE_HASP_HTTP
    httpSetup(settings[F("http")]);
#endif

    // otaSetup(settings[F("ota")]);
#endif
}

void loop()
{
    /* Storage Loops */
    // eepromLoop();
    // spiffsLoop();
#if LV_USE_HASP_SDCARD
    // sdcardLoop();
#endif

    configLoop();

    /* Graphics Loops */
    // tftLoop();
    guiLoop();

    /* Application Loops */
    // haspLoop();

    /* Network Services Loops */
#if LV_USE_HASP_WIFI > 0
    isConnected = wifiLoop();

#if LV_USE_HASP_MQTT > 0
    mqttLoop(isConnected);
#endif

#if LV_USE_HASP_HTTP > 0
    httpLoop(isConnected);
#endif

#if LV_USE_HASP_MDNS > 0
    mdnsLoop(wifiIsConnected);
#endif

    // otaLoop();
#endif

    delay(5);
}