/**
 * @file hasp.h
 *
 */

#ifndef HASP_H
#define HASP_H

#include <Arduino.h>
#include "lvgl.h"
#include "hasp_conf.h"
#include "hasp_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if HASP_USE_APP > 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

enum hasp_event_t { // even = released, odd = pressed
    HASP_EVENT_OFF  = 0,
    HASP_EVENT_ON   = 1,
    HASP_EVENT_UP   = 2,
    HASP_EVENT_DOWN = 3,

    HASP_EVENT_SHORT  = 4,
    HASP_EVENT_LONG   = 5,
    HASP_EVENT_LOST   = 6,
    HASP_EVENT_HOLD   = 7,
    HASP_EVENT_DOUBLE = 8
};

enum lv_hasp_obj_type_t {
    LV_HASP_BUTTON   = 10,
    LV_HASP_CHECKBOX = 11,
    LV_HASP_LABEL    = 12,

    LV_HASP_CPICKER   = 20,
    LV_HASP_PRELOADER = 21,
    LV_HASP_ARC       = 22,

    LV_HASP_SLIDER = 30,
    LV_HASP_GAUGE  = 31,
    LV_HASP_BAR    = 32,
    LV_HASP_LMETER = 33,

    LV_HASP_SWITCH = 40,
    LV_HASP_LED    = 41,

    LV_HASP_DDLIST = 50,
    LV_HASP_ROLLER = 51,

    LV_HASP_IMAGE = 60,

    LV_HASP_TABVIEW  = 70,
    LV_HASP_TILEVIEW = 71,

    LV_HASP_CONTAINER = 90,
    LV_HASP_OBJECT    = 91,
    LV_HASP_PAGE      = 92,
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a hasp application
 */
void haspSetup();
void haspLoop(void);

void haspSetPage(uint8_t id);
uint8_t haspGetPage();
void haspClearPage(uint16_t pageid);
String haspGetNodename();
String haspGetVersion();
void haspBackground(uint16_t pageid, uint16_t imageid);

void hasp_send_obj_attribute_str(lv_obj_t * obj, const char * attribute, const char * data);
void hasp_send_obj_attribute_int(lv_obj_t * obj, const char * attribute, int32_t val);
void hasp_send_obj_attribute_color(lv_obj_t * obj, const char * attribute, lv_color_t color);
void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char * attr, const char * payload);
void hasp_set_group_objects(uint8_t groupid, uint8_t eventid, lv_obj_t * src_obj);

void haspNewObject(const JsonObject & config, uint8_t & saved_page_id);

void haspReconnect(void);
void haspDisconnect(void);
void haspWakeUp(void);
void haspProgressVal(uint8_t val);

bool haspGetConfig(const JsonObject & settings);
bool haspSetConfig(const JsonObject & settings);

lv_obj_t * hasp_find_obj_from_id(lv_obj_t * parent, uint8_t objid);

void IRAM_ATTR btn_event_handler(lv_obj_t * obj, lv_event_t event);
void IRAM_ATTR toggle_event_handler(lv_obj_t * obj, lv_event_t event);

/**********************
 *      MACROS
 **********************/

#endif /*HASP_USE_APP*/

#ifdef __cplusplus
} /* extern "C" */
#endif

void haspProgressMsg(const char * msg);
void haspProgressMsg(const __FlashStringHelper * msg);

#endif /*HASP_H*/
