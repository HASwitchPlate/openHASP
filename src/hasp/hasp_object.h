/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_OBJECT_H
#define HASP_OBJECT_H

#include <ArduinoJson.h>
#include "lvgl.h"

const char FP_PAGE[] PROGMEM     = "page";
const char FP_ID[] PROGMEM       = "id";
const char FP_OBJ[] PROGMEM      = "obj";
const char FP_OBJID[] PROGMEM    = "objid";
const char FP_PARENTID[] PROGMEM = "parentid";
const char FP_GROUPID[] PROGMEM  = "groupid";

enum lv_hasp_obj_type_t {
    /* Controls */
    LV_HASP_OBJECT    = 91, // 10
    LV_HASP_BUTTON    = 10, // 12
    LV_HASP_BTNMATRIX = 13,
    LV_HASP_IMGBTN    = 14, // placeholder
    LV_HASP_CHECKBOX  = 11, // 15
    LV_HASP_SWITCH    = 40, // 16
    LV_HASP_SLIDER    = 30, // 17
    LV_HASP_TEXTAREA  = 18, // placeholder
    LV_HASP_SPINBOX   = 19, // placeholder
    LV_HASP_CPICKER   = 20,

    /* Selectors */
    LV_HASP_DROPDOWN = 50,
    LV_HASP_ROLLER   = 51,
    LV_HASP_LIST     = 52, // placeholder
    LV_HASP_TABLE    = 53,
    LV_HASP_CALENDER = 54,

    /* Containers */
    LV_HASP_CONTAINER = 70,
    LV_HASP_WINDOW    = 71, // placeholder
    LV_HASP_MSGBOX    = 72, // placeholder
    LV_HASP_TILEVIEW  = 73, // placeholder
    LV_HASP_TABVIEW   = 74, // placeholder
    LV_HASP_TAB       = 75, // placeholder
    LV_HASP_PAGE      = 79, // Obsolete in v8

    /* Visualizers */
    LV_HASP_LABEL   = 12, // 30
    LV_HASP_GAUGE   = 31,
    LV_HASP_BAR     = 32,
    LV_HASP_LMETER  = 33,
    LV_HASP_LED     = 41, // 34
    LV_HASP_ARC     = 22, // 35
    LV_HASP_SPINNER = 21, // 36
    LV_HASP_CHART   = 37,

    /* Graphics */
    LV_HASP_LINE   = 60,
    LV_HASP_IMAGE  = 61, // placeholder
    LV_HASP_CANVAS = 62, // placeholder
    LV_HASP_MASK   = 63, // placeholder
};

void hasp_new_object(const JsonObject & config, uint8_t & saved_page_id);

lv_obj_t * hasp_find_obj_from_parent_id(lv_obj_t * parent, uint8_t objid);
// lv_obj_t * hasp_find_obj_from_page_id(uint8_t pageid, uint8_t objid);
bool hasp_find_id_from_obj(lv_obj_t * obj, uint8_t * pageid, uint8_t * objid);
// bool check_obj_type_str(const char * lvobjtype, lv_hasp_obj_type_t haspobjtype);
bool check_obj_type(lv_obj_t * obj, lv_hasp_obj_type_t haspobjtype);
void hasp_object_tree(lv_obj_t * parent, uint8_t pageid, uint16_t level);
void hasp_object_delete(lv_obj_t * obj);

void hasp_send_obj_attribute_str(lv_obj_t * obj, const char * attribute, const char * data);
void hasp_send_obj_attribute_int(lv_obj_t * obj, const char * attribute, int32_t val);
void hasp_send_obj_attribute_color(lv_obj_t * obj, const char * attribute, lv_color_t color);
void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char * attr, const char * payload);

void object_set_group_state(uint8_t groupid, uint8_t eventid, lv_obj_t * src_obj);

void generic_event_handler(lv_obj_t * obj, lv_event_t event);
void toggle_event_handler(lv_obj_t * obj, lv_event_t event);
void slider_event_handler(lv_obj_t * obj, lv_event_t event);
void wakeup_event_handler(lv_obj_t * obj, lv_event_t event);

#define HASP_OBJ_BAR 1971
#define HASP_OBJ_BTN 3164
#define HASP_OBJ_CPICKER 3313
#define HASP_OBJ_CHECKBOX 1923
#define HASP_OBJ_SPINNER 7097
#define HASP_OBJ_MSGBOX 7498
#define HASP_OBJ_TABLE 12078
#define HASP_OBJ_ROLLER 13258
#define HASP_OBJ_LABEL 13684
#define HASP_OBJ_KEYBOARD 14343
#define HASP_OBJ_PAGE 19759
#define HASP_OBJ_WIN 20284
#define HASP_OBJ_TEXTAREA 24186
#define HASP_OBJ_IMGBTN 24441
#define HASP_OBJ_SPINBOX 25641
#define HASP_OBJ_CALENDAR 30334
#define HASP_OBJ_IMG 30499
#define HASP_OBJ_GAUGE 33145
#define HASP_OBJ_CHART 34654
#define HASP_OBJ_LINE 34804
#define HASP_OBJ_LIST 35134
#define HASP_OBJ_SLIDER 35265
#define HASP_OBJ_CANVAS 35480
#define HASP_OBJ_TILEVIEW 36019
#define HASP_OBJ_CONT 36434
#define HASP_OBJ_SWITCH 38484
#define HASP_OBJ_LED 41899
#define HASP_OBJ_DROPDOWN 49169
#define HASP_OBJ_BTNMATRIX 49629
#define HASP_OBJ_OBJ 53623
#define HASP_OBJ_OBJMASK 55395
#define HASP_OBJ_LMETER 62749
#define HASP_OBJ_TABVIEW 63226
#define HASP_OBJ_ARC 64594

#endif