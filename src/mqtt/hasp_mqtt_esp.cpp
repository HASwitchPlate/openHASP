/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"

#if HASP_USE_MQTT > 0
#ifdef HASP_USE_ESP_MQTT

#include "mqtt_client.h"
#include "esp_crt_bundle.h"
#include "Preferences.h"

#include "hasp/hasp.h"
#include "hasp_mqtt.h"
#include "hasp_mqtt_ha.h"

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"

#include "../hasp/hasp_dispatch.h"
#include "freertos/queue.h"

#include "esp_http_server.h"
#include "esp_tls.h"

#define MQTT_DEFAULT_NODE_TOPIC MQTT_PREFIX "/%hostname%/%topic%"
#define MQTT_DEFAULT_GROUP_TOPIC MQTT_PREFIX "/" MQTT_GROUPNAME "/%topic%"
#define MQTT_DEFAULT_BROADCAST_TOPIC MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST "/%topic%"
#define MQTT_DEFAULT_HASS_TOPIC "homeassistant/status"

QueueHandle_t queue;
typedef struct
{
    char* topic;   //[64];
    char* payload; //[512];
} mqtt_message_t;

char mqttClientId[64];
String mqttNodeLwtTopic;
String mqttHassLwtTopic;
String mqttNodeStateTopic;
String mqttNodeCommandTopic;
String mqttGroupCommandTopic;
String mqttBroadcastCommandTopic;
bool mqttEnabled        = false;
bool mqttHAautodiscover = true;
uint32_t mqttPublishCount;
uint32_t mqttReceiveCount;
uint32_t mqttFailedCount;

String mqttServer   = MQTT_HOSTNAME;
String mqttUsername = MQTT_USERNAME;
String mqttPassword = MQTT_PASSWORD;

// char mqttServer[MAX_HOSTNAME_LENGTH]   = MQTT_HOSTNAME;
// char mqttUsername[MAX_USERNAME_LENGTH] = MQTT_USERNAME;
// char mqttPassword[MAX_PASSWORD_LENGTH] = MQTT_PASSWORD;
// char mqttNodeName[16]  = MQTT_NODENAME;
// char mqttGroupName[16] = MQTT_GROUPNAME;
uint16_t mqttPort = MQTT_PORT;
int mqttQos       = 0;
esp_mqtt_client_handle_t mqttClient;
static esp_mqtt_client_config_t mqtt_cfg;

// extern const uint8_t rootca_crt_bundle_start[] asm("_binary_data_cert_x509_crt_bundle_bin_start");
// extern const uint8_t rootca_crt_bundle_end[] asm("_binary_data_cert_x509_crt_bundle_bin_end");

bool last_mqtt_state            = false;
bool current_mqtt_state         = false;
uint16_t mqtt_reconnect_counter = 0;

static inline void mqtt_run_scripts()
{
    if(last_mqtt_state != current_mqtt_state) {
        // mqtt_message_t data;
        // snprintf(data.topic, sizeof(data.topic), "run");

        // if(current_mqtt_state) {
        //     snprintf(data.payload, sizeof(data.payload), "L:/mqtt_on.cmd");
        //     // networkStart();
        // } else {
        //     snprintf(data.payload, sizeof(data.payload), "L:/mqtt_off.cmd");
        //     // networkStop();
        // }

        // size_t attempt = 0;
        // while(xQueueSend(queue, &data, (TickType_t)0) == errQUEUE_FULL && attempt < 100) {
        //     vTaskDelay(5 / portTICK_PERIOD_MS);
        //     attempt++;
        // };

        if(current_mqtt_state) {
            dispatch_run_script(NULL, "L:/mqtt_on.cmd", TAG_HASP);
        } else {
            dispatch_run_script(NULL, "L:/mqtt_off.cmd", TAG_HASP);
        }

        last_mqtt_state = current_mqtt_state;
    }
}

void mqtt_disconnected()
{
    current_mqtt_state = false; // now we are disconnected
    mqtt_reconnect_counter++;
    // mqtt_run_scripts(); // must happen in LVGL loop
}

void mqtt_connected()
{
    if(!current_mqtt_state) {
        mqtt_reconnect_counter = 0;
        current_mqtt_state     = true; // now we are connected
        LOG_VERBOSE(TAG_MQTT, F("%s"), current_mqtt_state ? PSTR(D_SERVICE_CONNECTED) : PSTR(D_SERVICE_DISCONNECTED));
    }
    // mqtt_run_scripts(); // must happen in LVGL loop
}

int mqttPublish(const char* topic, const char* payload, size_t len, bool retain)
{
    if(!mqttEnabled) return MQTT_ERR_DISABLED;

    // Write directly to the client, don't use the buffer
    if(current_mqtt_state && esp_mqtt_client_publish(mqttClient, topic, payload, len, mqttQos, retain) != ESP_FAIL) {

        // Enqueue a message to the outbox, to be sent later
        // if(current_mqtt_state && esp_mqtt_client_enqueue(mqttClient, topic, payload, len, 0, retain, true) !=
        // ESP_FAIL) {
        mqttPublishCount++;
        return MQTT_ERR_OK;
    }

    mqttFailedCount++;
    return current_mqtt_state ? MQTT_ERR_PUB_FAIL : MQTT_ERR_NO_CONN;
}

int mqttPublish(const char* topic, const char* payload, bool retain)
{
    return mqttPublish(topic, payload, strlen(payload), retain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Send changed values OUT

bool mqttIsConnected()
{
    return mqttEnabled && current_mqtt_state;
}

bool mqtt_send_lwt(bool online)
{
    char tmp_payload[8];
    size_t len = snprintf_P(tmp_payload, sizeof(tmp_payload), online ? PSTR("online") : PSTR("offline"));
    bool res   = mqttPublish(mqttNodeLwtTopic.c_str(), tmp_payload, len, true);
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
    String tmp_topic((char*)0);
    tmp_topic.reserve(128);
    tmp_topic = mqttNodeStateTopic;
    // tmp_topic += MQTT_TOPIC_STATE;
    // tmp_topic += "/";
    tmp_topic += subtopic;

    return mqttPublish(tmp_topic.c_str(), payload, retain);
}

int mqtt_send_discovery(const char* payload, size_t len)
{
    char tmp_topic[128];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR(MQTT_PREFIX "/" MQTT_TOPIC_DISCOVERY "/%s"),
               haspDevice.get_hardware_id());
    return mqttPublish(tmp_topic, payload, len, false);
}

static inline size_t mqtt_msg_length(size_t len)
{
    return (len / 64) * 64 + 64;
}

void mqtt_enqueue_message(const char* topic, const char* payload, size_t payload_len)
{
    // Add new message to the queue
    mqtt_message_t data;

    size_t topic_len = strlen(topic);
    data.topic       = (char*)hasp_calloc(sizeof(char), mqtt_msg_length(topic_len + 1));
    data.payload     = (char*)hasp_calloc(sizeof(char), mqtt_msg_length(payload_len + 1));

    if(!data.topic || !data.payload) {
        LOG_ERROR(TAG_MQTT_RCV, D_ERROR_OUT_OF_MEMORY);
        hasp_free(data.topic);
        hasp_free(data.payload);
        return;
    }
    memcpy(data.topic, topic, topic_len);
    memcpy(data.payload, payload, payload_len);

    {
        size_t attempt = 0;
        while(xQueueSend(queue, &data, (TickType_t)0) == errQUEUE_FULL && attempt < 100) {
            delay(5);
            attempt++;
        };
        if(attempt >= 100) {
            LOG_ERROR(TAG_MQTT_RCV, D_ERROR_OUT_OF_MEMORY);
        }
    }
}

void mqtt_process_topic_payload(const char* topic, const char* payload, unsigned int length)
{
    if(gui_acquire(pdMS_TO_TICKS(30))) {
        mqttLoop(); // First empty the MQTT queue
        LOG_TRACE(TAG_MQTT_RCV, F("%s = %s"), topic, payload);
        dispatch_topic_payload(topic, payload, length > 0, TAG_MQTT);
        gui_release();
    } else {
        mqtt_enqueue_message(topic, payload, length);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Receive incoming messages
static void mqtt_message_cb(const char* topic, byte* payload, unsigned int length)
{ // Handle incoming commands from MQTT
    mqttReceiveCount++;
    // LOG_TRACE(TAG_MQTT_RCV, F("%s = %s"), topic, (char*)payload);

    if(topic == strstr(topic, mqttNodeCommandTopic.c_str())) { // startsWith mqttNodeCommandTopic
        topic += strlen(mqttNodeCommandTopic.c_str());         // shorten Node topic

    } else if(topic == strstr(topic, mqttGroupCommandTopic.c_str())) { // startsWith mqttGroupCommandTopic
        topic += strlen(mqttGroupCommandTopic.c_str());                // shorten Group topic

#ifdef HASP_USE_BROADCAST
    } else if(topic == strstr_P(topic, mqttBroadcastCommandTopic.c_str())) { // broadcast  topic
        topic += strlen_P(mqttBroadcastCommandTopic.c_str());                // shorten Broadcast topic
#endif

#ifdef HASP_USE_HA
    } else if(topic == strstr_P(topic, PSTR("homeassistant/status"))) { // HA discovery topic
        if(mqttHAautodiscover && !strcasecmp_P((char*)payload, PSTR("online"))) {
            mqtt_ha_register_auto_discovery(); // auto-discovery first
            dispatch_current_state(TAG_MQTT);  // send the data
        }
        return;
#endif
    } else if(topic == strstr(topic, mqttHassLwtTopic.c_str())) { // startsWith mqttGroupCommandTopic
        String state = String((const char*)payload);
        state.toLowerCase();
        LOG_VERBOSE(TAG_MQTT, "Home Automation System: %s", state);
        return;

    } else {
        LOG_ERROR(TAG_MQTT, F(D_MQTT_INVALID_TOPIC ": %s"), topic); // Other topic
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
        if(topic[0] == '/') topic++;
        mqtt_process_topic_payload(topic, (const char*)payload, length);
    }

    /*    {
            mqtt_message_t data;
            snprintf(data.topic, sizeof(data.topic), topic);
            snprintf(data.payload, sizeof(data.payload), (const char*)payload);
            size_t attempt = 0;
            while(xQueueSend(queue, &data, (TickType_t)0) == errQUEUE_FULL && attempt < 100) {
                vTaskDelay(5 / portTICK_PERIOD_MS);
                attempt++;
            };
            // dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
        }  */
}

static int mqttSubscribeTo(String topic)
{
    int err = esp_mqtt_client_subscribe(mqttClient, topic.c_str(), mqttQos);
    if(err == ESP_FAIL) {
        LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_SUBSCRIBED), topic.c_str());
        mqttFailedCount++;
    } else {
        LOG_VERBOSE(TAG_MQTT, F(D_BULLET D_MQTT_SUBSCRIBED), topic.c_str());
    }
    return err;
}

/*
String mqttGetTopic(Preferences preferences, String subtopic, String key, String value, bool add_slash)
{
    String topic = preferences.getString(key.c_str(), value);

    topic.replace(F("%hostname%"), haspDevice.get_hostname());
    topic.replace(F("%hwid%"), haspDevice.get_hardware_id());
    topic.replace(F("%topic%"), subtopic);
    topic.replace(F("%prefix%"), MQTT_PREFIX);

    if(add_slash && !topic.endsWith("/")) {
        topic += "/";
    }
    return topic;
}
*/

void mqttParseTopic(String* topic, String subtopic, bool add_slash)
{

    topic->replace(F("%hostname%"), haspDevice.get_hostname());
    topic->replace(F("%hwid%"), haspDevice.get_hardware_id());
    topic->replace(F("%topic%"), subtopic);
    topic->replace(F("%prefix%"), MQTT_PREFIX);

    if(add_slash && !topic->endsWith("/")) {
        *topic += "/";
    }
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    LOG_INFO(TAG_MQTT, F(D_MQTT_CONNECTED), mqttServer, mqttClientId);

    LOG_DEBUG(TAG_MQTT, F(D_BULLET "%s"), mqttNodeCommandTopic.c_str());
    LOG_DEBUG(TAG_MQTT, F(D_BULLET "%s"), mqttGroupCommandTopic.c_str());
    LOG_DEBUG(TAG_MQTT, F(D_BULLET "%s"), mqttBroadcastCommandTopic.c_str());
    LOG_DEBUG(TAG_MQTT, F(D_BULLET "%s"), mqttHassLwtTopic.c_str());

    // Subscribe to our incoming topics
    mqttSubscribeTo(mqttGroupCommandTopic + "/#");
    mqttSubscribeTo(mqttNodeCommandTopic + "/#");

#ifdef HASP_USE_BROADCAST
    mqttSubscribeTo(mqttBroadcastCommandTopic + "/#");
#endif

    // subtopic = F(MQTT_TOPIC_CONFIG "/#");
    // mqttSubscribeTo(mqttGroupTopic + subtopic);
    // mqttSubscribeTo(mqttNodeTopic + subtopic);

#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0
    String subtopic = F(MQTT_TOPIC_CUSTOM "/#");
    mqttSubscribeTo(mqttGroupCommandTopic + subtopic);
    mqttSubscribeTo(mqttNodeCommandTopic + subtopic);
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

    mqttSubscribeTo(mqttHassLwtTopic);

    // Force any subscribed clients to toggle offline/online when we first connect to
    // make sure we get a full panel refresh at power on.  Sending offline,
    // "online" will be sent by the mqttStatusTopic subscription action.
    mqtt_send_lwt(true);

    // mqttFirstConnect   = false;
    // mqttReconnectCount = 0;

    // haspReconnect();
    // haspProgressVal(255);

    dispatch_current_state(TAG_MQTT);
}

static void onMqttData(esp_mqtt_event_handle_t event)
{
    String topic = String(event->topic).substring(0, event->topic_len);
    String msg   = String(event->data).substring(0, event->data_len);
    // Serial.printf("onMqttData topic '%s' msg '%s'\n", topic.c_str(), msg.c_str());
    mqtt_message_cb(topic.c_str(), (byte*)msg.c_str(), msg.length());
}

static void onMqttSubscribed(esp_mqtt_event_handle_t event)
{
    // String topic = String(event->topic).substring(0, event->topic_len);
    String msg        = String(event->data).substring(0, event->data_len);
    const char* topic = "topic";
    LOG_VERBOSE(TAG_MQTT, F(D_BULLET D_MQTT_SUBSCRIBED "(%d)"), topic, event->msg_id);
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    // LOG_WARNING(TAG_MQTT, "mqtt_event_handler %d", event->event_id);

    int msg_id;
    switch(event->event_id) {
        case MQTT_EVENT_DISCONNECTED:
            LOG_WARNING(TAG_MQTT, F(D_MQTT_DISCONNECTED));
            mqtt_disconnected();
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            // LOG_INFO(TAG_MQTT, F(D_MQTT_CONNECTING));
            break;
        case MQTT_EVENT_CONNECTED:
            LOG_INFO(TAG_MQTT, F(D_SERVICE_STARTED));
            mqtt_connected();
            onMqttConnect(event->client);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            onMqttSubscribed(event);
            break;
        case MQTT_EVENT_DATA:
            onMqttData(event);
            break;
        case MQTT_EVENT_ERROR: {
            mqtt_disconnected();
            esp_mqtt_error_codes_t* error_handle = event->error_handle;
            switch(error_handle->error_type) {
                case MQTT_ERROR_TYPE_TCP_TRANSPORT:
                    LOG_ERROR(TAG_MQTT, "Transport error");
                    break;
                case MQTT_ERROR_TYPE_CONNECTION_REFUSED:
                    switch(error_handle->connect_return_code) {
                        case MQTT_CONNECTION_REFUSE_PROTOCOL: /*!< MQTT connection refused reason: Wrong protocol */
                            LOG_WARNING(TAG_MQTT, "Connection refused: Wrong protocol");
                            break;
                        case MQTT_CONNECTION_REFUSE_ID_REJECTED: /*!< MQTT connection refused reason: ID rejected */
                            LOG_WARNING(TAG_MQTT, "Connection refused: ID rejected");
                            break;
                        case MQTT_CONNECTION_REFUSE_SERVER_UNAVAILABLE: /*!< MQTT connection refused reason: Server
                                                                           unavailable */
                            LOG_WARNING(TAG_MQTT, "Connection refused: Server unavailable");
                            break;
                        case MQTT_CONNECTION_REFUSE_BAD_USERNAME: /*!< MQTT connection refused reason: Wrong user */
                            LOG_WARNING(TAG_MQTT, "Connection refused: Wrong user");
                            break;
                        case MQTT_CONNECTION_REFUSE_NOT_AUTHORIZED: /*!< MQTT connection refused reason: Wrong
                                                                       username or password */
                            LOG_WARNING(TAG_MQTT, "Connection refused: Authentication error");
                            break;
                        default:;
                    }
                    break;
                case MQTT_ERROR_TYPE_NONE:
                default:;
            }
            break;
        }
        default:
            LOG_WARNING(TAG_MQTT, "mqtt_event_handler %d", event->event_id);
    }
    return ESP_OK;
}

void mqttSetup()
{
    queue = xQueueCreate(64, sizeof(mqtt_message_t));
    // esp_crt_bundle_set(rootca_crt_bundle_start, rootca_crt_bundle_end-rootca_crt_bundle_start);
    //    arduino_esp_crt_bundle_set(rootca_crt_bundle_start);
    mqttStart();
}

IRAM_ATTR void mqttLoop(void)
{
    // mqttClient.loop();

    if(!uxQueueMessagesWaiting(queue)) return;

    mqtt_message_t data;
    while(xQueueReceive(queue, &data, (TickType_t)0)) {
        LOG_WARNING(TAG_MQTT, F("[%d] QUE %s => %s"), uxQueueMessagesWaiting(queue), data.topic, data.payload);
        size_t length = strlen(data.payload);
        dispatch_topic_payload(data.topic, data.payload, length > 0, TAG_MQTT);
        hasp_free(data.topic);
        hasp_free(data.payload);
        // delay(1);
    }
}

void mqttEverySecond()
{
    mqtt_run_scripts();
}

void mqttEvery5Seconds(bool networkIsConnected)
{
    // if(mqttEnabled && networkIsConnected && !current_mqtt_state) {
    //     LOG_TRACE(TAG_MQTT, F(D_MQTT_RECONNECTING));
    //     mqttStart();
    // }
}

void mqttStart()
{
    {
        Preferences preferences;
        nvs_user_begin(preferences, FP_MQTT, true);
        mqttServer   = preferences.getString(FP_CONFIG_HOST, mqttServer);   // Update from NVS if it exists
        mqttUsername = preferences.getString(FP_CONFIG_USER, mqttUsername); // Update from NVS if it exists
        mqttPassword = preferences.getString(FP_CONFIG_PASS, mqttPassword); // Update from NVS if it exists
        mqttPort     = preferences.getUShort(FP_CONFIG_PORT, mqttPort);     // Update from NVS if it exists

        String subtopic((char*)0);
        subtopic.reserve(64);

        String nvsOldGroup = MQTT_PREFIX "/"; // recover group setting
        nvsOldGroup += preferences.getString(FP_CONFIG_GROUP, MQTT_GROUPNAME);
        nvsOldGroup += "/%topic%";

        subtopic             = F(MQTT_TOPIC_COMMAND);
        mqttNodeCommandTopic = preferences.getString(FP_CONFIG_NODE_TOPIC, MQTT_DEFAULT_NODE_TOPIC);
        mqttParseTopic(&mqttNodeCommandTopic, subtopic, false);
        mqttGroupCommandTopic = preferences.getString(FP_CONFIG_GROUP_TOPIC, nvsOldGroup.c_str());
        mqttParseTopic(&mqttGroupCommandTopic, subtopic, false);

#ifdef HASP_USE_BROADCAST
        mqttBroadcastCommandTopic = preferences.getString(FP_CONFIG_BROADCAST_TOPIC, MQTT_DEFAULT_BROADCAST_TOPIC);
        mqttParseTopic(&mqttBroadcastCommandTopic, subtopic, false);

#endif

        subtopic           = F(MQTT_TOPIC_STATE);
        mqttNodeStateTopic = preferences.getString(FP_CONFIG_NODE_TOPIC, MQTT_DEFAULT_NODE_TOPIC);
        mqttParseTopic(&mqttNodeStateTopic, subtopic, true);

        subtopic         = F(MQTT_TOPIC_LWT);
        mqttNodeLwtTopic = preferences.getString(FP_CONFIG_NODE_TOPIC, MQTT_DEFAULT_NODE_TOPIC);
        mqttParseTopic(&mqttNodeLwtTopic, subtopic, false);
        LOG_WARNING(TAG_MQTT, mqttNodeLwtTopic.c_str());

        subtopic         = F(MQTT_TOPIC_LWT);
        mqttHassLwtTopic = preferences.getString(FP_CONFIG_HASS_TOPIC, MQTT_DEFAULT_HASS_TOPIC);
        mqttParseTopic(&mqttHassLwtTopic, subtopic, false);
        LOG_WARNING(TAG_MQTT, mqttNodeLwtTopic.c_str());

        preferences.end();
    }

    mqttEnabled = mqttServer.length() > 0 && mqttPort > 0;
    if(!mqttEnabled) {
        LOG_WARNING(TAG_MQTT, F(D_MQTT_NOT_CONFIGURED));
        return;
    }

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

    mqtt_cfg.event_handle           = mqtt_event_handler;
    mqtt_cfg.buffer_size            = MQTT_MAX_PACKET_SIZE;
    mqtt_cfg.out_buffer_size        = 512;
    mqtt_cfg.reconnect_timeout_ms   = 5000;
    mqtt_cfg.disable_auto_reconnect = false;
    mqtt_cfg.keepalive              = 15; /* seconds */
    mqtt_cfg.disable_clean_session  = true;

    mqtt_cfg.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
    mqtt_cfg.transport    = MQTT_TRANSPORT_OVER_TCP;
    mqtt_cfg.host         = mqttServer.c_str();
    mqtt_cfg.port         = mqttPort;
    mqtt_cfg.username     = mqttUsername.c_str();
    mqtt_cfg.password     = mqttPassword.c_str();
    mqtt_cfg.client_id    = mqttClientId;

    mqtt_cfg.lwt_msg    = "offline";
    mqtt_cfg.lwt_retain = true;
    mqtt_cfg.lwt_topic  = mqttNodeLwtTopic.c_str();
    mqtt_cfg.lwt_qos    = 1;

    mqtt_cfg.task_prio = 1;

    // mqtt_cfg.crt_bundle_attach = esp_crt_bundle_attach;

    // test Mosquitto doesn't need a user/pwd
    //    // mqtt_cfg.username=(const char *)mqtt_user;
    //    // mqtt_cfg.password=(const char *)mqtt_pwd;

    if(mqttClient) {
        esp_mqtt_set_config(mqttClient, &mqtt_cfg);
    } else {
        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        if(esp_mqtt_client_start(mqttClient) != ESP_OK) {
            LOG_WARNING(TAG_MQTT, F(D_SERVICE_START_FAILED));
            return;
        }
    }

    LOG_INFO(TAG_MQTT, F(D_SERVICE_STARTING));
}

void mqttStop()
{
    if(!mqttEnabled) {
        LOG_WARNING(TAG_MQTT, F(D_SERVICE_DISABLED));
        return;
    }

    if(mqttClient != NULL) {
        if(current_mqtt_state) {
            LOG_TRACE(TAG_MQTT, F(D_MQTT_DISCONNECTING));
        }
        mqtt_send_lwt(false);
        // esp_err_t err = esp_mqtt_client_stop(mqttClient); // Cannot be called from the *MQTT* event handler
        mqtt_cfg.disable_auto_reconnect = true;
        esp_mqtt_set_config(mqttClient, &mqtt_cfg);
        esp_err_t err = esp_mqtt_client_disconnect(mqttClient);
        if(err == ESP_OK) {
            mqtt_disconnected();
            LOG_INFO(TAG_MQTT, F(D_MQTT_DISCONNECTED));
        } else {
            LOG_ERROR(TAG_MQTT, F(D_MQTT_FAILED " %d"), err);
        }
    } else {
        LOG_INFO(TAG_MQTT, F(D_SERVICE_STOPPED));
    }
}

void mqtt_get_info(JsonDocument& doc)
{
    char buffer[64];
    String mac((char*)0);
    mac.reserve(64);

    JsonObject info          = doc.createNestedObject(F("MQTT"));
    info[F(D_INFO_SERVER)]   = mqttServer;
    info[F(D_INFO_USERNAME)] = mqttUsername;
    info[F(D_INFO_CLIENTID)] = mqttClientId;

    // switch(mqttClient.state()) {
    //     case MQTT_CONNECT_UNAUTHORIZED:
    //         snprintf_P(buffer, sizeof(buffer), PSTR(D_NETWORK_CONNECTION_UNAUTHORIZED));
    //         break;
    //     case MQTT_CONNECT_FAILED:
    //         snprintf_P(buffer, sizeof(buffer), PSTR(D_NETWORK_CONNECTION_FAILED));
    //         break;
    //     case MQTT_DISCONNECTED:
    //         snprintf_P(buffer, sizeof(buffer), PSTR(D_SERVICE_DISCONNECTED));
    //         break;
    //     case MQTT_CONNECTED:
    //         snprintf_P(buffer, sizeof(buffer), PSTR(D_SERVICE_CONNECTED));
    //         break;
    //     case MQTT_CONNECTION_TIMEOUT:
    //     case MQTT_CONNECTION_LOST:
    //     case MQTT_CONNECT_BAD_PROTOCOL:
    //     case MQTT_CONNECT_BAD_CLIENT_ID:
    //     case MQTT_CONNECT_UNAVAILABLE:
    //     case MQTT_CONNECT_BAD_CREDENTIALS:
    //     default:
    //         snprintf_P(buffer, sizeof(buffer), PSTR(D_SERVICE_DISCONNECTED " (%d)"), mqttClient.state());
    //         break;
    // }
    // info[F(D_INFO_STATUS)] = buffer;
    info[F(D_INFO_STATUS)] = !mqttEnabled         ? D_SERVICE_DISABLED
                             : !mqttClient        ? D_SERVICE_STOPPED
                             : current_mqtt_state ? D_SERVICE_CONNECTED
                                                  : D_SERVICE_DISCONNECTED;

    info[F(D_INFO_RECEIVED)]  = mqttReceiveCount;
    info[F(D_INFO_PUBLISHED)] = mqttPublishCount;
    info[F(D_INFO_FAILED)]    = mqttFailedCount;
}

#if HASP_USE_CONFIG > 0
bool mqttGetConfig(const JsonObject& settings)
{
    bool changed = false;
    Preferences preferences;
    nvs_user_begin(preferences, FP_MQTT, false);

    {
        String nvsServer = preferences.getString(FP_CONFIG_HOST, mqttServer); // Read from NVS if it exists
        if(strcmp(nvsServer.c_str(), settings[FP_CONFIG_HOST].as<String>().c_str()) != 0) changed = true;
        settings[FP_CONFIG_HOST] = nvsServer;
    }

    {
        String nvsUsername = preferences.getString(FP_CONFIG_USER, mqttUsername); // Read from NVS if it exists
        if(strcmp(nvsUsername.c_str(), settings[FP_CONFIG_USER].as<String>().c_str()) != 0) changed = true;
        settings[FP_CONFIG_USER] = nvsUsername;
    }

    {
        String nvsPassword = preferences.getString(FP_CONFIG_PASS, mqttPassword); // Read from NVS if it exists
        if(strcmp(D_PASSWORD_MASK, settings[FP_CONFIG_PASS].as<String>().c_str()) != 0) changed = true;
        settings[FP_CONFIG_PASS] = D_PASSWORD_MASK;
    }

    {
        String nvsNodeTopic =
            preferences.getString(FP_CONFIG_NODE_TOPIC, MQTT_DEFAULT_NODE_TOPIC); // Read from NVS if it exists
        if(strcmp(nvsNodeTopic.c_str(), settings["topic"][FP_CONFIG_NODE].as<String>().c_str()) != 0) changed = true;
        settings["topic"][FP_CONFIG_NODE] = nvsNodeTopic;
    }

    {
        String nvsOldGroup = MQTT_PREFIX "/"; // recover group setting
        nvsOldGroup += preferences.getString(FP_CONFIG_GROUP, MQTT_GROUPNAME);
        nvsOldGroup += "/%topic%";

        String nvsGroupTopic =
            preferences.getString(FP_CONFIG_GROUP_TOPIC, nvsOldGroup.c_str()); // Read from NVS if it exists
        if(strcmp(nvsGroupTopic.c_str(), settings["topic"][FP_CONFIG_GROUP].as<String>().c_str()) != 0) changed = true;
        settings["topic"][FP_CONFIG_GROUP] = nvsGroupTopic;
    }

    {
        String nvsBroadcastTopic = preferences.getString(FP_CONFIG_BROADCAST_TOPIC,
                                                         MQTT_DEFAULT_BROADCAST_TOPIC); // Read from NVS if it exists
        if(strcmp(nvsBroadcastTopic.c_str(), settings["topic"][FP_CONFIG_BROADCAST].as<String>().c_str()) != 0)
            changed = true;
        settings["topic"][FP_CONFIG_BROADCAST] = nvsBroadcastTopic;
    }

    {
        String nvsHassTopic =
            preferences.getString(FP_CONFIG_HASS_TOPIC, MQTT_DEFAULT_HASS_TOPIC); // Read from NVS if it exists
        if(strcmp(nvsHassTopic.c_str(), settings["topic"][FP_CONFIG_HASS].as<String>().c_str()) != 0) changed = true;
        settings["topic"][FP_CONFIG_HASS] = nvsHassTopic;
    }

    {
        uint16_t nvsPort = preferences.getUShort(FP_CONFIG_PORT, mqttPort); // Read from NVS if it exists
        if(nvsPort != settings[FP_CONFIG_PORT].as<uint16_t>()) changed = true;
        settings[FP_CONFIG_PORT] = nvsPort;
    }

    if(strcmp(haspDevice.get_hostname(), settings[FP_CONFIG_NAME].as<String>().c_str()) != 0) changed = true;
    settings[FP_CONFIG_NAME] = haspDevice.get_hostname();

    // if(strcmp(mqttGroupName, settings[FP_CONFIG_GROUP].as<String>().c_str()) != 0) changed = true;
    // settings[FP_CONFIG_GROUP] = mqttGroupName;

    preferences.end();
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
    Preferences preferences;
    nvs_user_begin(preferences, FP_MQTT, false);

    configOutput(settings, TAG_MQTT);
    bool changed = false;

    if(!settings[FP_CONFIG_PORT].isNull()) {
        // changed |= configSet(mqttPort, settings[FP_CONFIG_PORT], F("mqttPort"));
        changed |= nvsUpdateUShort(preferences, FP_CONFIG_PORT, settings[FP_CONFIG_PORT]);
    }

    if(!settings[FP_CONFIG_NAME].isNull()) {
        changed |= strcmp(haspDevice.get_hostname(), settings[FP_CONFIG_NAME]) != 0;
        // strncpy(mqttNodeName, settings[FP_CONFIG_NAME], sizeof(mqttNodeName));
        haspDevice.set_hostname(settings[FP_CONFIG_NAME].as<const char*>());
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

    if(!settings[FP_CONFIG_GROUP].isNull()) {
        // changed |= strcmp(mqttGroupName, settings[FP_CONFIG_GROUP]) != 0;
        // strncpy(mqttGroupName, settings[FP_CONFIG_GROUP], sizeof(mqttGroupName));
        changed |= nvsUpdateString(preferences, FP_CONFIG_GROUP, settings[FP_CONFIG_GROUP]);
    }

    // if(strlen(mqttGroupName) == 0) {
    //     strcpy_P(mqttGroupName, PSTR(MQTT_GROUPNAME));
    //     changed = true;
    // }

    if(!settings[FP_CONFIG_HOST].isNull()) {
        // changed |= strcmp(mqttServer, settings[FP_CONFIG_HOST]) != 0;
        // strncpy(mqttServer, settings[FP_CONFIG_HOST], sizeof(mqttServer));
        changed |= nvsUpdateString(preferences, FP_CONFIG_HOST, settings[FP_CONFIG_HOST]);
    }

    if(!settings[FP_CONFIG_USER].isNull()) {
        // changed |= strcmp(mqttUsername, settings[FP_CONFIG_USER]) != 0;
        // strncpy(mqttUsername, settings[FP_CONFIG_USER], sizeof(mqttUsername));
        changed |= nvsUpdateString(preferences, FP_CONFIG_USER, settings[FP_CONFIG_USER]);
    }

    if(!settings[FP_CONFIG_PASS].isNull() && settings[FP_CONFIG_PASS].as<String>() != String(D_PASSWORD_MASK)) {
        // changed |= strcmp(mqttPassword, settings[FP_CONFIG_PASS]) != 0;
        // strncpy(mqttPassword, settings[FP_CONFIG_PASS], sizeof(mqttPassword));
        changed |= nvsUpdateString(preferences, FP_CONFIG_PASS, settings[FP_CONFIG_PASS]);
    }

    JsonVariant topic;
    topic = settings["topic"][FP_CONFIG_NODE];
    if(topic.is<const char*>()) {
        changed |= nvsUpdateString(preferences, FP_CONFIG_NODE_TOPIC, topic);
    }
    topic = settings["topic"][FP_CONFIG_GROUP];
    if(topic.is<const char*>()) {
        changed |= nvsUpdateString(preferences, FP_CONFIG_GROUP_TOPIC, topic);
    }
    topic = settings["topic"][FP_CONFIG_BROADCAST];
    if(topic.is<const char*>()) {
        changed |= nvsUpdateString(preferences, FP_CONFIG_BROADCAST_TOPIC, topic);
    }
    topic = settings["topic"][FP_CONFIG_HASS];
    if(topic.is<const char*>()) {
        changed |= nvsUpdateString(preferences, FP_CONFIG_HASS_TOPIC, topic);
    }

    // snprintf_P(mqttNodeTopic, sizeof(mqttNodeTopic), PSTR(MQTT_PREFIX "/%s/"), haspDevice.get_hostname());
    // snprintf_P(mqttGroupTopic, sizeof(mqttGroupTopic), PSTR(MQTT_PREFIX "/%s/"), mqttGroupName);

    preferences.end();
    return changed;
}
#endif // HASP_USE_CONFIG

#endif // PUBSUBCLIENT

#endif // HASP_USE_MQTT
