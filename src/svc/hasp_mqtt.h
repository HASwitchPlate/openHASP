/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MQTT_H
#define HASP_MQTT_H

#include "ArduinoJson.h"

void mqttSetup();
void mqttLoop();
void mqttEvery5Seconds(bool wifiIsConnected);
void mqttStart();
void mqttStop();

void mqtt_send_object_state(uint8_t pageid, uint8_t btnid, char * payload);
void mqtt_send_state(const __FlashStringHelper * subtopic, const char * payload);

bool mqttIsConnected();

#if HASP_USE_CONFIG > 0
bool mqttGetConfig(const JsonObject & settings);
bool mqttSetConfig(const JsonObject & settings);
#endif

String mqttGetNodename(void);

#endif