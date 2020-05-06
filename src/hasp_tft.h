#ifndef HASP_TFT_H
#define HASP_TFT_H

#include "TFT_eSPI.h"

void tftSetup(TFT_eSPI & screen);
void tftLoop(void);
void tftStop(void);

void tftShowConfig(TFT_eSPI & tft);
String tftDriverName();

#endif