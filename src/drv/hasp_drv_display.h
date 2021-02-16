/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DRV_DISPLAY_H
#define HASP_DRV_DISPLAY_H

#include "lvgl.h"

// Select Display Driver
#if defined(USE_FSMC)
    #include "fsmc_ili9341.h"
#else
    #include "tft_espi_drv.h"
#endif

void drv_display_init(lv_disp_drv_t * disp_drv, uint8_t rotation, bool invert_display);
void drv_display_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);

#endif