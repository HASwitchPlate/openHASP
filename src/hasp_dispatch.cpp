#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "StringStream.h"
#include "CharStream.h"

#include "hasp_dispatch.h"
#include "hasp_config.h"
#include "hasp_debug.h"
#include "hasp_gui.h"
#include "hasp_hal.h"
#include "hasp.h"

#include "hasp_conf.h"

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
        Log.notice(F("PIN OUTPUT STATE %d"),state);

#if defined(ARDUINO_ARCH_ESP32)
        ledcWrite(99, state ? 1023 : 0); // ledChannel and value
#elif defined(STM32F4xx)
        digitalWrite(HASP_OUTPUT_PIN, state);
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
            hasp_process_attribute((uint8_t)pageid, (uint8_t)objid, strAttr.c_str(), payload);
        } // valid page
    }
}

// objectattribute=value
void dispatchAttribute(String strTopic, const char * payload)
{
    if(strTopic.startsWith("p[")) {
        dispatchButtonAttribute(strTopic, payload);

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

    } else if(strTopic == F("update")) {
        dispatchWebUpdate(payload);

    } else if(strTopic == F("setupap")) {
        // haspDisplayAP(String(F("HASP-ABC123")).c_str(), String(F("haspadmin")).c_str());

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
#if HASP_USE_MQTT > 0
    mqtt_send_state(F("page"), strPage.c_str());
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("page"), strPage.c_str());
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

#if HASP_USE_MQTT > 0
    char buffer[8];
    itoa(guiGetDim(), buffer, DEC);
    mqtt_send_state(F("dim"), buffer);
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    char buffer[8];
    itoa(guiGetDim(), buffer, DEC);
    slave_send_state(F("dim"), buffer);
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
#if HASP_USE_MQTT > 0
    mqtt_send_state(F("light"), strPayload.c_str());
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("light"), strPayload.c_str());
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
    } else {

        JsonArray arr = haspCommands.as<JsonArray>();
        for(JsonVariant command : arr) {
            dispatchCommand(command.as<String>());
        }
    }
}

void dispatchJsonl(Stream & stream)
{
    DynamicJsonDocument jsonl(3 * 128u);
    uint8_t savedPage = haspGetPage();

    Log.notice(F("DISPATCH: jsonl"));

    while(deserializeJson(jsonl, stream) == DeserializationError::Ok) {
        // serializeJson(jsonl, Serial);
        // Serial.println();
        haspNewObject(jsonl.as<JsonObject>(), savedPage);
    }
}

void dispatchJsonl(char * payload)
{
    CharStream stream(payload);
    dispatchJsonl(stream);
}

void dispatchIdle(const char * state)
{
#if HASP_USE_MQTT > 0
    mqtt_send_state(F("idle"), state);
#elif HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("idle"), state);
#else
    Log.notice(F("OUT: idle = %s"), state);
#endif
}

void dispatchReboot(bool saveConfig)
{
    if(saveConfig) configWriteConfig();
#if HASP_USE_MQTT > 0
    mqttStop(); // Stop the MQTT Client first
#endif
    debugStop();
#if HASP_USE_WIFI > 0
    wifiStop();
#endif
    Log.verbose(F("-------------------------------------"));
    Log.notice(F("STOP: Properly Rebooting the MCU now!"));
    Serial.flush();
    //halRestart();
}

void dispatch_button(uint8_t id, const char * event)
{
#if HASP_USE_MQTT > 0
    mqtt_send_input(id, event);
#else
    Log.notice(F("OUT: input%d = %s"), id, event);
#endif
#if HASP_USE_TASMOTA_SLAVE
    slave_send_input(id, event);
#endif
}

void dispatchWebUpdate(const char * espOtaUrl)
{
#if HASP_USE_OTA > 0
    Log.verbose(F("FWUP: Checking for updates at URL: %s"), espOtaUrl);
    otaHttpUpdate(espOtaUrl);
#endif
}

void IRAM_ATTR dispatch_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data)
{
#if HASP_USE_MQTT > 0
    mqtt_send_obj_attribute_str(pageid, btnid, attribute, data);
#elif HASP_USE_TASMOTA_SLAVE > 0
    slave_send_obj_attribute_str(pageid, btnid, attribute, data);
#else
    Log.notice(F("OUT: json = {\"p[%u].b[%u].%s\":\"%s\"}"), pageid, btnid, attribute, data);
#endif
}

void dispatchConfig(const char * topic, const char * payload)
{
    DynamicJsonDocument doc(128 * 2);
    char buffer[128 * 2];
    JsonObject settings;
    bool update;

    if(strlen(payload) == 0) {
        // Make sure we have a valid JsonObject to start from
        settings = doc.to<JsonObject>().createNestedObject(topic);
        update   = false;

    } else {
        DeserializationError jsonError = deserializeJson(doc, payload);
        if(jsonError) { // Couldn't parse incoming JSON command
            Log.warning(F("JSON: Failed to parse incoming JSON command with error: %s"), jsonError.c_str());
            return;
        }
        settings = doc.as<JsonObject>();
        update   = true;
    }

    if(strcmp_P(topic, PSTR("debug")) == 0) {
        if(update)
            debugSetConfig(settings);
        else
            debugGetConfig(settings);
    }

    else if(strcmp_P(topic, PSTR("gui")) == 0) {
        if(update)
            guiSetConfig(settings);
        else
            guiGetConfig(settings);
    }

    else if(strcmp_P(topic, PSTR("hasp")) == 0) {
        if(update)
            haspSetConfig(settings);
        else
            haspGetConfig(settings);
    }

#if HASP_USE_WIFI > 0
    else if(strcmp_P(topic, PSTR("wifi")) == 0) {
        if(update)
            wifiSetConfig(settings);
        else
            wifiGetConfig(settings);
    }
#if HASP_USE_MQTT > 0
    else if(strcmp_P(topic, PSTR("mqtt")) == 0) {
        if(update)
            mqttSetConfig(settings);
        else
            mqttGetConfig(settings);
    }
#endif
#if HASP_USE_TELNET > 0
    //   else if(strcmp_P(topic, PSTR("telnet")) == 0)
    //       telnetGetConfig(settings[F("telnet")]);
#endif
#if HASP_USE_MDNS > 0
    else if(strcmp_P(topic, PSTR("mdns")) == 0) {
        if(update)
            mdnsSetConfig(settings);
        else
            mdnsGetConfig(settings);
    }
#endif
#if HASP_USE_HTTP > 0
    else if(strcmp_P(topic, PSTR("http")) == 0) {
        if(update)
            httpSetConfig(settings);
        else
            httpGetConfig(settings);
    }
#endif
#endif

    // Send output
    if(!update) {
        settings.remove(F("pass")); // hide password in output
        size_t size = serializeJson(doc, buffer, sizeof(buffer));
#if HASP_USE_MQTT > 0
        mqtt_send_state(F("config"), buffer);
#elif HASP_USE_TASMOTA > 0
        slave_send_state(F("config"), buffer);
#else
    Log.notice(F("OUT: config %s = %s"),topic,buffer);
#endif
    }
}
