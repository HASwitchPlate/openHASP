#ifndef HASP_GUI_H
#define HASP_GUI_H

//#include "TFT_eSPI.h"
#include "ArduinoJson.h"

#include "lvgl.h"

#define HASP_SLEEP_OFF 0
#define HASP_SLEEP_SHORT 1
#define HASP_SLEEP_LONG 2

// #if defined(ARDUINO_ARCH_ESP8266)
// #include <ESP8266WebServer.h>
// void guiTakeScreenshot(ESP8266WebServer & client);
// #endif

// #if defined(ARDUINO_ARCH_ESP32)
// #include <WebServer.h>
// void guiTakeScreenshot(WebServer & client);
// #endif // ESP32

void guiTakeScreenshot();

void guiSetup();
void guiStart(void);
void guiLoop(void);
void guiStop(void);

void guiCalibrate();
void guiTakeScreenshot(const char * pFileName);

void guiSetDim(int8_t level);
int8_t guiGetDim(void);
void guiSetBacklight(bool lighton);
bool guiGetBacklight();
bool guiCheckSleep();

bool guiGetConfig(const JsonObject & settings);
bool guiSetConfig(const JsonObject & settings);

// lv_res_t guiChangeTheme(uint8_t themeid, uint16_t hue, String font, uint8_t fontsize);

#endif