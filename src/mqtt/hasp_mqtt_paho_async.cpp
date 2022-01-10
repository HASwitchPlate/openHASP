/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Multi threaded asynchronous paho client */

#include <stdint.h>

#include "hasp_conf.h"

#if HASP_USE_MQTT_ASYNC > 0
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
#include <mutex>

#include "MQTTAsync.h"

#include "hasp_mqtt.h" // functions to implement here

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

#define ADDRESS "10.4.0.5:1883"
#define CLIENTID "ExampleClientSub"
#define TOPIC "hasp/plate35/"
#define QOS 1
#define TIMEOUT 10000L

const char* mqttNodeTopic  = TOPIC;
const char* mqttGroupTopic = TOPIC;
// char mqttNodeTopic[24];
// char mqttGroupTopic[24];
bool mqttEnabled        = false;
bool mqttHAautodiscover = true;

std::recursive_mutex dispatch_mtx;
std::recursive_mutex publish_mtx;

char mqttServer[MAX_HOSTNAME_LENGTH]   = MQTT_HOST;
char mqttUser[MAX_USERNAME_LENGTH]     = MQTT_USER;
char mqttPassword[MAX_PASSWORD_LENGTH] = MQTT_PASSW;
// char mqttNodeName[16]  = MQTT_NODENAME;
char mqttGroupName[16] = MQTT_GROUPNAME;
uint16_t mqttPort      = MQTT_PORT;

MQTTAsync mqtt_client;

int disc_finished = 0;
int subscribed    = 0;
int connected     = 0;

static bool mqttPublish(const char* topic, const char* payload, size_t len, bool retain = false);

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
        LOG_ERROR(TAG_MQTT_RCV, F(D_MQTT_PAYLOAD_TOO_LONG), (uint32_t)length);
        return;
    } else {
        payload[length] = '\0';
    }

    LOG_TRACE(TAG_MQTT_RCV, F("%s = %s"), topic, (char*)payload);

    if(topic == strstr(topic, mqttNodeTopic)) { // startsWith mqttNodeTopic

        // Node topic
        topic += strlen(mqttNodeTopic); // shorten topic

    } else if(topic == strstr(topic, mqttGroupTopic)) { // startsWith mqttGroupTopic

        // Group topic
        topic += strlen(mqttGroupTopic); // shorten topic
        dispatch_mtx.lock();
        dispatch_topic_payload(topic, (const char*)payload);
        dispatch_mtx.unlock();
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
    if(!strcmp_P(topic, PSTR(MQTT_TOPIC_LWT))) { // endsWith LWT
        if(!strcasecmp_P((char*)payload, PSTR("offline"))) {
            {
                char msg[8];
                char tmp_topic[strlen(mqttNodeTopic) + 8];
                snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" MQTT_TOPIC_LWT), mqttNodeTopic);
                snprintf_P(msg, sizeof(msg), PSTR("online"));

                mqttPublish(tmp_topic, msg, true);
            }

        } else {
            // LOG_TRACE(TAG_MQTT, F("ignoring LWT = online"));
        }
    } else {
        dispatch_mtx.lock();
        dispatch_topic_payload(topic, (const char*)payload);
        dispatch_mtx.unlock();
    }
}

int msgarrvd(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
    // printf("MQT RCV >> ");
    // printf("%s => %.*s (%d)\n", topicName, message->payloadlen, (char *)message->payload, message->payloadlen);

    char msg[message->payloadlen + 1];
    memcpy(msg, (char*)message->payload, message->payloadlen);
    msg[message->payloadlen] = '\0';

    mqtt_message_cb(topicName, (char*)message->payload, message->payloadlen);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Disconnect failed, rc %d\n", response->code);
    disc_finished = 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
    printf("Successful disconnection\n");
    disc_finished = 1;
    connected     = 0;
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
    printf("Subscribe succeeded %d\n", response->token);
    subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response->code);
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    connected = 0;
    printf("Connect failed, rc %d\n", response->code);
}

void mqtt_subscribe(void* context, const char* topic)
{
    MQTTAsync client               = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", topic, CLIENTID, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context   = client;
    if((rc = MQTTAsync_subscribe(client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
    }
}

void onConnect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    connected        = 1;

    printf("Successful connection\n");

    mqtt_subscribe(context, TOPIC MQTT_TOPIC_COMMAND "/#");
    mqtt_subscribe(context, TOPIC MQTT_TOPIC_COMMAND);
    mqtt_subscribe(context, TOPIC "light");
    mqtt_subscribe(context, TOPIC "dim");

    mqttPublish(TOPIC MQTT_TOPIC_LWT, "online", false);

    mqtt_send_object_state(0, 0, "connected");
    std::cout << std::endl;
}

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
    MQTTAsync client                 = (MQTTAsync)context;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    printf("Message send failed token %d error code %d\n", response->token, response->code);
    opts.onSuccess = onDisconnect;
    opts.onFailure = onDisconnectFailure;
    opts.context   = client;
    if((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        // exit(EXIT_FAILURE);
    }
}

void onSend(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client                 = (MQTTAsync)context;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    // printf("Message with token value %d delivery confirmed\n", response->token);

    // opts.onSuccess = onDisconnect;
    // opts.onFailure = onDisconnectFailure;
    // opts.context = client;
    // if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
    // {
    //     printf("Failed to start disconnect, return code %d\n", rc);
    //     exit(EXIT_FAILURE);
    // }
}

/* ===== Local HASP MQTT functions ===== */

static bool mqttPublish(const char* topic, const char* payload, size_t len, bool retain)
{
    if(mqttIsConnected()) {
        MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
        MQTTAsync_message pubmsg       = MQTTAsync_message_initializer;
        int rc;

        opts.onSuccess    = onSend;
        opts.onFailure    = onSendFailure;
        opts.context      = mqtt_client;
        pubmsg.payload    = (char*)payload;
        pubmsg.payloadlen = (int)strlen(payload);
        pubmsg.qos        = QOS;
        pubmsg.retained   = 0;
        dispatch_mtx.lock();
        if((rc = MQTTAsync_sendMessage(mqtt_client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
            dispatch_mtx.unlock();
            LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " %s => %s"), topic, payload);
        } else {
            dispatch_mtx.unlock();
            LOG_TRACE(TAG_MQTT_PUB, F("%s => %s"), topic, payload);
            return true;
        }
    } else {
        LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_CONNECTED));
    }
    return false;
}

/* ===== Public HASP MQTT functions ===== */

bool mqttIsConnected()
{
    return connected == 1;
}

void mqtt_send_state(const __FlashStringHelper* subtopic, const char* payload)
{
    char tmp_topic[strlen(mqttNodeTopic) + 20];
    printf(("%s" MQTT_TOPIC_STATE "/%s\n"), mqttNodeTopic, subtopic);
    snprintf_P(tmp_topic, sizeof(tmp_topic), ("%s" MQTT_TOPIC_STATE "/%s"), mqttNodeTopic, subtopic);
    mqttPublish(tmp_topic, payload, false);
}

void mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload)
{
    char tmp_topic[strlen(mqttNodeTopic) + 20];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" MQTT_TOPIC_STATE "/p%ub%u"), mqttNodeTopic, pageid, btnid);
    mqttPublish(tmp_topic, payload, false);
}

void mqttStart()
{
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_willOptions will_opts    = MQTTAsync_willOptions_initializer;
    int rc;
    int ch;

    if((rc = MQTTAsync_create(&mqtt_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) !=
       MQTTASYNC_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        return;
    }

    if((rc = MQTTAsync_setCallbacks(mqtt_client, mqtt_client, connlost, msgarrvd, NULL)) != MQTTASYNC_SUCCESS) {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        return;
    }

    conn_opts.will            = &will_opts;
    conn_opts.will->message   = "offline";
    conn_opts.will->qos       = 1;
    conn_opts.will->retained  = 0;
    conn_opts.will->topicName = "hasp/plate35/LWT";

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession      = 1;
    conn_opts.onSuccess         = onConnect;
    conn_opts.onFailure         = onConnectFailure;
    conn_opts.context           = mqtt_client;

    if((rc = MQTTAsync_connect(mqtt_client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        //   goto destroy_exit;
    } else {
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
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.onSuccess                   = onDisconnect;
    disc_opts.onFailure                   = onDisconnectFailure;
    if((rc = MQTTAsync_disconnect(mqtt_client, &disc_opts)) != MQTTASYNC_SUCCESS) {
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
    //     MQTTAsync_destroy(&client);
    // exit:
    //     return rc;
}

void mqttSetup(){};

IRAM_ATTR void mqttLoop(){};

void mqttEvery5Seconds(bool wifiIsConnected){};

#endif // USE_PAHO
#endif // USE_MQTT
