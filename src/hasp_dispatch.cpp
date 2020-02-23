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

void dispatchSetup()
{}

void dispatchLoop()
{}

// objectattribute=value
void IRAM_ATTR dispatchAttribute(String & strTopic, const char * payload)
{
    if(strTopic.startsWith("p[")) {
        String strPageId = strTopic.substring(2, strTopic.indexOf("]"));
        String strTemp   = strTopic.substring(strTopic.indexOf("]") + 1, strTopic.length());
        if(strTemp.startsWith(".b[")) {
            String strObjId = strTemp.substring(3, strTemp.indexOf("]"));
            String strAttr  = strTemp.substring(strTemp.indexOf("]") + 1, strTemp.length());
            // debugPrintln(strPageId + " && " + strObjId + " && " + strAttr);

            int pageid = strPageId.toInt();
            int objid  = strObjId.toInt();

            if(pageid >= 0 && pageid <= 255 && objid > 0 && objid <= 255) {
                haspProcessAttribute((uint8_t)pageid, (uint8_t)objid, strAttr, payload);
            } // valid page
        }
    } else if(strTopic == "page") {
        dispatchPage(payload);
    } else if(strTopic == "dim") {
        dispatchDim(payload);
    }
}

void IRAM_ATTR dispatchPage(String strPageid)
{
    debugPrintln("PAGE: " + strPageid);

    if(strPageid.length() == 0) {
        String strPayload = String(haspGetPage());
        mqttSendState("page", strPayload.c_str());
    } else {
        if(strPageid.toInt() <= 250) haspSetPage(strPageid.toInt());
    }
}

void dispatchDim(String strDimLevel)
{
    debugPrintln("DIM: " + strDimLevel);

    if(strDimLevel.length() == 0) {
        String strPayload = String(guiGetDim());
        mqttSendState("dim", strPayload.c_str());
    } else {
        guiSetDim(strDimLevel.toInt());
    }
}

void IRAM_ATTR dispatchCommand(String cmnd)
{
    debugPrintln("CMND: " + cmnd);

    if(cmnd.startsWith(F("page ")) || cmnd.startsWith(F("page="))) {
        cmnd = cmnd.substring(5, cmnd.length());
        dispatchPage(cmnd);
    } else if(cmnd == F("calibrate")) {
        guiCalibrate();
    } else if(cmnd == F("wakeup")) {
        haspWakeUp();
    } else if(cmnd == F("screenshot")) {
        guiTakeScreenshot("/screenhot.bmp");
    } else if(cmnd == F("reboot") || cmnd == F("restart")) {
        dispatchReboot(true);
    } else if(cmnd == "" || cmnd == F("statusupdate")) {
        mqttStatusUpdate();
    } else {

        int pos = cmnd.indexOf("=");
        if(pos > 0) {
            String strTopic   = cmnd.substring(0, pos);
            String strPayload = cmnd.substring(pos + 1, cmnd.length());
            // debugPrintln("CMND: '" + strTopic + "'='" + strPayload + "'");
            dispatchAttribute(strTopic, strPayload.c_str());
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

void IRAM_ATTR dispatchIdle(const __FlashStringHelper * state)
{
    mqttSendState(String(F("idle")).c_str(), String(state).c_str());
}

void dispatchReboot(bool saveConfig)
{
    mqttStop(); // Stop the MQTT Client first
    if(saveConfig) configWriteConfig();
    debugStop();
    delay(250);
    wifiStop();
    debugPrintln(F("STOP: Properly Rebooting the MCU now!"));
    debugPrintln(F("-------------------------------------"));
    ESP.restart();
    delay(5000);
}