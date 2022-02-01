/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#include <fstream>

#if defined(ARDUINO)
#include "StreamUtils.h" // For EEPromStream

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#include "hasp_filesystem.h"
#endif
#endif // ARDUINO

namespace hasp {

bool Page::is_valid(uint8_t pageid)
{
    if(pageid > 0 && pageid <= HASP_NUM_PAGES) return true;

    LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    return false;
}

Page::Page()
{
    // LVGL is not yet initialized at construction time
}

uint8_t Page::count()
{
    return (uint8_t)(sizeof(_pages) / sizeof(*_pages));
}

void Page::init(uint8_t start_page)
{
    lv_obj_t* scr_act = lv_scr_act();
    lv_obj_clean(lv_layer_top());

    for(int i = 0; i < count(); i++) {
        lv_obj_t* prev_page_obj = _pages[i];

        _pages[i]                  = lv_obj_create(NULL, NULL);
        _pages[i]->user_data.objid = LV_HASP_SCREEN;
        lv_obj_set_event_cb(_pages[i], generic_event_handler);

        /**< If the `indev` was pressing this object but swiped out while pressing do not search other object.*/
        lv_obj_add_protect(_pages[i], LV_PROTECT_PRESS_LOST);

        uint16_t thispage  = i + PAGE_START_INDEX;
        _meta_data[i].prev = thispage == PAGE_START_INDEX ? HASP_NUM_PAGES : thispage - PAGE_START_INDEX;
        _meta_data[i].next = thispage == HASP_NUM_PAGES ? PAGE_START_INDEX : thispage + PAGE_START_INDEX;
        _meta_data[i].back = start_page;

        if(prev_page_obj) {
            if(scr_act == prev_page_obj) {
                lv_scr_load_anim(_pages[i], LV_SCR_LOAD_ANIM_NONE, 500, 0, false); // update page screen obj
                lv_obj_del_async(prev_page_obj);
            } else
                lv_obj_del(prev_page_obj);
        }
    }
}

void Page::clear(uint8_t pageid)
{
    lv_obj_t* page = get_obj(pageid);
    if(page == lv_layer_top() || is_valid(pageid)) {
        LOG_TRACE(TAG_HASP, F(D_HASP_CLEAR_PAGE), pageid);
        lv_obj_clean(page);
    } else {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_LAYER)); // lv_layer_sys
    }
}

// void Page::set(uint8_t pageid)
// {
//     set(pageid, LV_SCR_LOAD_ANIM_NONE);
// }

void Page::set(uint8_t pageid, lv_scr_load_anim_t animation)
{
    lv_obj_t* page = get_obj(pageid);
    if(!is_valid(pageid)) {
        return;
    } else if(!page) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    } else {
        _current_page = pageid;
        if(page != lv_scr_act()) {
            LOG_TRACE(TAG_HASP, F(D_HASP_CHANGE_PAGE), pageid);
            lv_scr_load_anim(page, animation, 500, 0, false);
#if defined(HASP_DEBUG_OBJ_TREE)
            hasp_object_tree(page, pageid, 0);
#endif
        }
    }
}

uint8_t Page::get_next(uint8_t pageid)
{
    return is_valid(pageid) ? _meta_data[pageid - PAGE_START_INDEX].next : 0;
}

uint8_t Page::get_prev(uint8_t pageid)
{
    return is_valid(pageid) ? _meta_data[pageid - PAGE_START_INDEX].prev : 0;
}

uint8_t Page::get_back(uint8_t pageid)
{
    return is_valid(pageid) ? _meta_data[pageid - PAGE_START_INDEX].back : 0;
}

void Page::set_next(uint8_t pageid, uint8_t nextid)
{
    if(is_valid(pageid) && is_valid(nextid)) _meta_data[pageid - PAGE_START_INDEX].next = nextid;
}

void Page::set_prev(uint8_t pageid, uint8_t previd)
{
    if(is_valid(pageid) && is_valid(previd)) _meta_data[pageid - PAGE_START_INDEX].prev = previd;
}

void Page::set_back(uint8_t pageid, uint8_t backid)
{
    if(is_valid(pageid) && is_valid(backid)) _meta_data[pageid - PAGE_START_INDEX].back = backid;
}

void Page::next(lv_scr_load_anim_t animation)
{
    set(_meta_data[_current_page - PAGE_START_INDEX].next, animation);
}

void Page::prev(lv_scr_load_anim_t animation)
{
    set(_meta_data[_current_page - PAGE_START_INDEX].prev, animation);
}

void Page::back(lv_scr_load_anim_t animation)
{
    set(_meta_data[_current_page - PAGE_START_INDEX].back, animation);
}

uint8_t Page::get()
{
    return _current_page;
}

void Page::load_jsonl(const char* pagesfile)
{
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    if(pagesfile[0] == '\0') return;

    if(!filesystemSetup()) {
        LOG_ERROR(TAG_HASP, F("FS not mounted. " D_FILE_LOAD_FAILED), pagesfile);
        return;
    }

    if(!HASP_FS.exists(pagesfile)) {
        LOG_WARNING(TAG_HASP, F(D_FILE_NOT_FOUND ": %s"), pagesfile);
        return;
    }

    LOG_TRACE(TAG_HASP, F(D_FILE_LOADING), pagesfile);

    File file = HASP_FS.open(pagesfile, "r");
    if(!file) {
        LOG_ERROR(TAG_HASP, F(D_FILE_LOAD_FAILED), pagesfile);
        return;
    }
    dispatch_parse_jsonl(file);
    file.close();

    LOG_INFO(TAG_HASP, F(D_FILE_LOADED), pagesfile);

#elif HASP_USE_EEPROM > 0
    LOG_TRACE(TAG_HASP, F("Loading jsonl from EEPROM..."));
    EepromStream eepromStream(4096, 1024);
    dispatch_parse_jsonl(eepromStream);
    LOG_INFO(TAG_HASP, F("Loaded jsonl from EEPROM"));

#else

    char path[strlen(pagesfile) + 4];
    path[0] = '.';
    path[1] = '\0';
    strcat(path, pagesfile);
    path[1] = '\\';

    LOG_TRACE(TAG_HASP, F("Loading %s from disk..."), path);
    std::ifstream f(path); // taking file as inputstream
    if(f) {
        dispatch_parse_jsonl(f);
    }
    f.close();
    LOG_INFO(TAG_HASP, F("Loaded %s from disk"), path);

    // char path[strlen(pagesfile) + 4];
    // path[0] = '\0';
    // strcat(path, "L:/");
    // strcat(path, pagesfile);

    // lv_fs_file_t file;
    // lv_fs_res_t res;
    // res = lv_fs_open(&file, path, LV_FS_MODE_RD);
    // if(res == LV_FS_RES_OK) {
    //     LOG_VERBOSE(TAG_HASP, F("Opening %s"), path);
    // } else {
    //     LOG_ERROR(TAG_HASP, F("TEST Opening %q from FS failed %d"), path, res);
    // }

    // dispatch_parse_jsonl(file);
    // res = lv_fs_close(&file);
    // if(res == LV_FS_RES_OK) {
    //     LOG_VERBOSE(TAG_HASP, F("Closing %s OK"), path);
    // } else {
    //     LOG_ERROR(TAG_HASP, F("Closing %s on FS failed %d"), path, res);
    // }

#endif
}

lv_obj_t* Page::get_obj(uint8_t pageid)
{
    if(pageid == 0) return lv_layer_top(); // 254
    if(pageid == 255) return lv_layer_sys();
    if(pageid > count()) return NULL; // >=0
    return _pages[pageid - PAGE_START_INDEX];
}

bool Page::get_id(const lv_obj_t* obj, uint8_t* pageid)
{
    lv_obj_t* page = lv_obj_get_screen(obj);

    if(!page) return false;

    if(page == lv_layer_top()) {
        *pageid = 0;
        return true;
    }
    if(page == lv_layer_sys()) {
        *pageid = 255;
        return true;
    }

    for(uint8_t i = 0; i < count(); i++) {
        if(page == _pages[i]) {
            *pageid = i + PAGE_START_INDEX;
            return true;
        }
    }
    return false;
}

} // namespace hasp

hasp::Page haspPages;
