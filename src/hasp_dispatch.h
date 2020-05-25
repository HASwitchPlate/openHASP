#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include "ArduinoJson.h"

void dispatchSetup(void);
void dispatchLoop(void);

void dispatchAttribute(String strTopic, const char * strPayload);
void dispatchCommand(String cmnd);
void dispatchConfig(const char * topic, const char * payload);

void dispatchJson(char * strPayload);
void dispatchJsonl(char * strPayload);
void dispatchJsonl(Stream & stream);

void dispatchPage(String strPageid);
void dispatchClearPage(String strPageid);
void dispatchDim(String strDimLevel);
void dispatchBacklight(String strPayload);

void dispatchWebUpdate(const char * espOtaUrl);
void dispatchIdle(const char * state);
void dispatchReboot(bool saveConfig);
void dispatchStatusUpdate(void);

void dispatch_button(uint8_t id, const char * event);

void dispatch_send_object_event(uint8_t pageid, uint8_t objid, uint8_t eventid);
void dispatch_send_group_event(uint8_t groupid, uint8_t eventid, bool update_hasp);
bool dispatch_get_event_state(uint8_t eventid);

void IRAM_ATTR dispatch_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute,
                                               const char * data);

#endif