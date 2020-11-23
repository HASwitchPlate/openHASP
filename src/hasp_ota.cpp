/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_conf.h"

#include "hasp_debug.h"
#include "hasp_dispatch.h"
#include "hasp_ota.h"
#include "hasp.h"

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
std::string otaUrl           = "http://ota.netwize.be";
int16_t otaPort              = HASP_OTA_PORT;
int8_t otaPrecentageComplete = -1;

static inline void otaProgress(void)
{
    Log.verbose(TAG_OTA, F("%s update in progress... %3u%"),
                (ArduinoOTA.getCommand() == U_FLASH ? PSTR("Firmware") : PSTR("Filesystem")), otaPrecentageComplete);
}

void otaSetup(void)
{
    if(strlen(otaUrl.c_str())) {
        Log.trace(TAG_OTA, otaUrl.c_str());
    }

    if(otaPort > 0) {
        ArduinoOTA.onStart([]() {
            if(ArduinoOTA.getCommand() == U_FLASH) {
            } else { // U_SPIFFS
                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            }

            Log.notice(TAG_OTA, F("Starting OTA update"));
            haspProgressVal(0);
            haspProgressMsg(F("Firmware Update"));
            otaPrecentageComplete = 0;
        });
        ArduinoOTA.onEnd([]() {
            otaPrecentageComplete = 100;
            Log.notice(TAG_OTA, F("OTA update complete"));
            haspProgressVal(100);
            haspProgressMsg(F("Applying Firmware & Reboot"));
            otaProgress();
            otaPrecentageComplete = -1;
            // setup();
            dispatchReboot(true);
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            if(total != 0) {
                otaPrecentageComplete = progress * 100 / total;
                haspProgressVal(otaPrecentageComplete);
            }
        });
        ArduinoOTA.onError([](ota_error_t error) {
            char buffer[16];
            switch(error) {
                case OTA_AUTH_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Auth"));
                    break;
                case OTA_BEGIN_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Begin"));
                    break;
                case OTA_CONNECT_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Connect"));
                    break;
                case OTA_RECEIVE_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Receive"));
                    break;
                case OTA_END_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("End"));
                    break;
                default:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Something"));
            }

            otaPrecentageComplete = -1;
            Log.error(TAG_OTA, F("%s failed (%s)"), buffer, error);
            haspProgressMsg(F("ESP OTA FAILED"));
            // delay(5000);
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
        ArduinoOTA.setRebootOnSuccess(false); // We do that ourselves

        ArduinoOTA.begin();
        Log.trace(TAG_OTA, F("Over the Air firmware update ready"));
    } else {
        Log.warning(TAG_OTA, F("Disabled"));
    }
}

void IRAM_ATTR otaLoop(void)
{
    ArduinoOTA.handle();
}

void otaEverySecond(void)
{
    if(otaPrecentageComplete >= 0) otaProgress();
}

void otaHttpUpdate(const char * espOtaUrl)
{ // Update ESP firmware from HTTP
  // nextionSendCmd("page 0");
  // nextionSetAttr("p[0].b[1].txt", "\"HTTP update\\rstarting...\"");
#if HASP_USE_MDNS > 0
    mdnsStop(); // Keep mDNS responder from breaking things
#endif

#if defined(ARDUINO_ARCH_ESP8266)

    // ESPhttpUpdate.onStart(update_started);
    // ESPhttpUpdate.onEnd(update_finished);
    // ESPhttpUpdate.onProgress(update_progress);
    // ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return returnCode = ESPhttpUpdate.update(otaClient, espOtaUrl);

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            Log.error(TAG_FWUP, "FWUP: HTTP_UPDATE_FAILED error %d %s", ESPhttpUpdate.getLastError(),
                      ESPhttpUpdate.getLastErrorString().c_str());
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rFAILED\"");
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.notice(TAG_FWUP, F("HTTP_UPDATE_NO_UPDATES"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rNo update\"");
            break;

        case HTTP_UPDATE_OK:
            Log.notice(TAG_FWUP, F("HTTP_UPDATE_OK"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rcomplete!\\r\\rRestarting.\"");
            dispatchReboot(true);
            delay(5000);
    }

#else
    t_httpUpdate_return returnCode = httpUpdate.update(otaClient, espOtaUrl);

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            Log.error(TAG_FWUP, F("HTTP_UPDATE_FAILED error %i %s"), httpUpdate.getLastError(),
                      httpUpdate.getLastErrorString().c_str());
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rFAILED\"");
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.notice(TAG_FWUP, F("HTTP_UPDATE_NO_UPDATES"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rNo update\"");
            break;

        case HTTP_UPDATE_OK:
            Log.notice(TAG_FWUP, F("HTTP_UPDATE_OK"));
            // nextionSetAttr("p[0].b[1].txt", "\"HTTP Update\\rcomplete!\\r\\rRestarting.\"");
            dispatchReboot(true);
            delay(5000);
    }

#endif

#if HASP_USE_MDNS > 0
    mdnsStart();
#endif
    // nextionSendCmd("page " + String(nextionActivePage));
}
#endif