#ifndef HASP_ATTR_SET_H
#define HASP_ATTR_SET_H

#include "Arduino.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void hasp_set_obj_attribute(lv_obj_t * obj, const char * attr_p, const char * payload);
void haspSetOpacity(lv_obj_t * obj, uint8_t val);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
