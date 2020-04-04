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

#if HASP_USE_APP

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

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

    LV_HASP_CONTAINER = 90,
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
void haspSetNodename(String name);
String haspGetNodename();
String haspGetVersion();
void haspBackground(uint16_t pageid, uint16_t imageid);

void hasp_send_obj_attribute_str(lv_obj_t * obj, const char * attribute, const char * data);
void hasp_send_obj_attribute_int(lv_obj_t * obj, const char * attribute, int32_t val);
void hasp_send_obj_attribute_color(lv_obj_t * obj, const char * attribute, lv_color_t color);
void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char * attr, const char * payload);

void haspSendCmd(String nextionCmd);
void haspParseJson(String & strPayload);
void haspNewObject(const JsonObject & config, uint8_t & saved_page_id);

void haspReconnect(void);
void haspDisconnect(void);
void haspWakeUp(void);

bool haspGetConfig(const JsonObject & settings);
bool haspSetConfig(const JsonObject & settings);

bool check_obj_type(const char * lvobjtype, lv_hasp_obj_type_t haspobjtype);

void IRAM_ATTR btn_event_handler(lv_obj_t * obj, lv_event_t event);
void IRAM_ATTR toggle_event_handler(lv_obj_t * obj, lv_event_t event);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DEMO*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEMO_H*/
