#include <Ticker.h>

#include "lvgl.h"
#include "lv_conf.h"

#include "TFT_eSPI.h"

#ifdef ESP32
//#include "png_decoder.h"
#endif
#include "lv_zifont.h"

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"

#define LVGL_TICK_PERIOD 30 // 30

uint16_t guiSleepTime = 150; // 0.1 second resolution
bool guiSleeping      = false;
uint8_t guiTickPeriod = 50;
Ticker tick;               /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

bool IRAM_ATTR guiCheckSleep()
{
    bool shouldSleep = lv_disp_get_inactive_time(NULL) > guiSleepTime * 100;
    if(shouldSleep && !guiSleeping) {
        debugPrintln(F("GUI: Going to sleep now..."));
        guiSleeping = true;
    }
    return shouldSleep;
}

#if LV_USE_LOG != 0
/* Serial debugging */
void debugLvgl(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{
    char msg[128];
    sprintf(msg, PSTR("LVGL: %s@%d->%s"), file, line, dsc);
    debugPrintln(msg);
}
#endif

/* Display flushing */
void tft_espi_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    uint16_t c;

    tft.startWrite(); /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1),
                      (area->y2 - area->y1 + 1)); /* set the working window */
    for(int y = area->y1; y <= area->y2; y++) {
        for(int x = area->x1; x <= area->x2; x++) {
            c = color_p->full;
            tft.writeColor(c, 1);
            color_p++;
        }
    }
    tft.endWrite();            /* terminate TFT transaction */
    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void IRAM_ATTR lv_tick_handler(void)
{
    lv_tick_inc(guiTickPeriod);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static int32_t last_diff = 0;
    int32_t diff             = 0;                  /* Dummy - no movement */
    int btn_state            = LV_INDEV_STATE_REL; /* Dummy - no press */

    data->enc_diff = diff - last_diff;
    data->state    = btn_state;
    last_diff      = diff;
    return false;
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);
    if(!touched) return false;

    bool shouldSleep = guiCheckSleep();
    if(!shouldSleep && guiSleeping) {
        debugPrintln(F("GUI: Waking up!"));
        guiSleeping = false;
    }

    // Ignore first press?

    if(touchX > tft.width() || touchY > tft.height()) {
        Serial.print(F("Y or y outside of expected parameters.. x: "));
        Serial.print(touchX);
        Serial.print(F("  / y: "));
        Serial.println(touchY);
    } else {
        /*Save the state and save the pressed coordinate*/
        data->state   = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        data->point.x = touchX;
        data->point.y = touchY;
        /*
                Serial.print("Data x");
                Serial.println(touchX);

                Serial.print("Data y");
                Serial.println(touchY);*/
    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

void guiSetup(TFT_eSPI & screen, JsonObject settings)
{
    size_t buffer_size;
    tft = screen;
    lv_init();

#if ESP32
    /* allocate on iram (or psram ?) */
    buffer_size                      = 1024 * 8;
    static lv_color_t * guiVdbBuffer = (lv_color_t *)malloc(sizeof(lv_color_t) * buffer_size);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, guiVdbBuffer, NULL, buffer_size);
#else
    /* allocate on heap */
    static lv_color_t guiVdbBuffer[1024 * 4];
    buffer_size = sizeof(guiVdbBuffer) / sizeof(guiVdbBuffer[0]);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, guiVdbBuffer, NULL, buffer_size);
#endif
    debugPrintln(String(F("LVGL: VDB size : ")) + String(buffer_size));

#if LV_USE_LOG != 0
    debugPrintln(F("LVGL: Registering lvgl logging handler"));
    lv_log_register_print_cb(debugLvgl); /* register print function for debugging */
#endif

    /* Initialize PNG decoder */
    // png_decoder_init();

    /* Initialize the display driver */
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = tft_espi_flush;
    disp_drv.buffer   = &disp_buf;
#if(TFT_ROTATION == 0 || TFT_ROTATION == 2 || TFT_ROTATION == 4 || TFT_ROTATION == 6)
    /* 1/3=Landscape or 0/2=Portrait orientation */
    // Normal width & height
    disp_drv.hor_res = TFT_WIDTH;  // From User_Setup.h
    disp_drv.ver_res = TFT_HEIGHT; // From User_Setup.h
#else
    // Swapped width & height
    disp_drv.hor_res = TFT_HEIGHT; // From User_Setup.h
    disp_drv.ver_res = TFT_WIDTH;  // From User_Setup.h
#endif
    lv_disp_drv_register(&disp_drv);

    /*Initialize the touch pad*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    // indev_drv.type = LV_INDEV_TYPE_ENCODER;
    // indev_drv.read_cb = read_encoder;
    indev_drv.type           = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb        = my_touchpad_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    lv_obj_t * label = lv_label_create(lv_layer_sys(), NULL);
    lv_label_set_text(label, "<");
    lv_indev_set_cursor(mouse_indev, label); // connect the object to the driver

    /*
        lv_obj_t * cursor = lv_obj_create(lv_layer_sys(), NULL); // show on every page
        lv_obj_set_size(cursor, 9, 9);
        static lv_style_t style_cursor;
        lv_style_copy(&style_cursor, &lv_style_pretty);
        style_cursor.body.radius     = LV_RADIUS_CIRCLE;
        style_cursor.body.main_color = LV_COLOR_RED;
        style_cursor.body.opa        = LV_OPA_COVER;
        lv_obj_set_style(cursor, &style_cursor);
        // lv_obj_set_click(cursor, false);
        lv_indev_set_cursor(mouse_indev, cursor); // connect the object to the driver
    */
    /* Initialize mouse pointer */
    /*// if(true) {
    debugPrintln(PSTR("LVGL: Initialize Cursor"));
    lv_obj_t * cursor;
    lv_obj_t * mouse_layer = lv_disp_get_layer_sys(NULL); // default display
    // cursor               = lv_obj_create(lv_scr_act(), NULL);
    cursor = lv_obj_create(mouse_layer, NULL); // show on every page
    lv_obj_set_size(cursor, 9, 9);
    static lv_style_t style_round;
    lv_style_copy(&style_round, &lv_style_plain);
    style_round.body.radius     = LV_RADIUS_CIRCLE;
    style_round.body.main_color = LV_COLOR_RED;
    style_round.body.opa        = LV_OPA_COVER;
    lv_obj_set_style(cursor, &style_round);
    lv_obj_set_click(cursor, false); // don't click on the cursor
    lv_indev_set_cursor(mouse_indev, cursor);
    // }*/

    /*Initialize the graphics library's tick*/
    tick.attach_ms(guiTickPeriod, lv_tick_handler);

    // guiLoop();
}

void IRAM_ATTR guiLoop()
{
    lv_task_handler(); /* let the GUI do its work */
    guiCheckSleep();
}
void guiStop()
{}

bool guiGetConfig(const JsonObject & settings)
{
    if(!settings.isNull() && settings[F_GUI_TICKPERIOD] == guiTickPeriod) return false;

    settings[F_GUI_TICKPERIOD] = guiTickPeriod;

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}