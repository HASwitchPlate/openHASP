#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "StringStream.h"

#include "hasp_dispatch.h"
#include "hasp_config.h"
#include "hasp_debug.h"
#include "hasp_http.h"
#include "hasp_mdns.h"
#include "hasp_wifi.h"
#include "hasp_gui.h"
#include "hasp.h"

#include "hasp_conf.h"
#if HASP_USE_MQTT
#include "hasp_mqtt.h"
#endif

inline void dispatchPrintln(String header, String & data)
{
    Log.notice(F("%s: %s"), header.c_str(), data.c_str());
}

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
#if HASP_USE_MQTT
    mqtt_send_statusupdate();
#endif
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

void dispatchOutput(String strTopic, const char * payload)
{
    String strTemp((char *)0);
    strTemp.reserve(128);
    strTemp = strTopic.substring(7, strTopic.length());
    dispatchOutput(strTemp.toInt(), isON(payload));
}

void dispatchButtonAttribute(String & strTopic, const char * payload)
{
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

        if(pageid >= 0 && pageid <= 255 && objid >= 0 && objid <= 255) {
            hasp_process_attribute((uint8_t)pageid, (uint8_t)objid, strAttr, payload);
        } // valid page
    }
}

// objectattribute=value
void dispatchAttribute(String strTopic, const char * payload)
{
    if(strTopic.startsWith("p[")) {
        dispatchButtonAttribute(strTopic, payload);
    } else if(strTopic.startsWith(F("output"))) {
#if defined(ARDUINO_ARCH_ESP8266)
        uint8_t state = isON(payload) ? HIGH : LOW;
        digitalWrite(D1, state);
#endif

    } else if(strTopic == F("page")) {
        dispatchPage(payload);

    } else if(strTopic == F("dim") || strTopic == F("brightness")) {
        dispatchDim(payload);

    } else if(strTopic == F("light")) {
        dispatchBacklight(payload);

    } else if(strTopic == F("reboot") || strTopic == F("restart")) {
        dispatchReboot(true);

    } else if(strTopic == F("clearpage")) {
        dispatchClearPage(payload);

    } else if(strTopic == F("setupap")) {
        haspDisplayAP(String(F("HASP-ABC123")).c_str(), String(F("haspadmin")).c_str());

    } else if(strTopic.length() == 7 && strTopic.startsWith(F("output"))) {
        dispatchOutput(strTopic, payload);
    }
}

void dispatchPage(String strPageid)
{
    dispatchPrintln(F("PAGE"), strPageid);

    if(strPageid.length() == 0) {
    } else {
        if(strPageid.toInt() <= 250) haspSetPage(strPageid.toInt());
    }
    String strPage((char *)0);
    strPage.reserve(128);
    strPage = haspGetPage();
#if HASP_USE_MQTT
    mqtt_send_state(F("page"), strPage.c_str());
#endif
}

void dispatchClearPage(String strPageid)
{
    dispatchPrintln(F("CLEAR"), strPageid);

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
    dispatchPrintln(F("DIM"), strDimLevel);

    // Return the current state
    String strPayload = String(guiGetDim());
#if HASP_USE_MQTT
    mqtt_send_state(F("dim"), strPayload.c_str());
#endif
}

void dispatchBacklight(String strPayload)
{
    strPayload.toUpperCase();
    dispatchPrintln(F("LIGHT"), strPayload);

    // Set the current state
    if(strPayload.length() != 0) guiSetBacklight(isON(strPayload.c_str()));

    // Return the current state
    strPayload = getOnOff(guiGetBacklight());
#if HASP_USE_MQTT
    mqtt_send_state(F("light"), strPayload.c_str());
#endif
}

void dispatchCommand(String cmnd)
{
    dispatchPrintln(F("CMND"), cmnd);

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
    } else if(cmnd == F("") || cmnd == F("statusupdate")) {
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
    size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 256;
    DynamicJsonDocument haspCommands(maxsize);
    DeserializationError jsonError = deserializeJson(haspCommands, payload);
    // haspCommands.shrinkToFit();

    if(jsonError) { // Couldn't parse incoming JSON command
        Log.warning(F("JSON: Failed to parse incoming JSON command with error: %s"), jsonError.c_str());
        return;
    }

    JsonArray arr = haspCommands.as<JsonArray>();
    for(JsonVariant command : arr) {
        dispatchCommand(command.as<String>());
    }
}

void dispatchJsonl(char * payload)
{
    DynamicJsonDocument config(256);

    String output((char *)0);
    StringStream stream((String &)output);

    size_t maxsize = (128u * ((strlen(payload) / 128) + 1));
    Log.verbose(F("CMND: payload %u => reserve %u"), strlen(payload), (128u * ((strlen(payload) / 128) + 1)));

    output.reserve((128u * ((strlen(payload) / 128) + 1)));
    stream.print(payload);

    while(deserializeJson(config, stream) == DeserializationError::Ok) {
        serializeJson(config, Serial);
        Serial.println();
        haspNewObject(config.as<JsonObject>());
    }
}

void dispatchIdle(const char * state)
{
#if HASP_USE_MQTT
    mqtt_send_state(F("idle"), state);
#endif
}

void dispatchReboot(bool saveConfig)
{
    if(saveConfig) configWriteConfig();
#if HASP_USE_MQTT
    mqttStop(); // Stop the MQTT Client first
#endif
    debugStop();
    delay(250);
    wifiStop();
    Log.notice(F("STOP: Properly Rebooting the MCU now!"));
    Log.verbose(F("-------------------------------------"));
    ESP.restart();
    delay(5000);
}

void dispatch_button(uint8_t id, const char * event)
{
#if HASP_USE_MQTT
    mqtt_send_input(id, event);
#endif
}