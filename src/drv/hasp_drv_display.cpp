/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_drv_display.h"
#include "tft_espi_drv.h"
//#include "fsmc_ili9341.h"

void drv_display_init(lv_disp_drv_t* disp_drv, uint8_t rotation, bool invert_display)
{
    /* TFT init */
#if defined(USE_FSMC)
    fsmc_ili9341_init(rotation, invert_display);
    disp_drv->flush_cb = fsmc_ili9341_flush; // Normal callback when flushing
    // xpt2046_init(rotation);
#else
    tft_espi_init(rotation, invert_display);
    disp_drv->flush_cb = tft_espi_flush; // Normal callback when flushing
#endif
}

/* Callback used for screenshots only: */

/* indirect callback to flush screenshot data to the screen */
void drv_display_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
#if defined(USE_FSMC)
    fsmc_ili9341_flush(disp, area, color_p);
#else
    tft_espi_flush(disp, area, color_p);
#endif
}