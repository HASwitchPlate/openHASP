/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MQTT_H
#define HASP_MQTT_H

#include <stdint.h>
#include "hasplib.h"

typedef enum {
    MQTT_ERR_OK       = 0,
    MQTT_ERR_DISABLED = -1,
    MQTT_ERR_NO_CONN  = -2,
    MQTT_ERR_SUB_FAIL = -3,
    MQTT_ERR_PUB_FAIL = -4,
    MQTT_ERR_UNKNOWN  = -128
} hasp_mqtt_error_t;

void mqttSetup();
IRAM_ATTR void mqttLoop();
void mqttEvery5Seconds(bool wifiIsConnected);
void mqttStart();
void mqttStop();

// int mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload);
int mqtt_send_state(const char* subtopic, const char* payload);
int mqtt_send_discovery(const char* payload, size_t len);
int mqttPublish(const char* topic, const char* payload, size_t len, bool retain);

bool mqttIsConnected();
void mqtt_get_info(JsonDocument& doc);

#if HASP_USE_CONFIG > 0
bool mqttGetConfig(const JsonObject& settings);
#endif
bool mqttSetConfig(const JsonObject& settings);

#ifndef MQTT_PREFIX
#define MQTT_PREFIX "hasp"
#endif

#ifndef MQTT_TOPIC_STATE
#define MQTT_TOPIC_STATE "state"
#endif

#ifndef MQTT_TOPIC_COMMAND
#define MQTT_TOPIC_COMMAND "command"
#endif

#ifndef MQTT_TOPIC_CONFIG
#define MQTT_TOPIC_CONFIG "config"
#endif

#ifndef MQTT_TOPIC_DISCOVERY
#define MQTT_TOPIC_DISCOVERY "discovery"
#endif

#ifndef MQTT_TOPIC_BROADCAST
#define MQTT_TOPIC_BROADCAST "broadcast"
#endif

#ifndef MQTT_TOPIC_SENSORS
#define MQTT_TOPIC_SENSORS "sensors"
#endif

#ifndef MQTT_TOPIC_CUSTOM
#define MQTT_TOPIC_CUSTOM "custom"
#endif

#define MQTT_TOPIC_LWT "LWT"

////////////////////////////////////////////////////////////////////////////////////////////////////
// These defaults may be overwritten with values saved by the web interface

#ifndef MQTT_GROUPNAME
#define MQTT_GROUPNAME "plates";
#endif

#ifndef MQTT_HOSTNAME
#ifndef MQTT_HOST
#define MQTT_HOSTNAME ""
#else
#define MQTT_HOSTNAME MQTT_HOST
#endif
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#ifndef MQTT_USERNAME
#ifndef MQTT_USER
#define MQTT_USERNAME ""
#else
#define MQTT_USERNAME MQTT_USER;
#endif
#endif

#ifndef MQTT_PASSWORD
#ifndef MQTT_PASSW
#define MQTT_PASSWORD ""
#else
#define MQTT_PASSWORD MQTT_PASSW
#endif
#endif

#endif // HASP_MQTT_H