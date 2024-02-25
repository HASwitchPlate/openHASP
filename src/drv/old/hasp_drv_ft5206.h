/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DRV_FT5206_H
#define HASP_DRV_FT5206_H

#if TOUCH_DRIVER == 0x5206

#define FT5206_address 0x38

#include "hasp_debug.h" // for TAG_DRVR

bool FT5206_getXY(int16_t* touchX, int16_t* touchY, bool debug);
void FT5206_init();

#endif
#endif