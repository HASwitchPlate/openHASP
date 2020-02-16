#include <Ticker.h>

#include "lv_conf.h"
#include "lvgl.h"
#include "lv_fs_if.h"

#include "TFT_eSPI.h"

#include "lv_zifont.h"

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_gui.h"
#include "hasp.h"

#if HASP_USE_PNGDECODE != 0
#include "png_decoder.h"
#endif

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
static ESP8266WebServer * webClient; // for snatshot
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
static WebServer * webClient; // for snatshot
#endif                        // ESP32

#define LVGL_TICK_PERIOD 30 // 30

#ifndef TFT_BCKL
#define TFT_BCKL -1 // No Backlight Control
#endif

int8_t guiDimLevel     = -1;
int8_t guiBacklightPin = TFT_BCKL;
bool guiAutoCalibrate  = true;
uint16_t guiSleepTime  = 150; // 0.1 second resolution
bool guiSleeping       = false;
uint8_t guiTickPeriod  = 50;
static Ticker tick;                      /* timer for interrupt handler */
static TFT_eSPI tft        = TFT_eSPI(); /* TFT instance */
static uint16_t calData[5] = {0, 65535, 0, 65535, 0};

static File pFileOut;
static uint8_t guiSnapshot = 0;

bool IRAM_ATTR guiCheckSleep()
{
    bool shouldSleep = lv_disp_get_inactive_time(NULL) > guiSleepTime * 100;
    if(shouldSleep && !guiSleeping) {
        dispatchIdle(F("LONG"));
        guiSleeping = true;
    } else if(!shouldSleep && guiSleeping) {
        dispatchIdle(F("OFF"));
        guiSleeping = false;
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
    if(guiSnapshot != 0) {
        int i = 0;
        uint8_t pixel[1024];

        for(int y = area->y1; y <= area->y2; y++) {
            for(int x = area->x1; x <= area->x2; x++) {
                /* Function for converting LittlevGL pixel format to RGB888 */
                // data = DISP_IMPL_lvgl_formatPixel(*color_p);

                pixel[i++] = (LV_COLOR_GET_B(*color_p) * 263 + 7) >> 5;
                pixel[i++] = (LV_COLOR_GET_G(*color_p) * 259 + 3) >> 6;
                pixel[i++] = (LV_COLOR_GET_R(*color_p) * 263 + 7) >> 5;
                pixel[i++] = 0xFF;

                color_p++;
                // i += 4;

                if(i + 4 >= sizeof(pixel)) {
                    switch(guiSnapshot) {
                        case 1:
                            // Save to local file
                            pFileOut.write(pixel, i);
                            break;
                        case 2:
                            // Send to remote client
                            if(webClient->client().write(pixel, i) != i) {
                                errorPrintln(F("GUI: %sPixelbuffer not completely sent"));
                                lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
                                return;
                            }
                    }
                    i = 0;
                }
            }
        }

        if(i > 0) {
            switch(guiSnapshot) {
                case 1:
                    // Save to local file
                    pFileOut.write(pixel, i);
                    break;
                case 2:
                    // Send to remote client
                    if(webClient->client().write(pixel, i) != i) {
                        errorPrintln(F("GUI: %sPixelbuffer not completely sent"));
                    }
            }
        }
    } else {
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
        tft.endWrite(); /* terminate TFT transaction */
    }

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

void guiFirstCalibration()
{
    dispatchCommand(F("calibrate"));
    guiAutoCalibrate = false;
    haspFirstSetup();
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);
    if(!touched) return false;

    if(guiAutoCalibrate) {
        guiFirstCalibration();
        return false;
    }

    bool shouldSleep = guiCheckSleep();

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

void guiCalibrate()
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println(PSTR("Touch corners as indicated"));

    tft.setTextFont(1);
    delay(500);
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    for(uint8_t i = 0; i < 5; i++) {
        Serial.print(calData[i]);
        if(i < 4) Serial.print(", ");
    }

    tft.setTouch(calData);
    delay(500);
    lv_obj_invalidate(lv_disp_get_layer_sys(NULL));
}

void guiSetup(TFT_eSPI & screen, JsonObject settings)
{
    size_t buffer_size;
    tft = screen;

    guiSetConfig(settings);
    tft.setTouch(calData);

    lv_init();

#if defined(ARDUINO_ARCH_ESP32)
    /* allocate on iram (or psram ?) */
    buffer_size                      = 19200; // 38 KBytes
    static lv_color_t * guiVdbBuffer = (lv_color_t *)malloc(sizeof(lv_color_t) * buffer_size);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, guiVdbBuffer, NULL, buffer_size);
#else
    /* allocate on heap */
    static lv_color_t guiVdbBuffer[1024 * 3]; // 6 KBytes
    buffer_size = sizeof(guiVdbBuffer) / sizeof(guiVdbBuffer[0]);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, guiVdbBuffer, NULL, buffer_size);
#endif
    debugPrintln(String(F("LVGL: VDB size : ")) + String((size_t)sizeof(lv_color_t) * buffer_size));

#if LV_USE_LOG != 0
    debugPrintln(F("LVGL: Registering lvgl logging handler"));
    lv_log_register_print_cb(debugLvgl); /* register print function for debugging */
#endif

    /* Initialize PNG decoder */
#if HASP_USE_PNGDECODE != 0
    png_decoder_init();
#endif

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

#if LV_USE_FS_IF != 0
    lv_fs_if_init();
#endif

    /* Setup Backlight Control Pin */
    if(guiBacklightPin >= 0) {
        char msg[128];
        sprintf(msg, PSTR("LVGL: Backlight Pin = %i"), guiBacklightPin);
        debugPrintln(msg);

#if defined(ARDUINO_ARCH_ESP32)
        // configure LED PWM functionalitites
        ledcSetup(0, 1000, 10);
        // attach the channel to the GPIO to be controlled
        pinMode(guiBacklightPin, OUTPUT);
        ledcAttachPin(guiBacklightPin, 0);
#else
        pinMode(guiBacklightPin, OUTPUT);
#endif
    }
}

void IRAM_ATTR guiLoop()
{
    lv_task_handler(); /* let the GUI do its work */
    guiCheckSleep();
}
void guiStop()
{}

void guiSetDim(uint8_t level)
{
    if(guiBacklightPin >= 0) {
        guiDimLevel = level >= 0 ? level : 0;
        guiDimLevel = guiDimLevel <= 100 ? guiDimLevel : 100;

#if defined(ARDUINO_ARCH_ESP32)
        ledcWrite(0, map(guiDimLevel, 0, 100, 0, 1023)); // ledChannel and value
#else
        analogWrite(D1, map(guiDimLevel, 0, 100, 0, 1023));
#endif
    } else {
        guiDimLevel = -1;
    }
}

int8_t guiGetDim()
{
    return guiDimLevel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool guiGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_GUI_TICKPERIOD)]   = guiTickPeriod;
    settings[FPSTR(F_GUI_IDLEPERIOD)]   = guiSleepTime;
    settings[FPSTR(F_GUI_BACKLIGHTPIN)] = guiBacklightPin;

    JsonArray array = settings[FPSTR(F_GUI_CALIBRATION)].to<JsonArray>();
    for(int i = 0; i < 5; i++) {
        array.add(calData[i]);
    }

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool guiSetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(!settings[FPSTR(F_GUI_TICKPERIOD)].isNull()) {
        if(guiTickPeriod != settings[FPSTR(F_GUI_TICKPERIOD)].as<uint8_t>()) {
            debugPrintln(F("guiTickPeriod set"));
        }
        changed |= guiTickPeriod != settings[FPSTR(F_GUI_TICKPERIOD)].as<uint8_t>();

        guiTickPeriod = settings[FPSTR(F_GUI_TICKPERIOD)].as<uint8_t>();
    }

    if(!settings[FPSTR(F_GUI_BACKLIGHTPIN)].isNull()) {
        if(guiBacklightPin != settings[FPSTR(F_GUI_BACKLIGHTPIN)].as<int8_t>()) {
            debugPrintln(F("guiBacklightPin set"));
        }
        changed |= guiBacklightPin != settings[FPSTR(F_GUI_BACKLIGHTPIN)].as<int8_t>();

        guiBacklightPin = settings[FPSTR(F_GUI_BACKLIGHTPIN)].as<int8_t>();
    }

    if(!settings[FPSTR(F_GUI_IDLEPERIOD)].isNull()) {
        if(guiSleepTime != settings[FPSTR(F_GUI_IDLEPERIOD)].as<uint8_t>()) {
            debugPrintln(F("guiSleepTime set"));
        }
        changed |= guiSleepTime != settings[FPSTR(F_GUI_IDLEPERIOD)].as<uint8_t>();

        guiSleepTime = settings[FPSTR(F_GUI_IDLEPERIOD)].as<uint8_t>();
    }

    if(!settings[FPSTR(F_GUI_CALIBRATION)].isNull()) {
        bool status = false;
        int i       = 0;

        JsonArray array = settings[FPSTR(F_GUI_CALIBRATION)].as<JsonArray>();
        for(JsonVariant v : array) {
            if(calData[i] != v.as<uint16_t>()) status = true;
            calData[i] = v.as<uint16_t>();
            i++;
        }

        if(status) {
            debugPrintln(F("calData set"));
            guiAutoCalibrate = false;
        }

        changed |= status;
    }

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return changed;
}

/** Flush buffer.
 *
 * Flush buffer into a binary file.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] pFileName   Output binary file name.
 *
 */
void guiTakeScreenshot(const char * pFileName)
{
    pFileOut = SPIFFS.open(pFileName, "w");

    uint8_t bmpheader[138] = {0x42, 0x4D, 0x8A, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8A, 0x00, 0x00, 0x00, 0x7C,
                              0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x20, 0x00,
                              0x03, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
                              0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x42, 0x47, 0x52, 0x73};

    pFileOut.write(bmpheader, sizeof(bmpheader));

    if(pFileOut == NULL) {
        printf(("[Display] error: %s cannot be opened", pFileName));
        return;
    }

    guiSnapshot = 1;
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL); /* Will call our disp_drv.disp_flush function */
    guiSnapshot = 0;

    pFileOut.close();
    printf(("[Display] data flushed to %s", pFileName));
}

#if defined(ARDUINO_ARCH_ESP8266)
void guiTakeScreenshot(ESP8266WebServer & client)
#endif
#if defined(ARDUINO_ARCH_ESP32)
    void guiTakeScreenshot(WebServer & client)
#endif // ESP32{
{
    webClient = &client;

    uint8_t bmpheader[138] = {0x42, 0x4D, 0x8A, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8A, 0x00, 0x00, 0x00, 0x7C,
                              0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x20, 0x00,
                              0x03, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
                              0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x42, 0x47, 0x52, 0x73};

    if(webClient->client().write(bmpheader, sizeof(bmpheader)) != sizeof(bmpheader)) {
        errorPrintln(F("GUI: %sData sent does not match header size"));
    } else {
        debugPrintln(F("GUI: Bitmap header sent"));
    }

    guiSnapshot = 2;
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL); /* Will call our disp_drv.disp_flush function */
    guiSnapshot = 0;

    debugPrintln(F("GUI: Bitmap data flushed to webclient"));
}
