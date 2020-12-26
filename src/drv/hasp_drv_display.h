/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DRV_DISPLAY_H
#define HASP_DRV_DISPLAY_H

// Select Display Driver
#if defined(USE_FSMC)
    #include "fsmc_ili9341.h"
#else
    #include "tft_espi_drv.h"
#endif

    void drv_display_init(uint8_t rotation);

#endif