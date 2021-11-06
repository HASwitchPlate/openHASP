/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "hasplib.h"

struct gui_conf_t
{
    bool show_pointer;
    int8_t backlight_pin;
    uint8_t rotation;
    uint8_t invert_display;
    uint16_t cal_data[5];
};

/* ===== Default Event Processors ===== */
void guiTftInit(void);
void guiSetup(void);
IRAM_ATTR void guiLoop(void);
void guiEverySecond(void);
void guiStart(void);
void guiStop(void);
void gui_hide_pointer(bool hidden);

/* ===== Special Event Processors ===== */
void guiCalibrate(void);
void guiTakeScreenshot(const char* pFileName); // to file
void guiTakeScreenshot(void);                  // webclient

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool guiGetConfig(const JsonObject& settings);
bool guiSetConfig(const JsonObject& settings);
#endif

#endif