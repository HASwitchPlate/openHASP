/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "ArduinoJson.h"
#include "lvgl.h"

struct gui_conf_t
{
    bool show_pointer;
    int8_t backlight_pin;
    uint8_t rotation;
    uint8_t invert_display;
    uint16_t cal_data[5];
};

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
// void guiSetDim(int8_t level);
// int8_t guiGetDim();
// void guiSetBacklight(bool lighton);
// bool guiGetBacklight();

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool guiGetConfig(const JsonObject & settings);
bool guiSetConfig(const JsonObject & settings);
#endif

#endif