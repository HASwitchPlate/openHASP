#ifndef HASP_SPIFFS_H
#define HASP_SPIFFS_H

#include <Arduino.h>

void spiffsSetup(void);

void spiffsList();
void spiffsInfo();
String spiffsFormatBytes(size_t bytes);

#endif