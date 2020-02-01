#include <Arduino.h>
#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP.h>
#include <DNSServer.h>
#endif
#include <PubSubClient.h>

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_dispatch.h"
#include "hasp.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

// Size of buffer for incoming MQTT message
#define mqttMaxPacketSize 2u * 1024u

String mqttClientId; // Auto-generated MQTT ClientID
/*
String mqttGetSubtopic;             // MQTT subtopic for incoming commands requesting .val
String mqttGetSubtopicJSON;         // MQTT object buffer for JSON status when requesting .val
String mqttStateTopic;              // MQTT topic for outgoing panel interactions
String mqttStateJSONTopic;          // MQTT topic for outgoing panel interactions in JSON format
String mqttCommandTopic;            // MQTT topic for incoming panel commands
String mqttGroupCommandTopic;       // MQTT topic for incoming group panel commands
String mqttStatusTopic;             // MQTT topic for publishing device connectivity state
String mqttSensorTopic;             // MQTT topic for publishing device information in JSON format
*/
String mqttLightCommandTopic;       // MQTT topic for incoming panel backlight on/off commands
String mqttLightStateTopic;         // MQTT topic for outgoing panel backlight on/off state
String mqttLightBrightCommandTopic; // MQTT topic for incoming panel backlight dimmer commands
String mqttLightBrightStateTopic;   // MQTT topic for outgoing panel backlight dimmer state
// String mqttMotionStateTopic;        // MQTT topic for outgoing motion sensor state

String mqttNodeTopic;
String mqttGroupTopic;
bool mqttEnabled;

////////////////////////////////////////////////////////////////////////////////////////////////////
// These defaults may be overwritten with values saved by the web interface
#ifdef MQTT_HOST
std::string mqttServer = MQTT_HOST;
#else
std::string mqttServer   = "";
#endif
#ifdef MQTT_PORT
uint16_t mqttPort = MQTT_PORT;
#else
uint16_t mqttPort        = 1883;
#endif
#ifdef MQTT_USER
std::string mqttUser = MQTT_USER;
#else
std::string mqttUser     = "";
#endif
#ifdef MQTT_PASSW
std::string mqttPassword = MQTT_PASSW;
#else
std::string mqttPassword = "";
#endif
std::string mqttGroupName = "plates";

/*
const String mqttLightSubscription        = "hasp/" + String(haspGetNodename()) + "/light/#";
const String mqttLightBrightSubscription  = "hasp/" + String(haspGetNodename()) + "/brightness/#";
*/

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Send changed values OUT

void mqttSendState(const char * subtopic, const char * payload)
{
    // page = 0
    // p[0].b[0].attr = abc
    // dim = 100
    // idle = 0/1
    // light = 0/1
    // brightness = 100

    char topic[128];
    sprintf_P(topic, PSTR("%sstate/%s"), mqttNodeTopic.c_str(), subtopic);
    mqttClient.publish(topic, payload);
    debugPrintln(String(F("MQTT OUT: ")) + String(topic) + " = " + String(payload));

    // as json
    char value[256];
    sprintf_P(topic, PSTR("%sstate/json"), mqttNodeTopic.c_str());
    sprintf_P(value, PSTR("{\"%s\":\"%s\"}"), subtopic, payload);
    mqttClient.publish(topic, value);
    debugPrintln(String(F("MQTT OUT: ")) + String(topic) + " = " + String(value));
}

void mqttSendNewValue(uint8_t pageid, uint8_t btnid, const char * attribute, String txt)
{
    char subtopic[32];
    sprintf_P(subtopic, PSTR("p[%u].b[%u].%s"), pageid, btnid, attribute);
    mqttSendState(subtopic, txt.c_str());
}

void mqttSendNewValue(uint8_t pageid, uint8_t btnid, int32_t val)
{
    char value[16];
    itoa(val, value, 10);
    mqttSendNewValue(pageid, btnid, "val", value);
}

void mqttSendNewValue(uint8_t pageid, uint8_t btnid, String txt)
{
    mqttSendNewValue(pageid, btnid, "txt", txt);
}

void mqttSendNewEvent(uint8_t pageid, uint8_t btnid, int32_t val)
{
    char value[16];
    itoa(val, value, 10);
    mqttSendNewValue(pageid, btnid, "event", value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Receive incoming messages
void mqttCallback(char * topic, byte * payload, unsigned int length)
{ // Handle incoming commands from MQTT
    payload[length]   = '\0';
    String strTopic   = topic;
    String strPayload = (char *)payload;

    // strTopic: homeassistant/haswitchplate/devicename/command/p[1].b[4].txt
    // strPayload: "Lights On"
    // subTopic: p[1].b[4].txt

    // Incoming Namespace (replace /device/ with /group/ for group commands)
    // '[...]/device/command' -m '' = No command requested, respond with mqttStatusUpdate()
    // '[...]/device/command' -m 'dim=50' = nextionSendCmd("dim=50")
    // '[...]/device/command/json' -m '["dim=5", "page 1"]' = nextionSendCmd("dim=50"), nextionSendCmd("page 1")
    // '[...]/device/command/page' -m '1' = nextionSendCmd("page 1")
    // '[...]/device/command/statusupdate' -m '' = mqttStatusUpdate()
    // '[...]/device/command/lcdupdate' -m 'http://192.168.0.10/local/HASwitchPlate.tft' =
    // nextionStartOtaDownload("http://192.168.0.10/local/HASwitchPlate.tft")
    // '[...]/device/command/lcdupdate' -m '' = nextionStartOtaDownload("lcdFirmwareUrl")
    // '[...]/device/command/espupdate' -m 'http://192.168.0.10/local/HASwitchPlate.ino.d1_mini.bin' =
    // espStartOta("http://192.168.0.10/local/HASwitchPlate.ino.d1_mini.bin")
    // '[...]/device/command/espupdate' -m '' = espStartOta("espFirmwareUrl")
    // '[...]/device/command/p[1].b[4].txt' -m '' = nextionGetAttr("p[1].b[4].txt")
    // '[...]/device/command/p[1].b[4].txt' -m '"Lights On"' = nextionSetAttr("p[1].b[4].txt", "\"Lights On\"")

    debugPrintln(String(F("MQTT IN: '")) + strTopic + "' : '" + strPayload + "'");

    if(strTopic.startsWith(mqttNodeTopic)) {
        strTopic = strTopic.substring(mqttNodeTopic.length(), strTopic.length());
    } else if(strTopic.startsWith(mqttGroupTopic)) {
        strTopic = strTopic.substring(mqttGroupTopic.length(), strTopic.length());
    } else {
        return;
    }
    // debugPrintln(String(F("MQTT Short Topic : '")) + strTopic + "'");

    if(strTopic == F("command")) {
        dispatchCommand(strPayload);
        return;
    }

    if(strTopic.startsWith(F("command/"))) {
        strTopic = strTopic.substring(8u, strTopic.length());
        // debugPrintln(String(F("MQTT Shorter Command Topic : '")) + strTopic + "'");

        if(strTopic == F("page")) { // '[...]/device/command/page' -m '1' == nextionSendCmd("page 1")
            dispatchPage(strPayload);
        } else if(strTopic == F("dim")) { // '[...]/device/command/page' -m '1' == nextionSendCmd("page 1")
#if defined(ARDUINO_ARCH_ESP32)
            ledcWrite(0, map(strPayload.toInt(), 0, 100, 0, 1023)); // ledChannel and value
#else
            analogWrite(D1, map(strPayload.toInt(), 0, 100, 0, 1023));
#endif

        } else if(strTopic == F("json")) { // '[...]/device/command/json' -m '["dim=5", "page 1"]' =
            // nextionSendCmd("dim=50"), nextionSendCmd("page 1")
            dispatchJson(strPayload);              // Send to nextionParseJson()
        } else if(strTopic == F("statusupdate")) { // '[...]/device/command/statusupdate' == mqttStatusUpdate()
            //  mqttStatusUpdate();                 // return status JSON via MQTT
        } else if(strTopic == F("espupdate")) { // '[...]/device/command/espupdate' -m
                                                // 'http://192.168.0.10/local/HASwitchPlate.ino.d1_mini.bin' ==
            // espStartOta("http://192.168.0.10/local/HASwitchPlate.ino.d1_mini.bin")
            if(strPayload == "") {
                // espStartOta(espFirmwareUrl);
            } else {
                // espStartOta(strPayload);
            }
        } else if(strTopic == F("reboot")) { // '[...]/device/command/reboot' == reboot microcontroller)
            debugPrintln(F("MQTT: Rebooting device"));
            haspReset(true);
        } else if(strTopic == F("lcdreboot")) { // '[...]/device/command/lcdreboot' == reboot LCD panel)
            debugPrintln(F("MQTT: Rebooting LCD"));
            haspReset(true);
        } else if(strTopic == F("factoryreset")) { // '[...]/device/command/factoryreset' == clear all saved settings)
            // configClearSaved();
            //} else if(strPayload == "") { // '[...]/device/command/p[1].b[4].txt' -m '' ==
            // nextionGetAttr("p[1].b[4].txt")
            //    haspProcessAttribute(strTopic, "");
        } else { // '[...]/device/command/p[1].b[4].txt' -m '"Lights On"' ==
                 // nextionSetAttr("p[1].b[4].txt", "\"Lights On\"")
            dispatchAttribute(strTopic, strPayload);
        }
        return;
    }

    if(strTopic == mqttLightBrightCommandTopic) { // change the brightness from the light topic
        int panelDim = map(strPayload.toInt(), 0, 255, 0, 100);
        // nextionSetAttr("dim", String(panelDim));
        // nextionSendCmd("dims=dim");
        // mqttClient.publish(mqttLightBrightStateTopic, strPayload);
    } else if(strTopic == mqttLightCommandTopic &&
              strPayload == F("OFF")) { // set the panel dim OFF from the light topic, saving current dim level first
        // nextionSendCmd("dims=dim");
        // nextionSetAttr("dim", "0");
        mqttClient.publish(mqttLightStateTopic.c_str(), PSTR("OFF"));
    } else if(strTopic == mqttLightCommandTopic &&
              strPayload == F("ON")) { // set the panel dim ON from the light topic, restoring saved dim level
        // nextionSendCmd("dim=dims");
        mqttClient.publish(mqttLightStateTopic.c_str(), PSTR("ON"));
    }

    if(strTopic == F("status") &&
       strPayload == F("OFF")) { // catch a dangling LWT from a previous connection if it appears
        char topicBuffer[64];
        sprintf_P(topicBuffer, PSTR("%sstatus"), mqttNodeTopic.c_str());
        debugPrintln(String(F("MQTT: binary_sensor state: [")) + topicBuffer + "] : ON");
        mqttClient.publish(topicBuffer, "ON", true);
        return;
    }
}

void mqttReconnect()
{
    static uint8_t mqttReconnectCount = 0;
    bool mqttFirstConnect             = true;
    String nodeName                   = haspGetNodename();
    // Generate an MQTT client ID as haspNode + our MAC address
    mqttClientId = nodeName + "-" + WiFi.macAddress();

    char topicBuffer[64];
    sprintf_P(topicBuffer, PSTR("hasp/%s/"), nodeName.c_str());
    mqttNodeTopic = topicBuffer;
    sprintf_P(topicBuffer, PSTR("hasp/%s/"), mqttGroupName.c_str());
    mqttGroupTopic = topicBuffer;

    // haspSetPage(0);
    debugPrintln(String(F("MQTT: Attempting connection to broker ")) + String(mqttServer.c_str()) +
                 String(F(" as clientID ")) + mqttClientId);

    // Attempt to connect and set LWT and Clean Session
    sprintf_P(topicBuffer, PSTR("%sstatus"), mqttNodeTopic.c_str());
    if(!mqttClient.connect(mqttClientId.c_str(), mqttUser.c_str(), mqttPassword.c_str(), topicBuffer, 0, false, "OFF",
                           true)) {
        // Retry until we give up and restart after connectTimeout seconds
        mqttReconnectCount++;

        Serial.print(String(F("failed, rc=")));
        Serial.print(mqttClient.state());
        // Wait 5 seconds before retrying
        // delay(50);
        return;
    }

    debugPrintln(F("MQTT: [SUCCESS] MQTT Client is Connected"));
    haspReconnect();

    /*
        // MQTT topic string definitions
        mqttStateTopic              = prefix + F("/state");
        mqttStateJSONTopic          = prefix + F("/state/json");
        mqttCommandTopic            = prefix + F("/page");
        mqttGroupCommandTopic       = "hasp/" + mqttGroupName + "/page";
        mqttCommandTopic            = prefix + F("/command");
        mqttGroupCommandTopic       = "hasp/" + mqttGroupName + "/command";
        mqttSensorTopic             = prefix + F("/sensor");
        mqttLightCommandTopic       = prefix + F("/light/switch");
        mqttLightStateTopic         = prefix + F("/light/state");
        mqttLightBrightCommandTopic = prefix + F("/brightness/set");
        mqttLightBrightStateTopic   = prefix + F("/brightness/state");
        mqttMotionStateTopic        = prefix + F("/motion/state");
    */
    // Set keepAlive, cleanSession, timeout
    // mqttClient.setOptions(30, true, 5000);

    // declare LWT
    // mqttClient.setWill(mqttStatusTopic.c_str(), "OFF");

    // Attempt to connect to broker, setting last will and testament
    // Subscribe to our incoming topics
    sprintf_P(topicBuffer, PSTR("%scommand/#"), mqttGroupTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        debugPrintln(String(F("MQTT:    * Subscribed to ")) + topicBuffer);
    }
    sprintf_P(topicBuffer, PSTR("%scommand/#"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        debugPrintln(String(F("MQTT:    * Subscribed to ")) + topicBuffer);
    }
    sprintf_P(topicBuffer, PSTR("%slight/#"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        debugPrintln(String(F("MQTT:    * Subscribed to ")) + topicBuffer);
    }
    sprintf_P(topicBuffer, PSTR("%sbrightness/#"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        debugPrintln(String(F("MQTT:    * Subscribed to ")) + topicBuffer);
    }

    sprintf_P(topicBuffer, PSTR("%sstatus"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        debugPrintln(String(F("MQTT:    * Subscribed to ")) + topicBuffer);
    }
    // Force any subscribed clients to toggle OFF/ON when we first connect to
    // make sure we get a full panel refresh at power on.  Sending OFF,
    // "ON" will be sent by the mqttStatusTopic subscription action.
    debugPrintln(String(F("MQTT: binary_sensor state: [")) + topicBuffer + "] : " + (mqttFirstConnect ? "OFF" : "ON"));
    mqttClient.publish(topicBuffer, mqttFirstConnect ? "OFF" : "ON", true); //, 1);

    mqttFirstConnect   = false;
    mqttReconnectCount = 0;
}

void mqttSetup(const JsonObject & settings)
{
    mqttSetConfig(settings);

    mqttEnabled = mqttServer != "" && mqttPort > 0;
    if(!mqttEnabled) return;

    mqttClient.setServer(mqttServer.c_str(), 1883);
    mqttClient.setCallback(mqttCallback);
}

void mqttLoop(bool wifiIsConnected)
{
    if(!mqttEnabled) return;

    if(wifiIsConnected && !mqttClient.connected())
        mqttReconnect();
    else
        mqttClient.loop();
}

bool mqttIsConnected()
{
    return mqttClient.connected();
}

void mqttStop()
{
    if(mqttClient.connected()) {
        char topicBuffer[64];

        sprintf_P(topicBuffer, PSTR("%sstatus"), mqttNodeTopic.c_str());
        mqttClient.publish(topicBuffer, "OFF");

        sprintf_P(topicBuffer, PSTR("%ssensor"), mqttNodeTopic.c_str());
        mqttClient.publish(topicBuffer, "{\"status\": \"unavailable\"}");

        mqttClient.disconnect();
        debugPrintln(String(F("MQTT: Disconnected from broker")));
    }
}

bool mqttGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_GROUP)] = String(mqttGroupName.c_str());
    settings[FPSTR(F_CONFIG_HOST)]  = String(mqttServer.c_str());
    settings[FPSTR(F_CONFIG_PORT)]  = mqttPort;
    settings[FPSTR(F_CONFIG_USER)]  = String(mqttUser.c_str());
    settings[FPSTR(F_CONFIG_PASS)]  = String(mqttPassword.c_str());

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool mqttSetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(!settings[FPSTR(F_CONFIG_GROUP)].isNull()) {
        if(mqttGroupName != settings[FPSTR(F_CONFIG_GROUP)].as<String>().c_str()) {
            debugPrintln(F("mqttGroupName set"));
        }
        changed |= mqttGroupName != settings[FPSTR(F_CONFIG_GROUP)].as<String>().c_str();

        mqttGroupName = settings[FPSTR(F_CONFIG_GROUP)].as<String>().c_str();
    }

    if(!settings[FPSTR(F_CONFIG_HOST)].isNull()) {
        if(mqttServer != settings[FPSTR(F_CONFIG_HOST)].as<String>().c_str()) {
            debugPrintln(F("mqttServer set"));
        }
        changed |= mqttServer != settings[FPSTR(F_CONFIG_HOST)].as<String>().c_str();

        mqttServer = settings[FPSTR(F_CONFIG_HOST)].as<String>().c_str();
    }

    if(!settings[FPSTR(F_CONFIG_PORT)].isNull()) {
        if(mqttPort != settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>()) {
            debugPrintln(F("mqttPort set"));
        }
        changed |= mqttPort != settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>();

        mqttPort = settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>();
    }

    if(!settings[FPSTR(F_CONFIG_USER)].isNull()) {
        if(mqttUser != settings[FPSTR(F_CONFIG_USER)].as<String>().c_str()) {
            debugPrintln(F("mqttUser set"));
        }
        changed |= mqttUser != settings[FPSTR(F_CONFIG_USER)].as<String>().c_str();

        mqttUser = settings[FPSTR(F_CONFIG_USER)].as<String>().c_str();
    }

    if(!settings[FPSTR(F_CONFIG_PASS)].isNull()) {
        if(mqttPassword != settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str()) {
            debugPrintln(F("mqttPassword set"));
        }
        changed |= mqttPassword != settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str();

        mqttPassword = settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str();
    }

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return changed;
}
