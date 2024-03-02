/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GSL1680_TOUCH_DRIVER_H
#define HASP_GSL1680_TOUCH_DRIVER_H

#ifdef ARDUINO
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include <Wire.h>
#include "GSL2038.h"

#include "touch_driver.h" // base class
#include "touch_helper.h" // i2c scanner

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

GSL2038 TS = GSL2038(1, 1); // error & info

// Interrupt handling
volatile uint8_t gsl16380IRQ = 0;

// Store touch points into global variable
static void ICACHE_RAM_ATTR ontouch_irq()
{
    noInterrupts();
    gsl16380IRQ = 1;
    interrupts();
}

namespace dev {

class TouchGsl1680 : public BaseTouch {

  public:
    IRAM_ATTR bool read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
    {
        noInterrupts();
        uint8_t irq = gsl16380IRQ;
        gsl16380IRQ = 0;
        interrupts();

        if(irq && TS.dataread() > 0) {

            if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // update Idle

            data->point.x = map(TS.readFingerX(0), 0, 1024, 0, TFT_WIDTH);
            data->point.y = map(TS.readFingerY(0), 45, 660, 0, TFT_HEIGHT);

            if(data->point.x < 0)
                data->point.x = 0;
            else if(data->point.x > TFT_WIDTH)
                data->point.x = TFT_WIDTH;

            if(data->point.y < 0)
                data->point.y = 0;
            else if(data->point.y > TFT_HEIGHT)
                data->point.y = TFT_HEIGHT;

            data->state = LV_INDEV_STATE_PR;
            hasp_set_sleep_offset(0); // Reset the offset

        } else {
            data->state = LV_INDEV_STATE_REL;
        }

        /*Return `false` because we are not buffering and no more data to read*/
        return false;
    }

    void init(int w, int h)
    {
        Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
        // delay(300); // already happens in touch.begin()
        touch_scan(Wire);

        // Startup sequence CONTROLLER part
        TS.begin(TOUCH_RST, TOUCH_IRQ);

        // Setup Interrupt handler
        pinMode(TOUCH_IRQ, INPUT);
        attachInterrupt(TOUCH_IRQ, ontouch_irq, RISING);

        // if(touch.begin(INT_PIN, RST_PIN)) {
        LOG_INFO(TAG_DRVR, F("GSL1680 " D_SERVICE_STARTED));
        // } else {
        //     LOG_WARNING(TAG_DRVR, F("GSL1680 " D_SERVICE_START_FAILED));
        // }
    }
};

} // namespace dev

using dev::TouchGsl1680;
extern dev::TouchGsl1680 haspTouch;

#endif // ARDUINO

#endif // HASP_GSL1680_TOUCH_DRIVER_H
