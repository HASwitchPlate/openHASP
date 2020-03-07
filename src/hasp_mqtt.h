#ifndef HASP_MQTT_H
#define HASP_MQTT_H

#include "ArduinoJson.h"

void mqttSetup(const JsonObject & settings);
void mqttLoop(bool wifiIsConnected);
void mqttStop();
void mqttReconnect();

void mqttSendState(const char * subtopic, const char * payload);
void mqttSendNewEvent(uint8_t pageid, uint8_t btnid, char * value); // int32_t val)
void mqttSendNewValue(uint8_t pageid, uint8_t btnid, int32_t val);
void mqttSendNewValue(uint8_t pageid, uint8_t btnid, String txt);
void mqttHandlePage(String strPageid);
void mqttStatusUpdate(void);
bool mqttIsConnected(void);

bool mqttGetConfig(const JsonObject & settings);
bool mqttSetConfig(const JsonObject & settings);

String mqttGetNodename(void);

#endif