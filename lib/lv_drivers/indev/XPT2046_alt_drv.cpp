/**
 * @file XPT2046.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "XPT2046_alt_drv.h"
#if USE_XPT2046_ALT_DRV

#include <stddef.h>
#include LV_DRV_INDEV_INCLUDE
#include LV_DRV_DELAY_INCLUDE
#include <XPT2046_Touchscreen.h>

/*********************
 *      DEFINES
 *********************/
#define CS_PIN PB12

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
XPT2046_Touchscreen ts(CS_PIN);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the XPT2046
 */
void xpt2046_alt_drv_init(uint8_t rotation)
{
    ts.begin();
    ts.setRotation(rotation);
}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no more data to be read
 */
bool xpt2046_alt_drv_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    data->state = ts.touched();
    if(data->state) {
        TS_Point p    = ts.getPoint();
        data->point.x = p.x;
        data->point.y = p.y;
        Serial.print(p.x);
        Serial.print(" - ");
        Serial.println(p.y);
    }
    return false;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
