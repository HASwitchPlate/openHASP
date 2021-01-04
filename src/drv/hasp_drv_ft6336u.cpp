#if TOUCH_DRIVER == 6336

    #include <Wire.h>
    #include "FT6336U.h"
    #include "ArduinoLog.h"

    #include "hasp_drv_ft6336u.h"

    #define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number

static FT6336U ft6336u(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_IRQ);

// Read touch points
bool IRAM_ATTR FT6336U_getXY(int16_t * touchX, int16_t * touchY, bool debug)
{
    FT6336U_TouchPointType tp = ft6336u.scan();

    if(debug) {
        char tempString[128];
        sprintf(tempString, "FT6336U TD Count %d / TD1 (%d, %4d, %4d) / TD2 (%d, %4d, %4d)\r", tp.touch_count,
                tp.tp[0].status, tp.tp[0].x, tp.tp[0].y, tp.tp[1].status, tp.tp[1].x, tp.tp[1].y);
        Serial.print(tempString);
    }

    if(tp.touch_count != 1) return false;

    int i = tp.tp[0].status == TouchStatusEnum::touch ? 0 : 1;

    *touchX = tp.tp[i].x;
    *touchY = tp.tp[i].y;
    return true;
}

void FT6336U_init()
{
    ft6336u.begin();
    Log.trace(TAG_DRVR, F("FT6336U touch driver started"));
}
#endif