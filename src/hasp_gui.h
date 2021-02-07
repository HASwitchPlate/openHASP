/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "ArduinoJson.h"
#include "lvgl.h"

/* ===== Default Event Processors ===== */
void guiSetup();
void guiLoop(void);
void guiEverySecond(void);
void guiStart(void);
void guiStop(void);

/* ===== Special Event Processors ===== */
void guiCalibrate();
void guiTakeScreenshot(const char * pFileName); // to file
void guiTakeScreenshot();                       // webclient

/* ===== Getter and Setter Functions ===== */
void guiSetDim(int8_t level);
int8_t guiGetDim(void);
void guiSetBacklight(bool lighton);
bool guiGetBacklight();

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool guiGetConfig(const JsonObject & settings);
bool guiSetConfig(const JsonObject & settings);
#endif

#endif