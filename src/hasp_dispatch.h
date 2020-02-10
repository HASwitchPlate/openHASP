#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include <Arduino.h>

void dispatchSetup(void);
void dispatchLoop(void);

void dispatchAttribute(String & strTopic, String & strPayload);
void dispatchCommand(String cmnd);
void dispatchJson(String & strPayload);

void dispatchPage(String & strPageid);
void dispatchDim(String & strDimLevel);

void dispatchIdle(const __FlashStringHelper * state);
void dispatchReboot(bool saveConfig);

#endif