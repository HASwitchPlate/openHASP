/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TFT_H
#define HASP_TFT_H

#ifndef USE_FSMC

#include "TFT_eSPI.h"

void tftSetup(TFT_eSPI & screen);
void IRAM_ATTR tftLoop(void);
void tftStop(void);

void tftShowConfig(TFT_eSPI & tft);

#endif

#endif