#ifndef HASP_ATTR_SET_H
#define HASP_ATTR_SET_H

#include "Arduino.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void haspSetObjAttribute(lv_obj_t * obj, String strAttr, String strPayload);
void haspSetOpacity(lv_obj_t * obj, uint8_t val);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
