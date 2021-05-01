/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Single threaded synchronous paho client */

#include "hasplib.h"

#if HASP_USE_MQTT > 0
#ifdef USE_PAHO

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// These defaults may be overwritten with values saved by the web interface
#ifndef MQTT_HOST
#define MQTT_HOST "10.1.0.208";
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883;
#endif

#ifndef MQTT_USER
#define MQTT_USER "hasp";
#endif

#ifndef MQTT_PASSW
#define MQTT_PASSW "hasp";
#endif

#ifndef MQTT_GROUPNAME
#define MQTT_GROUPNAME "plates";
#endif

#ifndef MQTT_PREFIX
#define MQTT_PREFIX "hasp"
#endif

#define LWT_TOPIC "LWT"

std::string mqttServer    = MQTT_HOST;
std::string mqttUser      = MQTT_USER;
std::string mqttPassword  = MQTT_PASSW;
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
    printf("\nConnection lost\n");
    if(cause) printf("     cause: %s\n", cause);

    printf("Reconnecting\n");
    mqttStart();
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
        dispatch_topic_payload(topic, (const char*)payload);
        return;

#ifdef HASP_USE_HA
    } else if(topic == strstr_P(topic, PSTR("homeassistant/status"))) { // HA discovery topic
        if(mqttHAautodiscover && !strcasecmp_P((char*)payload, PSTR("online"))) {
            dispatch_current_state();
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
    if(!strcmp_P(topic, PSTR(LWT_TOPIC))) { // endsWith LWT
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
        dispatch_topic_payload(topic, (const char*)payload);
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
    // printf(("%sstate/%s\n"), mqttNodeTopic, subtopic);
    snprintf_P(tmp_topic, sizeof(tmp_topic), ("%sstate/%s"), mqttNodeTopic.c_str(), subtopic);
    return mqttPublish(tmp_topic, payload, strlen(payload), false);
}

int mqtt_send_discovery(const char* payload, size_t len)
{
    char tmp_topic[20];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR(MQTT_PREFIX "/discovery"));
    return mqttPublish(tmp_topic, payload, len, false);
}

int mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload)
{
    char tmp_topic[mqttNodeTopic.length() + 20];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%sstate/p%ub%u"), mqttNodeTopic.c_str(), pageid, btnid);
    return mqttPublish(tmp_topic, payload, strlen(payload), false);
}

static void onConnect(void* context)
{
    MQTTClient client = (MQTTClient)context;
    connected         = 1;
    std::string topic;

    LOG_VERBOSE(TAG_MQTT, "Successful connection");

    topic = mqttGroupTopic + "command/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttNodeTopic + "command/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttGroupTopic + "config/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    topic = mqttNodeTopic + "config/#";
    mqtt_subscribe(mqtt_client, topic.c_str());

    /* Home Assistant auto-configuration */
#ifdef HASP_USE_HA
    topic = "homeassistant/status";
    mqtt_subscribe(mqtt_client, topic.c_str());
#endif

    mqttPublish(mqttLwtTopic.c_str(), "online", 6, true);
}

void mqttStart()
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions will_opts    = MQTTClient_willOptions_initializer;
    int rc;
    int ch;

    if((rc = MQTTClient_create(&mqtt_client, mqttServer.c_str(), haspDevice.get_hostname(), MQTTCLIENT_PERSISTENCE_NONE,
                               NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        return;
    }

    // if((rc = MQTTClient_setCallbacks(mqtt_client, mqtt_client, connlost, msgarrvd, NULL)) != MQTTCLIENT_SUCCESS) {
    //     printf("Failed to set callbacks, return code %d\n", rc);
    //     rc = EXIT_FAILURE;
    //     return;
    // }

    conn_opts.will            = &will_opts;
    conn_opts.will->message   = "offline";
    conn_opts.will->qos       = 1;
    conn_opts.will->retained  = 1;
    conn_opts.will->topicName = mqttLwtTopic.c_str();

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession      = 1;

    conn_opts.username = mqttUser.c_str();
    conn_opts.password = mqttPassword.c_str();

    if((rc = MQTTClient_connect(mqtt_client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        //   goto destroy_exit;
    } else {
        onConnect(&mqtt_client);
    }

    //     while (!subscribed && !finished)
    // #if defined(_WIN32)
    //         Sleep(100);
    // #else
    //         usleep(10000L);
    // #endif

    // if (finished)
    //     goto exit;
}

void mqttStop()
{
    int rc;
    // MQTTClient_disconnectOptions disc_opts = MQTTClient_disconnectOptions_initializer;
    // disc_opts.onSuccess                    = onDisconnect;
    // disc_opts.onFailure                    = onDisconnectFailure;
    if((rc = MQTTClient_disconnect(mqtt_client, 1000)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        // goto destroy_exit;
    }
    //     while (!disc_finished)
    //     {
    // #if defined(_WIN32)
    //         Sleep(100);
    // #else
    //         usleep(10000L);
    // #endif
    //     }

    // destroy_exit:
    //     MQTTClient_destroy(&client);
    // exit:
    //     return rc;
}

void mqttSetup()
{
    mqttNodeTopic = MQTT_PREFIX;
    mqttNodeTopic += "/";
    mqttNodeTopic += haspDevice.get_hostname();
    mqttNodeTopic += "/";

    mqttGroupTopic = MQTT_PREFIX;
    mqttGroupTopic += "/";
    mqttGroupTopic += mqttGroupName;
    mqttGroupTopic += "/";

    mqttLwtTopic = mqttNodeTopic;
    mqttLwtTopic += LWT_TOPIC;
}

void mqttLoop()
{
    int topicLen;
    char* topicName;             // Freed by msgarrvd
    MQTTClient_message* message; // Freed by msgarrvd

    int rc = MQTTClient_receive(mqtt_client, &topicName, &topicLen, &message, 4);
    if(rc == MQTTCLIENT_SUCCESS && message) mqtt_message_arrived(mqtt_client, topicName, topicLen, message);
};

void mqttEvery5Seconds(bool wifiIsConnected){};

void mqtt_get_info(JsonDocument& doc)
{
    char mqttClientId[64];

    JsonObject info     = doc.createNestedObject(F("MQTT"));
    info[F("Server")]   = mqttServer;
    info[F("User")]     = mqttUser;
    info[F("ClientID")] = haspDevice.get_hostname();

    if(mqttIsConnected()) { // Check MQTT connection
        info[F("Status")] = F("Connected");
    } else {
        info[F("Status")] = F("<font color='red'><b>Disconnected</b></font>, return code: ");
        //     +String(mqttClient.returnCode());
    }

    info[F("Received")]  = mqttReceiveCount;
    info[F("Published")] = mqttPublishCount;
    info[F("Failed")]    = mqttFailedCount;
}

#endif // USE_PAHO
#endif // USE_MQTT
