#include <Ticker.h>
#include "ArduinoLog.h"

#include "lv_conf.h"
#include "lvgl.h"
#include "lv_fs_if.h"
#include "TFT_eSPI.h"
#include "lv_zifont.h"

#include "hasp_tft.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_gui.h"
#include "hasp.h"

#if HASP_USE_PNGDECODE
#include "png_decoder.h"
#endif

#if HASP_USE_SPIFFS
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#endif

/* ---------- Screenshot Variables ---------- */
File pFileOut;
uint8_t guiSnapshot = 0;
size_t guiVDBsize   = 0;

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
ESP8266WebServer * webClient; // for snatshot
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
WebServer * webClient; // for snatshot
#endif                 // ESP32
/* ------------------------------------------- */

// #define LVGL_TICK_PERIOD 30

#ifndef TFT_BCKL
#define TFT_BCKL -1 // No Backlight Control
#endif
#ifndef TFT_ROTATION
#define TFT_ROTATION 0
#endif

static bool guiShowPointer    = false;
static bool guiBacklightIsOn  = true;
static int8_t guiDimLevel     = -1;
static int8_t guiBacklightPin = TFT_BCKL;
static bool guiAutoCalibrate  = true;
static uint16_t guiSleepTime1 = 60;  // 1 second resolution
static uint16_t guiSleepTime2 = 120; // 1 second resolution
static uint8_t guiSleeping    = 0;   // 0 = off, 1 = short, 2 = long
static uint8_t guiTickPeriod  = 50;
static uint8_t guiRotation    = TFT_ROTATION;
static Ticker tick;  /* timer for interrupt handler */
static TFT_eSPI tft; // = TFT_eSPI(); /* TFT instance */
static uint16_t calData[5] = {0, 65535, 0, 65535, 0};

bool guiCheckSleep()
{
    uint32_t idle = lv_disp_get_inactive_time(NULL);
    if(idle >= (guiSleepTime1 + guiSleepTime2) * 1000U) {
        if(guiSleeping != 2) {
            dispatchIdle(F("LONG"));
            guiSleeping = 2;
        }
        return true;
    } else if(idle >= guiSleepTime1 * 1000U) {
        if(guiSleeping != 1) {
            dispatchIdle(F("SHORT"));
            guiSleeping = 1;
        }
        return true;
    }
    if(guiSleeping != 0) {
        dispatchIdle(F("OFF"));
        guiSleeping = 0;
    }
    return false;
}

#if LV_USE_LOG != 0
/* Serial debugging */
void debugLvgl(lv_log_level_t level, const char * file, uint32_t line, const char * funcname, const char * descr)
{
    switch(level) {
        case LV_LOG_LEVEL_TRACE:
            Log.trace(F("LVGL: %s:%u %s -> %s"), file, line, funcname, descr);
            break;
        case LV_LOG_LEVEL_INFO:
            Log.notice(F("LVGL: %s:%u %s -> %s"), file, line, funcname, descr);
            break;
        case LV_LOG_LEVEL_WARN:
            Log.warning(F("LVGL: %s:%u %s -> %s"), file, line, funcname, descr);
            break;
        case LV_LOG_LEVEL_ERROR:
            Log.error(F("LVGL: %s:%u %s -> %s"), file, line, funcname, descr);
            break;
        default:
            Log.verbose(F("LVGL: %s:%u %s -> %s"), file, line, funcname, descr);
    }
}
#endif

/* Display flushing */
void tft_espi_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    uint16_t c;

    tft.startWrite(); /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1),
                      (area->y2 - area->y1 + 1)); /* set the working window */

    if(guiSnapshot != 0) {
        uint i = 0;
        uint8_t pixel[1024];

        for(int y = area->y1; y <= area->y2; y++) {
            for(int x = area->x1; x <= area->x2; x++) {
                /* Function for converting LittlevGL pixel format to RGB888 */
                // data = DISP_IMPL_lvgl_formatPixel(*color_p);

                // Complex 32 bpp
                /* pixel[i++] = (LV_COLOR_GET_B(*color_p) * 263 + 7) >> 5;
                 pixel[i++] = (LV_COLOR_GET_G(*color_p) * 259 + 3) >> 6;
                 pixel[i++] = (LV_COLOR_GET_R(*color_p) * 263 + 7) >> 5;
                 pixel[i++] = 0xFF;*/

                // Simple 32 bpp
                // pixel[i++] = (LV_COLOR_GET_B(*color_p) << 3);
                // pixel[i++] = (LV_COLOR_GET_G(*color_p) << 2);
                // pixel[i++] = (LV_COLOR_GET_R(*color_p) << 3);
                // pixel[i++] = 0xFF;

                c = color_p->full;
                tft.writeColor(c, 1); // also update tft

                // Simple 16 bpp
                pixel[i++] = c & 0xFF;
                pixel[i++] = (c >> 8) & 0xFF;

                color_p++;

                if(i + 4 >= sizeof(pixel)) {
                    switch(guiSnapshot) {
                        case 1:
                            // Save to local file
                            pFileOut.write(pixel, i);
                            break;
                        case 2:
                            // Send to remote client
                            if(webClient->client().write(pixel, i) != i) {
                                Log.warning(F("GUI: Pixelbuffer not completely sent"));
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
                        Log.warning(F("GUI: Pixelbuffer not completely sent"));
                    }
            }
        }
    } else {
        for(int y = area->y1; y <= area->y2; y++) {
            for(int x = area->x1; x <= area->x2; x++) {
                c = color_p->full;
                tft.writeColor(c, 1);
                color_p++;
            }
        }
    }
    tft.endWrite(); /* terminate TFT transaction */

    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void IRAM_ATTR lv_tick_handler(void)
{
    lv_tick_inc(guiTickPeriod);
}

/* Reading input device (simulated encoder here) */
/*bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static int32_t last_diff = 0;
    int32_t diff             = 0;                  // Dummy - no movement
    int btn_state            = LV_INDEV_STATE_REL; // Dummy - no press

    data->enc_diff = diff - last_diff;
    data->state    = btn_state;
    last_diff      = diff;
    return false;
}*/

void guiFirstCalibration()
{
    guiSetDim(100);
    dispatchCommand(F("calibrate"));
    guiAutoCalibrate = false;
    // haspFirstSetup();
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
#ifdef TOUCH_CS
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);
    if(!touched) return false;

    if(guiAutoCalibrate) {
        guiFirstCalibration();
        return false;
    }

    guiCheckSleep();

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
#endif

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

void guiCalibrate()
{
#ifdef TOUCH_CS
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
#endif
}

void guiSetup(JsonObject settings)
{
    guiSetConfig(settings);
    // guiBacklightIsOn = guiDimLevel > 0;

    tft.begin(); /* TFT init */
#ifdef TOUCH_CS
    tft.setTouch(calData);
#endif
    tftSetup(tft, settings[F("tft")]);

    tft.setRotation(guiRotation); /* 1/3=Landscape or 0/2=Portrait orientation */
    lv_init();

#if defined(ARDUINO_ARCH_ESP32)
    /* allocate on iram (or psram ?) */
    guiVDBsize                        = 16 * 1024u; // 32 KBytes * 2
    static lv_color_t * guiVdbBuffer1 = (lv_color_t *)malloc(sizeof(lv_color_t) * guiVDBsize);
    static lv_color_t * guiVdbBuffer2 = (lv_color_t *)malloc(sizeof(lv_color_t) * guiVDBsize);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, guiVdbBuffer1, guiVdbBuffer2, guiVDBsize);
#else
    /* allocate on heap */
    static lv_color_t guiVdbBuffer1[5 * 512u]; // 6 KBytes
    // static lv_color_t guiVdbBuffer2[3 * 1024u]; // 6 KBytes
    guiVDBsize = sizeof(guiVdbBuffer1) / sizeof(guiVdbBuffer1[0]);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, guiVdbBuffer1, NULL, guiVDBsize);
#endif

    // char buffer[128];
    Log.verbose(F("LVGL: Version  : %u.%u.%u %s"), LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH,
                PSTR(LVGL_VERSION_INFO));
    Log.verbose(F("LVGL: Rotation : %d"), guiRotation);
    Log.verbose(F("LVGL: MEM size : %d"), LV_MEM_SIZE);
    Log.verbose(F("LVGL: VFB size : %d"), (size_t)sizeof(lv_color_t) * guiVDBsize);

#if LV_USE_LOG != 0
    Log.warning(F("LVGL: Registering lvgl logging handler"));
    lv_log_register_print_cb(debugLvgl); /* register print function for debugging */
#endif

    /* Initialize PNG decoder */
#if HASP_USE_PNGDECODE != 0
    png_decoder_init();
#endif

#if LV_USE_FS_IF != 0
    lv_fs_if_init();
#endif

    /* Initialize the display driver */
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = tft_espi_flush;
    disp_drv.buffer   = &disp_buf;
    if(guiRotation == 0 || guiRotation == 2 || guiRotation == 4 || guiRotation == 6) {
        /* 1/3=Landscape or 0/2=Portrait orientation */
        // Normal width & height
        disp_drv.hor_res = TFT_WIDTH;  // From User_Setup.h
        disp_drv.ver_res = TFT_HEIGHT; // From User_Setup.h
    } else {
        // Swapped width & height
        disp_drv.hor_res = TFT_HEIGHT; // From User_Setup.h
        disp_drv.ver_res = TFT_WIDTH;  // From User_Setup.h
    }
    lv_disp_drv_register(&disp_drv);

    /*Initialize the touch pad*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    // indev_drv.type = LV_INDEV_TYPE_ENCODER;
    // indev_drv.read_cb = read_encoder;
    indev_drv.type           = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb        = my_touchpad_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    if(guiShowPointer) {
        lv_obj_t * label = lv_label_create(lv_layer_sys(), NULL);
        lv_label_set_text(label, "<");
        lv_indev_set_cursor(mouse_indev, label); // connect the object to the driver
    }

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

    /* Setup Backlight Control Pin */
    if(guiBacklightPin >= 0) {
        Log.verbose(F("LVGL: Backlight: Pin %i"), guiBacklightPin);

#if defined(ARDUINO_ARCH_ESP32)
        // configure LED PWM functionalitites
        ledcSetup(100, 1000, 10);
        // attach the channel to the GPIO to be controlled
        pinMode(guiBacklightPin, OUTPUT);
        ledcAttachPin(guiBacklightPin, 99);
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

bool guiGetBacklight()
{
    return guiBacklightIsOn;
}

void guiSetBacklight(bool lighton)
{
    guiBacklightIsOn = lighton;

    if(guiBacklightPin >= 0) {

#if defined(ARDUINO_ARCH_ESP32)
        ledcWrite(99, lighton ? map(guiDimLevel, 0, 100, 0, 1023) : 0); // ledChannel and value
#else
        analogWrite(guiBacklightPin, lighton ? map(guiDimLevel, 0, 100, 0, 1023) : 0);
#endif

    } else {
        guiBacklightIsOn = true;
    }
}

void guiSetDim(uint8_t level)
{
    if(guiBacklightPin >= 0) {
        guiDimLevel = level >= 0 ? level : 0;
        guiDimLevel = guiDimLevel <= 100 ? guiDimLevel : 100;

        if(guiBacklightIsOn) { // The backlight is ON
#if defined(ARDUINO_ARCH_ESP32)
            ledcWrite(99, map(guiDimLevel, 0, 100, 0, 1023)); // ledChannel and value
#else
            analogWrite(guiBacklightPin, map(guiDimLevel, 0, 100, 0, 1023));
#endif
        }

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
    settings[FPSTR(F_GUI_IDLEPERIOD1)]  = guiSleepTime1;
    settings[FPSTR(F_GUI_IDLEPERIOD2)]  = guiSleepTime2;
    settings[FPSTR(F_GUI_BACKLIGHTPIN)] = guiBacklightPin;
    settings[FPSTR(F_GUI_ROTATION)]     = guiRotation;
    settings[FPSTR(F_GUI_POINTER)]      = guiShowPointer;

    JsonArray array = settings[FPSTR(F_GUI_CALIBRATION)].to<JsonArray>();
    for(uint8_t i = 0; i < 5; i++) {
        array.add(calData[i]);
    }

    configOutput(settings);
    return true;
}

/** Set GUI Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool guiSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    changed |= configSet(guiTickPeriod, settings[FPSTR(F_GUI_TICKPERIOD)], PSTR("guiTickPeriod"));
    changed |= configSet(guiBacklightPin, settings[FPSTR(F_GUI_BACKLIGHTPIN)], PSTR("guiBacklightPin"));
    changed |= configSet(guiSleepTime1, settings[FPSTR(F_GUI_IDLEPERIOD1)], PSTR("guiSleepTime1"));
    changed |= configSet(guiSleepTime2, settings[FPSTR(F_GUI_IDLEPERIOD2)], PSTR("guiSleepTime2"));
    changed |= configSet(guiRotation, settings[FPSTR(F_GUI_ROTATION)], PSTR("guiRotation"));

    if(!settings[FPSTR(F_GUI_POINTER)].isNull()) {
        if(guiShowPointer != settings[FPSTR(F_GUI_POINTER)].as<bool>()) {
            Log.trace(F("guiShowPointer set"));
        }
        changed |= guiShowPointer != settings[FPSTR(F_GUI_POINTER)].as<bool>();

        guiShowPointer = settings[FPSTR(F_GUI_POINTER)].as<bool>();
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
            Log.trace(F("calData set"));
            guiAutoCalibrate = false;
        }

        changed |= status;
    }

    return changed;
}

/** Send Bitmap Header.
 *
 * Sends a header in BMP format for the size of the screen.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 **/
void guiSendBmpHeader()
{
    uint8_t buffer[128];
    memset(buffer, 0, sizeof(buffer));

    lv_disp_t * disp = lv_disp_get_default();
    buffer[0]        = 0x42;
    buffer[1]        = 0x4D;

    buffer[10 + 0] = 122;      // full header size
    buffer[14 + 0] = 122 - 14; // dib header size
    buffer[26 + 0] = 1;        // number of color planes
    buffer[28 + 0] = 16;       // 24;                  // bbp
    buffer[30 + 0] = 3;        // compression, 0 = RGB / 3 = RGBA

    // file size
    int32_t res   = 122 + disp->driver.hor_res * disp->driver.ver_res * buffer[28] / 8;
    buffer[2 + 3] = (res >> 24) & 0xFF;
    buffer[2 + 2] = (res >> 16) & 0xFF;
    buffer[2 + 1] = (res >> 8) & 0xFF;
    buffer[2 + 0] = res & 0xFF;

    // horizontal resolution
    res            = disp->driver.hor_res;
    buffer[18 + 3] = (res >> 24) & 0xFF;
    buffer[18 + 2] = (res >> 16) & 0xFF;
    buffer[18 + 1] = (res >> 8) & 0xFF;
    buffer[18 + 0] = res & 0xFF;

    // vertical resolution
    res            = -disp->driver.ver_res; // top down lines
    buffer[22 + 3] = (res >> 24) & 0xFF;
    buffer[22 + 2] = (res >> 16) & 0xFF;
    buffer[22 + 1] = (res >> 8) & 0xFF;
    buffer[22 + 0] = res & 0xFF;

    // bitmp size
    res            = disp->driver.hor_res * disp->driver.ver_res * buffer[28 + 0] / 8; //* 2;
    buffer[34 + 3] = (res >> 24) & 0xFF;
    buffer[34 + 2] = (res >> 16) & 0xFF;
    buffer[34 + 1] = (res >> 8) & 0xFF;
    buffer[34 + 0] = res & 0xFF;

    res            = 2836;
    buffer[38 + 3] = (res >> 24) & 0xFF;
    buffer[38 + 2] = (res >> 16) & 0xFF;
    buffer[38 + 1] = (res >> 8) & 0xFF;
    buffer[38 + 0] = res & 0xFF;
    buffer[42 + 3] = (res >> 24) & 0xFF;
    buffer[42 + 2] = (res >> 16) & 0xFF;
    buffer[42 + 1] = (res >> 8) & 0xFF;
    buffer[42 + 0] = res & 0xFF;

    // R: 1111 1000 | 0000 0000
    buffer[54 + 1] = 0xF8;
    // G: 0000 0111 | 1110 0000
    buffer[58 + 0] = 0xE0;
    buffer[58 + 1] = 0x07;
    // B: 0000 0000 | 0001 1111
    buffer[62 + 0] = 0x1F;
    // A: 0000 0000 | 0000 0000
    // buffer[66 + 0] = 0x00;

    // "Win
    buffer[70 + 3] = 0x57;
    buffer[70 + 2] = 0x69;
    buffer[70 + 1] = 0x6E;
    buffer[70 + 0] = 0x20;

    if(guiSnapshot == 1) {
        size_t len = pFileOut.write(buffer, 122);
        if(len != sizeof(buffer)) {
            Log.warning(F("GUI: Data written does not match header size"));
        } else {
            Log.verbose(F("GUI: Bitmap header written"));
        }

    } else if(guiSnapshot == 2) {
        if(webClient->client().write(buffer, 122) != 122) {
            Log.warning(F("GUI: Data sent does not match header size"));
        } else {
            Log.verbose(F("GUI: Bitmap header sent"));
        }
    }
}

/** Take Screenshot.
 *
 * Flush buffer into a binary file.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] pFileName   Output binary file name.
 *
 **/
void guiTakeScreenshot(const char * pFileName)
{
    pFileOut = SPIFFS.open(pFileName, "w");

    if(pFileOut == 0) {
        Log.warning(F("GUI: %s cannot be opened"), pFileName);
        return;
    }

    guiSnapshot = 1;
    guiSendBmpHeader();

    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL); /* Will call our disp_drv.disp_flush function */
    guiSnapshot = 0;

    pFileOut.close();
    Log.verbose(F("[Display] data flushed to %s"), pFileName);
}

#if defined(ARDUINO_ARCH_ESP8266)
void guiTakeScreenshot(ESP8266WebServer & client)
#endif
#if defined(ARDUINO_ARCH_ESP32)
    void guiTakeScreenshot(WebServer & client)
#endif // ESP32{
{
    webClient = &client;

    guiSnapshot = 2;
    webClient->setContentLength(122 + 320 * 240 * 2);
    webClient->send(200, PSTR("image/bmp"), "");

    guiSendBmpHeader();

    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL); /* Will call our disp_drv.disp_flush function */
    guiSnapshot = 0;

    Log.verbose(F("GUI: Bitmap data flushed to webclient"));
}
