/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MQTT_H
#define HASP_MQTT_H

#include <stdint.h>

#include "hasp_conf.h"

#ifdef WINDOWS
#define __FlashStringHelper char
#endif

void mqttSetup();
void mqttLoop();
void mqttEvery5Seconds(bool wifiIsConnected);
void mqttStart();
void mqttStop();

void mqtt_send_object_state(uint8_t pageid, uint8_t btnid, char* payload);
void mqtt_send_state(const __FlashStringHelper* subtopic, const char* payload);

bool mqttPublish(const char* topic, const char* payload, size_t len, bool retain);

bool mqttIsConnected();

#if HASP_USE_CONFIG > 0
bool mqttGetConfig(const JsonObject& settings);
bool mqttSetConfig(const JsonObject& settings);
#endif

// #ifndef WINDOWS
// String mqttGetNodename(void);
// #endif

#endif