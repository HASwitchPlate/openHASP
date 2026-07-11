/* MIT License - Copyright (c) 2019-2026 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(ARDUINO) && (TOUCH_DRIVER == 0x0911) && !defined(HASP_USE_LGFX_TOUCH)
#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"
#include "touch_driver_gt911.h"

#include <Wire.h>
#include "Goodix.h"

#include "touch_driver.h" // base class
#include "touch_helper.h" // i2c scanner

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

static Goodix touch = Goodix();

// Global callback handler required by the Goodix library pipeline
IRAM_ATTR void GT911_setXY(int8_t contacts, GTPoint* points)
{
    // NOOP - Handled directly inside the polling strategy via touch.readInput
}

namespace dev {

IRAM_ATTR bool TouchGt911::read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    // Zero branches inside execution loop
    return _read_strategy(this, indev_driver, data);
}

// STRATEGY 1: Device is present, initialized and operational
IRAM_ATTR bool TouchGt911::read_active(TouchGt911* instance, lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    static GTPoint points[5];
    data->state = LV_INDEV_STATE_REL;

    // touch is static global, no instance-> pointer invocation needed
    if(touch.readInput((uint8_t*)points) > 0) {
        data->point.x = map(points[0].x, 0, instance->xResolution - 1, 0, instance->tftWidth - 1);
        data->point.y = map(points[0].y, 0, instance->yResolution - 1, 0, instance->tftHeight - 1);

        data->state = LV_INDEV_STATE_PR;
        
        if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state();
        hasp_set_sleep_offset(0);
    }
    return false;
}

// STRATEGY 2: Hardware failure or unsupported configuration fallback
IRAM_ATTR bool TouchGt911::read_failed(TouchGt911* instance, lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    data->state = LV_INDEV_STATE_REL;
    return false;
}

void TouchGt911::init(int w, int h)
{
    Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
    touch.setHandler(GT911_setXY);
    GTInfo* info = nullptr;

    tftWidth  = w;
    tftHeight = h;

#if defined(TOUCH_WIDTH) && defined(TOUCH_HEIGHT)
    xResolution = TOUCH_WIDTH;
    yResolution = TOUCH_HEIGHT;
#else
    xResolution = 0;
    yResolution = 0;
#endif

#ifdef I2C_TOUCH_ADDRESS
    uint8_t i2c_addresses[] = {I2C_TOUCH_ADDRESS, GOODIX_I2C_ADDR_28, GOODIX_I2C_ADDR_BA};
#else
    uint8_t i2c_addresses[] = {GOODIX_I2C_ADDR_28, GOODIX_I2C_ADDR_BA};
#endif

    // Probe addresses sequentially to check for chip existence
    uint8_t len = sizeof(i2c_addresses) / sizeof(i2c_addresses[0]);
    for(uint8_t i = 0; i < len; i++) {
        if(touch.begin(TOUCH_IRQ, TOUCH_RST, i2c_addresses[i])) {
            info = touch.readInfo();
            if(info && info->xResolution > 0 && info->yResolution > 0) {
                break;
            }
        }
    }

    if(info && info->xResolution > 0 && info->yResolution > 0) {
        xResolution = info->xResolution;
        yResolution = info->yResolution;
        _read_strategy = &TouchGt911::read_active;
        LOG_INFO(TAG_DRVR, "GT911 %s (%dx%d)", D_SERVICE_STARTED, info->xResolution, info->yResolution);
    } else if (xResolution > 0 && yResolution > 0) {
        _read_strategy = &TouchGt911::read_active; // Use pre-configured macro fallback targets
        LOG_WARNING(TAG_DRVR, "GT911 read failed, using macro fallbacks (%dx%d)", xResolution, yResolution);
    } else {
        _read_strategy = &TouchGt911::read_failed;
        LOG_WARNING(TAG_DRVR, "GT911 %s", D_SERVICE_START_FAILED);
    }

    touch_scan(Wire);
}

} // namespace dev

dev::TouchGt911 haspTouch;

#endif // ARDUINO