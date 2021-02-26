/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include "ArduinoJson.h"
#include "lvgl.h"

struct dispatch_conf_t
{
    uint16_t teleperiod;
};

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
void dispatchLoop(void);
void dispatchEverySecond(void);
void dispatchStart(void);
void dispatchStop(void);

/* ===== Special Event Processors ===== */
void dispatch_topic_payload(const char* topic, const char* payload);
void dispatch_text_line(const char* cmnd);

#ifdef ARDUINO
void dispatch_parse_jsonl(Stream& stream);
#else
void dispatch_parse_jsonl(std::istringstream& stream);
#endif

void dispatch_clear_page(const char* page);
void dispatch_json_error(uint8_t tag, DeserializationError& jsonError);

// void dispatchPage(uint8_t page);
void dispatch_page_next();
void dispatch_page_prev();

void dispatch_dim(const char* level);
void dispatch_backlight(const char* payload);

void dispatch_web_update(const char* espOtaUrl);
void dispatch_reboot(bool saveConfig);

void dispatch_output_idle_state(uint8_t state);
void dispatch_output_statusupdate(const char*, const char*);
void dispatch_current_state();

void dispatch_gpio_input_event(uint8_t pin, uint8_t group, uint8_t eventid);
void dispatch_object_event(lv_obj_t* obj, uint8_t eventid);
bool dispatch_get_event_state(uint8_t eventid);
void dispatch_get_event_name(uint8_t eventid, char* buffer, size_t size);
void dispatch_object_value_changed(lv_obj_t* obj, int16_t state);

void dispatch_normalized_group_value(uint8_t groupid, uint16_t value, lv_obj_t* obj);

void dispatch_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char* attribute, const char* data);
void dispatch_send_obj_attribute_int(uint8_t pageid, uint8_t btnid, const char* attribute, int32_t val);
void dispatch_send_obj_attribute_color(uint8_t pageid, uint8_t btnid, const char* attribute, uint8_t r, uint8_t g,
                                       uint8_t b);

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */

/* ===== Structs and Constants ===== */
struct haspCommand_t
{
    const char* p_cmdstr;
    void (*func)(const char*, const char*);
};

#endif