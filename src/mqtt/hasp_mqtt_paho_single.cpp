/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Single threaded synchronous paho client */

#include <stdint.h>

#include "hasp_conf.h"

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

#define ADDRESS "10.1.0.208:1883"
#define CLIENTID "test1123"
#define TOPIC "hasp/plate35/"
#define QOS 1
#define TIMEOUT 1000L

char mqttNodeTopic[24]     = "hasp/plate35/";
const char* mqttGroupTopic = "hasp/plates/";
// char mqttNodeTopic[24];
// char mqttGroupTopic[24];
bool mqttEnabled        = false;
bool mqttHAautodiscover = true;

////////////////////////////////////////////////////////////////////////////////////////////////////
// These defaults may be overwritten with values saved by the web interface
#ifndef MQTT_HOST
#define MQTT_HOST "";
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883;
#endif

#ifndef MQTT_USER
#define MQTT_USER "";
#endif

#ifndef MQTT_PASSW
#define MQTT_PASSW "";
#endif
#ifndef MQTT_NODENAME
#define MQTT_NODENAME "";
#endif
#ifndef MQTT_GROUPNAME
#define MQTT_GROUPNAME "";
#endif

#ifndef MQTT_PREFIX
#define MQTT_PREFIX "hasp"
#endif

#define LWT_TOPIC "LWT"

char mqttServer[16]   = MQTT_HOST;
char mqttUser[23]     = MQTT_USER;
char mqttPassword[32] = MQTT_PASSW;
// char mqttNodeName[16]  = MQTT_NODENAME;
char mqttGroupName[16] = MQTT_GROUPNAME;
uint16_t mqttPort      = MQTT_PORT;

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
static void mqtt_message_cb(char* topic, char* payload, unsigned int length)
{ // Handle incoming commands from MQTT
    if(length + 1 >= MQTT_MAX_PACKET_SIZE) {
        LOG_ERROR(TAG_MQTT_RCV, F("Payload too long (%d bytes)"), length);
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
        dispatch_topic_payload(topic, (const char*)payload);
        return;

    } else if(topic == strstr_P(topic, PSTR("homeassistant/status"))) { // HA discovery topic
        if(mqttHAautodiscover && !strcasecmp_P((char*)payload, PSTR("online"))) {
            dispatch_current_state();
            mqtt_ha_register_auto_discovery();
        }
        return;

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
                char tmp_topic[strlen(mqttNodeTopic) + 8];
                snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%s" LWT_TOPIC), mqttNodeTopic);
                snprintf_P(msg, sizeof(msg), PSTR("online"));

                // /*bool res =*/mqttClient.publish(tmp_topic, msg, true);
                mqttPublish(tmp_topic, msg, strlen(msg), true);
            }

        } else {
            // LOG_TRACE(TAG_MQTT, F("ignoring LWT = online"));
        }
    } else {
        dispatch_topic_payload(topic, (const char*)payload);
    }
}

int msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message)
{
    // printf("MQT RCV >> ");
    // printf("%s => %.*s (%d)\n", topicName, message->payloadlen, (char *)message->payload, message->payloadlen);

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

    printf("Subscribing to topic %s\n", topic);
    //\nfor client %s using QoS%d\n\n", topic, CLIENTID, QOS);
    if((rc = MQTTClient_subscribe(client, topic, QOS)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
    }
}

/* ===== Local HASP MQTT functions ===== */

int mqttPublish(const char* topic, const char* payload, size_t len, bool retain)
{
    if(!mqttIsConnected()) return MQTT_ERR_NO_CONN;

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload    = (char*)payload;
    pubmsg.payloadlen = len; // (int)strlen(payload);
    pubmsg.qos        = QOS;
    pubmsg.retained   = retain;

    MQTTClient_publishMessage(mqtt_client, topic, &pubmsg, &token);
    int rc = MQTTClient_waitForCompletion(mqtt_client, token, TIMEOUT);

    if(rc != MQTTCLIENT_SUCCESS) {
        LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " '%s' => %s"), topic, payload);
        return MQTT_ERR_PUB_FAIL;
    } else {
        LOG_TRACE(TAG_MQTT_PUB, F("'%s' => %s OK"), topic, payload);
        return MQTT_ERR_OK;
    }
}

// static bool mqttPublish(const char* topic, const char* payload, bool retain)
// {
//     return mqttPublish(topic, payload, strlen(payload), retain);
// }

/* ===== Public HASP MQTT functions ===== */

bool mqttIsConnected()
{
    return connected == 1;
}

int mqtt_send_state(const __FlashStringHelper* subtopic, const char* payload)
{
    char tmp_topic[strlen(mqttNodeTopic) + 20];
    // printf(("%sstate/%s\n"), mqttNodeTopic, subtopic);
    snprintf_P(tmp_topic, sizeof(tmp_topic), ("%sstate/%s"), mqttNodeTopic, subtopic);
    return mqttPublish(tmp_topic, payload, strlen(payload), false);
}

int mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload)
{
    char tmp_topic[strlen(mqttNodeTopic) + 20];
    snprintf_P(tmp_topic, sizeof(tmp_topic), PSTR("%sstate/p%ub%u"), mqttNodeTopic, pageid, btnid);
    return mqttPublish(tmp_topic, payload, strlen(payload), false);
}

static void onConnect(void* context)
{
    MQTTClient client = (MQTTClient)context;
    connected         = 1;

    printf("Successful connection\n");

    mqtt_subscribe(mqtt_client, TOPIC "command/#");
    // mqtt_subscribe(mqtt_client, TOPIC "command");
    mqtt_subscribe(mqtt_client, TOPIC "light/#");
    mqtt_subscribe(mqtt_client, TOPIC "brightness/#");
    mqtt_subscribe(mqtt_client, "hass/status");

    /* Home Assistant auto-configuration */
    if(mqttHAautodiscover) mqtt_subscribe(mqtt_client, "homeassistant/status");

    mqttPublish(TOPIC LWT_TOPIC, "online", 6, false);

    mqtt_send_object_state(0, 0, "connected");
    std::cout << std::endl;
}

void mqttStart()
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions will_opts    = MQTTClient_willOptions_initializer;
    int rc;
    int ch;

    if((rc = MQTTClient_create(&mqtt_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) !=
       MQTTCLIENT_SUCCESS) {
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
    conn_opts.will->retained  = 0;
    conn_opts.will->topicName = "hasp/plate35/LWT";

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession      = 1;

    conn_opts.username = "hasp";
    conn_opts.password = "hasp";

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

void mqttSetup(){};

char* topicName;
int topicLen;
MQTTClient_message* message;
void mqttLoop()
{

    int rc = MQTTClient_receive(mqtt_client, &topicName, &topicLen, &message, 4);
    if(rc == MQTTCLIENT_SUCCESS && message) msgarrvd(mqtt_client, topicName, topicLen, message);
};

void mqttEvery5Seconds(bool wifiIsConnected){};

#endif // USE_PAHO
#endif // USE_MQTT