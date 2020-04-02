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

#include "hasp_hal.h"
#include "hasp_tft.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_dispatch.h"
#include "hasp.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

extern unsigned long debugLastMillis; // UpdateStatus timer

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

void mqtt_log_no_connection()
{
    Log.error(F("MQTT: Not connected"));
}

bool IRAM_ATTR mqttIsConnected()
{
    return mqttEnabled && mqttClient.connected();
}

void IRAM_ATTR mqtt_send_state(const __FlashStringHelper * subtopic, const char * payload)
{
    // page = 0
    // p[0].b[0].attr = abc
    // dim = 100
    // idle = 0/1
    // light = 0/1
    // brightness = 100

    if(mqttIsConnected()) {
        char topic[64];
        snprintf_P(topic, sizeof(topic), PSTR("%sstate/%S"), mqttNodeTopic, subtopic);
        mqttClient.publish(topic, payload);
    } else {
        return mqtt_log_no_connection();
    }

    // Log after char buffers are cleared
    Log.notice(F("MQTT OUT: %sstate/%S = %s"), mqttNodeTopic, subtopic, payload);
}

void IRAM_ATTR mqtt_send_input(uint8_t id, const char * payload)
{
    Log.trace(F("MQTT TST: %sstate/input%u = %s"), mqttNodeTopic, id, payload); // to be remove

    if(mqttIsConnected()) {
        char topic[40];
        snprintf_P(topic, sizeof(topic), PSTR("%sstate/input%u"), mqttNodeTopic, id);
        mqttClient.publish(topic, payload);
    } else {
        return mqtt_log_no_connection();
    }

    // Log after char buffers are cleared
    Log.notice(F("MQTT OUT: %sstate/input%u = %s"), mqttNodeTopic, id, payload);
}

void IRAM_ATTR mqtt_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data)
{
    if(mqttIsConnected()) {
        char topic[64];
        char payload[128];

        snprintf_P(topic, sizeof(topic), PSTR("%sstate/json"), mqttNodeTopic);
        unsigned int len =
            snprintf_P(payload, sizeof(payload), PSTR("{\"p[%u].b[%u].%s\":\"%s\"}"), pageid, btnid, attribute, data);

        mqttClient.publish(topic, (uint8_t *)payload, len, false);
    } else {
        return mqtt_log_no_connection();
    }

    // Log after char buffers are cleared
    Log.notice(F("MQTT OUT: %sstate/json = {\"p[%u].b[%u].%s\":\"%s\"}"), mqttNodeTopic, pageid, btnid, attribute,
               data);
}

/*void IRAM_ATTR mqtt_send_attribute_P(uint8_t pageid, uint8_t btnid, const char * attr, const char * data)
{
    char * buffer;
    buffer = (char *)malloc(strlen_P(attr) + 1);
    strcpy_P(buffer, attr);
    mqtt_send_attribute_str(pageid, btnid, buffer, data);
    free(buffer);
}

void IRAM_ATTR mqtt_send_attribute_txt(uint8_t pageid, uint8_t btnid, const char * txt)
{
    mqtt_send_attribute_P(pageid, btnid, PSTR("txt"), txt);
}

void IRAM_ATTR mqtt_send_attribute_val(uint8_t pageid, uint8_t btnid, int32_t val)
{
    char data[64];
    itoa(val, data, 10);
    mqtt_send_attribute_P(pageid, btnid, PSTR("val"), data);
}

void IRAM_ATTR mqtt_send_attribute_event(uint8_t pageid, uint8_t btnid, const char * event)
{
    mqtt_send_attribute_P(pageid, btnid, PSTR("event"), event);
}*/

void mqtt_send_statusupdate()
{ // Periodically publish a JSON string indicating system status
    //    String mqttStatusPayload((char *)0);
    //    mqttStatusPayload.reserve(512);
    debugLastMillis = millis();

    DynamicJsonDocument doc(3 * 128);

    doc[F("status")] = F("available");

    doc[F("version")] = haspGetVersion();
    doc[F("uptime")]  = long(millis() / 1000);

    doc[F("rssi")] = WiFi.RSSI();
    doc[F("ip")]   = WiFi.localIP().toString();

    doc[F("heapFree")] = ESP.getFreeHeap();
    doc[F("heapFrag")] = halGetHeapFragmentation();

    doc[F("espCanUpdate")] = false;
    doc[F("espCore")]      = halGetCoreVersion();

    doc[F("page")]     = haspGetPage();
    doc[F("numPages")] = (HASP_NUM_PAGES);

    doc[F("tftDriver")] = tftDriverName();
    doc[F("tftWidth")]  = (TFT_WIDTH);
    doc[F("tftHeight")] = (TFT_HEIGHT);

#if defined(ARDUINO_ARCH_ESP8266)
    doc[F("espVcc")] = (float)ESP.getVcc() / 1000;
#endif

    char buffer[2 * 128];
    size_t n = serializeJson(doc, buffer, sizeof(buffer));
    mqtt_send_state(F("statusupdate"), buffer);

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
*/

    // mqttClient.publish(mqttSensorTopic, mqttStatusPayload);
    // mqttClient.publish(mqttStatusTopic, "ON", true); //, 1);

    // debugPrintln(String(F("MQTT: status update: ")) + String(mqttStatusPayload));
    // debugPrintln(String(F("MQTT: binary_sensor state: [")) + mqttStatusTopic + "] : [ON]");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Receive incoming messages
static void mqtt_message_cb(char * topic_p, byte * payload, unsigned int length)
{ // Handle incoming commands from MQTT
    payload[length] = '\0';

    // String strTopic((char *)0);
    // strTopic.reserve(MQTT_MAX_PACKET_SIZE);

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

    char * topic = (char *)topic_p;
    Log.notice(F("MQTT  IN: %s = %s"), topic, (char *)payload);

    if(topic == strstr(topic, mqttNodeTopic)) { // startsWith mqttNodeTopic
        topic += strlen(mqttNodeTopic);
    } else if(topic == strstr(topic, mqttGroupTopic)) { // startsWith mqttGroupTopic
        topic += strlen(mqttGroupTopic);
    } else {
        Log.error(F("MQTT: Message received with invalid topic"));
        return;
    }
    // Log.trace(F("MQTT IN: short topic: %s"), topic);

    if(!strcmp_P(topic, PSTR("command"))) {
        dispatchCommand((char *)payload);
        return;
    }

    if(topic == strstr_P(topic, PSTR("command/"))) { // startsWith command/
        topic += 8u;
        // Log.trace(F("MQTT IN: command subtopic: %s"), topic);

        if(!strcmp_P(topic, PSTR("json"))) { // '[...]/device/command/json' -m '["dim=5", "page 1"]' =
            // nextionSendCmd("dim=50"), nextionSendCmd("page 1")
            dispatchJson((char *)payload); // Send to nextionParseJson()
        } else if(!strcmp_P(topic, PSTR("jsonl"))) {
            dispatchJsonl((char *)payload);
        } else if(length == 0) {
            dispatchCommand(topic);
        } else { // '[...]/device/command/p[1].b[4].txt' -m '"Lights On"' ==
                 // nextionSetAttr("p[1].b[4].txt", "\"Lights On\"")
            dispatchAttribute(topic, (char *)payload);
        }
        return;
    }

    // catch a dangling LWT from a previous connection if it appears
    if(!strcmp_P(topic, PSTR("status")) && !strcmp_P((char *)payload, PSTR("OFF"))) {
        char topicBuffer[128];
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic);
        mqttClient.publish(topicBuffer, "ON", true);
        Log.notice(F("MQTT: binary_sensor state: [status] : ON"));
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
}

void mqttSubscribeTo(const char * format, const char * data)
{
    char topic[128];
    snprintf_P(topic, sizeof(topic), format, data);
    if(mqttClient.subscribe(topic)) {
        Log.verbose(F("MQTT:    * Subscribed to %s"), topic);
    } else {
        Log.error(F("MQTT: Failed to subscribe to %s"), topic);
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

        if(mqttReconnectCount > 50) {
            Log.error(F("MQTT: %sRetry count exceeded, rebooting..."));
            dispatchReboot(false);
        }
        return;
    }

    Log.notice(F("MQTT: [SUCCESS] Connected to broker %s as clientID %s"), mqttServer, mqttClientId);

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

    haspReconnect();
    dispatchPage(String(haspGetPage()));
    mqtt_send_statusupdate();
}

void mqttSetup(const JsonObject & settings)
{
    // mqttSetConfig(settings);

    mqttEnabled = strlen(mqttServer) > 0 && mqttPort > 0;
    if(!mqttEnabled) return Log.notice(F("MQTT: Broker not configured"));

    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setCallback(mqtt_message_cb);

    Log.notice(F("MQTT: Setup Complete"));
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

/** Set MQTT Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
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
