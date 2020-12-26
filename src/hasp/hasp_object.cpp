/* MIT License - Copyright (c) 2020 Francis Van Roie
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

#include "ArduinoLog.h"

#include "lvgl.h"
#if LVGL_VERSION_MAJOR != 7
#include "../lv_components.h"
#endif

#include "hasp.h"
#include "hasp_object.h"
#include "hasp_dispatch.h"
#include "hasp_attribute.h"

// ##################### Object Finders ########################################################

lv_obj_t * hasp_find_obj_from_parent_id(lv_obj_t * parent, uint8_t objid)
{
    if(objid == 0 || parent == nullptr) return parent;

    lv_obj_t * child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        /* child found, return it */
        if(objid == child->user_data.id) return child;

        /* check grandchildren */
        lv_obj_t * grandchild = hasp_find_obj_from_parent_id(child, objid);
        if(grandchild) return grandchild; /* grandchild found, return it */

        /* check tabs */
        if(check_obj_type(child, LV_HASP_TABVIEW)) {
            //#if LVGL_VERSION_MAJOR == 7
            uint16_t tabcount = lv_tabview_get_tab_count(child);
            for(uint16_t i = 0; i < tabcount; i++) {
                lv_obj_t * tab = lv_tabview_get_tab(child, i);
                Log.verbose(TAG_HASP, "Found tab %i", i);
                if(tab->user_data.objid && objid == tab->user_data.objid) return tab; /* tab found, return it */

                /* check grandchildren */
                grandchild = hasp_find_obj_from_parent_id(tab, objid);
                if(grandchild) return grandchild; /* grandchild found, return it */
            }
            //#endif
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

bool hasp_find_id_from_obj(lv_obj_t * obj, uint8_t * pageid, uint8_t * objid)
{
    if(!get_page_id(obj, pageid)) return false;
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
bool check_obj_type_str(const char * lvobjtype, lv_hasp_obj_type_t haspobjtype)
{
    lvobjtype += 3; // skip "lv_"

    switch(haspobjtype) {
        case LV_HASP_BTNMATRIX:
            return (strcmp_P(lvobjtype, PSTR("btnmatrix")) == 0);
        case LV_HASP_TABLE:
            return (strcmp_P(lvobjtype, PSTR("table")) == 0);
        case LV_HASP_BUTTON:
            return (strcmp_P(lvobjtype, PSTR("btn")) == 0);
        case LV_HASP_LABEL:
            return (strcmp_P(lvobjtype, PSTR("label")) == 0);
        case LV_HASP_CHECKBOX:
            return (strcmp_P(lvobjtype, PSTR("checkbox")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cb")) == 0);
        case LV_HASP_DDLIST:
            return (strcmp_P(lvobjtype, PSTR("dropdown")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_ddlist")) == 0);
        case LV_HASP_CPICKER:
            return (strcmp_P(lvobjtype, PSTR("cpicker")) == 0);
        case LV_HASP_PRELOADER:
            return (strcmp_P(lvobjtype, PSTR("spinner")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_preload")) == 0);
        case LV_HASP_SLIDER:
            return (strcmp_P(lvobjtype, PSTR("slider")) == 0);
        case LV_HASP_GAUGE:
            return (strcmp_P(lvobjtype, PSTR("gauge")) == 0);
        case LV_HASP_ARC:
            return (strcmp_P(lvobjtype, PSTR("arc")) == 0);
        case LV_HASP_BAR:
            return (strcmp_P(lvobjtype, PSTR("bar")) == 0);
        case LV_HASP_LMETER:
            return (strcmp_P(lvobjtype, PSTR("linemeter")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_lmeter")) == 0)
        case LV_HASP_ROLLER:
            return (strcmp_P(lvobjtype, PSTR("roller")) == 0);
        case LV_HASP_SWITCH:
            return (strcmp_P(lvobjtype, PSTR("switch")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_sw")) == 0)
        case LV_HASP_LED:
            return (strcmp_P(lvobjtype, PSTR("led")) == 0);
        case LV_HASP_IMAGE:
            return (strcmp_P(lvobjtype, PSTR("img")) == 0);
        case LV_HASP_IMGBTN:
            return (strcmp_P(lvobjtype, PSTR("imgbtn")) == 0);
        case LV_HASP_CONTAINER:
            return (strcmp_P(lvobjtype, PSTR("container")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0)
        case LV_HASP_OBJECT:
            return (strcmp_P(lvobjtype, PSTR("page")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0)
        case LV_HASP_PAGE:
            return (strcmp_P(lvobjtype, PSTR("obj")) == 0); // || (strcmp_P(lvobjtype, PSTR("lv_cont")) == 0)
        case LV_HASP_TABVIEW:
            return (strcmp_P(lvobjtype, PSTR("tabview")) == 0);
        case LV_HASP_TILEVIEW:
            return (strcmp_P(lvobjtype, PSTR("tileview")) == 0);
        case LV_HASP_CHART:
            return (strcmp_P(lvobjtype, PSTR("chart")) == 0);
        case LV_HASP_CANVAS:
            return (strcmp_P(lvobjtype, PSTR("canvas")) == 0);
        case LV_HASP_CALENDER:
            return (strcmp_P(lvobjtype, PSTR("calender")) == 0);
        case LV_HASP_MSGBOX:
            return (strcmp_P(lvobjtype, PSTR("msgbox")) == 0);
        case LV_HASP_WINDOW:
            return (strcmp_P(lvobjtype, PSTR("win")) == 0);

        default:
            return false;
    }
}

/**
 * Check if an lvgl objecttype name corresponds to a given HASP object ID
 * @param obj an lv_obj_t* of the object to check its type
 * @param haspobjtype the HASP object ID to check against
 * @return true or false wether the types match
 * @note
 */
bool check_obj_type(lv_obj_t * obj, lv_hasp_obj_type_t haspobjtype)
{
#if 1
    return obj->user_data.objid == (uint8_t)haspobjtype;
#else
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];
    return check_obj_type(objtype, haspobjtype);
#endif
}

void hasp_object_tree(lv_obj_t * parent, uint8_t pageid, uint16_t level)
{
    if(parent == nullptr) return;

    /* Output parent info */
    lv_obj_type_t list;
    lv_obj_get_type(parent, &list);
    const char * objtype = list.type[0];
    Log.verbose(TAG_HASP, F("[%d] p[%d].b[%d] %s"), level, pageid, parent->user_data, objtype);

    lv_obj_t * child;
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
            lv_obj_t * tab = lv_tabview_get_tab(child, i);
            Log.verbose(TAG_HASP, "Found tab %i", i);
            if(tab->user_data.objid) hasp_object_tree(tab, pageid, level + 1);
        }
#endif
    }
}

// ##################### Value Dispatchers ########################################################

void hasp_send_obj_attribute_str(lv_obj_t * obj, const char * attribute, const char * data)
{
    uint8_t pageid;
    uint8_t objid;

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        dispatch_send_obj_attribute_str(pageid, objid, attribute, data);
    }
}

void hasp_send_obj_attribute_int(lv_obj_t * obj, const char * attribute, int32_t val)
{
    char data[64];
    itoa(val, data, 10);
    hasp_send_obj_attribute_str(obj, attribute, data);
}

void hasp_send_obj_attribute_color(lv_obj_t * obj, const char * attribute, lv_color_t color)
{
    char buffer[16];
    lv_color32_t c32;
    c32.full = lv_color_to32(color);
    snprintf(buffer, sizeof(buffer), PSTR("#%02x%02x%02x"), c32.ch.red, c32.ch.green, c32.ch.blue);
    hasp_send_obj_attribute_str(obj, attribute, buffer);
}

// ##################### Value Senders ########################################################

static void hasp_send_obj_attribute_P(lv_obj_t * obj, const char * attr, const char * data)
{
    char * buffer;
    buffer = (char *)malloc(strlen_P(attr) + 1);
    strcpy_P(buffer, attr);
    hasp_send_obj_attribute_str(obj, buffer, data);
    free(buffer);
}

static inline void hasp_send_obj_attribute_val(lv_obj_t * obj, int32_t val)
{
    char data[32];
    itoa(val, data, DEC);
    hasp_send_obj_attribute_P(obj, PSTR("val"), data);
}

static inline void hasp_send_obj_attribute_event(lv_obj_t * obj, const char * event)
{
    hasp_send_obj_attribute_P(obj, PSTR("event"), event);
}

static inline void hasp_send_obj_attribute_txt(lv_obj_t * obj, const char * txt)
{
    hasp_send_obj_attribute_P(obj, PSTR("txt"), txt);
}

// ##################### Event Handlers ########################################################

/**
 * Called when a button-style object is clicked
 * @param obj pointer to a button object
 * @param event type of event that occured
 */
void IRAM_ATTR btn_event_handler(lv_obj_t * obj, lv_event_t event)
{
    uint8_t eventid;

    switch(event) {
        case LV_EVENT_PRESSED:
            eventid = HASP_EVENT_DOWN;
            break;
        case LV_EVENT_CLICKED:
            // UP = the same object was release then was pressed and press was not lost!
            eventid = HASP_EVENT_UP;
            break;
        case LV_EVENT_SHORT_CLICKED:
            eventid = HASP_EVENT_SHORT;
            break;
        case LV_EVENT_LONG_PRESSED:
            eventid = HASP_EVENT_LONG;
            break;
        case LV_EVENT_LONG_PRESSED_REPEAT:
            return; // we don't care about hold
                    // eventid = HASP_EVENT_HOLD;
                    // break;
        case LV_EVENT_PRESS_LOST:
            eventid = HASP_EVENT_LOST;
            break;
        case LV_EVENT_PRESSING:
        case LV_EVENT_FOCUSED:
        case LV_EVENT_DEFOCUSED:
        case LV_EVENT_RELEASED:
            return;

        case LV_EVENT_VALUE_CHANGED:
            Log.warning(TAG_HASP, F("Value changed Event %d occured"), event);
            return;

        case LV_EVENT_DELETE:
            Log.verbose(TAG_HASP, F("Object deleted Event %d occured"), event);
            // TODO:free and destroy persistent memory allocated for certain objects
            return;
        default:
            Log.warning(TAG_HASP, F("Unknown Event %d occured"), event);
            return;
    }

    hasp_update_sleep_state();                          // wakeup?
    dispatch_object_event(obj, eventid); // send object event
}

/**
 * Called when a press on the system layer is detected
 * @param obj pointer to a button matrix
 * @param event type of event that occured
 */
void wakeup_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(obj == lv_disp_get_layer_sys(NULL)) {
        hasp_update_sleep_state();              // wakeup?
        lv_obj_set_click(obj, false); // disable fist click
    }
}

/**
 * Called when a button matrix object is clicked
 * @param obj pointer to a button matrix
 * @param event type of event that occured
 */
static void btnmap_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_update_sleep_state(); // wakeup?
        hasp_send_obj_attribute_val(obj, lv_btnmatrix_get_active_btn(obj));
    }
}

/**
 * Called when a table object is clicked
 * @param obj pointer to a table
 * @param event type of event that occured
 */
static void table_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_update_sleep_state(); // wakeup?

        uint16_t row;
        uint16_t col;
        if(lv_table_get_pressed_cell(obj, &row, &col) == LV_RES_OK) hasp_send_obj_attribute_val(obj, row);
    }
}

/**
 * Called when a toggle button is clicked
 * @param obj pointer to a button
 * @param event type of event that occured
 */
void IRAM_ATTR toggle_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_update_sleep_state(); // wakeup?
        hasp_send_obj_attribute_val(obj, lv_checkbox_is_checked(obj));
    }
}

/**
 * Called when a switch is toggled
 * @param obj pointer to a switch object
 * @param event type of event that occured
 */
static void switch_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_update_sleep_state(); // wakeup?
        hasp_send_obj_attribute_val(obj, lv_switch_get_state(obj));
    }
}

/**
 * Called when a checkboxed is clicked
 * @param obj pointer to a checkbox
 * @param event type of event that occured
 */
static void checkbox_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_obj_attribute_val(obj, lv_checkbox_is_checked(obj));
}

/**
 * Called when a dropdown list is clicked
 * @param obj pointer to a dropdown list
 * @param event type of event that occured
 */
static void ddlist_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_send_obj_attribute_val(obj, lv_dropdown_get_selected(obj));
        char buffer[128];
        lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
        hasp_send_obj_attribute_txt(obj, buffer);
    }
}

/**
 * Called when a slider is clicked
 * @param obj pointer to a slider
 * @param event type of event that occured
 */
static void slider_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_obj_attribute_val(obj, lv_slider_get_value(obj));
}

/**
 * Called when a color picker is clicked
 * @param obj pointer to a color picker
 * @param event type of event that occured
 */
static void cpicker_event_handler(lv_obj_t * obj, lv_event_t event)
{
    char color[6];
    snprintf_P(color, sizeof(color), PSTR("color"));

    if(event == LV_EVENT_VALUE_CHANGED) hasp_send_obj_attribute_color(obj, color, lv_cpicker_get_color(obj));
}

/**
 * Called when a roller object is clicked
 * @param obj pointer to a roller object
 * @param event type of event that occured
 */
static void roller_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        hasp_send_obj_attribute_val(obj, lv_roller_get_selected(obj));
        char buffer[128];
        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
        hasp_send_obj_attribute_txt(obj, buffer);
    }
}

// ##################### State Changers ########################################################

// TODO make this a recursive function that goes over all objects only ONCE
void object_set_group_state(uint8_t groupid, uint8_t eventid, lv_obj_t * src_obj)
{
    bool state = dispatch_get_event_state(eventid);
    for(uint8_t page = 0; page < HASP_NUM_PAGES; page++) {
        uint8_t startid = 100 + groupid * 10; // groups start at id 100
        for(uint8_t objid = startid; objid < (startid + 10); objid++) {
            lv_obj_t * obj = hasp_find_obj_from_parent_id(get_page_obj(page), objid);
            if(obj && obj != src_obj) { // skip source object, if set
                lv_obj_set_state(obj, state ? LV_STATE_PRESSED | LV_STATE_CHECKED : LV_STATE_DEFAULT);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Used in the dispatcher & hasp_new_object
void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char * attr, const char * payload)
{
    if(lv_obj_t * obj = hasp_find_obj_from_parent_id(get_page_obj(pageid), objid)) {
        hasp_process_obj_attribute(obj, attr, payload, strlen(payload) > 0);
    } else {
        Log.warning(TAG_HASP, F("Unknown object p[%d].b[%d]"), pageid, objid);
    }
}

// ##################### Object Creator ########################################################

/**
 * Create a new object according to the json config
 * @param config Json representation for this object
 * @param saved_page_id the pageid to use when no pageid is specified in the Json, updated when it is specified so
 * following objects in the file can share the pageid
 */
void hasp_new_object(const JsonObject & config, uint8_t & saved_page_id)
{
    /* Validate page */
    // uint8_t pageid = config[F("page")].isNull() ? haspGetPage() : config[F("page")].as<uint8_t>();
    uint8_t pageid = config[F("page")].isNull() ? saved_page_id : config[F("page")].as<uint8_t>();

    /* Page selection */
    lv_obj_t * page = get_page_obj(pageid);
    if(!page) {
        return Log.warning(TAG_HASP, F("Page ID %u not defined"), pageid);
    } else {
        saved_page_id = pageid; /* save the current pageid */
    }

    /* Validate type */
    if(config[F("objid")].isNull()) return; // comments

    lv_obj_t * parent_obj = page;
    if(!config[F("parentid")].isNull()) {
        uint8_t parentid = config[F("parentid")].as<uint8_t>();
        parent_obj       = hasp_find_obj_from_parent_id(page, parentid);
        if(!parent_obj) {
            return Log.warning(TAG_HASP, F("Parent ID p[%u].b[%u] not found, skipping..."), pageid, parentid);
            // parent_obj = page; // don't create on the page instead ??
        } else {
            Log.verbose(TAG_HASP, F("Parent ID p[%u].b[%u] found"), pageid, parentid);
        }
    }

    uint8_t groupid = config[F("groupid")].as<uint8_t>();
    uint8_t objid   = config[F("objid")].as<uint8_t>();
    uint8_t id      = config[F("id")].as<uint8_t>();

    /* Define Objects*/
    lv_obj_t * obj = hasp_find_obj_from_parent_id(parent_obj, id);
    if(obj) {
        return Log.warning(TAG_HASP, F("Object ID %u already exists!"), id);
    }

    switch(objid) {
        /* ----- Basic Objects ------ */
        case LV_HASP_BTNMATRIX:
            obj = lv_btnmatrix_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, btnmap_event_handler);
            break;
        case LV_HASP_TABLE:
            obj = lv_table_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, table_event_handler);
            break;

        case LV_HASP_BUTTON:
            obj = lv_btn_create(parent_obj, NULL);
            if(obj) {
                lv_obj_t * lbl = lv_label_create(obj, NULL);
                if(lbl) {
                    lv_label_set_text(lbl, "");
                    lbl->user_data.objid = LV_HASP_LABEL;
                    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);
                }
                lv_obj_set_event_cb(obj, btn_event_handler);
            }
            break;

        case LV_HASP_CHECKBOX:
            obj = lv_checkbox_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, checkbox_event_handler);
            break;

        case LV_HASP_LABEL: {
            obj = lv_label_create(parent_obj, NULL);
            /* click area padding */
            //  uint8_t padh = config[F("padh")].as<uint8_t>();
            //  uint8_t padv = config[F("padv")].as<uint8_t>();
            /* text align */
            // if(padh > 0 || padv > 0) {
            //     lv_obj_set_ext_click_area(obj, padh, padh, padv, padv);
            // }
            // if(!config[F("align")].isNull()) {
            //     lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
            // }
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_IMAGE: {
            obj = lv_img_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_ARC: {
            obj = lv_arc_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_CONTAINER: {
            obj = lv_cont_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_OBJECT: {
            obj = lv_obj_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_PAGE: {
            obj = lv_page_create(parent_obj, NULL);
            // No event handler for pages
            break;
        }
#if LV_USE_WIN && LVGL_VERSION_MAJOR == 7
        case LV_HASP_WINDOW: {
            obj = lv_win_create(parent_obj, NULL);
            // No event handler for pages
            break;
        }
#endif
#if LVGL_VERSION_MAJOR == 8
        case LV_HASP_LED: {
            obj = lv_led_create(parent_obj);
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_TILEVIEW: {
            obj = lv_tileview_create(parent_obj);
            // No event handler for tileviews
            break;
        }
        case LV_HASP_TABVIEW: {
            obj = lv_tabview_create(parent_obj, LV_DIR_TOP, 100);
            // No event handler for tabs
            if(obj) {
                lv_obj_t * tab;
                tab = lv_tabview_add_tab(obj, "tab 1");
                // lv_obj_set_user_data(tab, id + 1);
                tab = lv_tabview_add_tab(obj, "tab 2");
                // lv_obj_set_user_data(tab, id + 2);
                tab = lv_tabview_add_tab(obj, "tab 3");
                // lv_obj_set_user_data(tab, id + 3);
            }
            break;
        }
#else
        case LV_HASP_LED: {
            obj = lv_led_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, btn_event_handler);
            break;
        }
        case LV_HASP_TILEVIEW: {
            obj = lv_tileview_create(parent_obj, NULL);
            // No event handler for tileviews
            break;
        }
        case LV_HASP_TABVIEW: {
            obj = lv_tabview_create(parent_obj, NULL);
            // No event handler for tabs
            if(obj) {
                lv_obj_t * tab;
                tab = lv_tabview_add_tab(obj, "tab 1");
                // lv_obj_set_user_data(tab, id + 1);
                tab = lv_tabview_add_tab(obj, "tab 2");
                // lv_obj_set_user_data(tab, id + 2);
                tab = lv_tabview_add_tab(obj, "tab 3");
                // lv_obj_set_user_data(tab, id + 3);
            }
            break;
        }
#endif
        /* ----- Color Objects ------ */
        case LV_HASP_CPICKER: {
            obj = lv_cpicker_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, cpicker_event_handler);
            break;
        }

#if LV_USE_PRELOAD != 0
        case LV_HASP_PRELOADER: {
            obj = lv_spinner_create(parent_obj, NULL);
            break;
        }
#endif
        /* ----- Range Objects ------ */
        case LV_HASP_SLIDER: {
            obj = lv_slider_create(parent_obj, NULL);
            if(obj) {
                lv_slider_set_range(obj, 0, 100);
                lv_obj_set_event_cb(obj, slider_event_handler);
            }
            // bool knobin = config[F("knobin")].as<bool>() | true;
            // lv_slider_set_knob_in(obj, knobin);
            break;
        }
        case LV_HASP_GAUGE: {
            obj = lv_gauge_create(parent_obj, NULL);
            if(obj) {
                lv_gauge_set_range(obj, 0, 100);
                lv_obj_set_event_cb(obj, btn_event_handler);
            }
            break;
        }
        case LV_HASP_BAR: {
            obj = lv_bar_create(parent_obj, NULL);
            if(obj) {
                lv_bar_set_range(obj, 0, 100);
                lv_obj_set_event_cb(obj, btn_event_handler);
            }
            break;
        }
        case LV_HASP_LMETER: {
            obj = lv_linemeter_create(parent_obj, NULL);
            if(obj) {
                lv_linemeter_set_range(obj, 0, 100);
                lv_obj_set_event_cb(obj, btn_event_handler);
            }
            break;
        }
        case LV_HASP_CHART: {
            obj = lv_chart_create(parent_obj, NULL);
            if(obj) {
                lv_chart_set_range(obj, 0, 100);
                lv_obj_set_event_cb(obj, btn_event_handler);

                lv_chart_add_series(obj, LV_COLOR_RED);
                lv_chart_add_series(obj, LV_COLOR_GREEN);
                lv_chart_add_series(obj, LV_COLOR_BLUE);

                lv_chart_series_t * ser = lv_chart_get_series(obj, 2);
                lv_chart_set_next(obj, ser, 10);
                lv_chart_set_next(obj, ser, 20);
                lv_chart_set_next(obj, ser, 30);
                lv_chart_set_next(obj, ser, 40);
            }
            break;
        }

        /* ----- On/Off Objects ------ */
        case LV_HASP_SWITCH: {
            obj = lv_switch_create(parent_obj, NULL);
            if(obj) lv_obj_set_event_cb(obj, switch_event_handler);
            break;
        }
        /* ----- List Object ------- */
        case LV_HASP_DDLIST: {
            obj = lv_dropdown_create(parent_obj, NULL);
            if(obj) {
                lv_dropdown_set_draw_arrow(obj, true);
                // lv_dropdown_set_anim_time(obj, 200);
                lv_obj_set_top(obj, true);
                // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
                lv_obj_set_event_cb(obj, ddlist_event_handler);
            }
            break;
        }
        case LV_HASP_ROLLER: {
            obj = lv_roller_create(parent_obj, NULL);
            // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
            if(obj) {
                lv_roller_set_auto_fit(obj, false);
                lv_obj_set_event_cb(obj, roller_event_handler);
            }
            break;
        }

            /* ----- Other Object ------ */
            // default:
            //    return Log.warning(TAG_HASP, F("Unsupported Object ID %u"), objid);
    }

    /* No object was actually created */
    if(!obj) {
        return Log.warning(TAG_HASP, F("Object ID %u is NULL, skipping..."), id);
    }

    /* id tag the object */
    // lv_obj_set_user_data(obj, id);
    obj->user_data.id      = id;
    obj->user_data.objid   = objid;   //& 0b11111;
    obj->user_data.groupid = groupid; // & 0b111;

    /* do not process these attributes */
    config.remove(F("page"));
    config.remove(F("id"));
    config.remove(F("objid"));
    config.remove(F("parentid"));
    String v((char *)0);
    v.reserve(64);

    for(JsonPair keyValue : config) {
        v = keyValue.value().as<String>();
        hasp_process_obj_attribute(obj, keyValue.key().c_str(), v.c_str(), true);
        // Log.verbose(TAG_HASP,F("     * %s => %s"), keyValue.key().c_str(), v.c_str());
    }

    /** testing start **/
    uint8_t temp;
    if(!hasp_find_id_from_obj(obj, &pageid, &temp)) {
        return Log.error(TAG_HASP, F("Lost track of the created object, not found!"));
    }

    /** verbose reporting **/
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    Log.verbose(TAG_HASP, F("    * p[%u].b[%u] = %s"), pageid, temp, list.type[0]);

    /* test double-check */
    lv_obj_t * test = hasp_find_obj_from_parent_id(get_page_obj(pageid), (uint8_t)temp);
    if(test != obj) {
        return Log.error(TAG_HASP, F("Objects DO NOT match!"));
    }
}
