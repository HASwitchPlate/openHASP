#ifndef HASP_TFT_H
#define HASP_TFT_H

#include "TFT_eSPI.h"
#include "ArduinoJson.h"

void tftSetup(TFT_eSPI & screen, JsonObject settings);
void tftLoop(void);
void tftStop(void);
void tftShowConfig(TFT_eSPI & tft);

#endif