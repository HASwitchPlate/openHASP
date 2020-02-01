#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "TFT_eSPI.h"
#include "ArduinoJson.h"

#include "lvgl.h"

void guiSetup(TFT_eSPI & screen, JsonObject settings);
void guiLoop(void);
void guiStop(void);

void guiCalibrate();

bool guiGetConfig(const JsonObject & settings);

// lv_res_t guiChangeTheme(uint8_t themeid, uint16_t hue, String font, uint8_t fontsize);

#endif