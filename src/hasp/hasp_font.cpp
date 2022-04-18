#include <string.h>

#include "hasplib.h"
#if HASP_USE_FREETYPE > 0
#include "lv_freetype.h"
#else
typedef struct
{
    const char* name; /* The name of the font file */
    lv_font_t* font;  /* point to lvgl font */
    uint16_t weight;  /* font size */
    uint16_t style;   /* font style */
} lv_ft_info_t;
#endif

#include "hasp_mem.h"
#include "font/hasp_font_loader.h"

static lv_ll_t hasp_fonts_ll;

typedef struct
{
    const char* payload; /* The payload with name and size */
    lv_font_t* font;     /* point to lvgl font */
} hasp_font_info_t;

void font_setup()
{
    _lv_ll_init(&hasp_fonts_ll, sizeof(hasp_font_info_t));
}

size_t font_split_payload(const char* payload)
{
    size_t pos = 0;
    while(*(payload + pos) != '\0') {
        if(Parser::is_only_digits(payload + pos)) return pos;
        pos++;
    }
    return 0;
}

static lv_font_t* font_find_in_list(const char* payload)
{
    hasp_font_info_t* font_p = (hasp_font_info_t*)_lv_ll_get_head(&hasp_fonts_ll);
    while(font_p) {
        if(strcmp(font_p->payload, payload) == 0) { // name and size
            LOG_WARNING(TAG_FONT, F("Payload %s found => line height = %d"), payload, font_p->font->line_height);
            return font_p->font;
        }
        font_p = (hasp_font_info_t*)_lv_ll_get_next(&hasp_fonts_ll, font_p);
    }

    return NULL;
}

static lv_font_t* font_add_to_list(const char* payload)
{
    char filename[64];

    // Try .bin file
    snprintf_P(filename, sizeof(filename), PSTR("L:\\%s.bin"), payload);
    lv_font_t* font = hasp_font_load(filename);
    char* name_p    = NULL;

#if defined(ARDUINO_ARCH_ESP32) && (HASP_USE_FREETYPE > 0)
    char* ext[] = {"ttf", "otf"};
    for(size_t i = 0; i < 2; i++) {
        if(!font) {

            size_t pos = font_split_payload(payload);
            if(pos > 0 && pos < 56) {
                uint16_t size = atoi(payload + pos);

                char fontname[64];
                memset(fontname, 0, sizeof(fontname));
                strncpy(fontname, payload, pos);
                snprintf_P(filename, sizeof(filename), PSTR("L:\\%s.%s"), fontname, ext[i]);

                // Test if the file exists and can be opened
                lv_fs_file_t f;
                lv_fs_res_t res;
                res = lv_fs_open(&f, filename, LV_FS_MODE_RD);
                if(res != LV_FS_RES_OK) {
                    LOG_VERBOSE(TAG_FONT, F(D_FILE_NOT_FOUND ": %s"), filename);
                    continue;
                } else {
                    lv_fs_close(&f);
                    LOG_VERBOSE(TAG_FONT, F(D_FILE_LOADING), filename);
                }

                lv_ft_info_t info;
                info.name   = filename;
                info.weight = size;
                info.style  = FT_FONT_STYLE_NORMAL;
                if(lv_ft_font_init(&info)) {
                    font = info.font;
                }
            }
        }
    }
#endif

    if(!font) return NULL;
    LOG_VERBOSE(TAG_FONT, F("Loaded font %s size %d"), filename, font->line_height);

    /* alloc payload str */
    size_t len = strlen(payload);
    name_p     = (char*)calloc(sizeof(char), len + 1);
    if(!name_p) return NULL;
    strncpy(name_p, payload, len);

    hasp_font_info_t* new_font_item;
    new_font_item = (hasp_font_info_t*)_lv_ll_ins_tail(&hasp_fonts_ll);
    if(!new_font_item) return NULL;

    new_font_item->payload = name_p;
    new_font_item->font    = font;
    return font;
}

// Convert the payload to a font pointer
lv_font_t* get_font(const char* payload)
{
    lv_font_t* font = font_find_in_list(payload);
    if(font) return font;

    return font_add_to_list(payload);
}
