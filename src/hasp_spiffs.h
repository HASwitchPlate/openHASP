#ifndef HASP_SPIFFS_H
#define HASP_SPIFFS_H

#include <Arduino.h>

void spiffsSetup(void);
void spiffsLoop(void);

void spiffsList();
String spiffsFormatBytes(size_t bytes);

#endif