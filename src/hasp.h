/**
 * @file hasp.h
 *
 */

#ifndef HASP_H
#define HASP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <Arduino.h>
#include "lvgl.h"

#include "hasp_conf.h"

#include "hasp_debug.h"

/* #ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#include "hasp_conf.h"
#else
#include "../lvgl/lvgl.h"
#include "hasp_conf.h"
#endif */

#if HASP_USE_APP

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

enum lv_hasp_obj_type_t {
    LV_HASP_BUTTON    = 10,
    LV_HASP_CHECKBOX  = 11,
    LV_HASP_LABEL     = 12,
    LV_HASP_CONTAINER = 13,

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
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a hasp application
 */
void haspSetup(JsonObject settings);
void haspLoop(void);
void haspFirstSetup(void);

void haspSetPage(uint16_t id);
uint16_t haspGetPage();
void haspSetNodename(String name);
String haspGetNodename();
String haspGetVersion();
void haspBackground(uint16_t pageid, uint16_t imageid);

void haspProcessAttribute(uint8_t pageid, uint8_t objid, String strAttr, String strPayload);
void haspSendCmd(String nextionCmd);
void haspParseJson(String & strPayload);

void haspReconnect(void);
void haspDisconnect(void);
void haspDisplayAP(const char * ssid, const char * pass);
void haspWakeUp(void);

bool haspGetConfig(const JsonObject & settings);
bool haspSetConfig(const JsonObject & settings);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DEMO*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEMO_H*/
