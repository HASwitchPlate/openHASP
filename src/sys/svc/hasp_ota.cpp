/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "hasplib.h"

#include "hasp_debug.h"
#include "hasp_ota.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#ifndef HASP_ARDUINOOTA_PORT
#define HASP_ARDUINOOTA_PORT 8266
#endif
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#ifndef HASP_ARDUINOOTA_PORT
#define HASP_ARDUINOOTA_PORT 3232
#endif

#if HASP_USE_MDNS > 0
#if defined(ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266mDNS.h>
#endif
#endif // HASP_USE_MDNS

#ifndef HASP_OTA_URL
#define HASP_OTA_URL ""
#endif

/**
 * This is lets-encrypt-x3-cross-signed.pem
 */
/*
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
                                */

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_data_cert_x509_crt_bundle_bin_start");
#endif // ARDUINO_ARCH_ESP32

static WiFiClientSecure secureClient;
std::string otaUrl = "http://ota.netwize.be";

uint16_t arduinoOtaPort      = HASP_ARDUINOOTA_PORT;
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

    StaticJsonDocument<1024> updateJson;
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
    LOG_VERBOSE(TAG_OTA, F("%s %d%%"),
                (ArduinoOTA.getCommand() == U_FLASH ? F(D_OTA_UPDATING_FIRMWARE) : F(D_OTA_UPDATING_FILESYSTEM)),
                otaPrecentageComplete);
}

static void ota_on_start(void)
{
    if(ArduinoOTA.getCommand() == U_FLASH) {
    } else { // U_SPIFFS
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    }

    LOG_TRACE(TAG_OTA, F(D_SERVICE_STARTING));
    haspProgressMsg(F(D_OTA_UPDATE_FIRMWARE));
    haspProgressVal(0);
    otaPrecentageComplete = 0;
}

#if HASP_USE_ARDUINOOTA > 0
static void ota_on_end(void)
{
    otaPrecentageComplete = 100;
    LOG_TRACE(TAG_OTA, F(D_OTA_UPDATE_COMPLETE));
    haspProgressVal(100);
    haspProgressMsg(F(D_OTA_UPDATE_APPLY));
    otaProgress();
    otaPrecentageComplete = -1;
    dispatch_reboot(true);
}

static void ota_on_progress(unsigned int progress, unsigned int total)
{
    if(total == 0) return;
    otaPrecentageComplete = progress * 100 / total;
    haspProgressVal(otaPrecentageComplete);
}

static void ota_on_error(ota_error_t error)
{
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
}

IRAM_ATTR void otaLoop(void)
{
    ArduinoOTA.handle();
}

void otaEverySecond(void)
{
    if(otaPrecentageComplete >= 0) otaProgress();
}
#endif // HASP_USE_ARDUINOOTA

void otaSetup(void)
{
#if ESP_ARDUINO_VERSION_MAJOR >= 2
    /* This method is similar to the single root certificate verfication, but it uses a standard set of root
     * certificates from Mozilla to authenticate against. This allows the client to connect to all public SSL
     * servers. */
    secureClient.setCACertBundle(rootca_crt_bundle_start);
#endif
    //  Reading data over SSL may be slow, use an adequate timeout
    secureClient.setTimeout(12); // timeout argument is defined in seconds

#if HASP_USE_ARDUINOOTA > 0
    if(strlen(otaUrl.c_str())) {
        LOG_INFO(TAG_OTA, otaUrl.c_str());
    }

    if(arduinoOtaPort > 0) {
        ArduinoOTA.onStart(ota_on_start);
        ArduinoOTA.onEnd(ota_on_end);
        ArduinoOTA.onProgress(ota_on_progress);
        ArduinoOTA.onError(ota_on_error);

        ArduinoOTA.setHostname(haspDevice.get_hostname());
        ArduinoOTA.setPort(arduinoOtaPort);
        ArduinoOTA.setRebootOnSuccess(false); // We do that ourselves
                                              // ArduinoOTA.setTimeout(1000); // default

#if ESP32
#if HASP_USE_MDNS > 0
        ArduinoOTA.setMdnsEnabled(false); // it's already started
#ifdef ARDUINOOTA_PASSWORD
        MDNS.enableArduino(arduinoOtaPort, strlen(ARDUINOOTA_PASSWORD) > 0);
#endif // ARDUINOOTA_PASSWORD
#endif // HASP_USE_MDNS
#endif // ESP32

#ifdef ARDUINOOTA_PASSWORD
        ArduinoOTA.setPassword(ARDUINOOTA_PASSWORD);
#endif // ARDUINOOTA_PASSWORD

        ArduinoOTA.begin();
        LOG_INFO(TAG_OTA, F(D_SERVICE_STARTED));
    } else {
        LOG_WARNING(TAG_OTA, F(D_SERVICE_DISABLED));
    }
#endif // HASP_USE_ARDUINOOTA
}

#if HASP_USE_HTTP_UPDATE > 0
static unsigned long htppLastLoopTime = 0;

static void ota_on_http_progress(unsigned int progress, unsigned int total)
{
    if(total == 0) return;
    otaPrecentageComplete = progress * 100 / total;
    haspProgressVal(otaPrecentageComplete);

    if(millis() - htppLastLoopTime < 1250) return;
    LOG_VERBOSE(TAG_OTA, F(D_OTA_UPDATING_FIRMWARE " %d%%"), otaPrecentageComplete);
    htppLastLoopTime = millis();
}

static void ota_on_http_end(void)
{
    otaPrecentageComplete = 100;
    LOG_TRACE(TAG_OTA, F(D_OTA_UPDATE_COMPLETE));
    haspProgressVal(100);
    haspProgressMsg(F(D_OTA_UPDATE_APPLY));
    otaPrecentageComplete = -1;
}

static void ota_on_http_error(int error)
{
    otaPrecentageComplete = -1;
    LOG_ERROR(TAG_OTA, F("%s (%d)"), "HTTP Update error", error);
    haspProgressMsg(F(D_OTA_UPDATE_FAILED));
}

void ota_http_update(const char* url)
{ // Update ESP firmware from HTTP

    t_httpUpdate_return returnCode;
    followRedirects_t redirectCode;

    if(secureClient.connected() && url == strstr_P(url, PSTR("https://"))) { // not start with https
        LOG_ERROR(TAG_OTA, F("HTTP_UPDATE_FAILED client is already connected"));
        return;
    } else {
        StaticJsonDocument<512> doc; // update update url, get redirect setting
        JsonObject settings;
        settings = doc.to<JsonObject>(); // force creation of an empty JsonObject
        otaGetConfig(settings);
        settings["url"] = url;
        switch(settings["redirect"].as<uint8_t>()) {
            case 1:
                redirectCode = HTTPC_STRICT_FOLLOW_REDIRECTS;
                LOG_VERBOSE(TAG_OTA, F("Strict follow redirects"));
                break;
            case 2:
                redirectCode = HTTPC_FORCE_FOLLOW_REDIRECTS;
                LOG_VERBOSE(TAG_OTA, F("Force follow redirects"));
                break;
            default:
                redirectCode = HTTPC_DISABLE_FOLLOW_REDIRECTS;
                LOG_VERBOSE(TAG_OTA, F("Disable follow redirects"));
                settings["redirect"] = 0;
        }
        otaSetConfig(settings);
    }

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
    returnCode = httpUpdate.update(otaClient, url);

#else
    HTTPUpdate httpUpdate;

    httpUpdate.onStart(ota_on_start);
    httpUpdate.onEnd(ota_on_http_end);
    httpUpdate.onProgress(ota_on_http_progress);
    httpUpdate.onError(ota_on_http_error);
    httpUpdate.rebootOnUpdate(false); // We do that ourselves
    httpUpdate.setFollowRedirects(redirectCode);

    if(url != strstr_P(url, PSTR("https://"))) { // not start with https
        WiFiClient otaClient;
        returnCode = httpUpdate.update(otaClient, url);
    } else {
        returnCode = httpUpdate.update(secureClient, url, haspDevice.get_version());
    }

#endif

    switch(returnCode) {
        case HTTP_UPDATE_FAILED:
            LOG_ERROR(TAG_OTA, F("HTTP_UPDATE_FAILED error %i %s"), httpUpdate.getLastError(),
                      httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            LOG_TRACE(TAG_OTA, F("HTTP_UPDATE_NO_UPDATES"));
            break;

        case HTTP_UPDATE_OK:
            LOG_TRACE(TAG_OTA, F("HTTP_UPDATE_OK"));
            dispatch_reboot(true);
            delay(5000);
    }

#if HASP_USE_MDNS > 0
    mdnsStart();
#endif // HASP_USE_MDNS
}
#endif // HASP_USE_HTTP_UPDATE

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool otaGetConfig(const JsonObject& settings)
{
    Preferences preferences;
    bool changed = false;

    nvs_user_begin(preferences,"ota", true);
    settings["url"]      = preferences.getString("url", HASP_OTA_URL);
    settings["redirect"] = preferences.getUInt("redirect", 0);
    preferences.end();

    // #if ESP_ARDUINO_VERSION_MAJOR >= 2
    //     nvs_iterator_t it = nvs_entry_find("nvs", "ota", NVS_TYPE_ANY);
    //     while(it != NULL) {
    //         nvs_entry_info_t info;
    //         nvs_entry_info(it, &info);
    //         it = nvs_entry_next(it);
    //         printf("key '%s', type '%d' \n", info.key, info.type);
    //     };
    // #endif

    if(changed) configOutput(settings, TAG_TIME);

    return changed;
}

bool otaSetConfig(const JsonObject& settings)
{
    Preferences preferences;
    nvs_user_begin(preferences,"ota", false);

    configOutput(settings, TAG_OTA);
    bool changed = false;
    changed |= nvsUpdateString(preferences, "url", settings["url"]);
    changed |= nvsUpdateUInt(preferences, "redirect", settings["redirect"]);

    preferences.end();
    return changed;
}
#endif // HASP_USE_CONFIG

#endif // ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32