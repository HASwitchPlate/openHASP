/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#include "lvgl.h"
#if LVGL_VERSION_MAJOR != 7
#include "../lv_components.h"
#endif

#include "hasplib.h"

LV_FONT_DECLARE(unscii_8_icon);
extern const char** btnmatrix_default_map; // memory pointer to lvgl default btnmatrix map

#if 0
static bool attribute_lookup_lv_property(uint16_t hash, uint8_t * prop)
{
    struct prop_hash_map
    {
        uint16_t hash;
        uint8_t  prop;
    };

    /* in order of prevalence */
    prop_hash_map props[] = {
        {ATTR_PAD_TOP, LV_STYLE_PAD_TOP & LV_STYLE_PROP_ALL},
        {ATTR_BORDER_WIDTH, LV_STYLE_BORDER_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_OUTLINE_WIDTH, LV_STYLE_OUTLINE_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_LETTER_SPACE, LV_STYLE_VALUE_LETTER_SPACE & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_LETTER_SPACE, LV_STYLE_TEXT_LETTER_SPACE & LV_STYLE_PROP_ALL},
        {ATTR_LINE_WIDTH, LV_STYLE_LINE_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_TIME, LV_STYLE_TRANSITION_TIME & LV_STYLE_PROP_ALL},
        {ATTR_SCALE_WIDTH, LV_STYLE_SCALE_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_RADIUS, LV_STYLE_RADIUS & LV_STYLE_PROP_ALL},
        {ATTR_PAD_BOTTOM, LV_STYLE_PAD_BOTTOM & LV_STYLE_PROP_ALL},
        {ATTR_BG_MAIN_STOP, LV_STYLE_BG_MAIN_STOP & LV_STYLE_PROP_ALL},
        {ATTR_BORDER_SIDE, LV_STYLE_BORDER_SIDE & LV_STYLE_PROP_ALL},
        {ATTR_OUTLINE_PAD, LV_STYLE_OUTLINE_PAD & LV_STYLE_PROP_ALL},
        {ATTR_PATTERN_REPEAT, LV_STYLE_PATTERN_REPEAT & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_LINE_SPACE, LV_STYLE_VALUE_LINE_SPACE & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_LINE_SPACE, LV_STYLE_TEXT_LINE_SPACE & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_DELAY, LV_STYLE_TRANSITION_DELAY & LV_STYLE_PROP_ALL},
        {ATTR_SCALE_BORDER_WIDTH, LV_STYLE_SCALE_BORDER_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_CLIP_CORNER, LV_STYLE_CLIP_CORNER & LV_STYLE_PROP_ALL},
        {ATTR_PAD_LEFT, LV_STYLE_PAD_LEFT & LV_STYLE_PROP_ALL},
        {ATTR_BG_GRAD_STOP, LV_STYLE_BG_GRAD_STOP & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_DECOR, LV_STYLE_TEXT_DECOR & LV_STYLE_PROP_ALL},
        {ATTR_LINE_DASH_WIDTH, LV_STYLE_LINE_DASH_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PROP_1, LV_STYLE_TRANSITION_PROP_1 & LV_STYLE_PROP_ALL},
        {ATTR_SCALE_END_BORDER_WIDTH, LV_STYLE_SCALE_END_BORDER_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_SIZE, LV_STYLE_SIZE & LV_STYLE_PROP_ALL},
        {ATTR_PAD_RIGHT, LV_STYLE_PAD_RIGHT & LV_STYLE_PROP_ALL},
        {ATTR_BG_GRAD_DIR, LV_STYLE_BG_GRAD_DIR & LV_STYLE_PROP_ALL},
        {ATTR_BORDER_POST, LV_STYLE_BORDER_POST & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_OFS_X, LV_STYLE_VALUE_OFS_X & LV_STYLE_PROP_ALL},
        {ATTR_LINE_DASH_GAP, LV_STYLE_LINE_DASH_GAP & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PROP_2, LV_STYLE_TRANSITION_PROP_2 & LV_STYLE_PROP_ALL},
        {ATTR_SCALE_END_LINE_WIDTH, LV_STYLE_SCALE_END_LINE_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_TRANSFORM_WIDTH, LV_STYLE_TRANSFORM_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_PAD_INNER, LV_STYLE_PAD_INNER & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_OFS_Y, LV_STYLE_VALUE_OFS_Y & LV_STYLE_PROP_ALL},
        {ATTR_LINE_ROUNDED, LV_STYLE_LINE_ROUNDED & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PROP_3, LV_STYLE_TRANSITION_PROP_3 & LV_STYLE_PROP_ALL},
        {ATTR_TRANSFORM_HEIGHT, LV_STYLE_TRANSFORM_HEIGHT & LV_STYLE_PROP_ALL},
        // {ATTR_MARGIN_TOP, LV_STYLE_MARGIN_TOP & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_ALIGN, LV_STYLE_VALUE_ALIGN & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PROP_4, LV_STYLE_TRANSITION_PROP_4 & LV_STYLE_PROP_ALL},
        // {ATTR_TRANSFORM_ANGLE, LV_STYLE_TRANSFORM_ANGLE & LV_STYLE_PROP_ALL},
        // {ATTR_MARGIN_BOTTOM, LV_STYLE_MARGIN_BOTTOM & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PROP_5, LV_STYLE_TRANSITION_PROP_5 & LV_STYLE_PROP_ALL},
        // {ATTR_TRANSFORM_ZOOM, LV_STYLE_TRANSFORM_ZOOM & LV_STYLE_PROP_ALL},
        // {ATTR_MARGIN_LEFT, LV_STYLE_MARGIN_LEFT & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PROP_6, LV_STYLE_TRANSITION_PROP_6 & LV_STYLE_PROP_ALL},
        // {ATTR_MARGIN_RIGHT, LV_STYLE_MARGIN_RIGHT & LV_STYLE_PROP_ALL},
        {ATTR_BG_COLOR, LV_STYLE_BG_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_BORDER_COLOR, LV_STYLE_BORDER_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_OUTLINE_COLOR, LV_STYLE_OUTLINE_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_PATTERN_RECOLOR, LV_STYLE_PATTERN_RECOLOR & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_COLOR, LV_STYLE_VALUE_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_COLOR, LV_STYLE_TEXT_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_LINE_COLOR, LV_STYLE_LINE_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_IMAGE_RECOLOR, LV_STYLE_IMAGE_RECOLOR & LV_STYLE_PROP_ALL},
        {ATTR_SCALE_GRAD_COLOR, LV_STYLE_SCALE_GRAD_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_BG_GRAD_COLOR, LV_STYLE_BG_GRAD_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_SEL_COLOR, LV_STYLE_TEXT_SEL_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_SCALE_END_COLOR, LV_STYLE_SCALE_END_COLOR & LV_STYLE_PROP_ALL},
        // {ATTR_TEXT_SEL_BG_COLOR, LV_STYLE_TEXT_SEL_BG_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_OPA_SCALE, LV_STYLE_OPA_SCALE & LV_STYLE_PROP_ALL},
        {ATTR_BG_OPA, LV_STYLE_BG_OPA & LV_STYLE_PROP_ALL},
        {ATTR_BORDER_OPA, LV_STYLE_BORDER_OPA & LV_STYLE_PROP_ALL},
        {ATTR_OUTLINE_OPA, LV_STYLE_OUTLINE_OPA & LV_STYLE_PROP_ALL},
        {ATTR_PATTERN_OPA, LV_STYLE_PATTERN_OPA & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_OPA, LV_STYLE_VALUE_OPA & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_OPA, LV_STYLE_TEXT_OPA & LV_STYLE_PROP_ALL},
        {ATTR_LINE_OPA, LV_STYLE_LINE_OPA & LV_STYLE_PROP_ALL},
        {ATTR_IMAGE_OPA, LV_STYLE_IMAGE_OPA & LV_STYLE_PROP_ALL},
        {ATTR_PATTERN_RECOLOR_OPA, LV_STYLE_PATTERN_RECOLOR_OPA & LV_STYLE_PROP_ALL},
        {ATTR_IMAGE_RECOLOR_OPA, LV_STYLE_IMAGE_RECOLOR_OPA & LV_STYLE_PROP_ALL},
        {ATTR_PATTERN_IMAGE, LV_STYLE_PATTERN_IMAGE & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_FONT, LV_STYLE_VALUE_FONT & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_FONT, LV_STYLE_TEXT_FONT & LV_STYLE_PROP_ALL},
        {ATTR_TRANSITION_PATH, LV_STYLE_TRANSITION_PATH & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_STR, LV_STYLE_VALUE_STR & LV_STYLE_PROP_ALL},

#if LV_USE_SHADOW
        {ATTR_SHADOW_WIDTH, LV_STYLE_SHADOW_WIDTH & LV_STYLE_PROP_ALL},
        {ATTR_SHADOW_OFS_X, LV_STYLE_SHADOW_OFS_X & LV_STYLE_PROP_ALL},
        {ATTR_SHADOW_OFS_Y, LV_STYLE_SHADOW_OFS_Y & LV_STYLE_PROP_ALL},
        {ATTR_SHADOW_SPREAD, LV_STYLE_SHADOW_SPREAD & LV_STYLE_PROP_ALL},
        {ATTR_SHADOW_COLOR, LV_STYLE_SHADOW_COLOR & LV_STYLE_PROP_ALL},
        {ATTR_SHADOW_OPA, LV_STYLE_SHADOW_OPA & LV_STYLE_PROP_ALL},
#endif

#if LV_USE_BLEND_MODES && LV_USE_SHADOW
        {ATTR_SHADOW_BLEND_MODE, LV_STYLE_SHADOW_BLEND_MODE & LV_STYLE_PROP_ALL},
#endif

#if LV_USE_BLEND_MODES
        {ATTR_BG_BLEND_MODE, LV_STYLE_BG_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_PATTERN_BLEND_MODE, LV_STYLE_PATTERN_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_IMAGE_BLEND_MODE, LV_STYLE_IMAGE_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_LINE_BLEND_MODE, LV_STYLE_LINE_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_BORDER_BLEND_MODE, LV_STYLE_BORDER_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_OUTLINE_BLEND_MODE, LV_STYLE_OUTLINE_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_VALUE_BLEND_MODE, LV_STYLE_VALUE_BLEND_MODE & LV_STYLE_PROP_ALL},
        {ATTR_TEXT_BLEND_MODE, LV_STYLE_TEXT_BLEND_MODE & LV_STYLE_PROP_ALL},
#endif
    };

    for(uint32_t i = 0; i < sizeof(props) / sizeof(props[0]); i++) {
        if(props[i].hash == hash) {
            *prop = props[1].prop;
            LOG_WARNING(TAG_ATTR, F("%d found and has propery %d"), hash, props[i].prop);
            return true;
        }
    }
    LOG_ERROR(TAG_ATTR, F("%d has no property id"), hash);
    return false;
}

static bool attribute_get_lv_property()
{
    lv_res_t res _lv_style_get_int(const lv_style_t * style, lv_style_property_t prop, void * res);
    return res == LV_RES_OK
}

static bool attribute_set_lv_property()
{
    lv_res_t res _lv_style_get_int(const lv_style_t * style, lv_style_property_t prop, void * res);
    return res == LV_RES_OK
}

static bool attribute_update_lv_property(lv_obj_t * obj, const char * attr_p, uint16_t attr_hash, const char * payload,
                                         bool update)
{
    uint8_t prop;
    uint8_t prop_type;

    // convert sdbm hash to lv property number
    if(!attribute_lookup_lv_property(attr_hash, &prop)) return false;

    // find the parameter type for this property
    prop_type = prop & 0xF;

    if(prop_type < LV_STYLE_ID_COLOR) {
        if(update) {
            _lv_obj_set_style_local_int(obj, part, prop | (state << LV_STYLE_STATE_POS), atoi(payload))
        } else {
            attr_out_str(obj, attr_p, lv_obj_get_style_value_str(obj, part));
        }
    } else if(prop_type < LV_STYLE_ID_OPA) {
    } else if(prop_type < LV_STYLE_ID_PTR) {
    } else {
    }
}
#endif

// OK - this function is missing in lvgl
static uint8_t my_roller_get_visible_row_count(lv_obj_t* roller)
{
    const lv_font_t* font     = lv_obj_get_style_text_font(roller, LV_ROLLER_PART_BG);
    lv_style_int_t line_space = lv_obj_get_style_text_line_space(roller, LV_ROLLER_PART_BG);
    lv_coord_t h              = lv_obj_get_height(roller);

    if((lv_font_get_line_height(font) + line_space) != 0)
        return (uint8_t)(h / (lv_font_get_line_height(font) + line_space));
    else
        return 0;
}

// OK - this function is missing in lvgl
static inline int16_t my_arc_get_rotation(lv_obj_t* arc)
{
    lv_arc_ext_t* ext = (lv_arc_ext_t*)lv_obj_get_ext_attr(arc);
    return ext->rotation_angle;
}

// OK - this function is missing in lvgl
static inline int16_t my_chart_get_min_value(lv_obj_t* chart)
{
    lv_chart_ext_t* ext = (lv_chart_ext_t*)lv_obj_get_ext_attr(chart);
    return ext->ymin[LV_CHART_AXIS_PRIMARY_Y];
}

// OK - this function is missing in lvgl
static inline int16_t my_chart_get_max_value(lv_obj_t* chart)
{
    lv_chart_ext_t* ext = (lv_chart_ext_t*)lv_obj_get_ext_attr(chart);
    return ext->ymax[LV_CHART_AXIS_PRIMARY_Y];
}

lv_chart_series_t* my_chart_get_series(lv_obj_t* chart, uint8_t ser_num)
{
    lv_chart_ext_t* ext    = (lv_chart_ext_t*)lv_obj_get_ext_attr(chart);
    lv_chart_series_t* ser = (lv_chart_series_t*)_lv_ll_get_tail(&ext->series_ll);
    while(ser_num > 0 && ser) {
        ser = (lv_chart_series_t*)_lv_ll_get_prev(&ext->series_ll, ser);
        ser_num--;
    }
    return ser;
}

/**
 * Set a new value_str for an object. Memory will be allocated to store the text by the object.
 * @param obj pointer to a object
 * @param text '\0' terminated character string. NULL to refresh with the current text.
 */
void my_obj_set_value_str_txt(lv_obj_t* obj, uint8_t part, lv_state_t state, const char* text)
{
    //  LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);

    const void* value_str_p = lv_obj_get_style_value_str(obj, part);
    lv_obj_invalidate(obj);

    if(text == NULL || text[0] == 0) {
        // LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        lv_obj_set_style_local_value_str(obj, part, state, NULL);
        lv_mem_free(value_str_p);
        // LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        return;
    }

    LV_ASSERT_STR(text);

    if(value_str_p == NULL) {
        /*Get the size of the text*/
        size_t len = strlen(text) + 1;

        /*Allocate space for the new text*/
        //   LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        value_str_p = (char*)lv_mem_alloc(len);
        LV_ASSERT_MEM(value_str_p);
        if(value_str_p == NULL) return;

        // LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        strncpy((char*)value_str_p, text, len);
        lv_obj_set_style_local_value_str(obj, part, state, (char*)value_str_p);
        // LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        return;
    }

    // lv_obj_set_style_local_value_str(obj, part, state, str_p);

    if(value_str_p == text) {
        /*If set its own text then reallocate it (maybe its size changed)*/
        LOG_DEBUG(TAG_ATTR, "%s %d", __FILE__, __LINE__);
        return; // don't touch the data

        // value_str_p = lv_mem_realloc(value_str_p, strlen(text) + 1);

        // LV_ASSERT_MEM(value_str_p);
        // if(value_str_p == NULL) return;
    } else {
        /*Free the old text*/
        if(value_str_p != NULL) {
            //        LOG_DEBUG(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
            lv_mem_free(value_str_p);
            value_str_p = NULL;
            //        LOG_DEBUG(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        }

        /*Get the size of the text*/
        size_t len = strlen(text) + 1;

        /*Allocate space for the new text*/
        value_str_p = lv_mem_alloc(len);
        LV_ASSERT_MEM(value_str_p);
        if(value_str_p != NULL) strcpy((char*)value_str_p, text);
        lv_obj_set_style_local_value_str(obj, part, state, (char*)value_str_p);
    }

    // LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
}

void my_btnmatrix_map_clear(lv_obj_t* obj)
{
    lv_btnmatrix_ext_t* ext = (lv_btnmatrix_ext_t*)lv_obj_get_ext_attr(obj);
    const char** map_p_tmp  = ext->map_p; // store current pointer

    LOG_VERBOSE(TAG_ATTR, "%s %d %x   btn_cnt: %d", __FILE__, __LINE__, map_p_tmp, ext->btn_cnt);

    if(ext->map_p && (ext->btn_cnt > 0)) {

        // The map exists and is not the default lvgl map anymore
        if((map_p_tmp == NULL) || (btnmatrix_default_map == NULL) || (map_p_tmp == btnmatrix_default_map)) return;

        LOG_DEBUG(TAG_ATTR, "%s %d %x", __FILE__, __LINE__,
                  *map_p_tmp);                                          // label buffer reserved as a contiguous block
        LOG_DEBUG(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, map_p_tmp); // label pointer array block
        lv_btnmatrix_set_map(obj, btnmatrix_default_map);               // reset to default btnmap pointer

        LOG_DEBUG(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        lv_mem_free(*map_p_tmp); // free label buffer reserved as a contiguous block
        LOG_DEBUG(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
        lv_mem_free(map_p_tmp); // free label pointer array block
        LOG_DEBUG(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
    }
}

static void my_btnmatrix_map_create(lv_obj_t* obj, const char* payload)
{
    // const char** map_p = lv_btnmatrix_get_map_array(obj);

    // Create new map
    // Reserve memory for JsonDocument
    size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 256;
    DynamicJsonDocument map_doc(maxsize);
    DeserializationError jsonError = deserializeJson(map_doc, payload);

    if(jsonError) { // Couldn't parse incoming JSON payload
        dispatch_json_error(TAG_ATTR, jsonError);
        return;
    }

    JsonArray arr = map_doc.as<JsonArray>(); // Parse payload

    size_t tot_len            = sizeof(char*) * (arr.size() + 1);
    const char** map_data_str = (const char**)lv_mem_alloc(tot_len);
    if(map_data_str == NULL) {
        LOG_ERROR(TAG_ATTR, F("Out of memory while creating button map"));
        return;
    }
    memset(map_data_str, 0, tot_len);

    // Create buffer
    tot_len = 0;
    for(JsonVariant btn : arr) {
        // tot_len += btn.as<String>().length() + 1;
        tot_len += strlen(btn.as<const char*>()) + 1;
    }
    tot_len++; // trailing '\0'
    LOG_VERBOSE(TAG_ATTR, F("Array Size = %d, Map Length = %d"), arr.size(), tot_len);

    char* buffer_addr = (char*)lv_mem_alloc(tot_len);
    if(buffer_addr == NULL) {
        lv_mem_free(map_data_str);
        LOG_ERROR(TAG_ATTR, F("Out of memory while creating button map"));
        return;
    }
    memset(buffer_addr, 0, tot_len); // Important, last index needs to be 0 => empty string ""

    /* Point of no return, destroy & free the previous map */
    LOG_VERBOSE(TAG_ATTR, F("%s %d   map addr:  %x"), __FILE__, __LINE__, map_data_str);
    my_btnmatrix_map_clear(obj); // Free previous map

    // Fill buffer
    size_t index = 0;
    size_t pos   = 0;
    LOG_VERBOSE(TAG_ATTR, F("%s %d   lbl addr:  %x"), __FILE__, __LINE__, buffer_addr);
    for(JsonVariant btn : arr) {
        // size_t len = btn.as<String>().length() + 1;
        size_t len = strlen(btn.as<const char*>()) + 1;
        LOG_VERBOSE(TAG_ATTR, F(D_BULLET "Adding button: %s (%d bytes) %x"), btn.as<const char*>(), len,
                    buffer_addr + pos);
        // LOG_VERBOSE(TAG_ATTR, F(D_BULLET "Adding button: %s (%d bytes) %x"), btn.as<String>().c_str(), len,
        // buffer_addr + pos);
        memccpy(buffer_addr + pos, btn.as<const char*>(), 0, len); // Copy the label text into the buffer
        // memccpy(buffer_addr + pos, btn.as<String>().c_str(), 0, len); // Copy the label text into the buffer
        map_data_str[index++] = buffer_addr + pos; // save pointer to the label in the array
        pos += len;
    }
    map_data_str[index] = buffer_addr + pos; // save pointer to the last \0 byte

    LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
    lv_btnmatrix_set_map(obj, map_data_str);
    LOG_VERBOSE(TAG_ATTR, F("%s %d"), __FILE__, __LINE__);
}

void line_clear_points(lv_obj_t* obj)
{
    lv_line_ext_t* ext = (lv_line_ext_t*)lv_obj_get_ext_attr(obj);
    if(ext->point_array && (ext->point_num > 0)) {
        const lv_point_t* ptr = ext->point_array;
        lv_line_set_points(obj, NULL, 0);
        lv_mem_free(ptr);
    }
}

static void line_set_points(lv_obj_t* obj, const char* payload)
{
    line_clear_points(obj); // delete pointmap

    // Create new points
    // Reserve memory for JsonDocument
    size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 256;
    DynamicJsonDocument doc(maxsize);
    DeserializationError jsonError = deserializeJson(doc, payload);

    if(jsonError) { // Couldn't parse incoming JSON payload
        dispatch_json_error(TAG_ATTR, jsonError);
        return;
    }

    JsonArray arr = doc.as<JsonArray>(); // Parse payload

    size_t tot_len        = sizeof(lv_point_t*) * (arr.size());
    lv_point_t* point_arr = (lv_point_t*)lv_mem_alloc(tot_len);
    if(point_arr == NULL) {
        LOG_ERROR(TAG_ATTR, F("Out of memory while creating line points"));
        return;
    }
    memset(point_arr, 0, tot_len);

    size_t index = 0;
    for(JsonVariant v : arr) {
        JsonArray point    = v.as<JsonArray>(); // Parse point
        point_arr[index].x = point[0].as<int16_t>();
        point_arr[index].y = point[1].as<int16_t>();
        LOG_VERBOSE(TAG_ATTR, F(D_BULLET "Adding point %d: %d,%d"), index, point_arr[index].x, point_arr[index].y);
        index++;
    }

    lv_line_set_points(obj, point_arr, arr.size());

    // TO DO : free & destroy previous pointlist!
}

// OK
static inline lv_color_t haspLogColor(lv_color_t color)
{
    // uint8_t r = (LV_COLOR_GET_R(color) * 263 + 7) >> 5;
    // uint8_t g = (LV_COLOR_GET_G(color) * 259 + 3) >> 6;
    // uint8_t b = (LV_COLOR_GET_B(color) * 263 + 7) >> 5;
    // LOG_VERBOSE(TAG_ATTR,F("Color: R%u G%u B%u"), r, g, b);
    return color;
}

static lv_font_t* haspPayloadToFont(const char* payload)
{
    uint8_t var = atoi(payload);

    switch(var) {
        case 0 ... 7:
            // LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, robotocondensed_regular_12);
            return hasp_get_font(var);

        case 8:
            return &unscii_8_icon;

#ifndef ARDUINO_ARCH_ESP8266

#ifdef HASP_FONT_1
        case 12:
            return &HASP_FONT_1;
#endif

#ifdef HASP_FONT_2
        case 16:
            LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, HASP_FONT_2);
            return &HASP_FONT_2;
#endif

#ifdef HASP_FONT_3
        case 22:
            LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, HASP_FONT_3);
            return &HASP_FONT_3;
#endif

#ifdef HASP_FONT_4
        case 28:
            LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, HASP_FONT_4);
            return &HASP_FONT_4;
#endif

#endif

        default:
            return nullptr;
    }
}

static void gauge_format_10(lv_obj_t* gauge, char* buf, int bufsize, int32_t value)
{
    snprintf(buf, bufsize, PSTR("%d"), value / 10);
}

static void gauge_format_100(lv_obj_t* gauge, char* buf, int bufsize, int32_t value)
{
    snprintf(buf, bufsize, PSTR("%d"), value / 100);
}

static void gauge_format_1k(lv_obj_t* gauge, char* buf, int bufsize, int32_t value)
{
    snprintf(buf, bufsize, PSTR("%d"), value / 1000);
}

static void gauge_format_10k(lv_obj_t* gauge, char* buf, int bufsize, int32_t value)
{
    snprintf(buf, bufsize, PSTR("%d"), value / 10000);
}

static void hasp_process_label_long_mode(lv_obj_t* obj, const char* payload, bool update)
{
    if(update) {
        lv_label_long_mode_t mode = LV_LABEL_LONG_EXPAND;
        if(!strcasecmp_P(payload, PSTR("expand"))) {
            mode = LV_LABEL_LONG_EXPAND;
        } else if(!strcasecmp_P(payload, PSTR("break"))) {
            mode = LV_LABEL_LONG_BREAK;
        } else if(!strcasecmp_P(payload, PSTR("dots"))) {
            mode = LV_LABEL_LONG_DOT;
        } else if(!strcasecmp_P(payload, PSTR("scroll"))) {
            mode = LV_LABEL_LONG_SROLL;
        } else if(!strcasecmp_P(payload, PSTR("loop"))) {
            mode = LV_LABEL_LONG_SROLL_CIRC;
        } else if(!strcasecmp_P(payload, PSTR("crop"))) {
            mode = LV_LABEL_LONG_CROP;
        } else {
            LOG_WARNING(TAG_ATTR, F("Invalid long mode"));
            return;
        }
        lv_label_set_long_mode(obj, mode);
    } else {
        // Getter needed
        attr_out_int(obj, "mode", lv_label_get_long_mode(obj));
    }
}

// OK
lv_obj_t* FindButtonLabel(lv_obj_t* btn)
{
    if(btn) {
        lv_obj_t* label = lv_obj_get_child_back(btn, NULL);
#if 1
        if(label) {
            if(check_obj_type(label, LV_HASP_LABEL)) {
                return label;
            }
#else
        if(label) {
            lv_obj_type_t list;
            lv_obj_get_type(label, &list);
            const char* objtype = list.type[0];

            if(check_obj_type(objtype, LV_HASP_LABEL)) {
                return label;
            }
#endif

        } else {
            LOG_ERROR(TAG_ATTR, F("FindButtonLabel NULL Pointer encountered"));
        }
    } else {
        LOG_WARNING(TAG_ATTR, F("Button not defined"));
    }
    return NULL;
}

// OK
static inline void haspSetLabelText(lv_obj_t* obj, const char* value)
{
    lv_obj_t* label = FindButtonLabel(obj);
    if(label) {
        lv_label_set_text(label, value);
    }
}

// OK
static bool haspGetLabelText(lv_obj_t* obj, char** text)
{
    if(!obj) {
        LOG_WARNING(TAG_ATTR, F("Button not defined"));
        return false;
    }

    lv_obj_t* label = lv_obj_get_child_back(obj, NULL);
    if(label) {
#if 1
        if(check_obj_type(label, LV_HASP_LABEL)) {
            *text = lv_label_get_text(label);
            return true;
        }
#else
        lv_obj_type_t list;
        lv_obj_get_type(label, &list);

        if(check_obj_type(list.type[0], LV_HASP_LABEL)) {
            text = lv_label_get_text(label);
            return true;
        }
#endif

    } else {
        LOG_WARNING(TAG_ATTR, F("haspGetLabelText NULL Pointer encountered"));
    }

    return false;
}

static void hasp_attribute_get_part_state(lv_obj_t* obj, const char* attr_in, char* attr_out, uint8_t& part,
                                          uint8_t& state)
{
    int len = strlen(attr_in);
    if(len <= 0 || len >= 32) {
        attr_out[0] = 0; // empty string
        part        = LV_OBJ_PART_MAIN;
        state       = LV_STATE_DEFAULT;
        return;
    }
    int index = atoi(&attr_in[len - 1]);

    // Drop Trailing partnumber
    if(attr_in[len - 1] == '0' || index > 0) {
        part = LV_TABLE_PART_BG;
        len--;
    }
    strncpy(attr_out, attr_in, len);
    attr_out[len] = 0;

    /* Attributes depending on objecttype */
    // lv_obj_type_t list;
    // lv_obj_get_type(obj, &list);
    // const char * objtype = list.type[0];

    if(check_obj_type(obj, LV_HASP_BUTTON)) {
        switch(index) {
            case 1:
                state = LV_BTN_STATE_PRESSED;
                break;
            case 2:
                state = LV_BTN_STATE_DISABLED;
                break;
            case 3:
                state = LV_BTN_STATE_CHECKED_RELEASED;
                break;
            case 4:
                state = LV_BTN_STATE_CHECKED_PRESSED;
                break;
            case 5:
                state = LV_BTN_STATE_CHECKED_DISABLED;
                break;
            default:
                state = LV_BTN_STATE_RELEASED;
        }
        part = LV_BTN_PART_MAIN;
        return;
    }

#if(LV_SLIDER_PART_INDIC != LV_SWITCH_PART_INDIC) || (LV_SLIDER_PART_KNOB != LV_SWITCH_PART_KNOB) ||                   \
    (LV_SLIDER_PART_BG != LV_SWITCH_PART_BG) || (LV_SLIDER_PART_INDIC != LV_ARC_PART_INDIC) ||                         \
    (LV_SLIDER_PART_KNOB != LV_ARC_PART_KNOB) || (LV_SLIDER_PART_BG != LV_ARC_PART_BG) ||                              \
    (LV_SLIDER_PART_INDIC != LV_BAR_PART_INDIC) || (LV_SLIDER_PART_BG != LV_BAR_PART_BG)
#error "LV_SLIDER, LV_BAR, LV_ARC, LV_SWITCH parts should match!"
#endif

    if(check_obj_type(obj, LV_HASP_SLIDER) || check_obj_type(obj, LV_HASP_SWITCH) || check_obj_type(obj, LV_HASP_ARC) ||
       check_obj_type(obj, LV_HASP_BAR)) {
        if(index == 1) {
            part = LV_SLIDER_PART_INDIC;
        } else if(index == 2) {
            if(!check_obj_type(obj, LV_HASP_BAR)) part = LV_SLIDER_PART_KNOB;
        } else {
            part = LV_SLIDER_PART_BG;
        }
        state = LV_STATE_DEFAULT;
        return;
    }

    if(check_obj_type(obj, LV_HASP_CHECKBOX)) {
        if(index == 1) {
            part = LV_CHECKBOX_PART_BULLET;
        } else {
            part = LV_CHECKBOX_PART_BG;
        }
        state = LV_STATE_DEFAULT;
        return;
    }

    if(check_obj_type(obj, LV_HASP_CPICKER)) {
        if(index == 1) {
            part = LV_CPICKER_PART_KNOB;
        } else {
            part = LV_CPICKER_PART_MAIN;
        }
        state = LV_STATE_DEFAULT;
        return;
    }

    if(check_obj_type(obj, LV_HASP_ROLLER)) {
        if(index == 1) {
            part = LV_ROLLER_PART_SELECTED;
        } else {
            part = LV_ROLLER_PART_BG;
        }
        state = LV_STATE_DEFAULT;
        return;
    }

    // if(check_obj_type(obj, LV_HASP_LMETER)) {
    //     state = LV_STATE_DEFAULT;
    //     return;
    // }
}

/**
 * Change or Retrieve the value of a local attribute of an object PART
 * @param obj lv_obj_t*: the object to get/set the attribute
 * @param attr_p char*: the attribute name (with or without leading ".")
 * @param attr_hash uint16_t: the sbdm hash of the attribute name without leading "."
 * @param payload char*: the new value of the attribute
 * @param update  bool: change/set the value if true, dispatch/get value if false
 * @note setting a value won't return anything, getting will dispatch the value
 */
static void hasp_local_style_attr(lv_obj_t* obj, const char* attr_p, uint16_t attr_hash, const char* payload,
                                  bool update)
{
    char attr[32];
    uint8_t part  = LV_OBJ_PART_MAIN;
    uint8_t state = LV_STATE_DEFAULT;
    int16_t var   = atoi(payload);

    // test_prop(attr_hash);

    hasp_attribute_get_part_state(obj, attr_p, attr, part, state);
    attr_hash = Parser::get_sdbm(attr); // attribute name without the index number

    /* ***** WARNING ****************************************************
     * when using hasp_out use attr_p for the original attribute name
     * *************************************************************** */

    switch(attr_hash) {

/* 1: Use other blend modes than normal (`LV_BLEND_MODE_...`)*/
#if LV_USE_BLEND_MODES
        case ATTR_BG_BLEND_MODE:
            return attribute_bg_blend_mode(obj, part, state, update, attr_p, (lv_blend_mode_t)var);
        case ATTR_TEXT_BLEND_MODE:
            return lv_obj_set_style_local_text_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_BORDER_BLEND_MODE:
            return lv_obj_set_style_local_border_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_OUTLINE_BLEND_MODE:
            return lv_obj_set_style_local_outline_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_SHADOW_BLEND_MODE:
            return lv_obj_set_style_local_shadow_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_LINE_BLEND_MODE:
            return lv_obj_set_style_local_line_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_VALUE_BLEND_MODE:
            return lv_obj_set_style_local_value_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_PATTERN_BLEND_MODE:
            return lv_obj_set_style_local_pattern_blend_mode(obj, part, state, (lv_blend_mode_t)var);
#endif

        case ATTR_SIZE:
            return attribute_size(obj, part, state, update, attr_p, var);
        case ATTR_RADIUS:
            return attribute_radius(obj, part, state, update, attr_p, var);
        case ATTR_CLIP_CORNER:
            return attribute_clip_corner(obj, part, state, update, attr_p, var);
        case ATTR_OPA_SCALE:
            return attribute_opa_scale(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_TRANSFORM_WIDTH:
            return attribute_transform_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_TRANSFORM_HEIGHT:
            return attribute_transform_height(obj, part, state, update, attr_p, (lv_style_int_t)var);

            /* Background attributes */
        case ATTR_BG_MAIN_STOP:
            return attribute_bg_main_stop(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_BG_GRAD_STOP:
            return attribute_bg_grad_stop(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_BG_GRAD_DIR:
            return attribute_bg_grad_dir(obj, part, state, update, attr_p, (lv_grad_dir_t)var);
        case ATTR_BG_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c) && part != 64)
                    lv_obj_set_style_local_bg_color(obj, part, state, lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_bg_color(obj, part));
            }
            return;
        }
        case ATTR_BG_GRAD_COLOR:
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_bg_grad_color(obj, part, state,
                                                         lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_bg_grad_color(obj, part));
            }
            return;

        case ATTR_BG_OPA:
            return attribute_bg_opa(obj, part, state, update, attr_p, (lv_opa_t)var);

        /* Margin attributes */
        case ATTR_MARGIN_TOP:
            return attribute_margin_top(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_MARGIN_BOTTOM:
            return attribute_margin_bottom(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_MARGIN_LEFT:
            return attribute_margin_left(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_MARGIN_RIGHT:
            return attribute_margin_right(obj, part, state, update, attr_p, (lv_style_int_t)var);

        /* Padding attributes */
        case ATTR_PAD_TOP:
            return attribute_pad_top(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_PAD_BOTTOM:
            return attribute_pad_bottom(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_PAD_LEFT:
            return attribute_pad_left(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_PAD_RIGHT:
            return attribute_pad_right(obj, part, state, update, attr_p, (lv_style_int_t)var);
#if LVGL_VERSION_MAJOR == 7
        case ATTR_PAD_INNER:
            return attribute_pad_inner(obj, part, state, update, attr_p, (lv_style_int_t)var);
#endif

        /* Scale attributes */
        case ATTR_SCALE_GRAD_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c) && part != 64)
                    lv_obj_set_style_local_scale_grad_color(obj, part, state,
                                                            lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_scale_grad_color(obj, part));
            }
            return;
        }
        case ATTR_SCALE_END_COLOR:
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_scale_end_color(obj, part, state,
                                                           lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_scale_end_color(obj, part));
            }
            return;
        case ATTR_SCALE_END_LINE_WIDTH:
            return attribute_scale_end_line_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SCALE_END_BORDER_WIDTH:
            return attribute_scale_end_border_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SCALE_BORDER_WIDTH:
            return attribute_scale_border_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SCALE_WIDTH:
            return attribute_scale_width(obj, part, state, update, attr_p, (lv_style_int_t)var);

        /* Text attributes */
        case ATTR_TEXT_LETTER_SPACE:
            return attribute_text_letter_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_TEXT_LINE_SPACE:
            return attribute_text_line_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_TEXT_DECOR:
            return attribute_text_decor(obj, part, state, update, attr_p, (lv_text_decor_t)var);
        case ATTR_TEXT_OPA:
            return attribute_text_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_TEXT_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_text_color(obj, part, state, lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_text_color(obj, part));
            }
            return;
        }
        case ATTR_TEXT_SEL_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_text_sel_color(obj, part, state,
                                                          lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_text_sel_color(obj, part));
            }
            return;
        }
        case ATTR_TEXT_FONT: {
            lv_font_t* font = haspPayloadToFont(payload);
            if(font) {
                LOG_WARNING(TAG_ATTR, "%s %d %x", __FILE__, __LINE__, *font);
                uint8_t count = 3;
                if(check_obj_type(obj, LV_HASP_ROLLER)) count = my_roller_get_visible_row_count(obj);
                lv_obj_set_style_local_text_font(obj, part, state, font);
                if(check_obj_type(obj, LV_HASP_ROLLER)) lv_roller_set_visible_row_count(obj, count);
                lv_obj_set_style_local_text_font(obj, part, state, font); // again, for roller

                if(check_obj_type(obj, LV_HASP_DROPDOWN)) { // issue #43
                    lv_obj_set_style_local_text_font(obj, LV_DROPDOWN_PART_MAIN, state, font);
                    lv_obj_set_style_local_text_font(obj, LV_DROPDOWN_PART_LIST, state, font);
                    lv_obj_set_style_local_text_font(obj, LV_DROPDOWN_PART_SELECTED, state, font);
                };

            } else {
                LOG_WARNING(TAG_ATTR, F("Unknown Font ID %s"), payload);
            }
            return;
        }

        /* Border attributes */
        case ATTR_BORDER_WIDTH:
            return attribute_border_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_BORDER_SIDE:
            return attribute_border_side(obj, part, state, update, attr_p, (lv_border_side_t)var);
        case ATTR_BORDER_POST:
            return attribute_border_post(obj, part, state, update, attr_p, Parser::is_true(payload));
        case ATTR_BORDER_OPA:
            return attribute_border_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_BORDER_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_border_color(obj, part, state,
                                                        lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_border_color(obj, part));
            }
            return;
        }

        /* Outline attributes */
        case ATTR_OUTLINE_WIDTH:
            return attribute_outline_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_OUTLINE_PAD:
            return attribute_outline_pad(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_OUTLINE_OPA:
            return attribute_outline_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_OUTLINE_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_outline_color(obj, part, state,
                                                         lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_outline_color(obj, part));
            }
            return;
        }

        /* Shadow attributes */
#if LV_USE_SHADOW
        case ATTR_SHADOW_WIDTH:
            return attribute_shadow_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_OFS_X:
            return attribute_shadow_ofs_x(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_OFS_Y:
            return attribute_shadow_ofs_y(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_SPREAD:
            return attribute_shadow_spread(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_OPA:
            return attribute_shadow_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_SHADOW_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_shadow_color(obj, part, state,
                                                        lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_shadow_color(obj, part));
            }
            return;
        }
#endif

        /* Line attributes */
        case ATTR_LINE_WIDTH:
            return attribute_line_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_LINE_DASH_WIDTH:
            return attribute_line_dash_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_LINE_DASH_GAP:
            return attribute_line_dash_gap(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_LINE_ROUNDED:
            return attribute_line_rounded(obj, part, state, update, attr_p, Parser::is_true(payload));
        case ATTR_LINE_OPA:
            return attribute_line_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_LINE_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_line_color(obj, part, state, lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_line_color(obj, part));
            }
            return;
        }

        /* Value attributes */
        case ATTR_VALUE_LETTER_SPACE:
            return attribute_value_letter_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_LINE_SPACE:
            return attribute_value_line_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_OFS_X:
            return attribute_value_ofs_x(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_OFS_Y:
            return attribute_value_ofs_y(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_ALIGN:
            return attribute_value_align(obj, part, state, update, attr_p, (lv_align_t)var);
        case ATTR_VALUE_OPA:
            return attribute_value_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_VALUE_STR: {
            if(update) {
                my_obj_set_value_str_txt(obj, part, state, payload);
            } else {
                attr_out_str(obj, attr, lv_obj_get_style_value_str(obj, part));
            }
            return;
        }
        case ATTR_VALUE_COLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_value_color(obj, part, state,
                                                       lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_value_color(obj, part));
            }
            return;
        }
        case ATTR_VALUE_FONT: {
            lv_font_t* font = haspPayloadToFont(payload);
            if(font) {
                return lv_obj_set_style_local_value_font(obj, part, state, font);
            } else {
                LOG_WARNING(TAG_ATTR, F("Unknown Font ID %s"), attr_p);
                return;
            }
        }

        /* Pattern attributes */
        case ATTR_PATTERN_REPEAT:
            return attribute_pattern_repeat(obj, part, state, update, attr_p, Parser::is_true(payload));
        case ATTR_PATTERN_OPA:
            return attribute_pattern_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_PATTERN_RECOLOR_OPA:
            return attribute_pattern_recolor_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_PATTERN_IMAGE:
            //   return lv_obj_set_style_local_pattern_image(obj, part, state, (constvoid *)var);
            break;
        case ATTR_PATTERN_RECOLOR: {
            if(update) {
                lv_color32_t c;
                if(Parser::haspPayloadToColor(payload, c))
                    lv_obj_set_style_local_pattern_recolor(obj, part, state,
                                                           lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
            } else {
                attr_out_color(obj, attr, lv_obj_get_style_pattern_recolor(obj, part));
            }
            return;
        }

            /* Image attributes */
            // Todo

            /* Transition attributes */
            // Todo
    }
    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN " (%d)"), attr_p, attr_hash);
}

static void hasp_process_arc_attribute(lv_obj_t* obj, const char* attr_p, uint16_t attr_hash, const char* payload,
                                       bool update)
{
    // We already know it's a arc object
    uint16_t val = atoi(payload);

    char* attr = (char*)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(attr_hash) {
        case ATTR_TYPE:
            return (update) ? lv_arc_set_type(obj, val % 3) : attr_out_int(obj, attr, lv_arc_get_type(obj));

        case ATTR_ROTATION:
            return (update) ? lv_arc_set_rotation(obj, val) : attr_out_int(obj, attr, my_arc_get_rotation(obj));

        case ATTR_ADJUSTABLE:
            if(update) {
                bool toggle = Parser::is_true(payload);
                lv_arc_set_adjustable(obj, toggle);
                lv_obj_set_event_cb(obj, toggle ? slider_event_handler : generic_event_handler);
            } else {
                attr_out_int(obj, attr, lv_arc_get_adjustable(obj));
            }
            return;

        case ATTR_START_ANGLE:
            return (update) ? lv_arc_set_bg_start_angle(obj, val)
                            : attr_out_int(obj, attr, lv_arc_get_bg_angle_start(obj));

        case ATTR_END_ANGLE:
            return (update) ? lv_arc_set_bg_end_angle(obj, val) : attr_out_int(obj, attr, lv_arc_get_bg_angle_end(obj));

        case ATTR_START_ANGLE1:
            return (update) ? lv_arc_set_start_angle(obj, val) : attr_out_int(obj, attr, lv_arc_get_angle_start(obj));

        case ATTR_END_ANGLE1:
            return (update) ? lv_arc_set_end_angle(obj, val) : attr_out_int(obj, attr, lv_arc_get_angle_end(obj));
    }

    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN), attr_p);
}

static void hasp_process_lmeter_attribute(lv_obj_t* obj, const char* attr_p, uint16_t attr_hash, const char* payload,
                                          bool update)
{
    // We already know it's a linemeter object
    uint16_t val = atoi(payload);

    uint16_t line_count = lv_linemeter_get_line_count(obj);
    uint16_t angle      = lv_linemeter_get_scale_angle(obj);

    char* attr = (char*)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(attr_hash) {
        case ATTR_TYPE:
            return (update) ? lv_linemeter_set_mirror(obj, val != 0)
                            : attr_out_int(obj, attr, lv_linemeter_get_mirror(obj));

        case ATTR_ROTATION:
            return (update) ? lv_linemeter_set_angle_offset(obj, val)
                            : attr_out_int(obj, attr, lv_linemeter_get_angle_offset(obj));

        case ATTR_LINE_COUNT:
            return (update) ? lv_linemeter_set_scale(obj, angle, val) : attr_out_int(obj, attr, line_count);

        case ATTR_ANGLE:
            return (update) ? lv_linemeter_set_scale(obj, val, line_count) : attr_out_int(obj, attr, angle);
    }

    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN), attr_p);
}

static void hasp_process_dropdown_attribute(lv_obj_t* obj, const char* attr_p, uint16_t attr_hash, const char* payload,
                                            bool update)
{
    // We already know it's a gauge object
    int16_t intval = atoi(payload);
    uint16_t val   = atoi(payload);

    char* attr = (char*)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(attr_hash) {
        case ATTR_DIRECTION:
            return (update) ? lv_dropdown_set_dir(obj, intval) : attr_out_int(obj, attr, lv_dropdown_get_dir(obj));

        case ATTR_SYMBOL:
            return (update) ? lv_dropdown_set_symbol(obj, payload)
                            : attr_out_str(obj, attr, lv_dropdown_get_symbol(obj));

        case ATTR_OPEN:
            return lv_dropdown_open(obj);

        case ATTR_CLOSE:
            return lv_dropdown_close(obj);

        case ATTR_MAX_HEIGHT:
            return (update) ? lv_dropdown_set_max_height(obj, intval)
                            : attr_out_int(obj, attr, lv_dropdown_get_max_height(obj));

        case ATTR_SHOW_SELECTED:
            return (update) ? lv_dropdown_set_show_selected(obj, val)
                            : attr_out_int(obj, attr, lv_dropdown_get_show_selected(obj));
    }

    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN), attr_p);
}

static void hasp_process_gauge_attribute(lv_obj_t* obj, const char* attr_p, uint16_t attr_hash, const char* payload,
                                         bool update)
{
    // We already know it's a gauge object
    int16_t intval = atoi(payload);
    uint16_t val   = atoi(payload);

    uint8_t label_count = lv_gauge_get_label_count(obj);
    uint16_t line_count = lv_gauge_get_line_count(obj);
    uint16_t angle      = lv_gauge_get_scale_angle(obj);

    char* attr = (char*)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(attr_hash) {
        case ATTR_CRITICAL_VALUE:
            return (update) ? lv_gauge_set_critical_value(obj, intval)
                            : attr_out_int(obj, attr, lv_gauge_get_critical_value(obj));

        case ATTR_ANGLE:
            return (update) ? lv_gauge_set_scale(obj, val, line_count, label_count) : attr_out_int(obj, attr, angle);

        case ATTR_LINE_COUNT:
            return (update) ? lv_gauge_set_scale(obj, angle, val, label_count) : attr_out_int(obj, attr, line_count);

        case ATTR_LABEL_COUNT:
            return (update) ? lv_gauge_set_scale(obj, angle, line_count, val) : attr_out_int(obj, attr, label_count);

        case ATTR_ROTATION:
            return (update) ? lv_gauge_set_angle_offset(obj, val)
                            : attr_out_int(obj, attr, lv_gauge_get_angle_offset(obj));

        case ATTR_FORMAT:
            if(update) {
                switch(val) {
                    case 1:
                        lv_gauge_set_formatter_cb(obj, gauge_format_10);
                        break;
                    case 2:
                        lv_gauge_set_formatter_cb(obj, gauge_format_100);
                        break;
                    case 3:
                        lv_gauge_set_formatter_cb(obj, gauge_format_1k);
                        break;
                    case 4:
                        lv_gauge_set_formatter_cb(obj, gauge_format_10k);
                        break;
                    default:
                        lv_gauge_set_formatter_cb(obj, NULL);
                }
            }
            return;
    }

    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN), attr_p);
}

// ##################### Common Attributes ########################################################

static void hasp_process_page_attributes(lv_obj_t* obj, const char* attr_p, uint16_t attr_hash, uint8_t val,
                                         bool update)
{
    uint8_t pageid;

    if(haspPages.get_id(obj, &pageid)) {
        switch(attr_hash) {
            case ATTR_NEXT:
                update ? haspPages.set_next(pageid, val) : attr_out_int(obj, attr_p, haspPages.get_next(pageid));
                break;

            case ATTR_PREV:
                update ? haspPages.set_prev(pageid, val) : attr_out_int(obj, attr_p, haspPages.get_prev(pageid));
                break;

            // case ATTR_BACK:
            default:
                update ? haspPages.set_back(pageid, val) : attr_out_int(obj, attr_p, haspPages.get_back(pageid));
                break;
        }
    }
}

static void hasp_process_obj_attribute_txt(lv_obj_t* obj, const char* attr, const char* payload, bool update)
{
    /* Attributes depending on objecttype */
    // lv_obj_type_t list;
    // lv_obj_get_type(obj, &list);
    // const char * objtype = list.type[0];

    if(check_obj_type(obj, LV_HASP_BUTTON)) {
        if(update) {
            haspSetLabelText(obj, payload);
        } else {
            char* text = NULL;
            if(haspGetLabelText(obj, &text) && text) attr_out_str(obj, attr, text);
        }
        return;
    }
    if(check_obj_type(obj, LV_HASP_LABEL)) {
        return update ? lv_label_set_text(obj, payload) : attr_out_str(obj, attr, lv_label_get_text(obj));
    }
    if(check_obj_type(obj, LV_HASP_CHECKBOX)) {
        return update ? lv_checkbox_set_text(obj, payload) : attr_out_str(obj, attr, lv_checkbox_get_text(obj));
    }
    if(check_obj_type(obj, LV_HASP_DROPDOWN)) {
        char buffer[128];
        lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
        return attr_out_str(obj, attr, buffer);
    }
    if(check_obj_type(obj, LV_HASP_ROLLER)) {
        char buffer[128];
        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
        return attr_out_str(obj, attr, buffer);
    }
    if(check_obj_type(obj, LV_HASP_MSGBOX)) {
        return update ? lv_msgbox_set_text(obj, payload) : attr_out_str(obj, attr, lv_msgbox_get_text(obj));
    }
#if LV_USE_WIN != 0
    if(check_obj_type(obj, LV_HASP_WINDOW)) {
        // return update ? lv_win_set_title(obj, (const char *)payload) : attr_out_str(obj, attr,
        // lv_win_get_title(obj));
    }
#endif

    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN), attr);
}

bool hasp_process_obj_attribute_val(lv_obj_t* obj, const char* attr, int16_t intval, bool boolval, bool update)
{
    if(check_obj_type(obj, LV_HASP_BUTTON)) {
        if(lv_btn_get_checkable(obj)) {
            if(update) {
                if(intval)
                    lv_obj_add_state(obj, LV_STATE_CHECKED);
                else
                    lv_obj_clear_state(obj, LV_STATE_CHECKED);
            } else {
                attr_out_int(obj, attr, lv_obj_get_state(obj, LV_BTN_PART_MAIN) & LV_STATE_CHECKED);
            }
        } else {
            return false; // not checkable
        }
    } else if(check_obj_type(obj, LV_HASP_CHECKBOX)) {
        update ? lv_checkbox_set_checked(obj, boolval) : attr_out_int(obj, attr, lv_checkbox_is_checked(obj));
    } else if(check_obj_type(obj, LV_HASP_SWITCH)) {
        if(update)
            boolval ? lv_switch_on(obj, LV_ANIM_ON) : lv_switch_off(obj, LV_ANIM_ON);
        else
            attr_out_int(obj, attr, lv_switch_get_state(obj));
    } else if(check_obj_type(obj, LV_HASP_DROPDOWN)) {
        lv_dropdown_set_selected(obj, (uint16_t)intval);
    } else if(check_obj_type(obj, LV_HASP_LMETER)) {
        update ? lv_linemeter_set_value(obj, intval) : attr_out_int(obj, attr, lv_linemeter_get_value(obj));
    } else if(check_obj_type(obj, LV_HASP_SLIDER)) {
        update ? lv_slider_set_value(obj, intval, LV_ANIM_ON) : attr_out_int(obj, attr, lv_slider_get_value(obj));
    } else if(check_obj_type(obj, LV_HASP_LED)) {
        update ? lv_led_set_bright(obj, (uint8_t)intval) : attr_out_int(obj, attr, lv_led_get_bright(obj));
    } else if(check_obj_type(obj, LV_HASP_ARC)) {
        update ? lv_arc_set_value(obj, intval) : attr_out_int(obj, attr, lv_arc_get_value(obj));
    } else if(check_obj_type(obj, LV_HASP_GAUGE)) {
        update ? lv_gauge_set_value(obj, 0, intval) : attr_out_int(obj, attr, lv_gauge_get_value(obj, 0));
    } else if(check_obj_type(obj, LV_HASP_ROLLER)) {
        lv_roller_set_selected(obj, (uint16_t)intval, LV_ANIM_ON);
    } else if(check_obj_type(obj, LV_HASP_BAR)) {
        update ? lv_bar_set_value(obj, intval, LV_ANIM_ON) : attr_out_int(obj, attr, lv_bar_get_value(obj));
    } else if(check_obj_type(obj, LV_HASP_TABVIEW)) {
        update ? lv_tabview_set_tab_act(obj, intval, LV_ANIM_ON) : attr_out_int(obj, attr, lv_tabview_get_tab_act(obj));
    } else {
        return false;
    }

    return true;
}

static void hasp_process_obj_attribute_range(lv_obj_t* obj, const char* attr, const char* payload, bool update,
                                             bool set_min, bool set_max)
{
    int16_t val   = atoi(payload);
    int32_t val32 = strtol(payload, nullptr, DEC);

    /* Attributes depending on objecttype */
    // lv_obj_type_t list;
    // lv_obj_get_type(obj, &list);
    // const char * objtype = list.type[0];

    if(check_obj_type(obj, LV_HASP_SLIDER)) {
        int16_t min = lv_slider_get_min_value(obj);
        int16_t max = lv_slider_get_max_value(obj);
        if(update && (set_min ? val : min) == (set_max ? val : max)) return; // prevent setting min=max
        return update ? lv_slider_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : attr_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type(obj, LV_HASP_GAUGE)) {
        int32_t min = lv_gauge_get_min_value(obj);
        int32_t max = lv_gauge_get_max_value(obj);
        if(update && (set_min ? val32 : min) == (set_max ? val32 : max)) return; // prevent setting min=max
        return update ? lv_gauge_set_range(obj, set_min ? val32 : min, set_max ? val32 : max)
                      : attr_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type(obj, LV_HASP_ARC)) {
        int16_t min = lv_arc_get_min_value(obj);
        int16_t max = lv_arc_get_max_value(obj);
        if(update && (set_min ? val : min) == (set_max ? val : max)) return; // prevent setting min=max
        return update ? lv_arc_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : attr_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type(obj, LV_HASP_BAR)) {
        int16_t min = lv_bar_get_min_value(obj);
        int16_t max = lv_bar_get_max_value(obj);
        if(update && (set_min ? val : min) == (set_max ? val : max)) return; // prevent setting min=max
        return update ? lv_bar_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : attr_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type(obj, LV_HASP_LMETER)) {
        int32_t min = lv_linemeter_get_min_value(obj);
        int32_t max = lv_linemeter_get_max_value(obj);
        if(update && (set_min ? val32 : min) == (set_max ? val32 : max)) return; // prevent setting min=max
        return update ? lv_linemeter_set_range(obj, set_min ? val32 : min, set_max ? val32 : max)
                      : attr_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type(obj, LV_HASP_CHART)) {
        int16_t min = my_chart_get_min_value(obj);
        int16_t max = my_chart_get_max_value(obj);
        if(update && (set_min ? val : min) == (set_max ? val : max)) return; // prevent setting min=max
        return update ? lv_chart_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : attr_out_int(obj, attr, set_min ? min : max);
    }

    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN), attr);
}

// ##################### Default Attributes ########################################################

/**
 * Change or Retrieve the value of the attribute of an object
 * @param obj lv_obj_t*: the object to get/set the attribute
 * @param attr_p char*: the attribute name (with or without leading ".")
 * @param payload char*: the new value of the attribute
 * @param update  bool: change/set the value if true, dispatch/get value if false
 * @note setting a value won't return anything, getting will dispatch the value
 */
void hasp_process_obj_attribute(lv_obj_t* obj, const char* attr_p, const char* payload, bool update)
{
    // unsigned long start = millis();
    if(!obj) {
        LOG_WARNING(TAG_ATTR, F(D_OBJECT_UNKNOWN));
        return;
    }

    char* attr = (char*)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    uint16_t attr_hash = Parser::get_sdbm(attr);
    int16_t val        = atoi(payload);

    /* 16-bit Hash Lookup Table */
    switch(attr_hash) {
        case ATTR_ID:
            update ? (void)(obj->user_data.id = (uint8_t)val) : attr_out_int(obj, attr, obj->user_data.id);
            break; // attribute_found

        case ATTR_GROUPID:
            update ? (void)(obj->user_data.groupid = (uint8_t)val) : attr_out_int(obj, attr, obj->user_data.groupid);
            break; // attribute_found

        case ATTR_ACTION:
            update ? (void)(obj->user_data.actionid = Parser::get_action_id(payload))
                   : attr_out_int(obj, attr, obj->user_data.actionid);
            break; // attribute_found

        case ATTR_TRANSITION:
            update ? (void)(obj->user_data.transitionid = (uint8_t)val)
                   : attr_out_int(obj, attr, obj->user_data.transitionid);
            break; // attribute_found

        case ATTR_OBJ:
            if(update) LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_READ_ONLY), attr_p);
            attr_out_str(obj, attr, get_obj_type_name(obj));
            break; // attribute_found

        case ATTR_OBJID:
            if(update) LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_READ_ONLY), attr_p);
            attr_out_int(obj, attr, obj->user_data.objid);
            break; // attribute_found

        case ATTR_X:
            update ? lv_obj_set_x(obj, val) : attr_out_int(obj, attr, lv_obj_get_x(obj));
            break; // attribute_found

        case ATTR_Y:
            update ? lv_obj_set_y(obj, val) : attr_out_int(obj, attr, lv_obj_get_y(obj));
            break; // attribute_found

        case ATTR_W:
            if(update) {
                lv_obj_set_width(obj, val);
                if(check_obj_type(obj, LV_HASP_CPICKER)) {
#if LVGL_VERSION_MAJOR == 7
                    lv_cpicker_set_type(obj, lv_obj_get_width(obj) == lv_obj_get_height(obj) ? LV_CPICKER_TYPE_DISC
                                                                                             : LV_CPICKER_TYPE_RECT);
#endif
                }
            } else {
                attr_out_int(obj, attr, lv_obj_get_width(obj));
            }
            break; // attribute_found

        case ATTR_H:
            if(update) {
                lv_obj_set_height(obj, val);
                if(check_obj_type(obj, LV_HASP_CPICKER)) {
#if LVGL_VERSION_MAJOR == 7
                    lv_cpicker_set_type(obj, lv_obj_get_width(obj) == lv_obj_get_height(obj) ? LV_CPICKER_TYPE_DISC
                                                                                             : LV_CPICKER_TYPE_RECT);
#endif
                }
            } else {
                attr_out_int(obj, attr, lv_obj_get_height(obj));
            }
            break; // attribute_found

        case ATTR_VIS:
            update ? lv_obj_set_hidden(obj, !Parser::is_true(payload))
                   : attr_out_int(obj, attr, !lv_obj_get_hidden(obj));
            break; // attribute_found

        case ATTR_HIDDEN:
            update ? lv_obj_set_hidden(obj, Parser::is_true(payload)) : attr_out_int(obj, attr, lv_obj_get_hidden(obj));
            break; // attribute_found

        case ATTR_TXT: // TODO: remove
            LOG_WARNING(TAG_HASP, F("txt property is obsolete, use text instead"));
        case ATTR_TEXT:
            hasp_process_obj_attribute_txt(obj, attr, payload, update);
            break; // attribute_found

        case ATTR_COLOR:
            if(check_obj_type(obj, LV_HASP_CPICKER)) {
                if(update) {
                    lv_color32_t c;
                    if(Parser::haspPayloadToColor(payload, c))
                        lv_cpicker_set_color(obj, lv_color_make(c.ch.red, c.ch.green, c.ch.blue));
                } else {
                    attr_out_color(obj, attr, lv_cpicker_get_color(obj));
                }
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_VAL:
            if(!hasp_process_obj_attribute_val(obj, attr, atoi(payload), Parser::is_true(payload), update))
                goto attribute_not_found;
            break; // attribute_found

        case ATTR_MIN:
            hasp_process_obj_attribute_range(obj, attr, payload, update, true, false);
            break; // attribute_found

        case ATTR_MAX:
            hasp_process_obj_attribute_range(obj, attr, payload, update, false, true);
            break; // attribute_found

        case ATTR_OPACITY:
            update ? lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, val)
                   : attr_out_int(obj, attr, lv_obj_get_style_opa_scale(obj, LV_OBJ_PART_MAIN));
            break; // attribute_found

        case ATTR_ENABLED:
            update ? lv_obj_set_click(obj, Parser::is_true(payload)) : attr_out_int(obj, attr, lv_obj_get_click(obj));
            break; // attribute_found

        case ATTR_SWIPE:
            update ? (void)(obj->user_data.swipeid = Parser::is_true(payload) % 16)
                   : attr_out_int(obj, attr, obj->user_data.swipeid);
            break; // attribute_found

        case ATTR_SRC:
            if(check_obj_type(obj, LV_HASP_IMAGE)) {
                if(update) {
                    return lv_img_set_src(obj, payload);
                } else {
                    switch(lv_img_src_get_type(obj)) {
                        case LV_IMG_SRC_FILE:
                            return attr_out_str(obj, attr, lv_img_get_file_name(obj));
                        case LV_IMG_SRC_SYMBOL:
                            return attr_out_str(obj, attr, (char*)lv_img_get_src(obj));
                        default:
                            return;
                    }
                }
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

            /*     case ATTR_ANIM_TIME:
                     return (update) ? lv_anim_time(obj, val)
                                     : attr_out_int(obj, attr, lv_gauge_get_angle_offset(obj)); */

        case ATTR_ROWS:
            if(check_obj_type(obj, LV_HASP_ROLLER)) {
                update ? lv_roller_set_visible_row_count(obj, (uint8_t)val)
                       : attr_out_int(obj, attr, my_roller_get_visible_row_count(obj));
            } else if(check_obj_type(obj, LV_HASP_TABLE)) {
                update ? lv_table_set_row_cnt(obj, (uint8_t)val) : attr_out_int(obj, attr, lv_table_get_row_cnt(obj));
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_COLS:
            if(check_obj_type(obj, LV_HASP_TABLE)) {
                update ? lv_table_set_col_cnt(obj, (uint8_t)val) : attr_out_int(obj, attr, lv_table_get_col_cnt(obj));
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

            // case ATTR_RECT:
            //     if(check_obj_type(obj, LV_HASP_CPICKER)) {
            //         lv_cpicker_set_type(obj, is_true(payload) ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
            //         return;
            //     }
            //     break;

        case ATTR_ALIGN:
            if(check_obj_type(obj, LV_HASP_BUTTON)) {
                lv_obj_t* label = FindButtonLabel(obj);
                if(label == NULL)
                    goto attribute_not_found;
                else
                    update ? lv_label_set_align(label, val) : attr_out_int(obj, attr, lv_label_get_align(label));

            } else if(check_obj_type(obj, LV_HASP_BTNMATRIX)) {
                update ? lv_btnmatrix_set_align(obj, val) : attr_out_int(obj, attr, lv_btnmatrix_get_align(obj));
            } else if(check_obj_type(obj, LV_HASP_LABEL)) {
                update ? lv_label_set_align(obj, val) : attr_out_int(obj, attr, lv_label_get_align(obj));
            } else if(check_obj_type(obj, LV_HASP_ROLLER)) {
                update ? lv_roller_set_align(obj, val) : attr_out_int(obj, attr, lv_roller_get_align(obj));
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_MODE:
            if(check_obj_type(obj, LV_HASP_BUTTON)) {
                lv_obj_t* label = FindButtonLabel(obj);
                if(label) {
                    hasp_process_label_long_mode(label, payload, update);
                    lv_obj_set_width(label, lv_obj_get_width(obj));
                }
            } else if(check_obj_type(obj, LV_HASP_LABEL)) {
                hasp_process_label_long_mode(obj, payload, update);
            } else if(check_obj_type(obj, LV_HASP_ROLLER)) {
                if(update) {
                    lv_roller_set_options(obj, lv_roller_get_options(obj), (lv_roller_mode_t)Parser::is_true(payload));
                } else {
                    lv_roller_ext_t* ext = (lv_roller_ext_t*)lv_obj_get_ext_attr(obj);
                    attr_out_int(obj, attr, ext->mode);
                }
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_TOGGLE:
            if(check_obj_type(obj, LV_HASP_BUTTON)) {
                if(update) {
                    bool toggle = Parser::is_true(payload);
                    lv_btn_set_checkable(obj, toggle);
                    lv_obj_set_event_cb(obj, toggle ? toggle_event_handler : generic_event_handler);
                } else {
                    attr_out_int(obj, attr, lv_btn_get_checkable(obj));
                }
            } else if(check_obj_type(obj, LV_HASP_BTNMATRIX)) {
                if(update) {
                    bool toggle = Parser::is_true(payload);
                    if(toggle) {
                        lv_btnmatrix_set_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECKABLE);
                    } else {
                        lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECKABLE);
                        lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);
                    }
                } else {
                    attr_out_int(obj, attr, lv_btn_get_checkable(obj));
                }
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_OPTIONS:
            if(check_obj_type(obj, LV_HASP_DROPDOWN)) {
                if(update) {
                    lv_dropdown_set_options(obj, payload);
                } else {
                    attr_out_str(obj, attr, lv_dropdown_get_options(obj));
                }
            } else if(check_obj_type(obj, LV_HASP_ROLLER)) {
                if(update) {
                    lv_roller_ext_t* ext = (lv_roller_ext_t*)lv_obj_get_ext_attr(obj);
                    lv_roller_set_options(obj, payload, ext->mode);
                } else {
                    attr_out_str(obj, attr, lv_roller_get_options(obj));
                }
            } else if(check_obj_type(obj, LV_HASP_BTNMATRIX)) {
                if(update) {
                    my_btnmatrix_map_create(obj, payload);
                } else {
                    attr_out_str(obj, attr_p, "Not implemented"); // TODO : Literal String
                }
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_ONE_CHECK:
            if(check_obj_type(obj, LV_HASP_BTNMATRIX)) {
                if(update) {
                    lv_btnmatrix_set_one_check(obj, Parser::is_true(payload));
                } else {
                    attr_out_int(obj, attr_p, lv_btnmatrix_get_one_check(obj));
                }
            }
            break;

        case ATTR_CRITICAL_VALUE:
        case ATTR_ANGLE:
        case ATTR_LABEL_COUNT:
        case ATTR_LINE_COUNT:
        case ATTR_FORMAT:
        case ATTR_TYPE:
        case ATTR_ROTATION:
        case ATTR_ADJUSTABLE:
        case ATTR_START_ANGLE:
        case ATTR_END_ANGLE:
        case ATTR_START_ANGLE1:
        case ATTR_END_ANGLE1:
            if(check_obj_type(obj, LV_HASP_ARC)) {
                hasp_process_arc_attribute(obj, attr_p, attr_hash, payload, update);
            } else if(check_obj_type(obj, LV_HASP_GAUGE)) {
                hasp_process_gauge_attribute(obj, attr_p, attr_hash, payload, update);
            } else if(check_obj_type(obj, LV_HASP_LMETER)) {
                hasp_process_lmeter_attribute(obj, attr_p, attr_hash, payload, update);
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_DIRECTION:
        case ATTR_SYMBOL:
        case ATTR_OPEN:
        case ATTR_CLOSE:
        case ATTR_MAX_HEIGHT:
        case ATTR_SHOW_SELECTED:
            if(check_obj_type(obj, LV_HASP_DROPDOWN)) {
                hasp_process_dropdown_attribute(obj, attr_p, attr_hash, payload, update);
            } else {
                goto attribute_not_found;
            }
            break; // attribute_found

        case ATTR_RED: // TODO: remove temp RED
            if(check_obj_type(obj, LV_HASP_BTNMATRIX)) {
                my_btnmatrix_map_clear(obj); // TODO : remove this test property
            } else {
                goto attribute_not_found;
            }
            break;

        case ATTR_DELETE:
            if(!lv_obj_get_parent(obj)) {
                LOG_ERROR(TAG_ATTR, F(D_ATTRIBUTE_PAGE_METHOD_INVALID), attr_p);
                return;
            }
            lv_obj_del_async(obj);
            break; // attribute_found

        case ATTR_TO_FRONT:
            if(!lv_obj_get_parent(obj)) {
                LOG_ERROR(TAG_ATTR, F(D_ATTRIBUTE_PAGE_METHOD_INVALID), attr_p);
                return;
            }
            lv_obj_move_foreground(obj);
            break; // attribute_found

        case ATTR_TO_BACK:
            if(!lv_obj_get_parent(obj)) {
                LOG_ERROR(TAG_ATTR, F(D_ATTRIBUTE_PAGE_METHOD_INVALID), attr_p);
                return;
            }
            lv_obj_move_background(obj);
            break; // attribute_found

        case ATTR_NEXT:
        case ATTR_PREV:
        case ATTR_BACK:
            if(check_obj_type(obj, LV_HASP_SCREEN)) {
                hasp_process_page_attributes(obj, attr_p, attr_hash, val, update);
                break; // attribute_found
            }
            goto attribute_not_found;

        default:
            hasp_local_style_attr(obj, attr, attr_hash, payload, update);
            goto attribute_found;
    }

attribute_found:
    // LOG_VERBOSE(TAG_ATTR, F("%s (%d)"), attr_p, attr_hash);
    // LOG_VERBOSE(TAG_ATTR, F("%s (%d) took %d ms."), attr_p, attr_hash, millis() - start);
    return;

attribute_not_found:
    LOG_WARNING(TAG_ATTR, F(D_ATTRIBUTE_UNKNOWN " (%d)"), attr_p, attr_hash);
}

void attr_out_str(lv_obj_t* obj, const char* attribute, const char* data)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        if(!attribute || !data) return;

        StaticJsonDocument<32> doc; // Total (recommended) size
        doc[attribute].set(data);

        size_t size = measureJson(doc); // strlen(data) + strlen(attribute);
        if(size < MQTT_MAX_PACKET_SIZE) {
            char payload[MQTT_MAX_PACKET_SIZE];
            serializeJson(doc, payload);
            object_dispatch_state(pageid, objid, payload);

        } else {
            LOG_ERROR(TAG_ATTR, F(D_MQTT_PAYLOAD_TOO_LONG), size);
        }
    }
}

void attr_out_int(lv_obj_t* obj, const char* attribute, int32_t val)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        if(!attribute) return;

        char payload[64 + strlen(attribute)];
        snprintf_P(payload, sizeof(payload), PSTR("{\"%s\":%d}"), attribute, val);

        object_dispatch_state(pageid, objid, payload);
    }
}

void attr_out_color(lv_obj_t* obj, const char* attribute, lv_color_t color)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        if(!attribute) return;

        char payload[64 + strlen(attribute)];
        lv_color32_t c32;
        c32.full = lv_color_to32(color);

        snprintf_P(payload, sizeof(payload), PSTR("{\"%s\":\"#%02x%02x%02x\",\"r\":%d,\"g\":%d,\"b\":%d}"), attribute,
                   c32.ch.red, c32.ch.green, c32.ch.blue, c32.ch.red, c32.ch.green, c32.ch.blue);
        object_dispatch_state(pageid, objid, payload);
    }
}