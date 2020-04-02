#ifndef HASP_MQTT_H
#define HASP_MQTT_H

#include "ArduinoJson.h"

void mqttSetup(const JsonObject & settings);
void mqttLoop();
void mqttEvery5Seconds(bool wifiIsConnected);
void mqttStop();
void mqttReconnect();

void IRAM_ATTR mqtt_send_state(const __FlashStringHelper * subtopic, const char * payload);
void IRAM_ATTR mqtt_send_input(uint8_t id, const char * payload);

// attributes
void IRAM_ATTR mqtt_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data);
/*void IRAM_ATTR mqtt_send_attribute_txt(uint8_t pageid, uint8_t btnid, const char * txt);
void IRAM_ATTR mqtt_send_attribute_val(uint8_t pageid, uint8_t btnid, int32_t val);
void IRAM_ATTR mqtt_send_attribute_event(uint8_t pageid, uint8_t btnid, const char * event);*/

void mqtt_send_statusupdate(void);
bool IRAM_ATTR mqttIsConnected(void);

bool mqttGetConfig(const JsonObject & settings);
bool mqttSetConfig(const JsonObject & settings);

String mqttGetNodename(void);

#endif