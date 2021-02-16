#if TOUCH_DRIVER == 0x2046B

    #include "lvgl.h"
    #include "hasp_drv_xpt2046.h"

    // #include <stddef.h>
    // #include LV_DRV_INDEV_INCLUDE
    // #include LV_DRV_DELAY_INCLUDE
    // #include <SPI.h>
    #include "XPT2046_Touchscreen.h"

    /*********************
     *      DEFINES
     *********************/
    // #define CMD_X_READ 0b10010000
    // #define CMD_Y_READ 0b11010000
    #define SPI_SETTING SPISettings(2000000, MSBFIRST, SPI_MODE0)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void xpt2046_corr(int16_t * x, int16_t * y);
static void xpt2046_avg(int16_t * x, int16_t * y);

/**********************
 *  STATIC VARIABLES
 **********************/
int16_t avg_buf_x[XPT2046_AVG];
int16_t avg_buf_y[XPT2046_AVG];
uint8_t avg_last;
    #if defined(ARDUINO_ARCH_ESP32)
SPIClass xpt2046_spi(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

    #elif defined(STM32F407ZG)
//#include "SoftSPI.h"
// SoftSPI xpt2046_spi(PF11, PB2, PB0);
XPT2046_Touchscreen ts(TOUCH_CS);

    #else
// SPIClass xpt2046_spi(PB15, PB14, PB13, PB12);
XPT2046_Touchscreen ts(TOUCH_CS);
    #endif
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the XPT2046
 */
void XPT2046_init(uint8_t rotation)
{
    // ts.begin();
    // ts.setRotation(rotation);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);
}

static int16_t last_x = 0;
static int16_t last_y = 0;

bool XPT2046_getXY(int16_t * touchX, int16_t * touchY, bool debug)
{
    // bool touched = ts.touched();
    uint16_t tmp;

    int16_t data[6];

    // if (!isrWake) return;
    uint32_t now = millis();
    // if (now - msraw < MSEC_THRESHOLD) return;

    xpt2046_spi.beginTransaction(SPI_SETTING);
    digitalWrite(TOUCH_CS, LOW);

    // Start YP sample request for x position, read 4 times and keep last sample
    xpt2046_spi.transfer(0xd0); // Start new YP conversion
    xpt2046_spi.transfer(0);    // Read first 8 bits
    xpt2046_spi.transfer(0xd0); // Read last 8 bits and start new YP conversion
    xpt2046_spi.transfer(0);    // Read first 8 bits
    xpt2046_spi.transfer(0xd0); // Read last 8 bits and start new YP conversion
    xpt2046_spi.transfer(0);    // Read first 8 bits
    xpt2046_spi.transfer(0xd0); // Read last 8 bits and start new YP conversion

    tmp = xpt2046_spi.transfer(0); // Read first 8 bits
    tmp = tmp << 5;
    tmp |= 0x1f & (xpt2046_spi.transfer(0x90) >> 3); // Read last 8 bits and start new XP conversion

    *touchX = tmp;

    // Start XP sample request for y position, read 4 times and keep last sample
    xpt2046_spi.transfer(0);    // Read first 8 bits
    xpt2046_spi.transfer(0x90); // Read last 8 bits and start new XP conversion
    xpt2046_spi.transfer(0);    // Read first 8 bits
    xpt2046_spi.transfer(0x90); // Read last 8 bits and start new XP conversion
    xpt2046_spi.transfer(0);    // Read first 8 bits
    xpt2046_spi.transfer(0x90); // Read last 8 bits and start new XP conversion

    tmp = xpt2046_spi.transfer(0); // Read first 8 bits
    tmp = tmp << 5;
    tmp |= 0x1f & (xpt2046_spi.transfer(0) >> 3); // Read last 8 bits

    *touchY = tmp;

    digitalWrite(TOUCH_CS, HIGH);
    xpt2046_spi.endTransaction();
    Serial.printf("z=%d  ::  z1=%d,  z2=%d  \n", *touchX, *touchY, *touchX);

    return false;
}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no ore data to be read
 */
bool XPT2046_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    uint8_t buf;

    int16_t x = 0;
    int16_t y = 0;

    // uint8_t irq = LV_DRV_INDEV_IRQ_READ;
    data->state = ts.touched();

    if(data->state) {
        TS_Point p = ts.getPoint();
        x          = p.x;
        y          = p.y;
        xpt2046_corr(&x, &y);

    } else {
        x           = last_x;
        y           = last_y;
        avg_last    = 0;
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = x;
    data->point.y = y;
    if(data->state) {
        Serial.print(x);
        Serial.print(" - ");
        Serial.println(y);
    } else {
        // Serial.print(".");
    }

    return false;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void xpt2046_corr(int16_t * x, int16_t * y)
{
    #if XPT2046_XY_SWAP != 0
    int16_t swap_tmp;
    swap_tmp = *x;
    *x       = *y;
    *y       = swap_tmp;
    #endif

    if((*x) > XPT2046_X_MIN)
        (*x) -= XPT2046_X_MIN;
    else
        (*x) = 0;

    if((*y) > XPT2046_Y_MIN)
        (*y) -= XPT2046_Y_MIN;
    else
        (*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * XPT2046_HOR_RES) / (XPT2046_X_MAX - XPT2046_X_MIN);

    (*y) = (uint32_t)((uint32_t)(*y) * XPT2046_VER_RES) / (XPT2046_Y_MAX - XPT2046_Y_MIN);

    #if XPT2046_X_INV != 0
    (*x) = XPT2046_HOR_RES - (*x);
    #endif

    #if XPT2046_Y_INV != 0
    (*y) = XPT2046_VER_RES - (*y);
    #endif
}

static void xpt2046_avg(int16_t * x, int16_t * y)
{
    /*Shift out the oldest data*/
    uint8_t i;
    for(i = XPT2046_AVG - 1; i > 0; i--) {
        avg_buf_x[i] = avg_buf_x[i - 1];
        avg_buf_y[i] = avg_buf_y[i - 1];
    }

    /*Insert the new point*/
    avg_buf_x[0] = *x;
    avg_buf_y[0] = *y;
    if(avg_last < XPT2046_AVG) avg_last++;

    /*Sum the x and y coordinates*/
    int32_t x_sum = 0;
    int32_t y_sum = 0;
    for(i = 0; i < avg_last; i++) {
        x_sum += avg_buf_x[i];
        y_sum += avg_buf_y[i];
    }

    /*Normalize the sums*/
    (*x) = (int32_t)x_sum / avg_last;
    (*y) = (int32_t)y_sum / avg_last;
}

#endif