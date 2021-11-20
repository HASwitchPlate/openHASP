/*
#if TOUCH_DRIVER == 0x091100

    #include <Wire.h>
    #include "Goodix.h"
    #include "ArduinoLog.h"

    #include "hasp_drv_gt911.h"

    #define INT_PIN (TOUCH_IRQ)
    #define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number

static Goodix touch = Goodix();
static int8_t GT911_num_touches;
static GTPoint * GT911_points;

// Store touch points into global variable
void GT911_setXY(int8_t contacts, GTPoint * points)
{
    GT911_num_touches = contacts;
    GT911_points      = points;

    // LOG_VERBOSE(TAG_GUI, F("Contacts: %d"), contacts);
    // for(int i = 0; i < contacts; i++) {
    //     LOG_VERBOSE(TAG_GUI, F("C%d: #%d %d,%d s:%d"), i, points[i].trackId, points[i].x, points[i].y, points[i].area);
    //     yield();
    // }
}

// Read touch points from global variable
bool GT911_getXY(int16_t * touchX, int16_t * touchY, bool debug)
{
    static GTPoint points[5];
    int16_t contacts = touch.readInput((uint8_t *)&points);
    if(contacts <= 0) return false;

    if(debug) {
        Serial.print(contacts);
        Serial.print(" : ");
        Serial.print(points[0].x);
        Serial.print(" x ");
        Serial.println(points[0].y);
    }

    *touchX = points[0].x;
    *touchY = points[0].y;
    return true;

    // ALTERNATE REGISTER READ METHOD
    // static uint8_t touchBuffer[6];

    // uint16_t first = 0x814E; // 8150
    // uint16_t last  = 0x8153;
    // uint16_t len   = first - last + 1;
    // uint8_t res = touch.read(first, touchBuffer, len);

    // if(res != GOODIX_OK || touchBuffer[0] - 128 == 0) return false;

    // *touchX = touchBuffer[2] + touchBuffer[3] * 256;
    // *touchY = touchBuffer[4] + touchBuffer[5] * 256;

    // if (debug) {
    //     Serial.print(touchBuffer[0] - 128);
    //     Serial.print(" : ");
    //     Serial.print(*touchX);
    //     Serial.print(" x ");
    //     Serial.println(*touchY);
    // }
    // return true;
}

static void touchStart()
{
    if(touch.begin(INT_PIN, RST_PIN) != true) {
        Serial.println("! Module reset failed");
    } else {
        Serial.println("Module reset OK");
    }

    Serial.print("Check ACK on addr request on 0x");
    Serial.print(touch.i2cAddr, HEX);

    Wire.beginTransmission(touch.i2cAddr);
    int error = Wire.endTransmission();
    if(error == 0) {
        Serial.println(": SUCCESS");
    } else {
        Serial.print(": ERROR #");
        Serial.println(error);
    }
}

void GT911_init()
{
    // Wire.setClock(400000);
    // Wire.begin();
    Wire.begin(TOUCH_SDA, TOUCH_SCL, (uint32_t)I2C_TOUCH_FREQUENCY);
    delay(300);

    touch.setHandler(GT911_setXY);
    touchStart();
    LOG_INFO(TAG_DRVR, F("Goodix GT911x touch driver started"));
}

void GT911_loop()
{
    touch.loop();
}
#endif
*/