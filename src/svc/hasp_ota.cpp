/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

    #include "hasp_conf.h"

    #include "hasp_debug.h"
    #include "hasp_ota.h"

    #include "../hasp/hasp_dispatch.h"
    #include "../hasp/hasp.h"

    #if defined(ARDUINO_ARCH_ESP8266)
        #include <ESP8266HTTPClient.h>
        #include <ESP8266httpUpdate.h>
        #include <ESP8266WiFi.h>
    #else
        #include <HTTPClient.h>
        #include <HTTPUpdate.h>
        #include <WiFi.h>
    #endif

    #include <ArduinoOTA.h>

static WiFiClient otaClient;
std::string otaUrl           = "http://ota.netwize.be";
int16_t otaPort              = HASP_OTA_PORT;
int8_t otaPrecentageComplete = -1;

bool otaUpdateCheck()
{ // firmware update check
    WiFiClientSecure wifiUpdateClientSecure;
    HTTPClient updateClient;
    Log.notice(TAG_OTA, F("UPDATE: Checking update URL: %s"), otaUrl.c_str());

   // wifiUpdateClientSecure.setInsecure();
   // wifiUpdateClientSecure.setBufferSizes(512, 512);
    updateClient.begin(wifiUpdateClientSecure, otaUrl.c_str());

    int httpCode = updateClient.GET(); // start connection and send HTTP header
    if(httpCode != HTTP_CODE_OK) {
        Log.error(TAG_OTA, F("Update check failed: %s"), updateClient.errorToString(httpCode).c_str());
        return false;
    }

    DynamicJsonDocument updateJson(1024);
    DeserializationError jsonError = deserializeJson(updateJson, updateClient.getString());
    updateClient.end();

    if(jsonError) { // Couldn't parse the returned JSON, so bail
        Log.error(TAG_OTA, F("JSON parsing failed: %s"), jsonError.c_str());
        // mqttClient.publish(mqttStateJSONTopic,
        //                    String(F("{\"event\":\"jsonError\",\"event_source\":\"updateCheck()\",\"event_description\":"
        //                             "\"Failed to parse incoming JSON command with error\"")) +
        //                        String(jsonError.c_str()));
        return false;
    } else {
        if(!updateJson["d1_mini"]["version"].isNull()) {
            // updateEspAvailableVersion = updateJson["d1_mini"]["version"].as<float>();
            // debugPrintln(String(F("UPDATE: updateEspAvailableVersion: ")) + String(updateEspAvailableVersion));
            // espFirmwareUrl = updateJson["d1_mini"]["firmware"].as<String>();
            // if(updateEspAvailableVersion > haspVersion) {
            //     updateEspAvailable = true;
            //     debugPrintln(String(F("UPDATE: New ESP version available: ")) + String(updateEspAvailableVersion));
            // }
        }
        Log.verbose(TAG_OTA, F("UPDATE: Update check completed"));
    }
    return true;
}

static inline void otaProgress(void)
{
    Log.verbose(TAG_OTA, F("%s update in progress... %3u%"),
                (ArduinoOTA.getCommand() == U_FLASH ? PSTR("Firmware") : PSTR("Filesystem")), otaPrecentageComplete);
}

void otaOnProgress(unsigned int progress, unsigned int total)
{
    if(total != 0) {
        otaPrecentageComplete = progress * 100 / total;
        haspProgressVal(otaPrecentageComplete);
    }
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
            dispatch_reboot(true);
        });
        ArduinoOTA.onProgress(otaOnProgress);
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
    ESP8266HTTPUpdate httpUpdate;
    #else
    HTTPUpdate httpUpdate;
    #endif

    httpUpdate.rebootOnUpdate(false); // We do that ourselves
    t_httpUpdate_return returnCode = httpUpdate.update(otaClient, espOtaUrl);

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            Log.error(TAG_FWUP, F("HTTP_UPDATE_FAILED error %i %s"), httpUpdate.getLastError(),
                      httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.notice(TAG_FWUP, F("HTTP_UPDATE_NO_UPDATES"));
            break;

        case HTTP_UPDATE_OK:
            Log.notice(TAG_FWUP, F("HTTP_UPDATE_OK"));
            dispatch_reboot(true);
    }

    #if HASP_USE_MDNS > 0
    mdnsStart();
    #endif // HASP_USE_MDNS
}

#endif // ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32