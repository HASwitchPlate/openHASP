#if TOUCH_DRIVER == 0x0610

#include "hasp_conf.h"

#include <SPI.h>
#include "Adafruit_STMPE610.h"
#include "ArduinoLog.h"

#include "hasp_drv_stmpe610.h"

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

static Adafruit_STMPE610 touch = Adafruit_STMPE610(TOUCH_CS);

// Read touch points from global variable
HASP_ATTRIBUTE_FAST_MEM bool STMPE610_getXY(int16_t* touchX, int16_t* touchY, uint8_t touchRotation, bool debug)
{
    uint16_t x, y;
    uint8_t z;
    if(!touch.touched()) return false;

    while(!touch.bufferEmpty()) {
        touch.readData(&x, &y, &z);
        if(debug) Log.trace(TAG_DRVR, F("STMPE610: x=%i y=%i z=%i r=%i"), x, y, z, touchRotation);
    }
    touch.writeRegister8(STMPE_INT_STA, 0xFF);
    if(1 == touchRotation) {
#if HX8357D_DRIVER == 1
        y = map(y, TS_MINX, TS_MAXX, 0, TFT_HEIGHT);
        x = map(x, TS_MINY, TS_MAXY, TFT_WIDTH, 0);
#else
        x = map(x, TS_MAXX, TS_MINX, 0, TFT_WIDTH);
        y = map(y, TS_MAXY, TS_MINY, 0, TFT_HEIGHT);
#endif
    } else if(2 == touchRotation) {
#if HX8357D_DRIVER == 1
        x = map(x, TS_MAXX, TS_MINX, TFT_WIDTH, 0);
        y = map(y, TS_MAXY, TS_MINY, 0, TFT_HEIGHT);
#else
        x = map(x, TS_MAXX, TS_MINX, 0, TFT_WIDTH);
        y = map(y, TS_MAXY, TS_MINY, 0, TFT_HEIGHT);
#endif
    } else {
#if HX8357D_DRIVER == 1
        x = map(x, TS_MINX, TS_MAXX, TFT_WIDTH, 0);
        y = map(y, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
#else
        x = map(x, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
        y = map(y, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
#endif
    }

    *touchX = x;
    *touchY = y;
    return true;
}

void STMPE610_init()
{
    LOG_TRACE(TAG_DRVR, F("STMPE610 " D_SERVICE_STARTING));
    if(!touch.begin()) {
        LOG_ERROR(TAG_DRVR, F("STMPE610 " D_SERVICE_START_FAILED));
    } else {
        LOG_INFO(TAG_DRVR, F("STMPE610 " D_SERVICE_STARTED));
    }
}
#endif