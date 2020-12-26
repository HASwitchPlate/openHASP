#include "hasp_drv_display.h"

void drv_display_init(uint8_t rotation)
{
    /* TFT init */
#if defined(USE_FSMC)
    fsmc_ili9341_init(rotation);
    // xpt2046_init(rotation);
#else
    tft_espi_init(rotation);
#endif
}
