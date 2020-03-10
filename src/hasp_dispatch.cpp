#include "StringStream.h"
#include "ArduinoJson.h"

#include "hasp_dispatch.h"
#include "hasp_config.h"
#include "hasp_debug.h"
#include "hasp_mqtt.h"
#include "hasp_http.h"
#include "hasp_mdns.h"
#include "hasp_wifi.h"
#include "hasp_log.h"
#include "hasp_gui.h"
#include "hasp.h"

bool isON(const char * payload)
{
    return strcmp_P(payload, PSTR("ON")) == 0;
}

String getOnOff(bool state)
{
    String result((char *)0);
    result.reserve(128);
    result = state ? F("ON") : F("OFF");
    return result;
}

void dispatchSetup()
{}

void dispatchLoop()
{}

void dispatchStatusUpdate()
{
    mqttStatusUpdate();
}

void dispatchOutput(int output, bool state)
{
    int pin = 0;

    if(pin >= 0) {

#if defined(ARDUINO_ARCH_ESP32)
        ledcWrite(99, state ? 1023 : 0); // ledChannel and value
#else
        analogWrite(pin, state ? 1023 : 0);
#endif
    }
}

// objectattribute=value
void dispatchAttribute(String & strTopic, const char * payload)
{
    if(strTopic.startsWith("p[")) {
        String strPageId((char *)0);
        String strTemp((char *)0);

        strPageId = strTopic.substring(2, strTopic.indexOf("]"));
        strTemp   = strTopic.substring(strTopic.indexOf("]") + 1, strTopic.length());

        if(strTemp.startsWith(".b[")) {
            String strObjId((char *)0);
            String strAttr((char *)0);

            strObjId = strTemp.substring(3, strTemp.indexOf("]"));
            strAttr  = strTemp.substring(strTemp.indexOf("]") + 1, strTemp.length());
            // debugPrintln(strPageId + " && " + strObjId + " && " + strAttr);

            int pageid = strPageId.toInt();
            int objid  = strObjId.toInt();

            if(pageid >= 0 && pageid <= 255 && objid > 0 && objid <= 255) {
                haspProcessAttribute((uint8_t)pageid, (uint8_t)objid, strAttr, payload);
            } // valid page
        }
    } else if(strTopic.startsWith(F("output"))) {
        uint8_t state = isON(payload) ? HIGH : LOW;
#if defined(ARDUINO_ARCH_ESP8266)
        digitalWrite(D1, state);
#endif

    } else if(strTopic == F("page")) {
        dispatchPage(payload);

    } else if(strTopic == F("dim") || strTopic == F("brightness")) {
        dispatchDim(payload);

    } else if(strTopic == F("light")) {
        dispatchBacklight(payload);

    } else if(strTopic == F("clearpage")) {
        dispatchClearPage(payload);

    } else if(strTopic == F("setupap")) {
        haspDisplayAP(String(F("HASP-ABC123")).c_str(), String(F("haspadmin")).c_str());

    } else if(strTopic.length() == 7 && strTopic.startsWith(F("output"))) {
        String strTemp((char *)0);
        strTemp = strTopic.substring(7, strTopic.length());
        dispatchOutput(strTemp.toInt(), true);
    }
}

void dispatchPage(String strPageid)
{
    debugPrintln("PAGE: " + strPageid);

    if(strPageid.length() == 0) {
    } else {
        if(strPageid.toInt() <= 250) haspSetPage(strPageid.toInt());
    }
    String strPayload = String(haspGetPage());
    mqttSendState("page", strPayload.c_str());
}

void dispatchClearPage(String strPageid)
{
    debugPrintln("Clear Page: " + strPageid);

    if(strPageid.length() == 0) {
        haspClearPage(haspGetPage());
    } else {
        haspClearPage(strPageid.toInt());
    }
}

void dispatchDim(String strDimLevel)
{
    // Set the current state
    if(strDimLevel.length() != 0) guiSetDim(strDimLevel.toInt());
    debugPrintln("DIM: " + strDimLevel);

    // Return the current state
    String strPayload = String(guiGetDim());
    mqttSendState("dim", strPayload.c_str());
}

void dispatchBacklight(String strPayload)
{
    strPayload.toUpperCase();
    debugPrintln("LIGHT: " + strPayload);

    // Set the current state
    if(strPayload.length() != 0) guiSetBacklight(isON(strPayload.c_str()));

    // Return the current state
    strPayload = getOnOff(guiGetBacklight());
    mqttSendState("light", strPayload.c_str());
}

void dispatchCommand(String cmnd)
{
    cmnd.toLowerCase();
    debugPrintln("CMND: " + cmnd);

    if(cmnd.startsWith(F("page "))) {
        cmnd = cmnd.substring(5, cmnd.length());
        String strTopic((char *)0);
        strTopic.reserve(128);
        strTopic = F("page");
        dispatchAttribute(strTopic, cmnd.c_str());
    } else if(cmnd == F("calibrate")) {
        guiCalibrate();
    } else if(cmnd == F("wakeup")) {
        haspWakeUp();
    } else if(cmnd == F("screenshot")) {
        // guiTakeScreenshot("/screenhot.bmp");
    } else if(cmnd == F("reboot") || cmnd == F("restart")) {
        dispatchReboot(true);
    } else if(cmnd == "" || cmnd == F("statusupdate")) {
        dispatchStatusUpdate();
    } else {

        int pos = cmnd.indexOf("=");
        if(pos > 0) {
            String strTopic((char *)0);
            String strPayload((char *)0);

            strTopic.reserve(128);
            strPayload.reserve(128);

            strTopic   = cmnd.substring(0, pos);
            strPayload = cmnd.substring(pos + 1, cmnd.length());

            dispatchAttribute(strTopic, strPayload.c_str());
        } else {
            dispatchAttribute(cmnd, "");
        }
    }
}

void dispatchJson(char * payload)
{ // Parse an incoming JSON array into individual commands
    /*  if(strPayload.endsWith(",]")) {
          // Trailing null array elements are an artifact of older Home Assistant automations
          // and need to be removed before parsing by ArduinoJSON 6+
          strPayload.remove(strPayload.length() - 2, 2);
          strPayload.concat("]");
      }*/

    DynamicJsonDocument haspCommands(MQTT_MAX_PACKET_SIZE + 512);
    DeserializationError jsonError = deserializeJson(haspCommands, payload);
    haspCommands.shrinkToFit();

    if(jsonError) { // Couldn't parse incoming JSON command
        errorPrintln(String(F("JSON: %sFailed to parse incoming JSON command with error: ")) +
                     String(jsonError.c_str()));
        return;
    }

    JsonArray arr = haspCommands.as<JsonArray>();
    for(JsonVariant command : arr) {
        dispatchCommand(command.as<String>());
    }
}

void dispatchJsonl(char * strPayload)
{
    Serial.println("JSONL\n");
    DynamicJsonDocument config(256);

    String output((char *)0);
    output.reserve(MQTT_MAX_PACKET_SIZE+ 256);

    StringStream stream((String &)output);
    stream.print(strPayload);

    while(deserializeJson(config, stream) == DeserializationError::Ok) {
        serializeJson(config, Serial);
        Serial.println();
        haspNewObject(config.as<JsonObject>());
    }
}

void dispatchIdle(const __FlashStringHelper * state)
{
    mqttSendState(String(F("idle")).c_str(), String(state).c_str());
}

void dispatchReboot(bool saveConfig)
{
    if(saveConfig) configWriteConfig();
    mqttStop(); // Stop the MQTT Client first
    debugStop();
    delay(250);
    wifiStop();
    debugPrintln(F("STOP: Properly Rebooting the MCU now!"));
    debugPrintln(F("-------------------------------------"));
    ESP.restart();
    delay(5000);
}

void dispatchButton(uint8_t id, char * event)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("INPUT%d"), id);
    mqttSendState(buffer, event);
}