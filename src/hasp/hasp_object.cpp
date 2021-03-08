/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* ********************************************************************************************
 *
 *  HASP Object Handlers
 *     - Object Finders      : Convert from object pointers to and from p[x].b[y] IDs
 *     - Value Dispatchers   : Forward output data to the dispatcher
 *     - Value Senders       : Convert values and events into topic/payload before forwarding
 *     - Event Handlers      : Callbacks for object event processing
 *     - Attribute processor : Decide if an attribute needs updating or querying and forward
 *     - Object creator      : Creates an object from a line of jsonl
 *
 ******************************************************************************************** */

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#include "lvgl.h"
#if LVGL_VERSION_MAJOR != 7
#include "../lv_components.h"
#endif

#include "hasplib.h"

const char** btnmatrix_default_map; // memory pointer to lvgl default btnmatrix map
// static unsigned long last_change_event = 0;
static bool last_press_was_short = false; // Avoid SHORT + UP double events

// ##################### Object Finders ########################################################

lv_obj_t* hasp_find_obj_from_parent_id(lv_obj_t* parent, uint8_t objid)
{
    if(objid == 0 || parent == nullptr) return parent;

    lv_obj_t* child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        /* child found, return it */
        if(objid == child->user_data.id) return child;

        /* check grandchildren */
        lv_obj_t* grandchild = hasp_find_obj_from_parent_id(child, objid);
        if(grandchild) return grandchild; /* grandchild found, return it */

        /* check tabs */
        if(check_obj_type(child, LV_HASP_TABVIEW)) {
            uint16_t tabcount = lv_tabview_get_tab_count(child);
            for(uint16_t i = 0; i < tabcount; i++) {
                lv_obj_t* tab = lv_tabview_get_tab(child, i);
                LOG_VERBOSE(TAG_HASP, "Found tab %i", i);
                if(tab->user_data.objid && objid == tab->user_data.objid) return tab; /* tab found, return it */

                /* check grandchildren */
                grandchild = hasp_find_obj_from_parent_id(tab, objid);
                if(grandchild) return grandchild; /* grandchild found, return it */
            }
        }

        /* try next sibling */
        child = lv_obj_get_child(parent, child);
    }
    return NULL;
}

// lv_obj_t * hasp_find_obj_from_page_id(uint8_t pageid, uint8_t objid)
// {
//     return hasp_find_obj_from_parent_id(get_page_obj(pageid), objid);
// }

bool hasp_find_id_from_obj(lv_obj_t* obj, uint8_t* pageid, uint8_t* objid)
{
    if(!haspPages.get_id(obj, pageid)) return false;
    if(!(obj->user_data.id > 0)) return false;
    //    memcpy(objid, &obj->user_data.id, sizeof(lv_obj_user_data_t));
    *objid = obj->user_data.id;
    return true;
}

/**
 * Check if an lvgl object typename corresponds to a given HASP object ID
 * @param lvobjtype a char* to a string
 * @param haspobjtype the HASP object ID to check against
 * @return true or false wether the types match
 * @note
 */
// bool check_obj_type_str(const char * lvobjtype, lv_hasp_obj_type_t haspobjtype)
// {
//     lvobjtype += 3; // skip "lv_"

//     switch(haspobjtype) {
//         case LV_HASP_BTNMATRIX:
//             return (strcmp_P(lvobjtype, PSTR("btnmatrix")) == 0);
//         case LV_HASP_TABLE:
//             return (strcmp_P(lvobjtype, PSTR("table")) == 0);
//         case LV_HASP_BUTTON:
//             return (strcmp_P(lvobjtype, PSTR("btn")) == 0);
//         case LV_HASP_LABEL:
//             return (strcmp_P(lvobjtype, PSTR("label")) == 0);
//         case LV_HASP_CHECKBOX:
//             return (strcmp_P(lvobjtype, PSTR("checkbox")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cb")) == 0);
//         case LV_HASP_DROPDOWN:
//             return (strcmp_P(lvobjtype, PSTR("dropdown")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_ddlist")) == 0);
//         case LV_HASP_CPICKER:
//             return (strcmp_P(lvobjtype, PSTR("cpicker")) == 0);
//         case LV_HASP_SPINNER:
//             return (strcmp_P(lvobjtype, PSTR("spinner")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_preload")) == 0);
//         case LV_HASP_SLIDER:
//             return (strcmp_P(lvobjtype, PSTR("slider")) == 0);
//         case LV_HASP_GAUGE:
//             return (strcmp_P(lvobjtype, PSTR("gauge")) == 0);
//         case LV_HASP_ARC:
//             return (strcmp_P(lvobjtype, PSTR("arc")) == 0);
//         case LV_HASP_BAR:
//             return (strcmp_P(lvobjtype, PSTR("bar")) == 0);
//         case LV_HASP_LMETER:
//             return (strcmp_P(lvobjtype, PSTR("linemeter")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_lmeter")) == 0)
//         case LV_HASP_ROLLER:
//             return (strcmp_P(lvobjtype, PSTR("roller")) == 0);
//         case LV_HASP_SWITCH:
//             return (strcmp_P(lvobjtype, PSTR("switch")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_sw")) == 0)
//         case LV_HASP_LED:
//             return (strcmp_P(lvobjtype, PSTR("led")) == 0);
//         case LV_HASP_IMAGE:
//             return (strcmp_P(lvobjtype, PSTR("img")) == 0);
//         case LV_HASP_IMGBTN:
//             return (strcmp_P(lvobjtype, PSTR("imgbtn")) == 0);
//         case LV_HASP_CONTAINER:
//             return (strcmp_P(lvobjtype, PSTR("container")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0)
//         case LV_HASP_OBJECT:
//             return (strcmp_P(lvobjtype, PSTR("page")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0)
//         case LV_HASP_PAGE:
//             return (strcmp_P(lvobjtype, PSTR("obj")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0)
//         case LV_HASP_TABVIEW:
//             return (strcmp_P(lvobjtype, PSTR("tabview")) == 0);
//         case LV_HASP_TILEVIEW:
//             return (strcmp_P(lvobjtype, PSTR("tileview")) == 0);
//         case LV_HASP_CHART:
//             return (strcmp_P(lvobjtype, PSTR("chart")) == 0);
//         case LV_HASP_CANVAS:
//             return (strcmp_P(lvobjtype, PSTR("canvas")) == 0);
//         case LV_HASP_CALENDER:
//             return (strcmp_P(lvobjtype, PSTR("calender")) == 0);
//         case LV_HASP_MSGBOX:
//             return (strcmp_P(lvobjtype, PSTR("msgbox")) == 0);
//         case LV_HASP_WINDOW:
//             return (strcmp_P(lvobjtype, PSTR("win")) == 0);

//         default:
//             return false;
//     }
// }

/**
 * Check if an lvgl objecttype name corresponds to a given HASP object ID
 * @param obj an lv_obj_t* of the object to check its type
 * @param haspobjtype the HASP object ID to check against
 * @return true or false wether the types match
 * @note
 */
bool check_obj_type(lv_obj_t* obj, lv_hasp_obj_type_t haspobjtype)
{
#if 1
    return obj->user_data.objid == (uint8_t)haspobjtype;
#else
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char* objtype = list.type[0];
    return check_obj_type(objtype, haspobjtype);
#endif
}

void hasp_object_tree(lv_obj_t* parent, uint8_t pageid, uint16_t level)
{
    if(parent == nullptr) return;

    /* Output parent info */
    lv_obj_type_t list;
    lv_obj_get_type(parent, &list);
    const char* objtype = list.type[0];
    LOG_VERBOSE(TAG_HASP, F("[%d] " HASP_OBJECT_NOTATION " %s"), level, pageid, parent->user_data.id, objtype);

    lv_obj_t* child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        /* child found, process it */
        hasp_object_tree(child, pageid, level + 1);

        /* try next sibling */
        child = lv_obj_get_child(parent, child);
    }

    /* check tabs */
    if(check_obj_type(parent, LV_HASP_TABVIEW)) {
#if 1
        uint16_t tabcount = lv_tabview_get_tab_count(parent);
        for(uint16_t i = 0; i < tabcount; i++) {
            lv_obj_t* tab = lv_tabview_get_tab(parent, i);
            LOG_VERBOSE(TAG_HASP, "Found tab %i", i);
            if(tab->user_data.objid) hasp_object_tree(tab, pageid, level + 1);
        }
#endif
    }
}

// ##################### Value Dispatchers ########################################################

void hasp_send_obj_attribute_str(lv_obj_t* obj, const char* attribute, const char* data)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        dispatch_send_obj_attribute_str(pageid, objid, attribute, data);
    }
}

void hasp_send_obj_attribute_int(lv_obj_t* obj, const char* attribute, int32_t val)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        dispatch_send_obj_attribute_int(pageid, objid, attribute, val);
    }
}

void hasp_send_obj_attribute_color(lv_obj_t* obj, const char* attribute, lv_color_t color)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        lv_color32_t c32;
        c32.full = lv_color_to32(color);
        dispatch_send_obj_attribute_color(pageid, objid, attribute, c32.ch.red, c32.ch.green, c32.ch.blue);
    }
}

// ##################### Value Senders ########################################################

// static void hasp_send_obj_attribute_P(lv_obj_t * obj, const char * attr, const char * data)
// {
//     char * buffer;
//     buffer = (char *)malloc(strlen_P(attr) + 1);
//     strcpy_P(buffer, attr);
//     hasp_send_obj_attribute_str(obj, buffer, data);
//     free(buffer);
// }

// static inline void hasp_obj_value_changed(lv_obj_t * obj, int32_t val)
// {
//     dispatch_object_value_changed(obj, val);
// char data[32];
// itoa(val, data, DEC);
// hasp_send_obj_attribute_P(obj, PSTR("val"), data);
//}

// static inline void hasp_send_obj_attribute_event(lv_obj_t * obj, const char * event)
// {
//     hasp_send_obj_attribute_P(obj, PSTR("event"), event);
// }

// static inline void hasp_send_obj_attribute_txt(lv_obj_t * obj, const char * txt)
// {
//     hasp_send_obj_attribute_P(obj, PSTR("txt"), txt);
// }

// ##################### Event Handlers ########################################################

/**
 * Called when a press on the system layer is detected
 * @param obj pointer to a button matrix
 * @param event type of event that occured
 */
void wakeup_event_handler(lv_obj_t* obj, lv_event_t event)
{
    if(obj == lv_disp_get_layer_sys(NULL)) {
        hasp_update_sleep_state(); // wakeup?

        if(event == LV_EVENT_CLICKED) {
            lv_obj_set_click(obj, false); // disable first touch
            LOG_VERBOSE(TAG_HASP, F("Wakeup touch disabled"));
        }
    }
}

/**
 * Called when a button-style object is clicked
 * @param obj pointer to a button object
 * @param event type of event that occured
 */
void generic_event_handler(lv_obj_t* obj, lv_event_t event)
{
    uint8_t eventid;

    switch(event) {
        case LV_EVENT_PRESSED:
            eventid              = HASP_EVENT_DOWN;
            last_press_was_short = false;
            break;

        case LV_EVENT_CLICKED:
            // UP = the same object was release then was pressed and press was not lost!
            eventid = last_press_was_short ? HASP_EVENT_SHORT : HASP_EVENT_UP;
            break;

        case LV_EVENT_SHORT_CLICKED:
            last_press_was_short = true; // Avoid SHORT + UP double events
            return;

            // eventid = HASP_EVENT_SHORT;
            // break;
        case LV_EVENT_LONG_PRESSED:
            eventid              = HASP_EVENT_LONG;
            last_press_was_short = false;
            break;

        case LV_EVENT_LONG_PRESSED_REPEAT:
            // last_press_was_short = false;
            return; // we don't care about hold
                    // eventid = HASP_EVENT_HOLD;
                    // break;
        case LV_EVENT_PRESS_LOST:
            eventid              = HASP_EVENT_LOST;
            last_press_was_short = false;
            break;

        case LV_EVENT_PRESSING:
        case LV_EVENT_FOCUSED:
        case LV_EVENT_DEFOCUSED:
        case LV_EVENT_RELEASED:
            return;

        case LV_EVENT_VALUE_CHANGED:
            LOG_WARNING(TAG_HASP, F("Value changed Event %d occured"), event); // Shouldn't happen in this event handler
            last_press_was_short = false;
            return;

        case LV_EVENT_DELETE:
            LOG_VERBOSE(TAG_HASP, F(D_OBJECT_DELETED));
            hasp_object_delete(obj); // free and destroy persistent memory allocated for certain objects
            last_press_was_short = false;
            return;

        default:
            LOG_WARNING(TAG_HASP, F(D_OBJECT_EVENT_UNKNOWN), event);
            last_press_was_short = false;
            return;
    }

    hasp_update_sleep_state(); // wakeup?

    /* If an actionid is attached, perform that action on UP event only */
    if(obj->user_data.actionid) {
        if(eventid == HASP_EVENT_UP || eventid == HASP_EVENT_SHORT) {
            lv_scr_load_anim_t transitionid = (lv_scr_load_anim_t)obj->user_data.transitionid;
            haspPages.set(obj->user_data.actionid, transitionid);
            dispatch_set_page(obj->user_data.actionid, transitionid);
        }
    } else {
        dispatch_object_generic_event(obj, eventid); // send object event
    }
    dispatch_normalized_group_value(obj->user_data.groupid, NORMALIZE(dispatch_get_event_state(eventid), 0, 1), obj);
}

/**
 * Called when a object state is toggled on/off
 * @param obj pointer to a switch object
 * @param event type of event that occured
 */
void toggle_event_handler(lv_obj_t* obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        char property[36]; // 4 for val only
        bool val = 0;
        hasp_update_sleep_state(); // wakeup?

        switch(obj->user_data.objid) {
            case LV_HASP_SWITCH:
                val = lv_switch_get_state(obj);
                break;

            case LV_HASP_CHECKBOX:
                val = lv_checkbox_is_checked(obj);
                break;

            case LV_HASP_BUTTON: {
                val = lv_obj_get_state(obj, LV_BTN_PART_MAIN) & LV_STATE_CHECKED;
                break;
            }

            default:
                return;
        }

        // snprintf_P(property, sizeof(property), PSTR("val"));
        // hasp_send_obj_attribute_int(obj, property, val);

        hasp_update_sleep_state(); // wakeup?
        dispatch_object_toggle_event(obj, val);
        dispatch_normalized_group_value(obj->user_data.groupid, NORMALIZE(val, 0, 1), obj);

    } else if(event == LV_EVENT_DELETE) {
        LOG_VERBOSE(TAG_HASP, F(D_OBJECT_DELETED));
        hasp_object_delete(obj);
    }
}

/**
 * Called when a range value has changed
 * @param obj pointer to a dropdown list or roller
 * @param event type of event that occured
 */
static void selector_event_handler(lv_obj_t* obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        char buffer[128];
        char property[36];
        uint16_t val = 0;
        uint16_t max = 0;
        hasp_update_sleep_state(); // wakeup?

        switch(obj->user_data.objid) {
            case LV_HASP_DROPDOWN:
                val = lv_dropdown_get_selected(obj);
                max = lv_dropdown_get_option_cnt(obj) - 1;
                lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
                break;

            case LV_HASP_ROLLER:
                val = lv_roller_get_selected(obj);
                max = lv_roller_get_option_cnt(obj) - 1;
                lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
                break;

            case LV_HASP_BTNMATRIX: {
                val             = lv_btnmatrix_get_active_btn(obj);
                const char* txt = lv_btnmatrix_get_btn_text(obj, val);
                strncpy(buffer, txt, sizeof(buffer));
                break;
            }

            case LV_HASP_TABLE: {
                uint16_t row;
                uint16_t col;
                if(lv_table_get_pressed_cell(obj, &row, &col) != LV_RES_OK) return; // outside any cell

                const char* txt = lv_table_get_cell_value(obj, row, col);
                strncpy(buffer, txt, sizeof(buffer));

                snprintf_P(property, sizeof(property), PSTR("row\":%d,\"col\":%d,\"text"), row, col);
                hasp_send_obj_attribute_str(obj, property, buffer);
                return;
            }

            default:
                return;
        }

        // set the property
        // snprintf_P(property, sizeof(property), PSTR("val\":%d,\"text"), val);
        // hasp_send_obj_attribute_str(obj, property, buffer);

        dispatch_object_selection_changed(obj, val, buffer);
        if(max > 0) dispatch_normalized_group_value(obj->user_data.groupid, NORMALIZE(val, 0, max), obj);

    } else if(event == LV_EVENT_DELETE) {
        LOG_VERBOSE(TAG_HASP, F(D_OBJECT_DELETED));
        hasp_object_delete(obj);
    }
}

/**
 * Called when a slider or adjustable arc is clicked
 * @param obj pointer to a slider
 * @param event type of event that occured
 */
void slider_event_handler(lv_obj_t* obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        /*        bool is_dragged;

                if(obj->user_data.objid == LV_HASP_SLIDER) {
                    is_dragged = lv_slider_is_dragged(obj);
                } else if(obj->user_data.objid == LV_HASP_ARC) {
                    is_dragged = lv_arc_is_dragged(obj);
                }

                if(is_dragged && (millis() - last_change_event < LV_INDEV_DEF_LONG_PRESS_TIME)) {
                    return;
                }
        */

        int16_t val = 0;
        int16_t min = 0;
        int16_t max = 0;
        hasp_update_sleep_state(); // wakeup?

        if(obj->user_data.objid == LV_HASP_SLIDER) {
            val = lv_slider_get_value(obj);
            min = lv_slider_get_min_value(obj);
            max = lv_slider_get_max_value(obj);
        } else if(obj->user_data.objid == LV_HASP_ARC) {
            val = lv_arc_get_value(obj);
            min = lv_arc_get_min_value(obj);
            max = lv_arc_get_max_value(obj);
        } else {
            return;
        }
        dispatch_object_value_changed(obj, val);
        dispatch_normalized_group_value(obj->user_data.groupid, NORMALIZE(val, min, max), obj);

    } else if(event == LV_EVENT_DELETE) {
        LOG_VERBOSE(TAG_HASP, F(D_OBJECT_DELETED));
        hasp_object_delete(obj);
    }
}

/**
 * Called when a color picker is clicked
 * @param obj pointer to a color picker
 * @param event type of event that occured
 */
static void cpicker_event_handler(lv_obj_t* obj, lv_event_t event)
{
    char color[6];
    snprintf_P(color, sizeof(color), PSTR("color"));

    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_update_sleep_state(); // wakeup?
        // hasp_send_obj_attribute_color(obj, color, lv_cpicker_get_color(obj));
        dispatch_object_color_changed(obj, lv_cpicker_get_color(obj));

    } else if(event == LV_EVENT_DELETE) {
        LOG_VERBOSE(TAG_HASP, F(D_OBJECT_DELETED));
        hasp_object_delete(obj);
    }
}

// ##################### State Changers ########################################################

// TODO make this a recursive function that goes over all objects only ONCE
void object_set_group_state(uint8_t groupid, uint8_t eventid, lv_obj_t* src_obj)
{
    if(groupid == 0) return;
    bool state = dispatch_get_event_state(eventid);

    for(uint8_t page = 0; page < HASP_NUM_PAGES; page++) {
        uint8_t startid = 1;
        for(uint8_t objid = startid; objid < 20; objid++) {
            lv_obj_t* obj = hasp_find_obj_from_parent_id(haspPages.get_obj(page), objid);
            if(obj && obj != src_obj && obj->user_data.groupid == groupid) { // skip source object, if set
                lv_obj_set_state(obj, state ? LV_STATE_PRESSED | LV_STATE_CHECKED : LV_STATE_DEFAULT);
            }
        }
    }
}

void object_set_group_value(lv_obj_t* parent, uint8_t groupid, const char* payload)
{
    if(groupid == 0 || parent == nullptr) return;

    lv_obj_t* child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        /* child found, update it */
        if(groupid == child->user_data.groupid) hasp_process_obj_attribute_val(child, NULL, payload, true);

        /* update grandchildren */
        object_set_group_value(child, groupid, payload);

        /* check tabs */
        if(check_obj_type(child, LV_HASP_TABVIEW)) {
            //#if LVGL_VERSION_MAJOR == 7
            uint16_t tabcount = lv_tabview_get_tab_count(child);
            for(uint16_t i = 0; i < tabcount; i++) {
                lv_obj_t* tab = lv_tabview_get_tab(child, i);
                LOG_VERBOSE(TAG_HASP, F("Found tab %i"), i);
                if(tab->user_data.groupid && groupid == tab->user_data.groupid)
                    hasp_process_obj_attribute_val(tab, NULL, payload, true); /* tab found, update it */

                /* check grandchildren */
                object_set_group_value(tab, groupid, payload);
            }
            //#endif
        }

        /* try next sibling */
        child = lv_obj_get_child(parent, child);
    }
}

void object_set_group_value(uint8_t groupid, int16_t state)
{
    char payload[16];
    itoa(state, payload, DEC);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Used in the dispatcher & hasp_new_object
void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char* attr, const char* payload)
{
    if(lv_obj_t* obj = hasp_find_obj_from_parent_id(haspPages.get_obj(pageid), objid)) {
        hasp_process_obj_attribute(obj, attr, payload, strlen(payload) > 0);
    } else {
        LOG_WARNING(TAG_HASP, F(D_OBJECT_UNKNOWN " " HASP_OBJECT_NOTATION), pageid, objid);
    }
}

// ##################### Object Creator ########################################################

int hasp_parse_json_attributes(lv_obj_t* obj, const JsonObject& doc)
{
    int i = 0;
#if defined(WINDOWS) || defined(POSIX)
    // String v((char *)0);
    // v.reserve(64);
    std::string v;

    for(JsonPair keyValue : doc) {
        LOG_VERBOSE(TAG_HASP, F(D_BULLET "%s=%s"), keyValue.key().c_str(), keyValue.value().as<std::string>().c_str());
        v = keyValue.value().as<std::string>();
        hasp_process_obj_attribute(obj, keyValue.key().c_str(), keyValue.value().as<std::string>().c_str(), true);
        i++;
    }
#else
    String v((char*)0);
    v.reserve(64);

    for(JsonPair keyValue : doc) {
        LOG_DEBUG(TAG_HASP, F(D_BULLET "%s=%s"), keyValue.key().c_str(), keyValue.value().as<String>().c_str());
        v = keyValue.value().as<String>();
        hasp_process_obj_attribute(obj, keyValue.key().c_str(), keyValue.value().as<String>().c_str(), true);
        i++;
    }
#endif
    LOG_VERBOSE(TAG_HASP, F("%d attributes processed"), i);
    return i;
}

/**
 * Create a new object according to the json config
 * @param config Json representation for this object
 * @param saved_page_id the pageid to use when no pageid is specified in the Json, updated when it is specified so
 * following objects in the file can share the pageid
 */
void hasp_new_object(const JsonObject& config, uint8_t& saved_page_id)
{
    /* Page selection */
    uint8_t pageid = saved_page_id;
    if(!config[FPSTR(FP_PAGE)].isNull()) {
        config[FPSTR(FP_PAGE)].as<uint8_t>();
        config.remove(FPSTR(FP_PAGE));
    }

    /* Page with pageid is the default parent_obj */
    lv_obj_t* parent_obj = haspPages.get_obj(pageid);
    if(!parent_obj) {
        LOG_WARNING(TAG_HASP, F(D_OBJECT_PAGE_UNKNOWN), pageid);
        return;
    } else {
        saved_page_id = pageid; /* save the current pageid for next objects */
    }

    /* A custom parentid was set */
    if(!config[FPSTR(FP_PARENTID)].isNull()) {
        uint8_t parentid = config[FPSTR(FP_PARENTID)].as<uint8_t>();
        parent_obj       = hasp_find_obj_from_parent_id(parent_obj, parentid);
        if(!parent_obj) {
            LOG_WARNING(TAG_HASP, F("Parent ID " HASP_OBJECT_NOTATION " not found, skipping..."), pageid, parentid);
            return;
        } else {
            LOG_VERBOSE(TAG_HASP, F("Parent ID " HASP_OBJECT_NOTATION " found"), pageid, parentid);
        }
        config.remove(FPSTR(FP_PARENTID));
    }

    uint16_t sdbm   = 0;
    uint8_t groupid = config[FPSTR(FP_GROUPID)].as<uint8_t>();
    uint8_t id      = config[FPSTR(FP_ID)].as<uint8_t>();
    config.remove(FPSTR(FP_ID));

    /* Create the object if it does not exist */
    lv_obj_t* obj = hasp_find_obj_from_parent_id(parent_obj, id);
    if(!obj) {

        /* Create the object first */

        /* Validate type */
        if(config[FPSTR(FP_OBJID)].isNull()) { // TODO: obsolete objid
            if(config[FPSTR(FP_OBJ)].isNull()) {
                return; // comments
            } else {
                sdbm = Utilities::get_sdbm(config[FPSTR(FP_OBJ)].as<const char*>());
                config.remove(FPSTR(FP_OBJ));
            }
        } else {
            sdbm = config[FPSTR(FP_OBJID)].as<uint8_t>();
            config.remove(FPSTR(FP_OBJID));
        }

        switch(sdbm) {
            /* ----- Basic Objects ------ */
            case LV_HASP_BTNMATRIX:
            case HASP_OBJ_BTNMATRIX:
                obj = lv_btnmatrix_create(parent_obj, NULL);
                if(obj) {
                    lv_btnmatrix_set_recolor(obj, true);
                    lv_obj_set_event_cb(obj, selector_event_handler);

                    lv_btnmatrix_ext_t* ext = (lv_btnmatrix_ext_t*)lv_obj_get_ext_attr(obj);
                    btnmatrix_default_map   = ext->map_p; // store the static pointer to the default lvgl btnmap
                    obj->user_data.objid    = LV_HASP_BTNMATRIX;
                }
                break;

            case LV_HASP_TABLE:
            case HASP_OBJ_TABLE:
                obj = lv_table_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, selector_event_handler);
                    obj->user_data.objid = LV_HASP_TABLE;
                }
                break;

            case LV_HASP_BUTTON:
            case HASP_OBJ_BTN:
                obj = lv_btn_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_t* lbl = lv_label_create(obj, NULL);
                    if(lbl) {
                        lv_label_set_text(lbl, "");
                        lv_label_set_recolor(lbl, true);
                        lbl->user_data.objid = LV_HASP_LABEL;
                        lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);
                    }
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_BUTTON;
                }
                break;

            case LV_HASP_CHECKBOX:
            case HASP_OBJ_CHECKBOX:
                obj = lv_checkbox_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, toggle_event_handler);
                    obj->user_data.objid = LV_HASP_CHECKBOX;
                }
                break;

            case LV_HASP_LABEL:
            case HASP_OBJ_LABEL:
                obj = lv_label_create(parent_obj, NULL);
                if(obj) {
                    lv_label_set_long_mode(obj, LV_LABEL_LONG_CROP);
                    lv_label_set_recolor(obj, true);
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_LABEL;
                }
                break;

            case LV_HASP_IMAGE:
            case HASP_OBJ_IMG:
                obj = lv_img_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_IMAGE;
                }
                break;

            case LV_HASP_ARC:
            case HASP_OBJ_ARC:
                obj = lv_arc_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_ARC;
                }
                break;

            case LV_HASP_CONTAINER:
            case HASP_OBJ_CONT:
                obj = lv_cont_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_CONTAINER;
                }
                break;

            case LV_HASP_OBJECT:
            case HASP_OBJ_OBJ:
                obj = lv_obj_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_OBJECT;
                }
                break;

            case LV_HASP_PAGE:
            case HASP_OBJ_PAGE:
                obj = lv_page_create(parent_obj, NULL);
                if(obj) obj->user_data.objid = LV_HASP_PAGE;
                // No event handler for pages
                break;

#if LV_USE_WIN && LVGL_VERSION_MAJOR == 7
            case LV_HASP_WINDOW:
            case HASP_OBJ_WIN:
                obj = lv_win_create(parent_obj, NULL);
                if(obj) obj->user_data.objid = LV_HASP_WINDOW;
                // No event handler for pages
                break;

#endif

#if LVGL_VERSION_MAJOR == 8
            case LV_HASP_LED:
            case HASP_OBJ_LED:
                obj = lv_led_create(parent_obj);
                if(obj) {
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_LED;
                }
                break;

            case LV_HASP_TILEVIEW:
            case HASP_OBJ_TILEVIEW:
                obj = lv_tileview_create(parent_obj);
                if(obj) obj->user_data.objid = LV_HASP_TILEVIEW;
                // No event handler for tileviews
                break;

            case LV_HASP_TABVIEW:
            case HASP_OBJ_TABVIEW:
                obj = lv_tabview_create(parent_obj, LV_DIR_TOP, 100);
                // No event handler for tabs
                if(obj) {
                    lv_obj_t* tab;
                    tab = lv_tabview_add_tab(obj, "tab 1");
                    // lv_obj_set_user_data(tab, id + 1);
                    tab = lv_tabview_add_tab(obj, "tab 2");
                    // lv_obj_set_user_data(tab, id + 2);
                    tab = lv_tabview_add_tab(obj, "tab 3");
                    // lv_obj_set_user_data(tab, id + 3);

                    obj->user_data.objid = LV_HASP_TABVIEW;
                }
                break;

#else
            case LV_HASP_LED:
            case HASP_OBJ_LED:
                obj = lv_led_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_LED;
                }
                break;

            case LV_HASP_TILEVIEW:
            case HASP_OBJ_TILEVIEW:
                obj = lv_tileview_create(parent_obj, NULL);
                if(obj) obj->user_data.objid = LV_HASP_TILEVIEW;

                // No event handler for tileviews
                break;

            case LV_HASP_TABVIEW:
            case HASP_OBJ_TABVIEW:
                obj = lv_tabview_create(parent_obj, NULL);
                // No event handler for tabs
                if(obj) {
                    lv_obj_t* tab;
                    tab = lv_tabview_add_tab(obj, "tab 1");
                    // lv_obj_set_user_data(tab, id + 1);
                    tab = lv_tabview_add_tab(obj, "tab 2");
                    // lv_obj_set_user_data(tab, id + 2);
                    tab = lv_tabview_add_tab(obj, "tab 3");
                    // lv_obj_set_user_data(tab, id + 3);

                    obj->user_data.objid = LV_HASP_TABVIEW;
                }
                break;

#endif
            /* ----- Color Objects ------ */
            case LV_HASP_CPICKER:
            case HASP_OBJ_CPICKER:
                obj = lv_cpicker_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, cpicker_event_handler);
                    obj->user_data.objid = LV_HASP_CPICKER;
                }
                break;

#if LV_USE_SPINNER != 0
            case LV_HASP_SPINNER:
            case HASP_OBJ_SPINNER:
                obj = lv_spinner_create(parent_obj, NULL);
                if(obj) obj->user_data.objid = LV_HASP_SPINNER;
                break;

#endif
            /* ----- Range Objects ------ */
            case LV_HASP_SLIDER:
            case HASP_OBJ_SLIDER:
                obj = lv_slider_create(parent_obj, NULL);
                if(obj) {
                    lv_slider_set_range(obj, 0, 100);
                    lv_obj_set_event_cb(obj, slider_event_handler);
                    obj->user_data.objid = LV_HASP_SLIDER;
                }
                // bool knobin = config[F("knobin")].as<bool>() | true;
                // lv_slider_set_knob_in(obj, knobin);
                break;

            case LV_HASP_GAUGE:
            case HASP_OBJ_GAUGE:
                obj = lv_gauge_create(parent_obj, NULL);
                if(obj) {
                    lv_gauge_set_range(obj, 0, 100);
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_GAUGE;
                }
                break;

            case LV_HASP_BAR:
            case HASP_OBJ_BAR:
                obj = lv_bar_create(parent_obj, NULL);
                if(obj) {
                    lv_bar_set_range(obj, 0, 100);
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_BAR;
                }
                break;

            case LV_HASP_LMETER:
            case HASP_OBJ_LMETER:
                obj = lv_linemeter_create(parent_obj, NULL);
                if(obj) {
                    lv_linemeter_set_range(obj, 0, 100);
                    lv_obj_set_event_cb(obj, generic_event_handler);
                    obj->user_data.objid = LV_HASP_LMETER;
                }
                break;

            case LV_HASP_CHART:
            case HASP_OBJ_CHART:
                obj = lv_chart_create(parent_obj, NULL);
                if(obj) {
                    lv_chart_set_range(obj, 0, 100);
                    lv_obj_set_event_cb(obj, generic_event_handler);

                    lv_chart_add_series(obj, LV_COLOR_RED);
                    lv_chart_add_series(obj, LV_COLOR_GREEN);
                    lv_chart_add_series(obj, LV_COLOR_BLUE);

                    lv_chart_series_t* ser = my_chart_get_series(obj, 2);
                    lv_chart_set_next(obj, ser, 10);
                    lv_chart_set_next(obj, ser, 20);
                    lv_chart_set_next(obj, ser, 30);
                    lv_chart_set_next(obj, ser, 40);

                    obj->user_data.objid = LV_HASP_CHART;
                }
                break;

            /* ----- On/Off Objects ------ */
            case LV_HASP_SWITCH:
            case HASP_OBJ_SWITCH:
                obj = lv_switch_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_set_event_cb(obj, toggle_event_handler);
                    obj->user_data.objid = LV_HASP_SWITCH;
                }
                break;

            /* ----- List Object ------- */
            case LV_HASP_DROPDOWN:
            case HASP_OBJ_DROPDOWN:
                obj = lv_dropdown_create(parent_obj, NULL);
                if(obj) {
                    lv_dropdown_set_draw_arrow(obj, true);
                    // lv_dropdown_set_anim_time(obj, 200);
                    lv_obj_set_top(obj, true);
                    // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
                    lv_obj_set_event_cb(obj, selector_event_handler);
                    obj->user_data.objid = LV_HASP_DROPDOWN;
                }
                break;

            case LV_HASP_ROLLER:
            case HASP_OBJ_ROLLER:
                obj = lv_roller_create(parent_obj, NULL);
                // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
                if(obj) {
                    lv_roller_set_auto_fit(obj, false);
                    lv_obj_set_event_cb(obj, selector_event_handler);
                    obj->user_data.objid = LV_HASP_ROLLER;
                }
                break;

                /* ----- Other Object ------ */
                // default:
                //    return LOG_WARNING(TAG_HASP, F("Unsupported Object ID %u"), objid);
        }

        /* No object was actually created */
        if(!obj) {
            LOG_ERROR(TAG_HASP, F(D_OBJECT_CREATE_FAILED), id);
            return;
        }

        // Prevent losing press when the press is slid out of the objects.
        // (E.g. a Button can be released out of it if it was being pressed)
        lv_obj_add_protect(obj, LV_PROTECT_PRESS_LOST);

        /* id tag the object */
        // lv_obj_set_user_data(obj, id);
        obj->user_data.id = id;
        // obj->user_data.groupid = groupid; // get/set in atttr

        /** testing start **/
        uint8_t temp;
        if(!hasp_find_id_from_obj(obj, &pageid, &temp)) {
            LOG_ERROR(TAG_HASP, F(D_OBJECT_LOST));
            return;
        }

        /** verbose reporting **/
        lv_obj_type_t list;
        lv_obj_get_type(obj, &list);
        LOG_VERBOSE(TAG_HASP, F(D_BULLET HASP_OBJECT_NOTATION " = %s"), pageid, temp, list.type[0]);

        /* test double-check */
        lv_obj_t* test = hasp_find_obj_from_parent_id(haspPages.get_obj(pageid), (uint8_t)temp);
        if(test != obj) {
            LOG_ERROR(TAG_HASP, F(D_OBJECT_MISMATCH));
            return;
        } else {
            // object created successfully
        }

    } else {
        // object already exists
    }

    hasp_parse_json_attributes(obj, config);
}

void hasp_object_delete(lv_obj_t* obj)
{
    switch(obj->user_data.objid) {
        case LV_HASP_LINE:
            line_clear_points(obj);
            break;

        case LV_HASP_BTNMATRIX:
            my_btnmatrix_map_clear(obj);
            _LV_WIN_PART_REAL_LAST;
            _LV_WIN_PART_VIRTUAL_LAST;
            break;

        case LV_HASP_GAUGE:
            break;
    }

    // TODO: delete value_str data for ALL parts
    my_obj_set_value_str_txt(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
}
