/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "hasplib.h"

struct bmp_header_t
{
    uint32_t bfSize;
    uint32_t bfReserved;
    uint32_t bfOffBits;

    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

    uint32_t bdMask[3]; // RGB
};

struct gui_conf_t
{
    bool show_pointer;
    int8_t backlight_pin;
    uint8_t rotation;
    uint8_t invert_display;
#if defined(USER_SETUP_LOADED)
    uint16_t cal_data[5];
#else
    uint16_t cal_data[8];
#endif
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
bool guiScreenshotIsDirty();

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool guiGetConfig(const JsonObject& settings);
bool guiSetConfig(const JsonObject& settings);
#endif

#endif