#include "hasp_conf.h"
#if HASP_USE_MQTT > 0

#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "PubSubClient.h"

#include "hasp_conf.h"
#include "hasp_mqtt.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
WiFiClient mqttNetworkClient;
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP.h>
WiFiClient mqttNetworkClient;
#else
#if defined(STM32F4xx) && HASP_USE_WIFI>0
// #include <WiFi.h>
WiFiSpiClient mqttNetworkClient;
#else
#if defined(W5500_MOSI) && defined(W5500_MISO) && defined(W5500_SCLK)
#define W5500_LAN
#include <Ethernet.h>
#else
#include <STM32Ethernet.h>
#endif

EthernetClient mqttNetworkClient;
#endif
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
#ifndef MQTT_PREFIX
#define MQTT_PREFIX "hasp"
#endif

/*
const String mqttLightSubscription        = "hasp/" + String(haspGetNodename()) + "/light/#";
const String mqttLightBrightSubscription  = "hasp/" + String(haspGetNodename()) + "/brightness/#";
*/

PubSubClient mqttClient(mqttNetworkClient);

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
        snprintf_P(topic, sizeof(topic), PSTR("%sstate/%s"), mqttNodeTopic, subtopic);
        mqttClient.publish(topic, payload);
    } else {
        return mqtt_log_no_connection();
    }

    // Log after char buffers are cleared
    Log.notice(F("MQTT PUB: %sstate/%S = %s"), mqttNodeTopic, subtopic, payload);
}

void mqtt_send_input(uint8_t id, const char * payload)
{
    // Log.trace(F("MQTT TST: %sstate/input%u = %s"), mqttNodeTopic, id, payload); // to be removed

    if(mqttIsConnected()) {
        char topic[64];
        snprintf_P(topic, sizeof(topic), PSTR("%sstate/input%u"), mqttNodeTopic, id);
        mqttClient.publish(topic, payload);
    } else {
        return mqtt_log_no_connection();
    }

    // Log after char buffers are cleared
    Log.notice(F("MQTT PUB: %sstate/input%u = %s"), mqttNodeTopic, id, payload);
}

void IRAM_ATTR mqtt_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data)
{
    if(mqttIsConnected()) {
        char payload[128];
        char topic[64];

        snprintf_P(topic, sizeof(topic), PSTR("%sstate/json"), mqttNodeTopic);
        unsigned int len =
            snprintf_P(payload, sizeof(payload), PSTR("{\"p[%u].b[%u].%s\":\"%s\"}"), pageid, btnid, attribute, data);

        mqttClient.publish(topic, (uint8_t *)payload, len, false);
    } else {
        return mqtt_log_no_connection();
    }

    // Log after char buffers are cleared
    Log.notice(F("MQTT PUB: %sstate/json = {\"p[%u].b[%u].%s\":\"%s\"}"), mqttNodeTopic, pageid, btnid, attribute,
               data);
}

void mqtt_send_statusupdate()
{ // Periodically publish a JSON string indicating system status
    char data[3 * 128];
    {
        char buffer[128];

        snprintf_P(data, sizeof(data), PSTR("{\"status\":\"available\",\"version\":\"%s\",\"uptime\":%lu,"),
                   haspGetVersion().c_str(), long(millis() / 1000));
        strcat(buffer, data);
#if HASP_USE_WIFI>0
#if defined(STM32F4xx)
        IPAddress ip;
        ip = WiFi.localIP();
        char espIp[16];
        memset(espIp, 0 ,sizeof(espIp));
        snprintf_P(buffer, sizeof(buffer), PSTR("\"ssid\":\"%s\",\"rssi\":%i,\"ip\":\"%d.%d.%d.%d\","), WiFi.SSID(),
                   WiFi.RSSI(), ip[0], ip[1], ip[2], ip[3]);
#else
        snprintf_P(buffer, sizeof(buffer), PSTR("\"ssid\":\"%s\",\"rssi\":%i,\"ip\":\"%s\","), WiFi.SSID().c_str(),
                   WiFi.RSSI(), WiFi.localIP().toString().c_str());
#endif
        strcat(data, buffer);
#endif
        snprintf_P(buffer, sizeof(buffer), PSTR("\"heapFree\":%u,\"heapFrag\":%u,\"espCore\":\"%s\","),
                   halGetFreeHeap(), halGetHeapFragmentation(), halGetCoreVersion().c_str());
        strcat(data, buffer);
        snprintf_P(buffer, sizeof(buffer), PSTR("\"espCanUpdate\":\"false\",\"page\":%u,\"numPages\":%u,"),
                   haspGetPage(), (HASP_NUM_PAGES));
        strcat(data, buffer);

#if defined(ARDUINO_ARCH_ESP8266)
        snprintf_P(buffer, sizeof(buffer), PSTR("\"espVcc\":%.2f,"), (float)ESP.getVcc() / 1000);
        strcat(data, buffer);
#endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"tftDriver\":\"%s\",\"tftWidth\":%u,\"tftHeight\":%u}"),
                   halDisplayDriverName().c_str(), (TFT_WIDTH), (TFT_HEIGHT));
        strcat(data, buffer);
    }
    mqtt_send_state(F("statusupdate"), data);
    debugLastMillis = millis();

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
    if(length >= MQTT_MAX_PACKET_SIZE) {
        Log.error(F("MQTT RCV: Payload too long (%d bytes)"), length);
        return;
    }
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
    Log.notice(F("MQTT RCV: %s = %s"), topic, (char *)payload);

    if(topic == strstr(topic, mqttNodeTopic)) { // startsWith mqttNodeTopic
        topic += strlen(mqttNodeTopic);
    } else if(topic == strstr(topic, mqttGroupTopic)) { // startsWith mqttGroupTopic
        topic += strlen(mqttGroupTopic);
    } else {
        Log.error(F("MQTT: Message received with invalid topic"));
        return;
    }

    // catch a dangling LWT from a previous connection if it appears
    if(!strcmp_P(topic, PSTR("status")) && !strcasecmp_P((char *)payload, PSTR("OFF"))) {
        char topicBuffer[128];
        snprintf_P(topicBuffer, sizeof(topicBuffer), PSTR("%sstatus"), mqttNodeTopic);
        mqttClient.publish(topicBuffer, "ON", true);
        Log.notice(F("MQTT: binary_sensor state: [status] : ON"));
        // return;
    } else {
        dispatchTopicPayload(topic, (char *)payload);
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
    char topic[64];
    snprintf_P(topic, sizeof(topic), format, data);
    if(mqttClient.subscribe(topic)) {
        Log.verbose(F("MQTT:    * Subscribed to %s"), topic);
    } else {
        Log.error(F("MQTT: Failed to subscribe to %s"), topic);
    }
}

void mqttReconnect()
{
    char buffer[128];
    char mqttClientId[64];
    static uint8_t mqttReconnectCount = 0;
    bool mqttFirstConnect             = true;

    {
        String mac = halGetMacAddress(3, "");
        mac.toLowerCase();
        memset(mqttClientId, 0, sizeof(mqttClientId));
        snprintf_P(mqttClientId, sizeof(mqttClientId), PSTR("plate_%s"), mac.c_str());
        Log.verbose(mqttClientId);
    }

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
    mqttSubscribeTo(PSTR("%sconfig/#"), mqttGroupTopic);
    mqttSubscribeTo(PSTR("%sconfig/#"), mqttNodeTopic);
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
    char page[4] = "999";
    itoa(haspGetPage(), page, DEC);
    dispatchPage(page);
    mqtt_send_statusupdate();
}

void mqttSetup()
{
    mqttEnabled = strlen(mqttServer) > 0 && mqttPort > 0;
    if(mqttEnabled) {
        mqttClient.setServer(mqttServer, 1883);
        mqttClient.setCallback(mqtt_message_cb);
        Log.notice(F("MQTT: Setup Complete"));
    } else {
        Log.notice(F("MQTT: Broker not configured"));
    }
}

void mqttLoop()
{
    if(mqttEnabled) mqttClient.loop();
}

void mqttEvery5Seconds(bool networkIsConnected)
{
    if(mqttEnabled && networkIsConnected && !mqttClient.connected()) mqttReconnect();
}

String mqttGetNodename()
{
    return mqttNodeName;
}

void mqttStop()
{
    if(mqttEnabled && mqttClient.connected()) {
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
    bool changed = false;

    if(strcmp(mqttNodeName, settings[FPSTR(F_CONFIG_NAME)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_NAME)] = mqttNodeName;

    if(strcmp(mqttGroupName, settings[FPSTR(F_CONFIG_GROUP)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_GROUP)] = mqttGroupName;

    if(strcmp(mqttServer, settings[FPSTR(F_CONFIG_HOST)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_HOST)] = mqttServer;

    if(mqttPort != settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(F_CONFIG_PORT)] = mqttPort;

    if(strcmp(mqttUser, settings[FPSTR(F_CONFIG_USER)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_USER)] = mqttUser;

    if(strcmp(mqttPassword, settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_PASS)] = mqttPassword;

    if(changed) configOutput(settings);
    return changed;
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
        String mac = halGetMacAddress(3, "");
        mac.toLowerCase();
        snprintf_P(mqttNodeName, sizeof(mqttNodeName), PSTR("plate_%s"), mac.c_str());
        changed = true;
    }

    if(!settings[FPSTR(F_CONFIG_GROUP)].isNull()) {
        changed |= strcmp(mqttGroupName, settings[FPSTR(F_CONFIG_GROUP)]) != 0;
        strncpy(mqttGroupName, settings[FPSTR(F_CONFIG_GROUP)], sizeof(mqttGroupName));
    }

    if(strlen(mqttGroupName) == 0) {
        strcpy_P(mqttGroupName, PSTR("plates"));
        changed = true;
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

    snprintf_P(mqttNodeTopic, sizeof(mqttNodeTopic), PSTR(MQTT_PREFIX "/%s/"), mqttNodeName);
    snprintf_P(mqttGroupTopic, sizeof(mqttGroupTopic), PSTR(MQTT_PREFIX "/%s/"), mqttGroupName);

    return changed;
}

#endif // HASP_USE_MQTT
