/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if TOUCH_DRIVER == 6336

    #include "hasp_debug.h" // for TAG_DRVR

    #ifndef HASP_DRV_FT6336U_H
        #define HASP_DRV_FT6336U_H

bool IRAM_ATTR FT6336U_getXY(uint16_t * touchX, uint16_t * touchY, bool debug);
void FT6336U_init();

    #endif
#endif