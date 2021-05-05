/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
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

// Object event Handlers
void IRAM_ATTR wakeup_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR generic_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR toggle_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR slider_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR selector_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR btnmatrix_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR msgbox_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR cpicker_event_handler(lv_obj_t* obj, lv_event_t event);
void IRAM_ATTR calendar_event_handler(lv_obj_t* obj, lv_event_t event);

#if HASP_USE_GPIO > 0
// GPIO event Handler
void IRAM_ATTR event_gpio_input(uint8_t pin, uint8_t group, uint8_t eventid);
#endif

#endif // HASP_EVENT_H