/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_EVENT_H
#define HASP_EVENT_H

#include "lvgl.h"
#include "hasp_conf.h"

#define HASP_NUM_PAGE_PREV (HASP_NUM_PAGES + 1)
#define HASP_NUM_PAGE_BACK (HASP_NUM_PAGES + 2)
#define HASP_NUM_PAGE_NEXT (HASP_NUM_PAGES + 3)

#define lv_task_t lv_timer_t

// Timer event Handlers
void event_timer_calendar(lv_task_t* task);
void event_timer_clock(lv_task_t* task);

// Object event Handlers
void delete_event_handler(lv_event_t* e);
void wakeup_event_handler(lv_event_t* e);
void generic_event_handler(lv_event_t* e);
void toggle_event_handler(lv_event_t* e);
void slider_event_handler(lv_event_t* e);
void selector_event_handler(lv_event_t* e);
void btnmatrix_event_handler(lv_event_t* e);
void msgbox_event_handler(lv_event_t* e);
void cpicker_event_handler(lv_event_t* e);
void calendar_event_handler(lv_event_t* e);

#endif // HASP_EVENT_H