/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include "ArduinoJson.h"
#include "lvgl.h"

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

/* ===== Default Event Processors ===== */
void dispatchSetup(void);
void IRAM_ATTR dispatchLoop(void);
void dispatchEverySecond(void);
void dispatchStart(void);
void dispatchStop(void);

/* ===== Special Event Processors ===== */
void dispatch_topic_payload(const char * topic, const char * payload);
void dispatch_text_line(const char * cmnd);
void dispatch_parse_jsonl(Stream & stream);
void dispatch_clear_page(const char * page);

// void dispatchPage(uint8_t page);
void dispatch_page_next();
void dispatch_page_prev();

void dispatch_dim(const char * level);
void dispatch_backlight(const char * payload);

void dispatch_web_update(const char * espOtaUrl);
void dispatch_reboot(bool saveConfig);

void dispatch_output_idle_state(uint8_t state);
void dispatch_output_statusupdate(const char *, const char *);
void dispatch_output_current_page();

void dispatch_gpio_input_event(uint8_t pin, uint8_t group, uint8_t eventid);
void dispatch_object_event(lv_obj_t * obj, uint8_t eventid);
bool dispatch_get_event_state(uint8_t eventid);
void dispatch_object_value_changed(lv_obj_t * obj, int16_t state);

void dispatch_normalized_group_value(uint8_t groupid, uint16_t value, lv_obj_t * obj);

void IRAM_ATTR dispatch_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute,
                                               const char * data);

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */

/* ===== Structs and Constants ===== */
struct haspCommand_t
{
    void (*func)(const char *, const char *);
    const char * p_cmdstr;
};

#endif