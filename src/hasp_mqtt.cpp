#include <Arduino.h>
#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP.h>
#endif
#include <PubSubClient.h>

#include "hasp_log.h"
#include "hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_dispatch.h"
#include "hasp.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

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

#ifdef MQTT_NODENAME
char mqttNodeName[16] = MQTT_NODENAME;
#else
char mqttNodeName[16]  = "";
#endif

#ifdef MQTT_GROUPNAME
char mqttGroupName[16] = MQTT_GROUPNAME;
#else
char mqttGroupName[16] = "";
#endif

String mqttClientId((char *)0); // Auto-generated MQTT ClientID
String mqttNodeTopic((char *)0);
String mqttGroupTopic((char *)0);
bool mqttEnabled;

////////////////////////////////////////////////////////////////////////////////////////////////////
// These defaults may be overwritten with values saved by the web interface
#ifdef MQTT_HOST
char mqttServer[16] = MQTT_HOST;
#else
char mqttServer[16]    = "";
#endif
#ifdef MQTT_PORT
uint16_t mqttPort = MQTT_PORT;
#else
uint16_t mqttPort      = 1883;
#endif
#ifdef MQTT_USER
char mqttUser[23] = MQTT_USER;
#else
char mqttUser[23]      = "";
#endif
#ifdef MQTT_PASSW
char mqttPassword[32] = MQTT_PASSW;
#else
char mqttPassword[32]  = "";
#endif

/*
const String mqttLightSubscription        = "hasp/" + String(haspGetNodename()) + "/light/#";
const String mqttLightBrightSubscription  = "hasp/" + String(haspGetNodename()) + "/brightness/#";
*/

WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Send changed values OUT

void IRAM_ATTR mqttSendState(const char * subtopic, const char * payload)
{
    // page = 0
    // p[0].b[0].attr = abc
    // dim = 100
    // idle = 0/1
    // light = 0/1
    // brightness = 100

    // char mqttPayload[128 * 5];

    if(mqttClient.connected()) {
        char mqttTopic[128];
        snprintf_P(mqttTopic, sizeof(mqttTopic), PSTR("%sstate/%s"), mqttNodeTopic.c_str(), subtopic);
        mqttClient.publish(mqttTopic, payload);

        String msg((char *)0);
        msg.reserve(512);
        msg = F("MQTT OUT: ");
        msg += mqttTopic;
        msg += " = ";
        msg += payload;
        debugPrintln(msg);
    } else {
        errorPrintln(F("MQTT: %sNot connected"));
    }

    // as json
    // snprintf_P(mqttTopic, sizeof(mqttTopic), PSTR("%sstate/json"), mqttNodeTopic.c_str());
    // snprintf_P(mqttPayload, sizeof(mqttPayload), PSTR("{\"%s\":\"%s\"}"), subtopic, payload);
    // mqttClient.publish(mqttTopic, mqttPayload);
    // debugPrintln(String(F("MQTT OUT: ")) + String(mqttTopic) + " = " + String(mqttPayload));
}

void IRAM_ATTR mqttSendNewValue(uint8_t pageid, uint8_t btnid, const char * attribute, String txt)
{
    char topic[128];
    char payload[128];
    snprintf_P(topic, sizeof(topic), PSTR("json"));
    snprintf_P(payload, sizeof(payload), PSTR("{\"p[%u].b[%u].%s\":\"%s\"}"), pageid, btnid, attribute, txt.c_str());
    mqttSendState(topic, payload);
}

void IRAM_ATTR mqttSendNewValue(uint8_t pageid, uint8_t btnid, int32_t val)
{
    char value[128];
    itoa(val, value, 10);
    mqttSendNewValue(pageid, btnid, "val", value);
}

void IRAM_ATTR mqttSendNewValue(uint8_t pageid, uint8_t btnid, String txt)
{
    mqttSendNewValue(pageid, btnid, "txt", txt);
}

void IRAM_ATTR mqttSendNewEvent(uint8_t pageid, uint8_t btnid, char * value) // int32_t val)
{
    // char value[128];
    // itoa(val, value, 10);
    mqttSendNewValue(pageid, btnid, "event", value);
}

void mqttStatusUpdate()
{ // Periodically publish a JSON string indicating system status
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), "%u.%u.%u", HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);

    String mqttStatusPayload((char *)0);
    mqttStatusPayload.reserve(512);

    mqttStatusPayload += "{";
    mqttStatusPayload += F("\"status\":\"available\",");
    mqttStatusPayload += F("\"espVersion\":\"");
    mqttStatusPayload += buffer;
    mqttStatusPayload += F("\",");
    /*    if(updateEspAvailable) {
            mqttStatusPayload += F("\"updateEspAvailable\":true,");
        } else {
            mqttStatusPayload += F("\"updateEspAvailable\":false,");
        }
        if(lcdConnected) {
            mqttStatusPayload += F("\"lcdConnected\":true,");
        } else {
            mqttStatusPayload += F("\"lcdConnected\":false,");
        }
    mqttStatusPayload += F("\"lcdVersion\":\"");
    mqttStatusPayload += String(lcdVersion);
    mqttStatusPayload += F("\",");
    if(updateLcdAvailable) {
        mqttStatusPayload += F("\"updateLcdAvailable\":true,");
    } else {
        mqttStatusPayload += F("\"updateLcdAvailable\":false,");
    }*/
    mqttStatusPayload += F("\"espUptime\":");
    mqttStatusPayload += String(long(millis() / 1000));
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"signalStrength\":");
    mqttStatusPayload += String(WiFi.RSSI());
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"haspIP\":\"");
    mqttStatusPayload += WiFi.localIP().toString();
    mqttStatusPayload += F("\",");
    mqttStatusPayload += F("\"heapFree\":");
    mqttStatusPayload += String(ESP.getFreeHeap());
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"heapFragmentation\":");
    mqttStatusPayload += String(halGetHeapFragmentation());
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"espCore\":\"");
    mqttStatusPayload += halGetCoreVersion();
    mqttStatusPayload += F("\"");
    mqttStatusPayload += "}";

    // mqttClient.publish(mqttSensorTopic, mqttStatusPayload);
    // mqttClient.publish(mqttStatusTopic, "ON", true); //, 1);
    mqttSendState(String(F("statusupdate")).c_str(), mqttStatusPayload.c_str());

    // debugPrintln(String(F("MQTT: status update: ")) + String(mqttStatusPayload));
    // debugPrintln(String(F("MQTT: binary_sensor state: [")) + mqttStatusTopic + "] : [ON]");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Receive incoming messages
void mqttCallback(char * topic, byte * payload, unsigned int length)
{ // Handle incoming commands from MQTT
    payload[length] = '\0';
    String strTopic = topic;

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

    debugPrintln(String(F("MQTT IN: '")) + strTopic + "' : '" + (char *)payload + "'");

    if(strTopic.startsWith(mqttNodeTopic)) {
        strTopic = strTopic.substring(mqttNodeTopic.length(), strTopic.length());
    } else if(strTopic.startsWith(mqttGroupTopic)) {
        strTopic = strTopic.substring(mqttGroupTopic.length(), strTopic.length());
    } else {
        return;
    }
    // debugPrintln(String(F("MQTT Short Topic : '")) + strTopic + "'");

    if(strTopic == F("command")) {
        dispatchCommand((char *)payload);
        return;
    }

    if(strTopic.startsWith(F("command/"))) {
        strTopic = strTopic.substring(8u, strTopic.length());
        // debugPrintln(String(F("MQTT Shorter Command Topic : '")) + strTopic + "'");

        if(strTopic == F("json")) { // '[...]/device/command/json' -m '["dim=5", "page 1"]' =
            // nextionSendCmd("dim=50"), nextionSendCmd("page 1")
            dispatchJson((char *)payload); // Send to nextionParseJson()
        } else if(strTopic == F("jsonl")) {
            dispatchJsonl((char *)payload);
        } else if(length == 0) {
            dispatchCommand(strTopic.c_str());
        } else { // '[...]/device/command/p[1].b[4].txt' -m '"Lights On"' ==
                 // nextionSetAttr("p[1].b[4].txt", "\"Lights On\"")
            dispatchAttribute(strTopic, (char *)payload);
        }
        return;
    }

    String strPayload = (char *)payload;

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
        char topicBuffer[128];
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic.c_str());
        mqttClient.publish(topicBuffer, "ON", true);

        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT: binary_sensor state: [%sstatus] : ON"),
                   mqttNodeTopic.c_str());
        debugPrintln(topicBuffer);
        return;
    }
}

void mqttReconnect()
{
    static uint8_t mqttReconnectCount = 0;
    bool mqttFirstConnect             = true;
    String nodeName((char *)0);
    nodeName.reserve(128);
    nodeName = haspGetNodename();
    char topicBuffer[128];

    // Generate an MQTT client ID as haspNode + our MAC address
    mqttClientId.reserve(128);
    mqttClientId = nodeName;
    mqttClientId += F("-");
    mqttClientId += wifiGetMacAddress(3, "");
    WiFi.macAddress();

    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("hasp/%s/"), mqttNodeName);
    mqttNodeTopic = topicBuffer;
    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("hasp/%s/"), mqttGroupName);
    mqttGroupTopic = topicBuffer;

    // haspSetPage(0);
    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT: Attempting connection to broker %s as clientID %s"),
               mqttServer, mqttClientId.c_str());
    debugPrintln(topicBuffer);

    // Attempt to connect and set LWT and Clean Session
    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic.c_str());
    if(!mqttClient.connect(mqttClientId.c_str(), mqttUser, mqttPassword, topicBuffer, 0, false, "OFF", true)) {
        // Retry until we give up and restart after connectTimeout seconds
        mqttReconnectCount++;

        Serial.print(String(F("failed, rc=")));
        Serial.print(mqttClient.state());
        // Wait 5 seconds before retrying
        // delay(50);
        return;
    }

    debugPrintln(F("MQTT: MQTT Client is Connected"));
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
    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%scommand/#"), mqttGroupTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT:    * Subscribed to %scommand/#"),
                   mqttGroupTopic.c_str());
        debugPrintln(topicBuffer);
    }

    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%scommand/#"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT:    * Subscribed to %scommand/#"),
                   mqttNodeTopic.c_str());
        debugPrintln(topicBuffer);
    }

    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%slight/#"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT:    * Subscribed to %slight/#"), mqttNodeTopic.c_str());
        debugPrintln(topicBuffer);
    }

    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sbrightness/#"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT:    * Subscribed to %sbrightness/#"),
                   mqttNodeTopic.c_str());
        debugPrintln(topicBuffer);
    }

    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic.c_str());
    if(mqttClient.subscribe(topicBuffer)) {
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT:    * Subscribed to %sstatus"), mqttNodeTopic.c_str());
        debugPrintln(topicBuffer);
    }
    // Force any subscribed clients to toggle OFF/ON when we first connect to
    // make sure we get a full panel refresh at power on.  Sending OFF,
    // "ON" will be sent by the mqttStatusTopic subscription action.
    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic.c_str());
    mqttClient.publish(topicBuffer, mqttFirstConnect ? "OFF" : "ON", true); //, 1);
    snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT: binary_sensor state: [%sstatus] : %s"),
               mqttNodeTopic.c_str(), mqttFirstConnect ? PSTR("OFF") : PSTR("ON"));
    debugPrintln(topicBuffer);

    mqttFirstConnect   = false;
    mqttReconnectCount = 0;
}

void mqttSetup(const JsonObject & settings)
{
    mqttClientId.reserve(128);
    mqttNodeTopic.reserve(128);
    mqttGroupTopic.reserve(128);

    mqttSetConfig(settings);

    mqttEnabled = strcmp(mqttServer, "") != 0 && mqttPort > 0;
    if(!mqttEnabled) return;

    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setCallback(mqttCallback);

    debugPrintln(F("MQTT: Setup Complete"));
}

void mqttLoop(bool wifiIsConnected)
{
    if(!mqttEnabled) return;

    if(wifiIsConnected && !mqttClient.connected())
        mqttReconnect();
    else
        mqttClient.loop();
}

String mqttGetNodename()
{
    return mqttNodeName;
}

bool mqttIsConnected()
{
    return mqttClient.connected();
}

void mqttStop()
{
    if(mqttClient.connected()) {
        char topicBuffer[128];

        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic.c_str());
        mqttClient.publish(topicBuffer, "OFF");

        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%ssensor"), mqttNodeTopic.c_str());
        mqttClient.publish(topicBuffer, "{\"status\": \"unavailable\"}");

        mqttClient.disconnect();
        debugPrintln(F("MQTT: Disconnected from broker"));
    }
}

bool mqttGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_NAME)]  = mqttNodeName;
    settings[FPSTR(F_CONFIG_GROUP)] = mqttGroupName;
    settings[FPSTR(F_CONFIG_HOST)]  = mqttServer;
    settings[FPSTR(F_CONFIG_PORT)]  = mqttPort;
    settings[FPSTR(F_CONFIG_USER)]  = mqttUser;
    settings[FPSTR(F_CONFIG_PASS)]  = mqttPassword;

    configOutput(settings);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool mqttSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    changed |= configSet(mqttPort, settings[FPSTR(F_CONFIG_PORT)], PSTR("mqttPort"));

    if(!settings[FPSTR(F_CONFIG_NAME)].isNull()) {
        changed |= strcmp(mqttNodeName, settings[FPSTR(F_CONFIG_NAME)]) != 0;
        strncpy(mqttNodeName, settings[FPSTR(F_CONFIG_NAME)], sizeof(mqttNodeName));
    }
    // Prefill node name
    if(strlen(mqttNodeName) == 0) {
        String mac = wifiGetMacAddress(3, "");
        mac.toLowerCase();
        snprintf_P(mqttNodeName, sizeof(mqttNodeName), PSTR("plate_%s"), mac.c_str());
    }

    if(!settings[FPSTR(F_CONFIG_GROUP)].isNull()) {
        changed |= strcmp(mqttGroupName, settings[FPSTR(F_CONFIG_GROUP)]) != 0;
        strncpy(mqttGroupName, settings[FPSTR(F_CONFIG_GROUP)], sizeof(mqttGroupName));
    }

    if(!settings[FPSTR(F_CONFIG_HOST)].isNull()) {
        changed |= strcmp(mqttServer, settings[FPSTR(F_CONFIG_HOST)]) != 0;
        strncpy(mqttServer, settings[FPSTR(F_CONFIG_HOST)], sizeof(mqttServer));
    }

    if(!settings[FPSTR(F_CONFIG_USER)].isNull()) {
        changed |= strcmp(mqttUser, settings[FPSTR(F_CONFIG_USER)]) != 0;
        strncpy(mqttUser, settings[FPSTR(F_CONFIG_USER)], sizeof(mqttUser));
    }

    if(!settings[FPSTR(F_CONFIG_PASS)].isNull() &&
       settings[FPSTR(F_CONFIG_PASS)].as<String>() != String(FPSTR("********"))) {
        changed |= strcmp(mqttPassword, settings[FPSTR(F_CONFIG_PASS)]) != 0;
        strncpy(mqttPassword, settings[FPSTR(F_CONFIG_PASS)], sizeof(mqttPassword));
    }

    return changed;
}
