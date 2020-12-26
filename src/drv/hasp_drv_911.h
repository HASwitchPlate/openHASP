/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if TOUCH_DRIVER == 911

    #include "hasp_debug.h" // for TAG_DRVR

    #ifndef HASP_DRV_911_H
        #define HASP_DRV_911_H

bool IRAM_ATTR GT911_getXY(uint16_t * touchX, uint16_t * touchY, bool debug);
void GT911_init();
void IRAM_ATTR GT911_loop();

    #endif
#endif