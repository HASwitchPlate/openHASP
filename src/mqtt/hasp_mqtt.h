/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MQTT_H
#define HASP_MQTT_H

#include <stdint.h>
#include "ArduinoJson.h"

#include "hasp_conf.h"

// #if defined(WINDOWS) || defined(POSIX)
// #define __FlashStringHelper char
// #endif

typedef enum {
    MQTT_ERR_OK       = 0,
    MQTT_ERR_DISABLED = -1,
    MQTT_ERR_NO_CONN  = -2,
    MQTT_ERR_SUB_FAIL = -3,
    MQTT_ERR_PUB_FAIL = -4,
    MQTT_ERR_UNKNOWN  = -128
} hasp_mqtt_error_t;

void mqttSetup();
void mqttLoop();
void mqttEvery5Seconds(bool wifiIsConnected);
void mqttStart();
void mqttStop();

int mqtt_send_object_state(uint8_t pageid, uint8_t btnid, const char* payload);
int mqtt_send_state(const char* subtopic, const char* payload);
int mqtt_send_discovery(const char* payload, size_t len);
int mqttPublish(const char* topic, const char* payload, size_t len, bool retain);

bool mqttIsConnected();
void mqtt_get_info(JsonDocument& doc);

#if HASP_USE_CONFIG > 0
bool mqttGetConfig(const JsonObject& settings);
bool mqttSetConfig(const JsonObject& settings);
#endif

// #ifndef WINDOWS
// String mqttGetNodename(void);
// #endif

#endif
