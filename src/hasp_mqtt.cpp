#include "hasp_conf.h"
#if HASP_USE_MQTT

#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "PubSubClient.h"

#include "hasp_conf.h"
#include "hasp_mqtt.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP.h>
#endif

//#include "hasp_log.h"
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

String mqttLightCommandTopic;       // MQTT topic for incoming panel backlight on/off commands
String mqttLightStateTopic;         // MQTT topic for outgoing panel backlight on/off state
String mqttLightBrightCommandTopic; // MQTT topic for incoming panel backlight dimmer commands
String mqttLightBrightStateTopic;   // MQTT topic for outgoing panel backlight dimmer state
// String mqttMotionStateTopic;        // MQTT topic for outgoing motion sensor state
*/

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

// String mqttClientId((char *)0); // Auto-generated MQTT ClientID
// String mqttNodeTopic((char *)0);
// String mqttGroupTopic((char *)0);
char mqttNodeTopic[24];
char mqttGroupTopic[24];
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

    if(mqttClient.connected()) {
        char topic[128];
        snprintf_P(topic, sizeof(topic), PSTR("%sstate/%s"), mqttNodeTopic, subtopic);
        mqttClient.publish(topic, payload);
        Log.notice(F("MQTT OUT: %s = %s"), topic, payload);

        // String msg((char *)0);
        /* msg.reserve(256 + 128);
        msg = F("MQTT OUT: ");
        msg += mqttTopic;
        msg += " = ";
        msg += payload;
        debugPrintln(msg);
        msg.clear();
        */
    } else {
        Log.error(F("MQTT: Not connected"));
    }
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
    mqttStatusPayload += F("\"version\":\"");
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
    mqttStatusPayload += F("\"uptime\":");
    mqttStatusPayload += String(long(millis() / 1000));
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"rssi\":");
    mqttStatusPayload += String(WiFi.RSSI());
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"ip\":\"");
    mqttStatusPayload += WiFi.localIP().toString();
    mqttStatusPayload += F("\",");
    mqttStatusPayload += F("\"heapFree\":");
    mqttStatusPayload += String(ESP.getFreeHeap());
    mqttStatusPayload += F(",");
    mqttStatusPayload += F("\"heapFrag\":");
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

    String strTopic((char *)0);
    strTopic.reserve(MQTT_MAX_PACKET_SIZE);

    Log.notice(F("MQTT  IN: %s = %s"), topic, (char *)payload);

    /* Debug feedback */
    /* strTopic = F("MQTT IN: '");
    strTopic += topic;
    strTopic += F("' : '");
    strTopic += (char *)payload;
    strTopic += F("'");
    debugPrintln(strTopic); */

    /* Reset the actual topic */
    strTopic = topic;

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

    if(strTopic.startsWith(mqttNodeTopic)) {
        strTopic = strTopic.substring(strlen(mqttNodeTopic), strTopic.length());
    } else if(strTopic.startsWith(mqttGroupTopic)) {
        strTopic = strTopic.substring(strlen(mqttGroupTopic), strTopic.length());
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

    /*
    String strPayload = (char *)payload;

        if(strTopic == mqttLightBrightCommandTopic) { // change the brightness from the light topic
            int panelDim = map(strPayload.toInt(), 0, 255, 0, 100);
            // nextionSetAttr("dim", String(panelDim));
            // nextionSendCmd("dims=dim");
            // mqttClient.publish(mqttLightBrightStateTopic, strPayload);
        } else if(strTopic == mqttLightCommandTopic &&
                  strPayload == F("OFF")) { // set the panel dim OFF from the light topic, saving current dim level
       first
            // nextionSendCmd("dims=dim");
            // nextionSetAttr("dim", "0");
            mqttClient.publish(mqttLightStateTopic.c_str(), PSTR("OFF"));
        } else if(strTopic == mqttLightCommandTopic &&
                  strPayload == F("ON")) { // set the panel dim ON from the light topic, restoring saved dim level
            // nextionSendCmd("dim=dims");
            mqttClient.publish(mqttLightStateTopic.c_str(), PSTR("ON"));
        }
    */

    // catch a dangling LWT from a previous connection if it appears
    if(strTopic == F("status") && strcmp_P((char *)payload, PSTR("OFF")) == 0) {
        char topicBuffer[128];
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic);
        mqttClient.publish(topicBuffer, "ON", true);

        Log.notice(F("MQTT: binary_sensor state: [status] : ON"));
        // snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("MQTT: binary_sensor state: [%sstatus] : ON"),
        // mqttNodeTopic); debugPrintln(topicBuffer);
        return;
    }
}

void mqttSubscribeTo(const char * format, const char * data)
{
    char buffer[128];
    char topic[128];
    snprintf_P(topic, sizeof(topic), format, data);
    if(mqttClient.subscribe(topic)) {
        Log.verbose(F("MQTT:    * Subscribed to %s"), topic);

        // snprintf_P(buffer, sizeof(buffer), PSTR("MQTT:    * Subscribed to %s"), topic);
        // debugPrintln(buffer);
    } else {
        Log.error(F("MQTT: Failed to subscribe to %s"), topic);

        // snprintf_P(buffer, sizeof(buffer), PSTR("MQTT: %%sFailed to subscribe to %s"), topic);
        // errorPrintln(buffer);
    }
}

void mqttReconnect()
{
    static uint8_t mqttReconnectCount = 0;
    bool mqttFirstConnect             = true;
    char mqttClientId[128];
    char buffer[128];

    snprintf(mqttClientId, sizeof(mqttClientId), PSTR("%s-%s"), mqttNodeName, wifiGetMacAddress(3, "").c_str());
    snprintf_P(mqttNodeTopic, sizeof(mqttNodeTopic), PSTR("hasp/%s/"), mqttNodeName);
    snprintf_P(mqttGroupTopic, sizeof(mqttGroupTopic), PSTR("hasp/%s/"), mqttGroupName);

    // Attempt to connect and set LWT and Clean Session
    snprintf_P(buffer, sizeof(buffer), PSTR("%sstatus"), mqttNodeTopic);
    if(!mqttClient.connect(mqttClientId, mqttUser, mqttPassword, buffer, 0, false, "OFF", true)) {
        // Retry until we give up and restart after connectTimeout seconds
        mqttReconnectCount++;
        snprintf_P(buffer, sizeof(buffer), PSTR("MQTT: %%s"));
        switch(mqttClient.state()) {
            case MQTT_CONNECTION_TIMEOUT:
                strcat_P(buffer, PSTR("Server didn't respond within the keepalive time"));
                break;
            case MQTT_CONNECTION_LOST:
                strcat_P(buffer, PSTR("Network connection was broken"));
                break;
            case MQTT_CONNECT_FAILED:
                strcat_P(buffer, PSTR("Network connection failed"));
                break;
            case MQTT_DISCONNECTED:
                strcat_P(buffer, PSTR("Client is disconnected cleanly"));
                break;
            case MQTT_CONNECTED:
                strcat_P(buffer, PSTR("(Client is connected"));
                break;
            case MQTT_CONNECT_BAD_PROTOCOL:
                strcat_P(buffer, PSTR("Server doesn't support the requested version of MQTT"));
                break;
            case MQTT_CONNECT_BAD_CLIENT_ID:
                strcat_P(buffer, PSTR("Server rejected the client identifier"));
                break;
            case MQTT_CONNECT_UNAVAILABLE:
                strcat_P(buffer, PSTR("Server was unable to accept the connection"));
                break;
            case MQTT_CONNECT_BAD_CREDENTIALS:
                strcat_P(buffer, PSTR("Username or Password rejected"));
                break;
            case MQTT_CONNECT_UNAUTHORIZED:
                strcat_P(buffer, PSTR("Client was not authorized to connect"));
                break;
            default:
                strcat_P(buffer, PSTR("Unknown failure"));
        }
        Log.warning(buffer);
        // errorPrintln(buffer);

        if(mqttReconnectCount > 50) {
            Log.error(F("MQTT: %sRetry count exceeded, rebooting..."));
            // errorPrintln(F("MQTT: %sRetry count exceeded, rebooting..."));
            dispatchReboot(false);
        }
        return;
    }

    Log.notice(F("MQTT: [SUCCESS] Connected to broker %s as clientID %s"), mqttServer, mqttClientId);
    /* snprintf_P(buffer, sizeof(buffer), PSTR("MQTT: [SUCCESS] Connected to broker %s as clientID %s"), mqttServer,
               mqttClientId);
    debugPrintln(buffer); */
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
    mqttSubscribeTo(PSTR("%scommand/#"), mqttGroupTopic);
    mqttSubscribeTo(PSTR("%scommand/#"), mqttNodeTopic);
    mqttSubscribeTo(PSTR("%slight/#"), mqttNodeTopic);
    mqttSubscribeTo(PSTR("%sbrightness/#"), mqttNodeTopic);
    mqttSubscribeTo(PSTR("%sstatus"), mqttNodeTopic);

    // Force any subscribed clients to toggle OFF/ON when we first connect to
    // make sure we get a full panel refresh at power on.  Sending OFF,
    // "ON" will be sent by the mqttStatusTopic subscription action.
    snprintf_P(buffer, sizeof(buffer), PSTR("%sstatus"), mqttNodeTopic);
    mqttClient.publish(buffer, mqttFirstConnect ? "OFF" : "ON", true); //, 1);

    Log.notice(F("MQTT: binary_sensor state: [%sstatus] : %s"), mqttNodeTopic,
               mqttFirstConnect ? PSTR("OFF") : PSTR("ON"));

    /* snprintf_P(buffer, sizeof(buffer), PSTR("MQTT: binary_sensor state: [%sstatus] : %s"), mqttNodeTopic,
               mqttFirstConnect ? PSTR("OFF") : PSTR("ON"));
    debugPrintln(buffer); */

    mqttFirstConnect   = false;
    mqttReconnectCount = 0;
}

void mqttSetup(const JsonObject & settings)
{
    mqttSetConfig(settings);

    mqttEnabled = strcmp(mqttServer, "") != 0 && mqttPort > 0;
    if(!mqttEnabled) return;

    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setCallback(mqttCallback);

    Log.notice(F("MQTT: Setup Complete"));
    //   debugPrintln(F("MQTT: Setup Complete"));
}

void mqttLoop()
{
    if(!mqttEnabled) return;
    mqttClient.loop();
}

void mqttEvery5Seconds(bool wifiIsConnected)
{
    if(!mqttEnabled) return;
    if(wifiIsConnected && !mqttClient.connected()) mqttReconnect();
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

        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic);
        mqttClient.publish(topicBuffer, "OFF");

        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%ssensor"), mqttNodeTopic);
        mqttClient.publish(topicBuffer, "{\"status\": \"unavailable\"}");

        mqttClient.disconnect();
        Log.notice(F("MQTT: Disconnected from broker"));
        //    debugPrintln(F("MQTT: Disconnected from broker"));
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

#endif // HASP_USE_MQTT
