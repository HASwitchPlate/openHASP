/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Single threaded synchronous paho client */

#include "hasplib.h"

#if HASP_USE_MQTT > 0
#ifdef HASP_USE_PAHO

const char FP_CONFIG_HOST[] PROGMEM  = "host";
const char FP_CONFIG_PORT[] PROGMEM  = "port";
const char FP_CONFIG_NAME[] PROGMEM  = "name";
const char FP_CONFIG_USER[] PROGMEM  = "user";
const char FP_CONFIG_PASS[] PROGMEM  = "pass";
const char FP_CONFIG_GROUP[] PROGMEM = "group";

/*******************************************************************************
 * Copyright (c) 2012, 2020 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   https://www.eclipse.org/legal/epl-2.0/
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "MQTTClient.h"

#include "hasp_mqtt.h"    // functions to implement here
#include "hasp_mqtt_ha.h" // HA functions

#include "hasp/hasp_dispatch.h" // for dispatch_topic_payload
#include "hasp_debug.h"         // for logging

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif

// #define ADDRESS "10.1.0.208:1883"
// #define CLIENTID "test1123"
// #define TOPIC "hasp/plate35/"
#define QOS 1
#define TIMEOUT 1000L

std::string mqttNodeTopic;
std::string mqttGroupTopic;
std::string mqttLwtTopic;
bool mqttEnabled        = false;
bool mqttHAautodiscover = true;
uint32_t mqttPublishCount;
uint32_t mqttReceiveCount;
uint32_t mqttFailedCount;

std::string mqttServer    = MQTT_HOSTNAME;
std::string mqttUsername  = MQTT_USERNAME;
std::string mqttPassword  = MQTT_PASSWORD;
std::string mqttGroupName = MQTT_GROUPNAME;
uint16_t mqttPort         = MQTT_PORT;

MQTTClient mqtt_client;

int disc_finished = 0;
int subscribed    = 0;
int connected     = 0;

int mqttPublish(const char* topic, const char* payload, size_t len, bool retain);

/* ===== Paho event callbacks ===== */

void connlost(void* context, char* cause)
{
    LOG_WARNING(TAG_MQTT, F(D_MQTT_DISCONNECTED ": %s"), cause);
}

// Receive incoming messages
static void mqtt_message_cb(char* topic, char* payload, size_t length)
{ // Handle incoming commands from MQTT
    if(length + 1 >= MQTT_MAX_PACKET_SIZE) {
        mqttFailedCount++;
        LOG_ERROR(TAG_MQTT_RCV, F(D_MQTT_PAYLOAD_TOO_LONG), (uint32_t)length);
        return;
    } else {
        mqttReceiveCount++;
        payload[length] = '\0';
    }

    LOG_TRACE(TAG_MQTT_RCV, F("%s = %s"), topic, (char*)payload);

    if(topic == strstr(topic, mqttNodeTopic.c_str())) { // startsWith mqttNodeTopic

        // Node topic
        topic += mqttNodeTopic.length(); // shorten topic

    } else if(topic == strstr(topic, mqttGroupTopic.c_str())) { // startsWith mqttGroupTopic

        // Group topic
        topic += mqttGroupTopic.length(); // shorten topic
        dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
        return;

#ifdef HASP_USE_BROADCAST
    } else if(topic == strstr_P(topic, PSTR(MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST
                                                        "/"))) { // /" MQTT_TOPIC_BROADCAST "/ discovery topic

        // /" MQTT_TOPIC_BROADCAST "/ topic
        topic += strlen(MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST "/"); // shorten topic
        dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
        return;
#endif

#ifdef HASP_USE_HA
    } else if(topic == strstr_P(topic, PSTR("homeassistant/status"))) { // HA discovery topic
        if(mqttHAautodiscover && !strcasecmp_P((char*)payload, PSTR("online"))) {
            dispatch_current_state(TAG_MQTT);
            mqtt_ha_register_auto_discovery();
        }
        return;
#endif

    } else {
        // Other topic
        LOG_ERROR(TAG_MQTT, F(D_MQTT_INVALID_TOPIC));
        return;
    }

    // catch a dangling LWT from a previous connection if it appears
    if(!strcmp_P(topic, PSTR(MQTT_TOPIC_LWT))) { // endsWith LWT
        if(!strcasecmp_P((char*)payload, PSTR("offline"))) {
            {
                char msg[8];
                snprintf_P(msg, sizeof(msg), PSTR("online"));
                mqttPublish(mqttLwtTopic.c_str(), msg, strlen(msg), true);
            }

        } else {
            // LOG_TRACE(TAG_MQTT, F("ignoring LWT = online"));
        }
    } else {
        dispatch_topic_payload(topic, (const char*)payload, length > 0, TAG_MQTT);
    }
}

int mqtt_message_arrived(void* context, char* topicName, int topicLen, MQTTClient_message* message)
{
    char msg[message->payloadlen + 1];
    memcpy(msg, (char*)message->payload, message->payloadlen);
    msg[message->payloadlen] = '\0';

    mqtt_message_cb(topicName, msg, message->payloadlen);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1; // the message was received properly
}

void mqtt_subscribe(void* context, const char* topic)
{
    MQTTClient client = (MQTTClient)context;
    int rc;

    if((rc = MQTTClient_subscribe(client, topic, QOS)) != MQTTCLIENT_SUCCESS) {
        LOG_WARNING(TAG_MQTT, D_BULLET D_MQTT_NOT_SUBSCRIBED, topic); // error code rc
    } else {
        LOG_VERBOSE(TAG_MQTT, D_BULLET D_MQTT_SUBSCRIBED, topic);
    }
}

/* ===== Local HASP MQTT functions ===== */

int mqttPublish(const char* topic, const char* payload, size_t len, bool retain)
{
    if(!mqttEnabled) return MQTT_ERR_DISABLED;

    if(!mqttIsConnected()) {
        mqttFailedCount++;
        return MQTT_ERR_NO_CONN;
    }

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload    = (char*)payload;
    pubmsg.payloadlen = len; // (int)strlen(payload);
    pubmsg.qos        = QOS;
    pubmsg.retained   = retain;

    MQTTClient_publishMessage(mqtt_client, topic, &pubmsg, &token);
    int rc = MQTTClient_waitForCompletion(mqtt_client, token, TIMEOUT); // time to wait in milliseconds

    if(rc != MQTTCLIENT_SUCCESS) {
        mqttFailedCount++;
        LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " '%s' => %s"), topic, payload);
        return MQTT_ERR_PUB_FAIL;
    } else {
        // LOG_TRACE(TAG_MQTT_PUB, F("'%s' => %s OK"), topic, payload);
        mqttPublishCount++;
        return MQTT_ERR_OK;
    }
}

/* ===== Public HASP MQTT functions ===== */

bool mqttIsConnected()
{
    return MQTTClient_isConnected(mqtt_client);
}

int mqtt_send_state(const __FlashStringHelper* subtopic, const char* payload)
{
    char tmp_topic[mqttNodeTopic.length() + 20];
    snprintf_P(tmp_topic, sizeof(tmp_topic), ("%s" MQTT_TOPIC_STATE "/%s"), mqttNodeTopic.c_str(), subtopic);
    return mqttPublish(tmp_topic, payload, strlen(payload), false);
}

int mqtt_send_discovery(const char* payload, size_t len)
{
    char tmp_topic[128];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR(MQTT_PREFIX "/" MQTT_TOPIC_DISCOVERY "/%s"),haspDevice.get_hardware_id());
    return mqttPublish(tmp_topic, payload, len, false);
}

// int mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload)
// {
//     char tmp_topic[mqttNodeTopic.length() + 20];
//     snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" MQTT_TOPIC_STATE "/p%ub%u"), mqttNodeTopic.c_str(), pageid,
//                btnid);
//     return mqttPublish(tmp_topic, payload, strlen(payload), false);
// }

static void onConnect(void* context)
{
    MQTTClient client = (MQTTClient)context;
    connected         = 1;
    std::string topic;

    LOG_VERBOSE(TAG_MQTT, D_MQTT_CONNECTED, mqttServer.c_str(), haspDevice.get_hostname());

    topic = mqttGroupTopic + MQTT_TOPIC_COMMAND "/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttNodeTopic + MQTT_TOPIC_COMMAND "/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttGroupTopic + "config/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttNodeTopic + "config/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

#if defined(HASP_USE_CUSTOM)
    topic = mqttGroupTopic + MQTT_TOPIC_CUSTOM "/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttNodeTopic + MQTT_TOPIC_CUSTOM "/#";
    mqtt_subscribe(mqtt_client, topic.c_str());
#endif

#ifdef HASP_USE_BROADCAST
    topic = MQTT_PREFIX "/" MQTT_TOPIC_BROADCAST "/" MQTT_TOPIC_COMMAND "/#";
    mqtt_subscribe(mqtt_client, topic.c_str());
#endif

    /* Home Assistant auto-configuration */
#ifdef HASP_USE_HA
    topic = "homeassistant/status";
    mqtt_subscribe(mqtt_client, topic.c_str());
#endif

    mqttPublish(mqttLwtTopic.c_str(), "online", 6, true);
}

void mqttStart()
{
    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions will_opts    = MQTTClient_willOptions_initializer;
    int rc;
    int ch;

    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
    if((rc = MQTTClient_create(&mqtt_client, mqttServer.c_str(), haspDevice.get_hostname(), MQTTCLIENT_PERSISTENCE_NONE,
                               NULL)) != MQTTCLIENT_SUCCESS) {
        LOG_ERROR(TAG_MQTT, "Failed to create client, return code %d", rc);
        rc = EXIT_FAILURE;
        return;
    }

    if((rc = MQTTClient_setCallbacks(mqtt_client, mqtt_client, connlost, mqtt_message_arrived, NULL)) !=
       MQTTCLIENT_SUCCESS) {
        LOG_ERROR(TAG_MQTT, "Failed to set callbacks, return code %d", rc);
        rc = EXIT_FAILURE;
        return;
    }

    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
    mqttEnabled = mqttServer.length() > 0 && mqttPort > 0;

    if(mqttEnabled) {
        conn_opts.will            = &will_opts;
        conn_opts.will->message   = "offline";
        conn_opts.will->qos       = 1;
        conn_opts.will->retained  = 1;
        conn_opts.will->topicName = mqttLwtTopic.c_str();

        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession      = 1;
        conn_opts.connectTimeout    = 2;  // seconds
        conn_opts.retryInterval     = 15; // 0 = no retry

        conn_opts.username = mqttUsername.c_str();
        conn_opts.password = mqttPassword.c_str();

        LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
        if((rc = MQTTClient_connect(mqtt_client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
            LOG_ERROR(TAG_MQTT, "Failed to connect, return code %d", rc);
            rc = EXIT_FAILURE;
            //   goto destroy_exit;
        } else {
            onConnect(&mqtt_client);
        }
    } else {
        rc = EXIT_FAILURE;
        LOG_WARNING(TAG_MQTT, "Mqtt server not configured");
    }

    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
}

void mqttStop()
{
    int rc;
    // MQTTClient_disconnectOptions disc_opts = MQTTClient_disconnectOptions_initializer;
    // disc_opts.onSuccess                    = onDisconnect;
    // disc_opts.onFailure                    = onDisconnectFailure;
    if((rc = MQTTClient_disconnect(mqtt_client, 1000)) != MQTTCLIENT_SUCCESS) {
        LOG_ERROR(TAG_MQTT, "Failed to disconnect, return code %d", rc);
        rc = EXIT_FAILURE;
    }
}

void mqttSetup()
{
    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
    mqttNodeTopic = MQTT_PREFIX;
    mqttNodeTopic += "/";
    mqttNodeTopic += haspDevice.get_hostname();
    mqttNodeTopic += "/";

    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
    mqttGroupTopic = MQTT_PREFIX;
    mqttGroupTopic += "/";
    mqttGroupTopic += mqttGroupName;
    mqttGroupTopic += "/";

    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
    mqttLwtTopic = mqttNodeTopic;
    mqttLwtTopic += MQTT_TOPIC_LWT;

    LOG_DEBUG(TAG_MQTT, "%s %d", __FILE__, __LINE__);
}

IRAM_ATTR void mqttLoop()
{
    int topicLen;
    char* topicName;             // Freed by msgarrvd
    MQTTClient_message* message; // Freed by msgarrvd

    int rc = MQTTClient_receive(mqtt_client, &topicName, &topicLen, &message, 4);
    if(rc == MQTTCLIENT_SUCCESS && message) mqtt_message_arrived(mqtt_client, topicName, topicLen, message);
};

void mqttEvery5Seconds(bool wifiIsConnected)
{
    if(!mqttIsConnected()) {
        LOG_WARNING(TAG_MQTT, F(D_MQTT_RECONNECTING));
        mqttStart();
    }
};

void mqtt_get_info(JsonDocument& doc)
{
    char mqttClientId[64];

    JsonObject info           = doc.createNestedObject(F("MQTT"));
    info[F(D_INFO_SERVER)]    = mqttServer;
    info[F(D_INFO_USERNAME)]  = mqttUsername;
    info[F(D_INFO_CLIENTID)]  = haspDevice.get_hostname();
    info[F(D_INFO_STATUS)]    = mqttIsConnected() ? F(D_SERVICE_CONNECTED) : F(D_SERVICE_DISCONNECTED);
    info[F(D_INFO_RECEIVED)]  = mqttReceiveCount;
    info[F(D_INFO_PUBLISHED)] = mqttPublishCount;
    info[F(D_INFO_FAILED)]    = mqttFailedCount;
}

/** Set MQTT Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool mqttSetConfig(const JsonObject& settings)
{
    // configOutput(settings, TAG_MQTT);
    bool changed = false;

    // changed |= configSet(mqttPort, settings[FPSTR(FP_CONFIG_PORT)], F("mqttPort"));
    changed |= mqttPort != settings[FPSTR(FP_CONFIG_PORT)];
    mqttPort = settings[FPSTR(FP_CONFIG_PORT)];

    if(!settings[FPSTR(FP_CONFIG_NAME)].isNull()) {
        LOG_VERBOSE(TAG_MQTT, "%s => %s", FP_CONFIG_NAME, settings[FPSTR(FP_CONFIG_NAME)].as<const char*>());
        changed |= strcmp(haspDevice.get_hostname(), settings[FPSTR(FP_CONFIG_NAME)]) != 0;
        // strncpy(mqttNodeName, settings[FPSTR(FP_CONFIG_NAME)], sizeof(mqttNodeName));
        haspDevice.set_hostname(settings[FPSTR(FP_CONFIG_NAME)].as<const char*>());
    }
    // Prefill node name
    // if(strlen(haspDevice.get_hostname()) == 0) {
    //     char mqttNodeName[64];
    //     std::string mac = halGetMacAddress(3, "");
    //     mac.toLowerCase();
    //     snprintf_P(mqttNodeName, sizeof(mqttNodeName), PSTR(D_MQTT_DEFAULT_NAME), mac.c_str());
    //     haspDevice.set_hostname(mqttNodeName);
    //     changed = true;
    // }

    if(!settings[FPSTR(FP_CONFIG_GROUP)].isNull()) {
        changed |= mqttGroupName != settings[FPSTR(FP_CONFIG_GROUP)];
        mqttGroupName = settings[FPSTR(FP_CONFIG_GROUP)].as<std::string>();
    }

    if(mqttGroupName.length() == 0) {
        mqttGroupName = "plates";
        changed       = true;
    }

    if(!settings[FPSTR(FP_CONFIG_HOST)].isNull()) {
        LOG_VERBOSE(TAG_MQTT, "%s => %s", FP_CONFIG_HOST, settings[FPSTR(FP_CONFIG_HOST)].as<const char*>());
        changed |= mqttServer != settings[FPSTR(FP_CONFIG_HOST)];
        mqttServer = settings[FPSTR(FP_CONFIG_HOST)].as<const char*>();
    }

    if(!settings[FPSTR(FP_CONFIG_USER)].isNull()) {
        changed |= mqttUsername != settings[FPSTR(FP_CONFIG_USER)];
        mqttUsername = settings[FPSTR(FP_CONFIG_USER)].as<const char*>();
    }

    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() &&
       settings[FPSTR(FP_CONFIG_PASS)].as<std::string>() != D_PASSWORD_MASK) {
        changed |= mqttPassword != settings[FPSTR(FP_CONFIG_PASS)];
        mqttPassword = settings[FPSTR(FP_CONFIG_PASS)].as<const char*>();
    }

    mqttNodeTopic = MQTT_PREFIX;
    mqttNodeTopic += haspDevice.get_hostname();
    mqttGroupTopic = MQTT_PREFIX;
    mqttGroupTopic += mqttGroupName;

    return changed;
}

#endif // HASP_USE_PAHO
#endif // USE_MQTT
