/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"
#include "hasplib.h"

#include "hasp_page.h"

namespace hasp {

Page::Page()
{
    // LVGL is not yet initialized at construction time
}

size_t Page::count()
{
    return sizeof(_pages) / sizeof(*_pages);
}

void Page::init(uint8_t start_page)
{
    for(int i = 0; i < count(); i++) {
        _pages[i] = lv_obj_create(NULL, NULL);

        uint16_t thispage  = i + PAGE_START_INDEX;
        _meta_data[i].prev = thispage == PAGE_START_INDEX ? HASP_NUM_PAGES : thispage - PAGE_START_INDEX;
        _meta_data[i].next = thispage == HASP_NUM_PAGES ? PAGE_START_INDEX : thispage + PAGE_START_INDEX;
        _meta_data[i].back = start_page;
    }
}

void Page::clear(uint16_t pageid)
{
    lv_obj_t* page = get_obj(pageid);
    if(!page || (pageid > HASP_NUM_PAGES)) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    } else if(page == lv_layer_sys() /*|| page == lv_layer_top()*/) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_LAYER));
    } else {
        LOG_TRACE(TAG_HASP, F(D_HASP_CLEAR_PAGE), pageid);
        lv_obj_clean(page);
    }
}

// void Page::set(uint8_t pageid)
// {
//     set(pageid, LV_SCR_LOAD_ANIM_NONE);
// }

void Page::set(uint8_t pageid, lv_scr_load_anim_t animation)
{
    lv_obj_t* page = get_obj(pageid);
    if(!page || pageid == 0 || pageid > HASP_NUM_PAGES) {
        LOG_WARNING(TAG_HASP, F(D_HASP_INVALID_PAGE), pageid);
    } else {
        LOG_TRACE(TAG_HASP, F(D_HASP_CHANGE_PAGE), pageid);
        _current_page = pageid;
        lv_scr_load_anim(page, animation, 1000, 0, false);
        hasp_object_tree(page, pageid, 0);
    }
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
        LOG_ERROR(TAG_HASP, F("FS not mounted. Failed to load %s"), pagesfile);
        return;
    }

    if(!HASP_FS.exists(pagesfile)) {
        LOG_ERROR(TAG_HASP, F("Non existing file %s"), pagesfile);
        return;
    }

    LOG_TRACE(TAG_HASP, F("Loading file %s"), pagesfile);

    File file = HASP_FS.open(pagesfile, "r");
    dispatch_parse_jsonl(file);
    file.close();

    LOG_INFO(TAG_HASP, F("File %s loaded"), pagesfile);
#else

#if HASP_USE_EEPROM > 0
    LOG_TRACE(TAG_HASP, F("Loading jsonl from EEPROM..."));
    EepromStream eepromStream(4096, 1024);
    dispatch_parse_jsonl(eepromStream);
    LOG_INFO(TAG_HASP, F("Loaded jsonl from EEPROM"));
#endif

#endif
}

lv_obj_t* Page::get_obj(uint8_t pageid)
{
    if(pageid == 0) return lv_layer_top(); // 254
    if(pageid == 255) return lv_layer_sys();
    if(pageid > count()) return NULL; // >=0
    return _pages[pageid - PAGE_START_INDEX];
}

bool Page::get_id(lv_obj_t* obj, uint8_t* pageid)
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
