/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Standalone XPT2046 resistive touch on its own SPI bus, for RGB-parallel
   boards driven by ArduinoGFX (which has no built-in touch companion). */

#if defined(ARDUINO) && (TOUCH_DRIVER == 0x2046) && !defined(USER_SETUP_LOADED) && !defined(LGFX_USE_V1)
#include <Arduino.h>
#include <SPI.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"

#include "touch_driver_xpt2046.h"
#include "touch_driver.h" // base class

#include "../../hasp/hasp.h" // for hasp_sleep_state
extern uint8_t hasp_sleep_state;

// ---- Pins (from the board's user_setup .ini) --------------------------------
#ifndef TOUCH_SCLK
#define TOUCH_SCLK 12
#endif
#ifndef TOUCH_MISO
#define TOUCH_MISO 13
#endif
#ifndef TOUCH_MOSI
#define TOUCH_MOSI 11
#endif
#ifndef TOUCH_CS
#define TOUCH_CS 38
#endif
#ifndef TOUCH_IRQ
#define TOUCH_IRQ 18
#endif

// ---- Calibration (12-bit raw ADC range -> pixels). Defaults proven on the
//      Sunton 4827S043R via ESPHome. Override per-board with build flags. ------
#ifndef TOUCH_XPT2046_XMIN
#define TOUCH_XPT2046_XMIN 280
#endif
#ifndef TOUCH_XPT2046_XMAX
#define TOUCH_XPT2046_XMAX 3860
#endif
#ifndef TOUCH_XPT2046_YMIN
#define TOUCH_XPT2046_YMIN 340
#endif
#ifndef TOUCH_XPT2046_YMAX
#define TOUCH_XPT2046_YMAX 3860
#endif
// Orientation tweaks (default: none -> matches ESPHome swap_xy/mirror = false)
#ifndef TOUCH_XPT2046_SWAP_XY
#define TOUCH_XPT2046_SWAP_XY 0
#endif
#ifndef TOUCH_XPT2046_MIRROR_X
#define TOUCH_XPT2046_MIRROR_X 0
#endif
#ifndef TOUCH_XPT2046_MIRROR_Y
#define TOUCH_XPT2046_MIRROR_Y 0
#endif

// XPT2046 control bytes (differential mode, 12-bit). Axis assignment matches
// ESPHome's xpt2046 component: X measured with 0x90, Y with 0xD0.
#define XPT2046_CMD_X 0x90
#define XPT2046_CMD_Y 0xD0
#define XPT2046_SAMPLES 5

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2)
static SPIClass touchSpi(FSPI);
#else
static SPIClass touchSpi(VSPI);
#endif
static const SPISettings touchSpiSettings(2000000, MSBFIRST, SPI_MODE0);

// Read one 12-bit conversion for the given control byte.
static uint16_t xpt2046_read_raw(uint8_t cmd)
{
    touchSpi.transfer(cmd);
    uint8_t hi   = touchSpi.transfer(0x00);
    uint8_t lo   = touchSpi.transfer(0x00);
    uint16_t val = ((uint16_t)hi << 8 | lo) >> 3; // 12-bit result
    return val & 0x0FFF;
}

// Median-of-samples for one axis to reject the noisy first/last conversions.
static uint16_t xpt2046_sample(uint8_t cmd)
{
    uint16_t buf[XPT2046_SAMPLES];
    for(uint8_t i = 0; i < XPT2046_SAMPLES; i++) buf[i] = xpt2046_read_raw(cmd);
    // simple insertion sort (tiny N) then take the middle sample
    for(uint8_t i = 1; i < XPT2046_SAMPLES; i++) {
        uint16_t k = buf[i];
        int8_t j   = i - 1;
        while(j >= 0 && buf[j] > k) {
            buf[j + 1] = buf[j];
            j--;
        }
        buf[j + 1] = k;
    }
    return buf[XPT2046_SAMPLES / 2];
}

namespace dev {

void TouchXpt2046::init(int w, int h)
{
    _width  = w;
    _height = h;

    pinMode(TOUCH_IRQ, INPUT);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    // Dedicated SPI bus; ss = -1 so we drive CS manually across the X+Y burst.
    touchSpi.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI, -1);

    LOG_INFO(TAG_DRVR, "XPT2046 %s (sclk=%d miso=%d mosi=%d cs=%d irq=%d)", D_SERVICE_STARTED, TOUCH_SCLK,
             TOUCH_MISO, TOUCH_MOSI, TOUCH_CS, TOUCH_IRQ);
}

void TouchXpt2046::set_rotation(uint8_t rotation)
{
    _rotation = rotation;
}

bool TouchXpt2046::is_driver_pin(uint8_t pin)
{
    return pin == TOUCH_SCLK || pin == TOUCH_MISO || pin == TOUCH_MOSI || pin == TOUCH_CS || pin == TOUCH_IRQ;
}

IRAM_ATTR bool TouchXpt2046::read(lv_indev_drv_t* indev_driver, lv_indev_data_t* data)
{
    // PENIRQ is LOW while the panel is pressed.
    if(digitalRead(TOUCH_IRQ) != LOW) {
        data->state = LV_INDEV_STATE_REL;
        return false;
    }

    touchSpi.beginTransaction(touchSpiSettings);
    digitalWrite(TOUCH_CS, LOW);
    uint16_t rawX = xpt2046_sample(XPT2046_CMD_X);
    uint16_t rawY = xpt2046_sample(XPT2046_CMD_Y);
    // Re-check IRQ: a release during the burst yields garbage.
    bool still_down = (digitalRead(TOUCH_IRQ) == LOW);
    digitalWrite(TOUCH_CS, HIGH);
    touchSpi.endTransaction();

    if(!still_down) {
        data->state = LV_INDEV_STATE_REL;
        return false;
    }

    // Select which raw ADC axis feeds each screen axis. On RGB/ArduinoGFX boards
    // the panel is often oriented so screen-X is driven by the touch Y wire.
    // XMIN/XMAX always calibrate whatever ADC axis feeds screen-X (same for Y).
#if TOUCH_XPT2046_SWAP_XY
    uint16_t adcX = rawY;
    uint16_t adcY = rawX;
#else
    uint16_t adcX = rawX;
    uint16_t adcY = rawY;
#endif
    if(adcX < TOUCH_XPT2046_XMIN) adcX = TOUCH_XPT2046_XMIN;
    if(adcX > TOUCH_XPT2046_XMAX) adcX = TOUCH_XPT2046_XMAX;
    if(adcY < TOUCH_XPT2046_YMIN) adcY = TOUCH_XPT2046_YMIN;
    if(adcY > TOUCH_XPT2046_YMAX) adcY = TOUCH_XPT2046_YMAX;

    int32_t x = map(adcX, TOUCH_XPT2046_XMIN, TOUCH_XPT2046_XMAX, 0, _width - 1);
    int32_t y = map(adcY, TOUCH_XPT2046_YMIN, TOUCH_XPT2046_YMAX, 0, _height - 1);

#if TOUCH_XPT2046_MIRROR_X
    x = _width - 1 - x;
#endif
#if TOUCH_XPT2046_MIRROR_Y
    y = _height - 1 - y;
#endif

    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
    data->state   = LV_INDEV_STATE_PR;

    if(hasp_sleep_state != HASP_SLEEP_OFF) hasp_update_sleep_state(); // wake on touch
    hasp_set_sleep_offset(0);

    LOG_VERBOSE(TAG_DRVR, "XPT2046 raw=%u,%u -> %d,%d", rawX, rawY, (int)x, (int)y);
    return false;
}

} // namespace dev

dev::TouchXpt2046 haspTouch;

#endif // ARDUINO && TOUCH_DRIVER == 0x2046 && !TFT_eSPI && !LovyanGFX
