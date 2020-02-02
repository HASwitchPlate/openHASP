#ifndef HASP_GUI_H
#define HASP_GUI_H

#include "TFT_eSPI.h"
#include "ArduinoJson.h"

#include "lvgl.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>
#endif

void guiSetup(TFT_eSPI & screen, JsonObject settings);
void guiLoop(void);
void guiStop(void);

void guiCalibrate();
void guiTakeScreenshot(const char * pFileName);
void guiTakeScreenshot(WiFiClient client);

bool guiGetConfig(const JsonObject & settings);
bool guiSetConfig(const JsonObject & settings);

// lv_res_t guiChangeTheme(uint8_t themeid, uint16_t hue, String font, uint8_t fontsize);

#endif