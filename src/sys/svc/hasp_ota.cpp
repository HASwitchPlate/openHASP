/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "hasp_conf.h"

#include "hasp_debug.h"
#include "hasp_ota.h"

#include "../../hasp/hasp_dispatch.h"
#include "../../hasp/hasp.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#ifndef HASP_OTA_PORT
#define HASP_OTA_PORT 8266
#endif
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#ifndef HASP_OTA_PORT
#define HASP_OTA_PORT 3232
#endif

/**
 * This is lets-encrypt-x3-cross-signed.pem
 */
const char* rootCACertificate = "-----BEGIN CERTIFICATE-----\n"
                                "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
                                "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
                                "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
                                "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
                                "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
                                "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
                                "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
                                "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
                                "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
                                "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
                                "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
                                "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
                                "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
                                "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
                                "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
                                "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
                                "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
                                "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
                                "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
                                "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
                                "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
                                "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
                                "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
                                "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
                                "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
                                "-----END CERTIFICATE-----\n";
#endif

static WiFiClient otaClient;
std::string otaUrl           = "http://ota.netwize.be";
int16_t otaPort              = HASP_OTA_PORT;
int8_t otaPrecentageComplete = -1;

bool otaUpdateCheck()
{ // firmware update check
    WiFiClientSecure wifiUpdateClientSecure;
    HTTPClient updateClient;
    LOG_TRACE(TAG_OTA, F(D_OTA_CHECK_UPDATE), otaUrl.c_str());

    // wifiUpdateClientSecure.setInsecure();
    // wifiUpdateClientSecure.setBufferSizes(512, 512);
    updateClient.begin(wifiUpdateClientSecure, otaUrl.c_str());

    int httpCode = updateClient.GET(); // start connection and send HTTP header
    if(httpCode != HTTP_CODE_OK) {
        LOG_ERROR(TAG_OTA, F(D_OTA_CHECK_FAILED), updateClient.errorToString(httpCode).c_str());
        return false;
    }

    DynamicJsonDocument updateJson(1024);
    DeserializationError jsonError = deserializeJson(updateJson, updateClient.getString());
    updateClient.end();

    if(jsonError) { // Couldn't parse the returned JSON, so bail
        dispatch_json_error(TAG_OTA, jsonError);
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
        LOG_VERBOSE(TAG_OTA, F(D_OTA_CHECK_COMPLETE));
    }
    return true;
}

static inline void otaProgress(void)
{
    LOG_VERBOSE(TAG_OTA, F("%s %3u%"),
                (ArduinoOTA.getCommand() == U_FLASH ? F(D_OTA_UPDATING_FIRMWARE) : F(D_OTA_UPDATING_FILESYSTEM)),
                otaPrecentageComplete);
}

void otaOnProgress(unsigned int progress, unsigned int total)
{
    if(total != 0) {
        otaPrecentageComplete = progress * 100 / total;
        haspProgressVal(otaPrecentageComplete);
    }
}

#if HASP_USE_OTA > 0
void otaSetup(void)
{
    if(strlen(otaUrl.c_str())) {
        LOG_INFO(TAG_OTA, otaUrl.c_str());
    }

    if(otaPort > 0) {
        ArduinoOTA.onStart([]() {
            if(ArduinoOTA.getCommand() == U_FLASH) {
            } else { // U_SPIFFS
                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            }

            LOG_TRACE(TAG_OTA, F(D_SERVICE_STARTING));
            haspProgressMsg(F(D_OTA_UPDATE_FIRMWARE));
            haspProgressVal(0);
            otaPrecentageComplete = 0;
        });
        ArduinoOTA.onEnd([]() {
            otaPrecentageComplete = 100;
            LOG_TRACE(TAG_OTA, F(D_OTA_UPDATE_COMPLETE));
            haspProgressVal(100);
            haspProgressMsg(F(D_OTA_UPDATE_APPLY));
            otaProgress();
            otaPrecentageComplete = -1;
            // setup();
            dispatch_reboot(true);
        });
        ArduinoOTA.onProgress(otaOnProgress);
        ArduinoOTA.onError([](ota_error_t error) {
            char buffer[32];
            switch(error) {
                case OTA_AUTH_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Auth failed"));
                    break;
                case OTA_BEGIN_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Begin failed"));
                    break;
                case OTA_CONNECT_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Connect failed"));
                    break;
                case OTA_RECEIVE_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Receive failed"));
                    break;
                case OTA_END_ERROR:
                    snprintf_P(buffer, sizeof(buffer), PSTR("End failed"));
                    break;
                default:
                    snprintf_P(buffer, sizeof(buffer), PSTR("Unknown Error"));
            }

            otaPrecentageComplete = -1;
            LOG_ERROR(TAG_OTA, F("%s (%d)"), buffer, error);
            haspProgressMsg(F(D_OTA_UPDATE_FAILED));
            // delay(5000);
        });

        ArduinoOTA.setHostname(haspDevice.get_hostname());
        // ArduinoOTA.setPassword(configPassword); // See OTA_PASSWORD
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

#ifdef OTA_PASSWORD
        ArduinoOTA.setPassword(OTA_PASSWORD); // TODO
#endif

        ArduinoOTA.begin();
        LOG_INFO(TAG_OTA, F(D_SERVICE_STARTED));
    } else {
        LOG_WARNING(TAG_OTA, F(D_SERVICE_DISABLED));
    }
}

IRAM_ATTR void otaLoop(void)
{
    ArduinoOTA.handle();
}

void otaEverySecond(void)
{
    if(otaPrecentageComplete >= 0) otaProgress();
}
#endif

void otaHttpUpdate(const char* espOtaUrl)
{ // Update ESP firmware from HTTP
#if HASP_USE_MDNS > 0
    mdnsStop(); // Keep mDNS responder from breaking things
#endif

#if defined(ARDUINO_ARCH_ESP8266)
    // ESPhttpUpdate.onStart(update_started);
    // ESPhttpUpdate.onEnd(update_finished);
    // ESPhttpUpdate.onProgress(update_progress);
    // ESPhttpUpdate.onError(update_error);
    ESP8266HTTPUpdate httpUpdate;
    httpUpdate.rebootOnUpdate(false); // We do that ourselves
    t_httpUpdate_return returnCode = httpUpdate.update(otaClient, espOtaUrl);

#else
    HTTPUpdate httpUpdate;
    WiFiClientSecure secClient;
    secClient.setCACert(rootCACertificate);

    // Reading data over SSL may be slow, use an adequate timeout
    secClient.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout
    httpUpdate.rebootOnUpdate(false);   // We do that ourselves
    t_httpUpdate_return returnCode = httpUpdate.update(secClient, espOtaUrl);

#endif

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            LOG_ERROR(TAG_FWUP, F("HTTP_UPDATE_FAILED error %i %s"), httpUpdate.getLastError(),
                      httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            LOG_TRACE(TAG_FWUP, F("HTTP_UPDATE_NO_UPDATES"));
            break;

        case HTTP_UPDATE_OK:
            LOG_TRACE(TAG_FWUP, F("HTTP_UPDATE_OK"));
            dispatch_reboot(true);
    }

#if HASP_USE_MDNS > 0
    mdnsStart();
#endif // HASP_USE_MDNS
}

#endif // ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32