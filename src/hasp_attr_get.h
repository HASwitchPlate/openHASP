#ifndef HASP_ATTR_GET_H
#define HASP_ATTR_GET_H

#include "Arduino.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

bool haspGetObjAttribute(lv_obj_t * obj, String strAttr, std::string & strPayload);
uint32_t get_cpicker_value(lv_obj_t * obj);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
