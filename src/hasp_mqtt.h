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

void IRAM_ATTR mqtt_send_state(const __FlashStringHelper * subtopic, const char * payload);
void mqtt_send_input(uint8_t id, const char * payload);
void IRAM_ATTR mqtt_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data);

void mqtt_send_statusupdate(void);
bool IRAM_ATTR mqttIsConnected();

bool mqttGetConfig(const JsonObject & settings);
bool mqttSetConfig(const JsonObject & settings);

String mqttGetNodename(void);

#endif