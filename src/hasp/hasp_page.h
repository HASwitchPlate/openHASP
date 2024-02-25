/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_PAGES_H
#define HASP_PAGES_H

#include "hasplib.h"

/*********************
 *      DEFINES
 *********************/
#define PAGE_START_INDEX 1 // Page number of array index 0

/**********************
 *      TYPEDEFS
 **********************/

struct hasp_page_meta_data_t
{
    uint8_t prev : 4;
    uint8_t next : 4;
    uint8_t back : 4;
};

namespace hasp {

class Page {
  private:
    char* _pagenames[HASP_NUM_PAGES + 1];             // index 0 = Page 0
    hasp_page_meta_data_t _meta_data[HASP_NUM_PAGES]; // index 0 = Page 1 etc.
    lv_obj_t* _pages[HASP_NUM_PAGES];                 // index 0 = Page 1 etc.
    uint8_t _current_page;

  public:
    Page();
    uint8_t count();
    void init(uint8_t start_page);
    void clear(uint8_t pageid);
    //    void set(uint8_t pageid);
    void set(uint8_t pageid, lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay);
    void swap(lv_obj_t* page, uint8_t id);

    void next(lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay);
    void prev(lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay);
    void back(lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay);

    uint8_t get_next(uint8_t pageid);
    uint8_t get_prev(uint8_t pageid);
    uint8_t get_back(uint8_t pageid);
    char* get_name(uint8_t pageid);

    void set_next(uint8_t pageid, uint8_t nextid);
    void set_prev(uint8_t pageid, uint8_t previd);
    void set_back(uint8_t pageid, uint8_t backid);
    void set_name(uint8_t pageid, const char* name);

    uint8_t get();
    void load_jsonl(const char* pagesfile);
    lv_obj_t* get_obj(uint8_t pageid);
    bool get_id(const lv_obj_t* obj, uint8_t* pageid);
    bool is_valid(uint8_t pageid);
};

} // namespace hasp

using hasp::Page;
extern hasp::Page haspPages;

#endif // HASP_PAGES_H
