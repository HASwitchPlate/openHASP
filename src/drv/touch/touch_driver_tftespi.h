/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TFTESPI_TOUCH_DRIVER_H
#define HASP_TFTESPI_TOUCH_DRIVER_H

#ifdef ARDUINO && defined(USER_SETUP_LOADED)
#include <Arduino.h>

#include "touch_driver.h" // base class
#include "dev/device.h"   // for haspTft
#include "drv/tft/tft_driver.h"

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

// IRAM_ATTR bool touch_read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
// {
//     if(haspTft.tft.getTouch((uint16_t*)&data->point.x, (uint16_t*)&data->point.y, 300)) {
//         if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle
//         data->state = LV_INDEV_STATE_PR;

//     } else {
//         data->state = LV_INDEV_STATE_REL;
//     }

//     /*Return `false` because we are not buffering and no more data to read*/
//     return false;
// }

namespace dev {

class TouchTftEspi : public BaseTouch {

  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
#if TOUCH_DRIVER == 0x2046 && defined(TOUCH_CS)
        if(haspTft.tft.getTouch((uint16_t*)&data->point.x, (uint16_t*)&data->point.y, 300)) {
            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle
            data->state = LV_INDEV_STATE_PR;
            hasp_set_sleep_offset(0); // Reset the offset
            return false;
        }
#endif
        data->state = LV_INDEV_STATE_REL;

        /*Return `false` because we are not buffering and no more data to read*/
        return false;
    }

    void set_calibration(uint16_t* calData)
    {
#if TOUCH_DRIVER == 0x2046 && defined(TOUCH_CS)
        haspTft.tft.setTouch(calData);
#endif
    }

    void calibrate(uint16_t* calData)
    {
#if TOUCH_DRIVER == 0x2046 && defined(TOUCH_CS)
        haspTft.tft.fillScreen(TFT_BLACK);
        haspTft.tft.setCursor(20, 0);
        haspTft.tft.setTextFont(1);
        haspTft.tft.setTextSize(1);
        haspTft.tft.setTextColor(TFT_WHITE, TFT_BLACK);

        // tft.println(PSTR("Touch corners as indicated"));

        haspTft.tft.setTextFont(1);
        delay(500);
        haspTft.tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
        set_calibration(calData);
        delay(500);
#endif
    }
};

} // namespace dev

using dev::TouchTftEspi;
dev::TouchTftEspi haspTouch;

#endif // ARDUINO

#endif // HASP_TFTESPI_TOUCH_DRIVER_H