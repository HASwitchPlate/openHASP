/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_EVENT_H
#define HASP_EVENT_H

#include "lvgl.h"
#include "hasp_conf.h"

#define HASP_NUM_PAGE_PREV (HASP_NUM_PAGES + 1)
#define HASP_NUM_PAGE_BACK (HASP_NUM_PAGES + 2)
#define HASP_NUM_PAGE_NEXT (HASP_NUM_PAGES + 3)

// Timer event Handlers
void event_timer_calendar(lv_task_t* task);
void event_timer_clock(lv_task_t* task);

// Object event Handlers
void delete_event_handler(lv_obj_t* obj, lv_event_t event);
void first_touch_event_handler(lv_obj_t* obj, lv_event_t event);
void generic_event_handler(lv_obj_t* obj, lv_event_t event);
void toggle_event_handler(lv_obj_t* obj, lv_event_t event);
void slider_event_handler(lv_obj_t* obj, lv_event_t event);
void selector_event_handler(lv_obj_t* obj, lv_event_t event);
void btnmatrix_event_handler(lv_obj_t* obj, lv_event_t event);
void msgbox_event_handler(lv_obj_t* obj, lv_event_t event);
void cpicker_event_handler(lv_obj_t* obj, lv_event_t event);
void calendar_event_handler(lv_obj_t* obj, lv_event_t event);
void textarea_event_handler(lv_obj_t* obj, lv_event_t event);
void alarm_event_handler(lv_obj_t* obj, lv_event_t event);

// Other functions
void event_reset_last_value_sent();

#endif // HASP_EVENT_H