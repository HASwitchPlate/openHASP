/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"

#if HASP_USE_MQTT > 0
#ifdef HASP_USE_PUBSUBCLIENT

#include "PubSubClient.h"

#include "hasp/hasp.h"
#include "hasp_mqtt.h"
#include "hasp_mqtt_ha.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
// #include <WiFiClientSecure.h>
WiFiClient mqttNetworkClient;
// WiFiClientSecure mqttNetworkClient;
// extern const uint8_t rootca_crt_bundle_start[] asm("_binary_data_cert_x509_crt_bundle_bin_start");
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Esp.h>
WiFiClient mqttNetworkClient;
#else
#if defined(STM32F4xx) && HASP_USE_WIFI > 0
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

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"

#include "../hasp/hasp_dispatch.h"

char mqttNodeTopic[24];
char mqttGroupTopic[24];
char mqttClientId[64];
bool mqttEnabled        = false;
bool mqttHAautodiscover = true;
uint32_t mqttPublishCount;
uint32_t mqttReceiveCount;
uint32_t mqttFailedCount;

char mqttServer[MAX_HOSTNAME_LENGTH]   = MQTT_HOSTNAME;
char mqttUsername[MAX_USERNAME_LENGTH] = MQTT_USERNAME;
char mqttPassword[MAX_PASSWORD_LENGTH] = MQTT_PASSWORD;
// char mqttNodeName[16]  = MQTT_NODENAME;
char mqttGroupName[16] = MQTT_GROUPNAME;
uint16_t mqttPort      = MQTT_PORT;
PubSubClient mqttClient(mqttNetworkClient);

int mqttPublish(const char* topic, const char* payload, size_t len, bool retain)
{
    if(!mqttEnabled) return MQTT_ERR_DISABLED;

    if(!mqttClient.connected()) {
        mqttFailedCount++;
        return MQTT_ERR_NO_CONN;
    }

    // Write directly to the client, don't use the buffer
    if(mqttClient.beginPublish(topic, len, retain)) {
        mqttClient.write((uint8_t*)payload, len);
        mqttClient.endPublish();
        mqttPublishCount++;
        return MQTT_ERR_OK;
    }

    mqttFailedCount++;
    return MQTT_ERR_PUB_FAIL;
}

int mqttPublish(const char* topic, const char* payload, bool retain)
{
    return mqttPublish(topic, payload, strlen(payload), retain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Send changed values OUT

bool mqttIsConnected()
{
    return mqttEnabled && mqttClient.connected();
}

bool mqtt_send_lwt(bool online)
{
    char tmp_payload[8];
    char tmp_topic[strlen(mqttNodeTopic) + 4];

    strncpy(tmp_topic, mqttNodeTopic, sizeof(tmp_topic));
    strncat_P(tmp_topic, PSTR(MQTT_TOPIC_LWT), sizeof(tmp_topic));

    size_t len = snprintf_P(tmp_payload, sizeof(tmp_payload), online ? PSTR("online") : PSTR("offline"));
    bool res   = mqttPublish(tmp_topic, tmp_payload, len, true);
    return res;
}

// int mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload)
// {
//     char tmp_topic[strlen(mqttNodeTopic) + 16];
//     snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" MQTT_TOPIC_STATE "/" HASP_OBJECT_NOTATION), mqttNodeTopic,
//                pageid, btnid);
//     return mqttPublish(tmp_topic, payload, false);
// }

int mqtt_send_state(const char* subtopic, const char* payload, bool retain)
{
    char tmp_topic[strlen(mqttNodeTopic) + strlen(subtopic) + 16];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" MQTT_TOPIC_STATE "/%s"), mqttNodeTopic, subtopic);
    return mqttPublish(tmp_topic, payload, retain);
}

int mqtt_send_discovery(const char* payload, size_t len)
{
    char tmp_topic[128];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR(MQTT_PREFIX "/" MQTT_TOPIC_DISCOVERY "/%s"),haspDevice.get_hardware_id());
    return mqttPublish(tmp_topic, payload, len, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Receive incoming messages
static void mqtt_message_cb(char* topic, byte* payload, unsigned int length)
{ // Handle incoming commands from MQTT
    if(length + 1 >= mqttClient.getBufferSize()) {
        mqttFailedCount++;
        LOG_ERROR(TAG_MQTT_RCV, F(D_MQTT_PAYLOAD_TOO_LONG), (uint32_t)length);
        return;
    } else {
        mqttReceiveCount++;
        payload[length] = '\0';
    }

    LOG_TRACE(TAG_MQTT_RCV, F("%s = %s"), topic, (char*)payload);

    if(topic == strstr(topic, mqttNodeTopic)) { // startsWith mqttNodeTopic

        // Node topic
        topic += strlen(mqttNodeTopic); // shorten topic

    } else if(topic == strstr(topic, mqttGroupTopic)) { // startsWith mqttGroupTopic

        // Group topic
        topic += strlen(mqttGroupTopic); // shorten topic
        dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
        return;

#ifdef HASP_USE_BROADCAST
    } else if(topic == strstr_P(topic, PSTR(MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST "/"))) { // broadcast  topic

        // Broadcast topic
        topic += strlen_P(PSTR(MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST "/")); // shorten topic
        dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
        return;
#endif

#ifdef HASP_USE_HA
    } else if(topic == strstr_P(topic, PSTR("homeassistant/status"))) { // HA discovery topic
        if(mqttHAautodiscover && !strcasecmp_P((char*)payload, PSTR("online"))) {
            mqtt_ha_register_auto_discovery(); // auto-discovery first
            dispatch_current_state(TAG_MQTT);  // send the data
        }
        return;
#endif

    } else {
        // Other topic
        LOG_ERROR(TAG_MQTT, F(D_MQTT_INVALID_TOPIC));
        return;
    }

    // catch a dangling LWT from a previous connection if it appears
    /*    if(!strcmp_P(topic, PSTR(MQTT_TOPIC_LWT))) { // endsWith LWT
            if(!strcasecmp_P((char*)payload, PSTR("offline"))) {
                {
                    char msg[8];
                    char tmp_topic[strlen(mqttNodeTopic) + 8];
                    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" MQTT_TOPIC_LWT), mqttNodeTopic);
                    snprintf_P(msg, sizeof(msg), PSTR("online"));

                    // bool res =
                    mqttClient.publish(tmp_topic, msg, true);
                    mqttPublish(tmp_topic, msg, true);
                }
                }
                else
                {
                    // LOG_TRACE(TAG_MQTT, F("ignoring LWT = online"));
                }
                }
                else */
    {
        dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
    }
}

static void mqttSubscribeTo(const char* topic)
{
    if(mqttClient.subscribe(topic)) {
        LOG_VERBOSE(TAG_MQTT, F(D_BULLET D_MQTT_SUBSCRIBED), topic);
    } else {
        LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_SUBSCRIBED), topic);
    }
}

void mqttStart()
{
    char buffer[64];
    char lastWillPayload[8];
    static uint8_t mqttReconnectCount = 0;
    //   bool mqttFirstConnect             = true;

    // mqttNetworkClient.setCACertBundle(rootca_crt_bundle_start);
    // mqttNetworkClient.setInsecure();
    mqttNetworkClient.setTimeout(12);
    mqttClient.setServer(mqttServer, mqttPort);
    // mqttClient.setSocketTimeout(10); //in seconds

    /* Construct unique Client ID*/
    {
        String mac = halGetMacAddress(3, "");
        mac.toLowerCase();
        memset(mqttClientId, 0, sizeof(mqttClientId));
        snprintf_P(mqttClientId, sizeof(mqttClientId), haspDevice.get_hostname());
        size_t len = strlen(mqttClientId);
        snprintf_P(mqttClientId + len, sizeof(mqttClientId) - len, PSTR("_%s"), mac.c_str());
        LOG_INFO(TAG_MQTT, mqttClientId);
    }

    // Attempt to connect and set LWT and Clean Session
    snprintf_P(buffer, sizeof(buffer), PSTR("%s" MQTT_TOPIC_LWT), mqttNodeTopic); // lastWillTopic
    snprintf_P(lastWillPayload, sizeof(lastWillPayload), PSTR("offline"));        // lastWillPayload

    haspProgressMsg(F(D_MQTT_CONNECTING));
    haspProgressVal(mqttReconnectCount * 5);
    if(!mqttClient.connect(mqttClientId, mqttUsername, mqttPassword, buffer, 0, true, lastWillPayload, true)) {
        // Retry until we give up and restart after connectTimeout seconds
        mqttReconnectCount++;

        switch(mqttClient.state()) {
            case MQTT_CONNECTION_TIMEOUT:
                LOG_WARNING(TAG_MQTT, F("Connection timeout"));
                break;
            case MQTT_CONNECTION_LOST:
                LOG_WARNING(TAG_MQTT, F("Connection lost"));
                break;
            case MQTT_CONNECT_FAILED: {
                LOG_WARNING(TAG_MQTT, F("Connection failed"));
                char err_buf[100];
                // if(mqttNetworkClient.lastError(err_buf, 100) < 0) {
                //     Serial.println(err_buf);
                // } else {
                Serial.println("Connection error");
                // }
                break;
            }
            case MQTT_DISCONNECTED:
                snprintf_P(buffer, sizeof(buffer), PSTR(D_MQTT_DISCONNECTED));
                break;
            case MQTT_CONNECTED:
                break;
            case MQTT_CONNECT_BAD_PROTOCOL:
                LOG_WARNING(TAG_MQTT, F("MQTT version not suported"));
                break;
            case MQTT_CONNECT_BAD_CLIENT_ID:
                LOG_WARNING(TAG_MQTT, F("Client ID rejected"));
                break;
            case MQTT_CONNECT_UNAVAILABLE:
                LOG_WARNING(TAG_MQTT, F("Server unavailable"));
                break;
            case MQTT_CONNECT_BAD_CREDENTIALS:
                LOG_WARNING(TAG_MQTT, F("Bad credentials"));
                break;
            case MQTT_CONNECT_UNAUTHORIZED:
                LOG_WARNING(TAG_MQTT, F("Unauthorized"));
                break;
            default:
                LOG_WARNING(TAG_MQTT, F("Unknown failure"));
        }

        if(mqttReconnectCount > 20) {
            LOG_ERROR(TAG_MQTT, F("Retry count exceeded, rebooting..."));
            dispatch_reboot(false);
        }
        return;
    }

    LOG_INFO(TAG_MQTT, F(D_MQTT_CONNECTED), mqttServer, mqttClientId);

    // Subscribe to our incoming topics
    char topic[64];
    snprintf_P(topic, sizeof(topic), PSTR("%s" MQTT_TOPIC_COMMAND "/#"), mqttGroupTopic);
    mqttSubscribeTo(topic);
    snprintf_P(topic, sizeof(topic), PSTR("%s" MQTT_TOPIC_COMMAND "/#"), mqttNodeTopic);
    mqttSubscribeTo(topic);
    snprintf_P(topic, sizeof(topic), PSTR("%s" MQTT_TOPIC_CONFIG "/#"), mqttGroupTopic);
    mqttSubscribeTo(topic);
    snprintf_P(topic, sizeof(topic), PSTR("%s" MQTT_TOPIC_CONFIG "/#"), mqttNodeTopic);
    mqttSubscribeTo(topic);

#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0
    snprintf_P(topic, sizeof(topic), PSTR("%s" MQTT_TOPIC_CUSTOM "/#"), mqttGroupTopic);
    mqttSubscribeTo(topic);
    snprintf_P(topic, sizeof(topic), PSTR("%s" MQTT_TOPIC_CUSTOM "/#"), mqttNodeTopic);
    mqttSubscribeTo(topic);
#endif

#ifdef HASP_USE_BROADCAST
    snprintf_P(topic, sizeof(topic), PSTR(MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST "/" MQTT_TOPIC_COMMAND "/#"));
    mqttSubscribeTo(topic);
#endif

    /* Home Assistant auto-configuration */
#ifdef HASP_USE_HA
    if(mqttHAautodiscover) {
        char topic[64];
        snprintf_P(topic, sizeof(topic), PSTR("hass/status"));
        mqttSubscribeTo(topic);
        snprintf_P(topic, sizeof(topic), PSTR("homeassistant/status"));
        mqttSubscribeTo(topic);
    }
#endif

    // Force any subscribed clients to toggle offline/online when we first connect to
    // make sure we get a full panel refresh at power on.  Sending offline,
    // "online" will be sent by the mqttStatusTopic subscription action.
    mqtt_send_lwt(true);

    // mqttFirstConnect   = false;
    mqttReconnectCount = 0;

    haspReconnect();
    haspProgressVal(255);

    dispatch_current_state(TAG_MQTT);
}

void mqttSetup()
{
    mqttEnabled = strlen(mqttServer) > 0 && mqttPort > 0;
    if(mqttEnabled) {
        mqttClient.setServer(mqttServer, mqttPort);
        mqttClient.setCallback(mqtt_message_cb);
        //  if(!mqttClient.setBufferSize(1024)) {
        //  LOG_ERROR(TAG_MQTT, F("Buffer allocation failed"));
        //  } else {
        LOG_INFO(TAG_MQTT, F(D_MQTT_STARTED), mqttClient.getBufferSize());
        // }
    } else {
        LOG_WARNING(TAG_MQTT, F(D_MQTT_NOT_CONFIGURED));
    }
}

IRAM_ATTR void mqttLoop(void)
{
    mqttClient.loop();
}

void mqttEvery5Seconds(bool networkIsConnected)
{
    if(mqttEnabled && networkIsConnected && !mqttClient.connected()) {
        LOG_TRACE(TAG_MQTT, F(D_MQTT_RECONNECTING));
        mqttStart();
    }
}

void mqttStop()
{
    if(mqttEnabled && mqttClient.connected()) {
        LOG_TRACE(TAG_MQTT, F(D_MQTT_DISCONNECTING));
        mqtt_send_lwt(false);
        mqttClient.disconnect();
        LOG_INFO(TAG_MQTT, F(D_MQTT_DISCONNECTED));
    }
}

void mqtt_get_info(JsonDocument& doc)
{
    char buffer[64];
    // String mac((char*)0);
    // mac.reserve(64);

    JsonObject info          = doc.createNestedObject(F("MQTT"));
    info[F(D_INFO_SERVER)]   = mqttServer;
    info[F(D_INFO_USERNAME)] = mqttUsername;
    info[F(D_INFO_CLIENTID)] = mqttClientId;

    switch(mqttClient.state()) {
        case MQTT_CONNECT_UNAUTHORIZED:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_NETWORK_CONNECTION_UNAUTHORIZED));
            break;
        case MQTT_CONNECT_FAILED:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_NETWORK_CONNECTION_FAILED));
            break;
        case MQTT_DISCONNECTED:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_SERVICE_DISCONNECTED));
            break;
        case MQTT_CONNECTED:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_SERVICE_CONNECTED));
            break;
        case MQTT_CONNECTION_TIMEOUT:
        case MQTT_CONNECTION_LOST:
        case MQTT_CONNECT_BAD_PROTOCOL:
        case MQTT_CONNECT_BAD_CLIENT_ID:
        case MQTT_CONNECT_UNAVAILABLE:
        case MQTT_CONNECT_BAD_CREDENTIALS:
        default:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_SERVICE_DISCONNECTED " (%d)"), mqttClient.state());
            break;
    }
    info[F(D_INFO_STATUS)] = buffer;

    info[F(D_INFO_RECEIVED)]  = mqttReceiveCount;
    info[F(D_INFO_PUBLISHED)] = mqttPublishCount;
    info[F(D_INFO_FAILED)]    = mqttFailedCount;
}

#if HASP_USE_CONFIG > 0
bool mqttGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(strcmp(haspDevice.get_hostname(), settings[FPSTR(FP_CONFIG_NAME)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_NAME)] = haspDevice.get_hostname();

    if(strcmp(mqttGroupName, settings[FPSTR(FP_CONFIG_GROUP)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_GROUP)] = mqttGroupName;

    if(strcmp(mqttServer, settings[FPSTR(FP_CONFIG_HOST)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_HOST)] = mqttServer;

    if(mqttPort != settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PORT)] = mqttPort;

    if(strcmp(mqttUsername, settings[FPSTR(FP_CONFIG_USER)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_USER)] = mqttUsername;

    if(strcmp(mqttPassword, settings[FPSTR(FP_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_PASS)] = mqttPassword;

    if(changed) configOutput(settings, TAG_MQTT);
    return changed;
}

/** Set MQTT Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formatted to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool mqttSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_MQTT);
    bool changed = false;

    changed |= configSet(mqttPort, settings[FPSTR(FP_CONFIG_PORT)], F("mqttPort"));

    if(!settings[FPSTR(FP_CONFIG_NAME)].isNull()) {
        changed |= strcmp(haspDevice.get_hostname(), settings[FPSTR(FP_CONFIG_NAME)]) != 0;
        // strncpy(mqttNodeName, settings[FPSTR(FP_CONFIG_NAME)], sizeof(mqttNodeName));
        haspDevice.set_hostname(settings[FPSTR(FP_CONFIG_NAME)].as<const char*>());
    }
    // Prefill node name
    if(strlen(haspDevice.get_hostname()) == 0) {
        char mqttNodeName[64];
        String mac = halGetMacAddress(3, "");
        mac.toLowerCase();
        snprintf_P(mqttNodeName, sizeof(mqttNodeName), PSTR(D_MQTT_DEFAULT_NAME), mac.c_str());
        haspDevice.set_hostname(mqttNodeName);
        changed = true;
    }

    if(!settings[FPSTR(FP_CONFIG_GROUP)].isNull()) {
        changed |= strcmp(mqttGroupName, settings[FPSTR(FP_CONFIG_GROUP)]) != 0;
        strncpy(mqttGroupName, settings[FPSTR(FP_CONFIG_GROUP)], sizeof(mqttGroupName));
    }

    if(strlen(mqttGroupName) == 0) {
        strcpy_P(mqttGroupName, PSTR("plates"));
        changed = true;
    }

    if(!settings[FPSTR(FP_CONFIG_HOST)].isNull()) {
        changed |= strcmp(mqttServer, settings[FPSTR(FP_CONFIG_HOST)]) != 0;
        strncpy(mqttServer, settings[FPSTR(FP_CONFIG_HOST)], sizeof(mqttServer));
    }

    if(!settings[FPSTR(FP_CONFIG_USER)].isNull()) {
        changed |= strcmp(mqttUsername, settings[FPSTR(FP_CONFIG_USER)]) != 0;
        strncpy(mqttUsername, settings[FPSTR(FP_CONFIG_USER)], sizeof(mqttUsername));
    }

    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() &&
       settings[FPSTR(FP_CONFIG_PASS)].as<String>() != String(FPSTR(D_PASSWORD_MASK))) {
        changed |= strcmp(mqttPassword, settings[FPSTR(FP_CONFIG_PASS)]) != 0;
        strncpy(mqttPassword, settings[FPSTR(FP_CONFIG_PASS)], sizeof(mqttPassword));
    }

    snprintf_P(mqttNodeTopic, sizeof(mqttNodeTopic), PSTR(MQTT_PREFIX "/%s/"), haspDevice.get_hostname());
    snprintf_P(mqttGroupTopic, sizeof(mqttGroupTopic), PSTR(MQTT_PREFIX "/%s/"), mqttGroupName);

    return changed;
}
#endif // HASP_USE_CONFIG

#endif // PUBSUBCLIENT

#endif // HASP_USE_MQTT
