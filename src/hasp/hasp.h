/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_H
#define HASP_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "hasplib.h"

#ifndef HASP_START_DIM
#define HASP_START_DIM 255
#endif

#ifndef HASP_START_PAGE
#define HASP_START_PAGE 1
#endif

#ifndef HASP_THEME_ID
#define HASP_THEME_ID 2
#endif

#if HASP_USE_DEBUG > 0
#include "../hasp_debug.h"
#include "dev/device.h"
#endif

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
#define HASP_SLEEP_LAST 3

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
IRAM_ATTR void haspLoop(void);
void haspEverySecond(void);

void haspReconnect(void);
void haspDisconnect(void);

// void haspBackground(uint16_t pageid, uint16_t imageid);

// void haspNewObject(const JsonObject & config, uint8_t & saved_page_id);

void haspProgressVal(uint8_t val);

#if HASP_USE_CONFIG > 0
bool haspGetConfig(const JsonObject& settings);
bool haspSetConfig(const JsonObject& settings);
#endif

lv_font_t* hasp_get_font(uint8_t fontid);

HASP_ATTRIBUTE_FAST_MEM void hasp_update_sleep_state();
void hasp_get_sleep_payload(uint8_t state, char* payload);
uint8_t hasp_get_sleep_state();
void hasp_set_sleep_state(uint8_t state);
void hasp_get_sleep_time(uint16_t& short_time, uint16_t& long_time);
void hasp_set_sleep_time(uint16_t short_time, uint16_t long_time);
void hasp_set_sleep_offset(uint32_t offset);
void hasp_set_wakeup_touch(bool en);
void hasp_set_antiburn(int32_t repeat_count, uint32_t period);
bool hasp_stop_antiburn();
hasp_event_t hasp_get_antiburn();

void hasp_init(void);
void hasp_load_json(void);

void hasp_get_info(JsonDocument& info);
void hasp_set_theme(uint8_t themeid);

/**********************
 *      MACROS
 **********************/

#endif /*HASP_USE_APP*/

#ifdef __cplusplus
} /* extern "C" */
#endif

void haspProgressMsg(const char* msg);
#ifdef ARDUINO
void haspProgressMsg(const __FlashStringHelper* msg);
#endif

#endif /*HASP_H*/
