/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include "hasplib.h"
// #include "freertos/queue.h"

// QueueHandle_t message_queue;
// typedef struct
// {
//     char* topic;   //[64];
//     char* payload; //[512];
//     uint source;
// } dispatch_message_t;

struct dispatch_conf_t
{
    uint16_t teleperiod;
};

struct moodlight_t
{
    uint8_t brightness;
    uint8_t power;
    uint8_t rgbww[5];
};

enum hasp_event_t { // even = released, odd = pressed
    HASP_EVENT_OFF     = 0,
    HASP_EVENT_ON      = 1,
    HASP_EVENT_UP      = 2,
    HASP_EVENT_DOWN    = 3,
    HASP_EVENT_RELEASE = 4,
    HASP_EVENT_HOLD    = 5,
    HASP_EVENT_LONG    = 6,
    HASP_EVENT_LOST    = 7,
    HASP_EVENT_DOUBLE  = 8,

    HASP_EVENT_OPEN    = 10,
    HASP_EVENT_OPENING = 11,
    HASP_EVENT_CLOSED  = 12,
    HASP_EVENT_CLOSING = 13,
    HASP_EVENT_STOP    = 14,

    HASP_EVENT_CHANGED = 32
};

/* ===== Default Event Processors ===== */
void dispatchSetup(void);
IRAM_ATTR void dispatchLoop(void);
void dispatchEverySecond(void);
void dispatchStart(void);
void dispatchStop(void);

/* ===== Special Event Processors ===== */
void dispatch_topic_payload(const char* topic, const char* payload, bool update, uint8_t source);
void dispatch_text_line(const char* cmnd, uint8_t source);

#ifdef ARDUINO
void dispatch_parse_jsonl(Stream& stream, uint8_t& saved_page_id);
#else
void dispatch_parse_jsonl(std::istream& stream, uint8_t& saved_page_id);
#endif
bool dispatch_json_variant(JsonVariant& json, uint8_t& savedPage, uint8_t source);

void dispatch_clear_page(const char* page);
void dispatch_json_error(uint8_t tag, DeserializationError& jsonError);

void dispatch_set_page(uint8_t pageid, lv_scr_load_anim_t animation, uint32_t time, uint32_t delay);
void dispatch_page_next(lv_scr_load_anim_t effectid);
void dispatch_page_prev(lv_scr_load_anim_t effectid);
void dispatch_page_back(lv_scr_load_anim_t effectid);
void dispatch_page(const char*, const char* payload, uint8_t source);

bool dispatch_factory_reset();
void dispatch_reboot(bool saveConfig);
void dispatch_current_state(uint8_t source);
void dispatch_current_page();
void dispatch_backlight(const char*, const char* payload, uint8_t source);
void dispatch_web_update(const char*, const char* espOtaUrl, uint8_t source);
void dispatch_statusupdate(const char*, const char*, uint8_t source);
void dispatch_send_discovery(const char*, const char*, uint8_t source);
void dispatch_send_sensordata(const char*, const char*, uint8_t source);
// void dispatch_idle(const char*, const char*, uint8_t source);
void dispatch_idle_state(uint8_t state);
void dispatch_calibrate(const char*, const char*, uint8_t source);
void dispatch_antiburn(const char*, const char* payload, uint8_t source);
void dispatch_wakeup(uint8_t source);
void dispatch_run_script(const char*, const char* payload, uint8_t source);
void dispatch_config(const char* topic, const char* payload, uint8_t source);

void dispatch_normalized_group_values(hasp_update_value_t& value);

void dispatch_state_subtopic(const char* subtopic, const char* payload);
void dispatch_state_eventid(const char* topic, hasp_event_t eventid);
void dispatch_state_brightness(const char* topic, hasp_event_t eventid, int32_t val);
void dispatch_state_val(const char* topic, hasp_event_t eventid, int32_t val);
void dispatch_state_antiburn(hasp_event_t eventid);

/* ===== Getter and Setter Functions ===== */
void dispatch_get_discovery_data(JsonDocument& doc);

/* ===== Read/Write Configuration ===== */

/* ===== Structs and Constants ===== */
struct haspCommand_t
{
    const char* p_cmdstr;
    void (*func)(const char*, const char*, uint8_t);
};

#endif