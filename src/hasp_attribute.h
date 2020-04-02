#ifndef HASP_ATTR_SET_H
#define HASP_ATTR_SET_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void hasp_process_obj_attribute(lv_obj_t * obj, const char * attr_p, const char * payload, bool update);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
