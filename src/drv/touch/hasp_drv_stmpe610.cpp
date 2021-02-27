#if TOUCH_DRIVER == 610

    #include <SPI.h>
    #include "Adafruit_STMPE610.h"
    #include "ArduinoLog.h"

    #include "hasp_drv_stmpe610.h"

// This is calibration data for the raw touch data to the screen coordinates
    #define TS_MINX 3800
    #define TS_MAXX 100
    #define TS_MINY 100
    #define TS_MAXY 3750

static Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS);

// Read touch points from global variable
bool IRAM_ATTR STMPE610_getXY(int16_t * touchX, int16_t * touchY, uint8_t touchRotation, bool debug)
{
    uint16_t x, y;
    uint8_t z;
    if(! touch.touched()) return false;

    while (! touch.bufferEmpty()) {
        touch.readData(&x, &y, &z);
        if(debug) Log.trace(TAG_DRVR, F("STMPE610: x=%i y=%i z=%i r=%i"), x, y, z, touchRotation);
    }
    touch.writeRegister8(STMPE_INT_STA, 0xFF);
    if (1 == touchRotation) {
        y = map(y, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
        x = map(x, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
    } else if (2 == touchRotation) {
        x = map(x, TS_MAXX, TS_MINX, 0, TFT_WIDTH);
        y = map(y, TS_MAXY, TS_MINY, 0, TFT_HEIGHT);
    } else {
        x = map(x, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
        y = map(y, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
    }

    *touchX = x;
    *touchY = y;
    return true;

}

void STMPE610_init()
{
    if (! touch.begin()) {
        Log.trace(TAG_DRVR, F("STMPE610 not found!"));
    } else {
        Log.trace(TAG_DRVR, F("STMPE610 touch driver started"));
    }
}
#endif