#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include "ArduinoJson.h"

void dispatchSetup(void);
void dispatchLoop(void);

void dispatchAttribute(String & strTopic, const char * strPayload);
void dispatchCommand(String cmnd);
void dispatchJson(char * strPayload);
void dispatchJsonl(char * strPayload);

void dispatchPage(String strPageid);
void dispatchClearPage(String strPageid);
void dispatchDim(String strDimLevel);
void dispatchBacklight(String strPayload);

void dispatchIdle(const __FlashStringHelper * state);
void dispatchReboot(bool saveConfig);
void dispatchStatusUpdate(void);

void dispatchButton(uint8_t id, char * event);

#endif