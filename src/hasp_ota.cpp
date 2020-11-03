#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_debug.h"
#include "hasp_dispatch.h"
#include "hasp_ota.h"

#include "hasp_conf.h"

#if HASP_USE_MQTT>0
#include "hasp_mqtt.h"
#endif

#if HASP_USE_MDNS>0
#include "hasp_mdns.h"
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#else
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#endif

static WiFiClient otaClient;

#define F_OTA_URL F("otaurl")

std::string otaUrl           = "http://10.1.0.3";
int8_t otaPrecentageComplete = -1;

int16_t otaPort              = HASP_OTA_PORT;

void otaProgress()
{
    Log.verbose(F("OTA: %s update in progress... %3u%"),
                (ArduinoOTA.getCommand() == U_FLASH ? PSTR("Firmware") : PSTR("Filesystem")), otaPrecentageComplete);
}

void otaSetup()
{
    if(strlen(otaUrl.c_str())) {
        Log.verbose(F("OTA: %s"), otaUrl.c_str());
    }

    if(otaPort > 0) {
        ArduinoOTA.onStart([]() {
            if(ArduinoOTA.getCommand() == U_FLASH) {
            } else { // U_SPIFFS
                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            }

            Log.notice(F("OTA: Start update"));
            // dispatchPage("0");
            otaPrecentageComplete = 0;
            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\"");
        });
        ArduinoOTA.onEnd([]() {
            otaPrecentageComplete = 100;
            otaProgress();
            otaPrecentageComplete = -1;
            // dispatchPage("0");
            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\\rComplete!\"");
            dispatchReboot(true);
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            if(total != 0) otaPrecentageComplete = progress * 100 / total;

            // haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\\rProgress: " + String(progress / (total / 100)) + "%\"");
        });
        ArduinoOTA.onError([](ota_error_t error) {
            otaPrecentageComplete = -1;
            Log.error(F("OTA: ERROR code %u"), error);
            if(error == OTA_AUTH_ERROR)
                Log.error(F("OTA: ERROR - Auth Failed"));
            else if(error == OTA_BEGIN_ERROR)
                Log.error(F("OTA: ERROR - Begin Failed"));
            else if(error == OTA_CONNECT_ERROR)
                Log.error(F("OTA: ERROR - Connect Failed"));
            else if(error == OTA_RECEIVE_ERROR)
                Log.error(F("OTA: ERROR - Receive Failed"));
            else if(error == OTA_END_ERROR)
                Log.error(F("OTA: ERROR - End Failed"));
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
        Log.notice(F("OTA: Over the Air firmware update ready"));
    } else {
        Log.notice(F("OTA: Disabled"));
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

void otaHttpUpdate(const char * espOtaUrl)
{ // Update ESP firmware from HTTP
    // nextionSendCmd("page 0");
    // nextionSetAttr("p[0].b[1].txt", "\"HTTP update\\rstarting...\"");
    mdnsStop(); // Keep mDNS responder from breaking things

#if defined(ARDUINO_ARCH_ESP8266)

    // ESPhttpUpdate.onStart(update_started);
    // ESPhttpUpdate.onEnd(update_finished);
    // ESPhttpUpdate.onProgress(update_progress);
    // ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return returnCode = ESPhttpUpdate.update(otaClient, espOtaUrl);

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            Log.error("FWUP: HTTP_UPDATE_FAILED error %d %s", ESPhttpUpdate.getLastError(),
                      ESPhttpUpdate.getLastErrorString().c_str());
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rFAILED\"");
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.notice(F("FWUP: HTTP_UPDATE_NO_UPDATES"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rNo update\"");
            break;

        case HTTP_UPDATE_OK:
            Log.notice(F("FWUP: HTTP_UPDATE_OK"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rcomplete!\\r\\rRestarting.\"");
            dispatchReboot(true);
            delay(5000);
    }

#else
    t_httpUpdate_return returnCode = httpUpdate.update(otaClient, espOtaUrl);

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            Log.error("FWUP: HTTP_UPDATE_FAILED error %i %s", httpUpdate.getLastError(),
                      httpUpdate.getLastErrorString().c_str());
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rFAILED\"");
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.notice(F("FWUP: HTTP_UPDATE_NO_UPDATES"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rNo update\"");
            break;

        case HTTP_UPDATE_OK:
            Log.notice(F("FWUP: HTTP_UPDATE_OK"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rcomplete!\\r\\rRestarting.\"");
            dispatchReboot(true);
            delay(5000);
    }

#endif
    mdnsStart();
    // nextionSendCmd("page " + String(nextionActivePage));
}
#endif