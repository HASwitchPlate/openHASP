/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a hasp application
 */
void haspSetup();
void IRAM_ATTR haspLoop(void);
void haspReconnect(void);
void haspDisconnect(void);

lv_obj_t * get_page_obj(uint8_t pageid);
bool get_page_id(lv_obj_t * obj, uint8_t * pageid);

void haspSetPage(uint8_t id);
uint8_t haspGetPage();
void haspClearPage(uint16_t pageid);

String haspGetNodename();
void haspGetVersion(char* version,size_t len);
void haspBackground(uint16_t pageid, uint16_t imageid);

void hasp_set_group_objects(uint8_t groupid, uint8_t eventid, lv_obj_t * src_obj);

// void haspNewObject(const JsonObject & config, uint8_t & saved_page_id);

void haspWakeUp(void);
void haspProgressVal(uint8_t val);

bool haspGetConfig(const JsonObject & settings);
bool haspSetConfig(const JsonObject & settings);

lv_font_t * hasp_get_font(uint8_t fontid);

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
