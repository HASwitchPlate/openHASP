#ifndef HASP_OTA_H
#define HASP_OTA_H

#include "ArduinoJson.h"

void otaSetup(JsonObject settings);
void otaLoop(void);

#endif