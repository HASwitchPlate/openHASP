#include "hasp_dispatch.h"
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
void dispatchAttribute(String & strTopic, String & strPayload)
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
                haspProcessAttribute((uint8_t)pageid, (uint8_t)objid, strAttr, strPayload);
            } // valid page
        }
    } else if(strTopic == "page") {
    } else if(strTopic == "dim") {
    }
}

void dispatchPage(String & strPageid)
{
    debugPrintln("PAGE:" + strPageid);

    if(strPageid.length() == 0) {
        String strPayload = String(haspGetPage());
        mqttSendState("page", strPayload.c_str());
    } else {
        if(strPageid.toInt() <= 250) haspSetPage(strPageid.toInt());
    }
}

void dispatchCommand(String cmnd)
{
    debugPrintln("CMND: " + cmnd);

    if(cmnd == F("calibrate")) {
        guiCalibrate();
        return;
    }

    if(cmnd == F("screenshot")) {
        guiTakeScreenshot("/screenhot.bmp");
        return;
    }

    if(cmnd == F("reboot") || cmnd == F("restart")) {
        haspReboot(true);
        return;
    }

    if(cmnd == "" || cmnd == F("statusupdate")) {
        mqttStatusUpdate();
        return;
    }

    int pos = cmnd.indexOf("=");
    if(pos > 0) {
        String strTopic   = cmnd.substring(0, pos);
        String strPayload = cmnd.substring(pos + 1, cmnd.length());
        debugPrintln("CMND: '" + strTopic + "'='" + strPayload + "'");
        dispatchAttribute(strTopic, strPayload);
    }
}

void dispatchJson(String & strPayload)
{ // Parse an incoming JSON array into individual commands
    if(strPayload.endsWith(",]")) {
        // Trailing null array elements are an artifact of older Home Assistant automations
        // and need to be removed before parsing by ArduinoJSON 6+
        strPayload.remove(strPayload.length() - 2, 2);
        strPayload.concat("]");
    }
    DynamicJsonDocument haspCommands(2048 + 512);
    DeserializationError jsonError = deserializeJson(haspCommands, strPayload);
    if(jsonError) { // Couldn't parse incoming JSON command
        errorPrintln(String(F("JSON: %sFailed to parse incoming JSON command with error: ")) +
                     String(jsonError.c_str()));
        return;
    }

    // Slow
    // for(uint8_t i = 0; i < haspCommands.size(); i++) {
    //    dispatchCommand(haspCommands[i]);
    //}

    // Get a reference to the root array
    JsonArray arr = haspCommands.as<JsonArray>();
    // Fast
    for(JsonVariant command : arr) {
        dispatchCommand(command.as<String>());
    }
}

void dispatchIdle(const __FlashStringHelper * state)
{
    mqttSendState(String(F("idle")).c_str(), String(state).c_str());
}