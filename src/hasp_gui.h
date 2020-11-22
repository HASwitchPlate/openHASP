/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "ArduinoJson.h"
#include "lvgl.h"

#define HASP_SLEEP_OFF 0
#define HASP_SLEEP_SHORT 1
#define HASP_SLEEP_LONG 2

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
bool guiCheckSleep();

/* ===== Read/Write Configuration ===== */
bool guiGetConfig(const JsonObject & settings);
bool guiSetConfig(const JsonObject & settings);

// lv_res_t guiChangeTheme(uint8_t themeid, uint16_t hue, String font, uint8_t fontsize);

#endif