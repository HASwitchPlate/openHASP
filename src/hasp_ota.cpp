#include <Arduino.h>
#include "ArduinoJson.h"
#include <ArduinoOTA.h>

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_ota.h"
#include "hasp.h"

#define F_OTA_URL F("otaurl")

std::string otaUrl = "http://ota.local";

void otaSetup(JsonObject settings)
{
    char buffer[256];

    if(!settings[F_OTA_URL].isNull()) {
        otaUrl = settings[F_OTA_URL].as<String>().c_str();
        sprintf_P(buffer, PSTR("ORA url: %s"), otaUrl.c_str());
        debugPrintln(buffer);
    }

    ArduinoOTA.setHostname(String(haspGetNodename()).c_str());
    // ArduinoOTA.setPassword(configPassword);

    ArduinoOTA.onStart([]() {
        debugPrintln(F("OTA: update start"));
        haspSendCmd("page 0");
        haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\"");
    });
    ArduinoOTA.onEnd([]() {
        haspSendCmd("page 0");
        debugPrintln(F("OTA: update complete"));
        haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\\rComplete!\"");
        haspReset();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        haspSetAttr("p[0].b[1].txt", "\"ESP OTA Update\\rProgress: " + String(progress / (total / 100)) + "%\"");
    });
    ArduinoOTA.onError([](ota_error_t error) {
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
        haspSetAttr("p[0].b[1].txt", "\"ESP OTA FAILED\"");
        delay(5000);
        // haspSendCmd("page " + String(nextionActivePage));
    });
    ArduinoOTA.begin();
    debugPrintln(F("OTA: Over the Air firmware update ready"));
}

void otaLoop()
{
    ArduinoOTA.handle(); // Arduino OTA loop
}