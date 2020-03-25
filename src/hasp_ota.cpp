#include <Arduino.h>
#include "ArduinoJson.h"
#include <ArduinoOTA.h>

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_dispatch.h"
#include "hasp_ota.h"

#include "hasp_conf.h"
#if HASP_USE_MQTT
#include "hasp_mqtt.h"
#endif

#define F_OTA_URL F("otaurl")

std::string otaUrl           = "http://10.1.0.3";
int8_t otaPrecentageComplete = -1;
int16_t otaPort              = 3232;

void otaProgress()
{
    debugPrintln(String(F("OTA: ")) + (ArduinoOTA.getCommand() == U_FLASH ? F("Firmware") : F("Filesystem")) +
                 F(" update in progress... ") + otaPrecentageComplete + "%");
}

void otaSetup(JsonObject settings)
{
    if(!settings[F_OTA_URL].isNull()) {
        char buffer[128];
        otaUrl = settings[F_OTA_URL].as<String>().c_str();
        sprintf_P(buffer, PSTR("ORA url: %s"), otaUrl.c_str());
        debugPrintln(buffer);
    }

    if(otaPort > 0) {
        ArduinoOTA.onStart([]() {
            if(ArduinoOTA.getCommand() == U_FLASH) {
            } else { // U_SPIFFS
                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            }

            debugPrintln(F("OTA: Start update"));
            dispatchCommand("page 0");
            otaPrecentageComplete = 0;
            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\"");
        });
        ArduinoOTA.onEnd([]() {
            otaPrecentageComplete = 100;
            otaProgress();
            otaPrecentageComplete = -1;
            dispatchPage("0");
            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\\rComplete!\"");
            dispatchReboot(true);
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            if(total != 0) otaPrecentageComplete = progress * 100 / total;

            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\\rProgress: " + String(progress / (total / 100)) + "%\"");
        });
        ArduinoOTA.onError([](ota_error_t error) {
            otaPrecentageComplete = -1;
            debugPrintln(String(F("OTA: ERROR code ")) + String(error));
            if(error == OTA_AUTH_ERROR)
                debugPrintln(F("OTA: ERROR - Auth Failed"));
            else if(error == OTA_BEGIN_ERROR)
                debugPrintln(F("OTA: ERROR - Begin Failed"));
            else if(error == OTA_CONNECT_ERROR)
                debugPrintln(F("OTA: ERROR - Connect Failed"));
            else if(error == OTA_RECEIVE_ERROR)
                debugPrintln(F("OTA: ERROR - Receive Failed"));
            else if(error == OTA_END_ERROR)
                debugPrintln(F("OTA: ERROR - End Failed"));
            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA FAILED\"");
            delay(5000);
            // haspSendCmd("page " + String(nextionActivePage));
        });

#if HASP_USE_MQTT > 0
        ArduinoOTA.setHostname(String(mqttGetNodename()).c_str());
#else
        ArduinoOTA.setHostname(String(mqttGetNodename()).c_str());
#endif
        // ArduinoOTA.setPassword(configPassword);
        ArduinoOTA.setPort(otaPort);

#if ESP32
#if HASP_USE_MDNS > 0
        ArduinoOTA.setMdnsEnabled(true);
#else
        ArduinoOTA.setMdnsEnabled(false);
#endif
        // ArduinoOTA.setTimeout(1000);
#endif
        ArduinoOTA.setRebootOnSuccess(true);

        ArduinoOTA.begin();
        debugPrintln(F("OTA: Over the Air firmware update ready"));
        debugPrintln(F("OTA: Setup Complete"));
    } else {
        debugPrintln(F("OTA: Disabled"));
    }
}

void otaLoop()
{
    ArduinoOTA.handle();
}

void otaEverySecond()
{
    if(otaPrecentageComplete >= 0) otaProgress();
}