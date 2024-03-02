/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#include <fstream>
#include "hasp_anim.h"

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

void Page::swap(lv_obj_t* page, uint8_t id)
{
    if(id > count()) {
        LOG_WARNING(TAG_HASP, "Invalid page id %d", id);
        return;
    }

    // Swap page objects
    lv_obj_t* prev_page_obj     = _pages[id];
    _pages[id]                  = page;
    _pages[id]->user_data.objid = LV_HASP_SCREEN;
    _pages[id]->user_data.id    = 0;

    /**< If the `indev` was pressing this object but swiped out while pressing do not search other object.*/
    lv_obj_add_protect(_pages[id], LV_PROTECT_PRESS_LOST);
    lv_obj_set_event_cb(_pages[id], generic_event_handler);

    // Delete previous page object
    if(prev_page_obj) {
        if(prev_page_obj == lv_scr_act()) {
            my_scr_load_anim(_pages[id], LV_SCR_LOAD_ANIM_NONE, 500, 0, false); // update page screen obj
            lv_obj_del_async(prev_page_obj);
        } else
            lv_obj_del(prev_page_obj);
    }
}

void Page::init(uint8_t start_page)
{
    lv_obj_t* scr_act = lv_scr_act();
    lv_obj_clean(lv_layer_top());

    for(int i = 0; i < count(); i++) {
        lv_obj_t* page = lv_obj_create(NULL, NULL);
        Page::swap(page, i);

        uint16_t thispage  = i + PAGE_START_INDEX;
        _meta_data[i].prev = thispage == PAGE_START_INDEX ? HASP_NUM_PAGES : thispage - PAGE_START_INDEX;
        _meta_data[i].next = thispage == HASP_NUM_PAGES ? PAGE_START_INDEX : thispage + PAGE_START_INDEX;
        _meta_data[i].back = start_page;

        set_name(i, NULL);
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

void Page::set(uint8_t pageid, lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay)
{
    if(!is_valid(pageid)) return; // produces a log warning if not between 1 and 12

    lv_obj_t* page = get_obj(pageid);
    if(!page) {
        // Invalid page object
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);

    } else if(page == lv_scr_act()) {
        // No change needed, just send current page again
        _current_page = pageid;
        dispatch_current_page();

    } else if((anim_type != LV_SCR_LOAD_ANIM_NONE && time > 0) || delay > 0) {
        // Change page after a delay or animation, don't publish it yet
        my_scr_load_anim(page, anim_type, time, delay, false); // dispatches when animation ends

    } else {
        // No delay or animation set, update now
        LOG_TRACE(TAG_HASP, F(D_HASP_CHANGE_PAGE), pageid);
        lv_scr_load_anim(page, anim_type, time, delay, false);
        _current_page = pageid;
        dispatch_current_page();
#if defined(HASP_DEBUG_OBJ_TREE)
        hasp_object_tree(page, pageid, 0);
#endif
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

char* Page::get_name(uint8_t pageid)
{
    return pageid <= HASP_NUM_PAGES ? _pagenames[pageid] : NULL;
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

void Page::set_name(uint8_t pageid, const char* name)
{
    if(pageid > HASP_NUM_PAGES) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
        return;
    }

    LOG_DEBUG(TAG_HASP, F("%s - %d"), __FILE__, __LINE__);

    if(_pagenames[pageid]) {
        hasp_free(_pagenames[pageid]);
        _pagenames[pageid] = NULL;
    }

    LOG_DEBUG(TAG_HASP, F("%s - %d"), __FILE__, __LINE__);
    if(!name) return;
    size_t size = strlen(name) + 1;

    LOG_DEBUG(TAG_HASP, F("%s - %d"), __FILE__, __LINE__);
    if(size > 1) {
        _pagenames[pageid] = (char*)hasp_calloc(sizeof(char), size);
        LOG_DEBUG(TAG_HASP, F("%s - %d"), __FILE__, __LINE__);
        if(_pagenames[pageid] == NULL) return;
        strncpy(_pagenames[pageid], name, size);
        LOG_VERBOSE(TAG_HASP, F("%s"), _pagenames[pageid]);
    }
    LOG_DEBUG(TAG_HASP, F("%s - %d"), __FILE__, __LINE__);
}

void Page::next(lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay)
{
    set(_meta_data[_current_page - PAGE_START_INDEX].next, anim_type, time, delay);
}

void Page::prev(lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay)
{
    set(_meta_data[_current_page - PAGE_START_INDEX].prev, anim_type, time, delay);
}

void Page::back(lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay)
{
    set(_meta_data[_current_page - PAGE_START_INDEX].back, anim_type, time, delay);
}

uint8_t Page::get()
{
    return _current_page;
}

void Page::load_jsonl(const char* pagesfile)
{
    uint8_t savedPage = haspPages.get();
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
    dispatch_parse_jsonl(file, savedPage);
    file.close();

    LOG_INFO(TAG_HASP, F(D_FILE_LOADED), pagesfile);

#elif HASP_USE_EEPROM > 0
    LOG_TRACE(TAG_HASP, F("Loading jsonl from EEPROM..."));
    EepromStream eepromStream(4096, 1024);
    dispatch_parse_jsonl(eepromStream, savedPage);
    LOG_INFO(TAG_HASP, F("Loaded jsonl from EEPROM"));

#else

    char path[strlen(pagesfile) + 4];
    path[0] = '.';
    path[1] = '\0';
    strcat(path, pagesfile);
#if defined(WINDOWS)
    path[1] = '\\';
#elif defined(POSIX)
    path[1] = '/';
#endif

    LOG_TRACE(TAG_HASP, F("Loading %s from disk..."), path);
    std::ifstream f(path); // taking file as inputstream
    if(f) {
        dispatch_parse_jsonl(f, savedPage);
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

    // dispatch_parse_jsonl(file, savedPage);
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
