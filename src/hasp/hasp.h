/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_H
#define HASP_H

#include <Arduino.h>
#include "lvgl.h"
#include "hasp_conf.h"

#include "../hasp_debug.h"

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
#define HASP_SLEEP_OFF 0
#define HASP_SLEEP_SHORT 1
#define HASP_SLEEP_LONG 2

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a hasp application
 */
void haspSetup(void);
void IRAM_ATTR haspLoop(void);
//void haspEverySecond(void); // See MACROS

void haspReconnect(void);
void haspDisconnect(void);

lv_obj_t * get_page_obj(uint8_t pageid);
bool get_page_id(lv_obj_t * obj, uint8_t * pageid);

void haspSetPage(uint8_t id);
uint8_t haspGetPage();
void haspClearPage(uint16_t pageid);

void haspGetVersion(char* version,size_t len);
//void haspBackground(uint16_t pageid, uint16_t imageid);

// void haspNewObject(const JsonObject & config, uint8_t & saved_page_id);

void haspProgressVal(uint8_t val);

bool haspGetConfig(const JsonObject & settings);
bool haspSetConfig(const JsonObject & settings);

lv_font_t * hasp_get_font(uint8_t fontid);

bool IRAM_ATTR hasp_update_sleep_state();
void hasp_wakeup(void);

/**********************
 *      MACROS
 **********************/
#define haspEverySecond hasp_update_sleep_state

#endif /*HASP_USE_APP*/

#ifdef __cplusplus
} /* extern "C" */
#endif

void haspProgressMsg(const char * msg);
void haspProgressMsg(const __FlashStringHelper * msg);

#endif /*HASP_H*/
