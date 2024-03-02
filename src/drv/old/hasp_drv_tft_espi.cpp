/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/*********************
 *      INCLUDES
 *********************/
#include "hasp_conf.h"

#if defined(TOUCH_CS) && defined(USER_SETUP_LAODED)

#include "dev/device.h"
#include "drv/tft/tft_driver.h"

#include "hasp_drv_tft_espi.h"
#include "ArduinoLog.h"
#include "hasp_debug.h"
#include "hasp_macro.h"

void tft_espi_calibrate(uint16_t* calData)
{
    haspTft.tft.fillScreen(TFT_BLACK);
    haspTft.tft.setCursor(20, 0);
    haspTft.tft.setTextFont(1);
    haspTft.tft.setTextSize(1);
    haspTft.tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // tft.println(PSTR("Touch corners as indicated"));

    haspTft.tft.setTextFont(1);
    delay(500);
    haspTft.tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    haspTft.tft.setTouch(calData);
}

void tft_espi_set_touch(uint16_t* calData)
{
   haspTft.tft.setTouch(calData);
}

bool tft_espi_get_touch(int16_t* touchX, int16_t* touchY, uint16_t threshold)
{
    return haspTft.tft.getTouch((uint16_t*)touchX, (uint16_t*)touchY, threshold);
}
#endif