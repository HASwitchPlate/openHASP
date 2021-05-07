/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* ********************************************************************************************
 *
 *  HASP Object Handlers
 *     - Object Finders      : Convert from object pointers to and from p[x].b[y] IDs
 *     - Value Dispatchers   : Forward output data to the dispatcher
 *     - Attribute processor : Decide if an attribute needs updating or querying and forward
 *     - Object creator      : Creates an object from a line of jsonl
 *
 ******************************************************************************************** */

#ifdef ARDUINO
#include "ArduinoLog.h"
#endif

#include "hasplib.h"

const char** btnmatrix_default_map;            // memory pointer to lvgl default btnmatrix map
const char* msgbox_default_map[] = {"OK", ""}; // memory pointer to hasp default msgbox map

// ##################### Object Finders ########################################################

// Return a child object from a parent with a specific objid
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
        if(obj_check_type(child, LV_HASP_TABVIEW)) {
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

// Return the object with a specific pageid and objid
lv_obj_t* hasp_find_obj_from_page_id(uint8_t pageid, uint8_t objid)
{
    return hasp_find_obj_from_parent_id(haspPages.get_obj(pageid), objid);
}

// Return the pageid and objid of an object
bool hasp_find_id_from_obj(lv_obj_t* obj, uint8_t* pageid, uint8_t* objid)
{
    if(!obj || !haspPages.get_id(obj, pageid)) return false;
    if(obj->user_data.id == 0 && obj != haspPages.get_obj(*pageid)) return false;
    *objid = obj->user_data.id;
    return true;
}

/**
 * Get the object type name of an object
 * @param obj an lv_obj_t* of the object to check its type
 * @return name of the object type
 * @note
 */
const char* obj_get_type_name(lv_obj_t* obj)
{
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char* objtype = list.type[0];
    return objtype + 3; // skip lv_
}

/**
 * Check if an lvgl objecttype name corresponds to a given HASP object ID
 * @param obj an lv_obj_t* of the object to check its type
 * @param haspobjtype the HASP object ID to check against
 * @return true or false wether the types match
 * @note
 */
bool obj_check_type(lv_obj_t* obj, lv_hasp_obj_type_t haspobjtype)
{
#if 1
    return obj->user_data.objid == (uint8_t)haspobjtype;
#else
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char* objtype = list.type[0];
    return obj_check_type(objtype, haspobjtype);
#endif
}

/**
 * Get the hasp object type of a given LVGL object
 * @param obj an lv_obj_t* of the object to check its type
 * @return lv_hasp_obj_type_t
 * @note
 */
lv_hasp_obj_type_t obj_get_type(lv_obj_t* obj)
{
    return (lv_hasp_obj_type_t)obj->user_data.objid;
}

void hasp_object_tree(lv_obj_t* parent, uint8_t pageid, uint16_t level)
{
    if(parent == nullptr) return;

    /* Output parent info */
    char indent[31];
    memset(indent, 32, 31);
    if(level < 15) indent[level * 2] = 0;
    indent[30] = 0;

    LOG_VERBOSE(TAG_HASP, F("%s- " HASP_OBJECT_NOTATION ": %s"), indent, pageid, parent->user_data.id,
                obj_get_type_name(parent));

    lv_obj_t* child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        /* child found, process it */
        hasp_object_tree(child, pageid, level + 1);

        /* try next sibling */
        child = lv_obj_get_child(parent, child);
    }

    /* check tabs */
    if(obj_check_type(parent, LV_HASP_TABVIEW)) {
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

/* Sends the data out on the state/pxby topic */
void object_dispatch_state(uint8_t pageid, uint8_t btnid, const char* payload)
{
    char topic[16];
    snprintf_P(topic, sizeof(topic), PSTR(HASP_OBJECT_NOTATION), pageid, btnid);
    dispatch_state_subtopic(topic, payload);
}

// ##################### State Changers ########################################################

void object_set_group_values(lv_obj_t* parent, uint8_t groupid, int16_t intval)
{
    if(groupid == 0 || parent == nullptr) return;

    lv_obj_t* child;
    child = lv_obj_get_child(parent, NULL);
    while(child) {
        /* child found, update it */
        if(groupid == child->user_data.groupid) hasp_process_obj_attribute_val(child, NULL, intval, intval, true);

        /* update grandchildren */
        object_set_group_values(child, groupid, intval);

        /* check tabs */
        if(obj_check_type(child, LV_HASP_TABVIEW)) {
            //#if LVGL_VERSION_MAJOR == 7
            uint16_t tabcount = lv_tabview_get_tab_count(child);
            for(uint16_t i = 0; i < tabcount; i++) {
                lv_obj_t* tab = lv_tabview_get_tab(child, i);
                LOG_VERBOSE(TAG_HASP, F("Found tab %i"), i);
                if(tab->user_data.groupid && groupid == tab->user_data.groupid)
                    hasp_process_obj_attribute_val(tab, NULL, intval, intval, true); /* tab found, update it */

                /* check grandchildren */
                object_set_group_values(tab, groupid, intval);
            }
            //#endif
        }

        /* try next sibling */
        child = lv_obj_get_child(parent, child);
    }
}

// Recursive function that goes over all objects only ONCE
void object_set_normalized_group_values(uint8_t groupid, lv_obj_t* src_obj, int16_t val, int16_t min, int16_t max)
{
    if(groupid == 0) return;
    if(min == max) return;

    for(uint8_t page = 0; page < HASP_NUM_PAGES; page++) {
        object_set_group_values(haspPages.get_obj(page), groupid, val);
        // uint8_t startid = 1;
        // for(uint8_t objid = startid; objid < 20; objid++) {
        //     lv_obj_t* obj = hasp_find_obj_from_parent_id(get_page_obj(page), objid);
        //     if(obj && obj != src_obj && obj->user_data.groupid == groupid) { // skip source object, if set
        //         LOG_VERBOSE(TAG_HASP, F("Found p%db%d in group %d"), page, objid, groupid);
        //         lv_obj_set_state(obj, val > 0 ? LV_STATE_PRESSED | LV_STATE_CHECKED : LV_STATE_DEFAULT);
        //         switch(obj->user_data.objid) {
        //             case HASP_OBJ_ARC:
        //             case HASP_OBJ_SLIDER:
        //             case HASP_OBJ_CHECKBOX:
        //                 hasp_process_obj_attribute_val();
        //             default:
        //         }
        //     }
        // }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Used in the dispatcher & hasp_new_object
void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char* attr, const char* payload, bool update)
{
    if(lv_obj_t* obj = hasp_find_obj_from_page_id(pageid, objid)) {
        hasp_process_obj_attribute(obj, attr, payload, update); // || strlen(payload) > 0);
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
        pageid = config[FPSTR(FP_PAGE)].as<uint8_t>();
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

    uint16_t sdbm = 0;
    uint8_t id    = config[FPSTR(FP_ID)].as<uint8_t>();
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
                sdbm = Parser::get_sdbm(config[FPSTR(FP_OBJ)].as<const char*>());
                config.remove(FPSTR(FP_OBJ));
            }
        } else {
            LOG_WARNING(TAG_HASP, F("objid property is obsolete, use obj instead")); // TODO: obsolete objid
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
                    lv_obj_set_event_cb(obj, btnmatrix_event_handler);

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
                    lv_obj_set_event_cb(obj, selector_event_handler);
                    obj->user_data.objid = LV_HASP_TABVIEW;
                }
                break;

            case LV_HASP_TAB:
            case HASP_OBJ_TAB:
                if(parent_obj && parent_obj->user_data.objid == LV_HASP_TABVIEW) {
                    obj = lv_tabview_add_tab(parent_obj, "Tab");
                    if(obj) {
                        lv_obj_set_event_cb(obj, generic_event_handler);
                        obj->user_data.objid = LV_HASP_TAB;
                    }
                } else {
                    LOG_WARNING(TAG_HASP, F("Parent of a tab must be a tabview object"));
                    return;
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

            case LV_HASP_MSGBOX:
            case HASP_OBJ_MSGBOX:
                obj = lv_msgbox_create(parent_obj, NULL);
                if(obj) {
                    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
                    lv_obj_set_auto_realign(obj, true);
                    lv_obj_set_event_cb(obj, msgbox_event_handler);
                    if(msgbox_default_map) lv_msgbox_add_btns(obj, msgbox_default_map);
                    obj->user_data.objid = LV_HASP_MSGBOX;
                }
                break;

            case LV_HASP_CALENDER:
            case HASP_OBJ_CALENDAR:
                obj = lv_calendar_create(parent_obj, NULL);
                // lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
                if(obj) {
                    lv_obj_set_event_cb(obj, calendar_event_handler);
                    obj->user_data.objid = LV_HASP_CALENDER;
                    lv_task_t* task      = lv_task_create(event_timer_calendar, 25, LV_TASK_PRIO_LOWEST, (void*)obj);
                    (void)task; // unused
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
        lv_obj_set_gesture_parent(obj, false);

        /* id tag the object */
        obj->user_data.id = id;

        uint8_t temp; // needed for debug tests
        (void)temp;

#ifdef HASP_DEBUG
        /** testing start **/
        if(!hasp_find_id_from_obj(obj, &pageid, &temp)) {
            LOG_ERROR(TAG_HASP, F(D_OBJECT_LOST));
            return;
        }
#endif

#if HASP_LOG_LEVEL >= LOG_LEVEL_VERBOSE
        /** verbose reporting **/
        LOG_VERBOSE(TAG_HASP, F(D_BULLET HASP_OBJECT_NOTATION " = %s"), pageid, id, obj_get_type_name(obj));
#endif

#ifdef HASP_DEBUG
        /* test double-check */
        lv_obj_t* test = hasp_find_obj_from_page_id(pageid, (uint8_t)temp);
        if(test != obj || temp != id) {
            LOG_ERROR(TAG_HASP, F(D_OBJECT_MISMATCH));
            return;
        } else {
            // object created successfully
        }
#endif

    } else {
        // object already exists
    }

    hasp_parse_json_attributes(obj, config);
}
