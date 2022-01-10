#include <string.h>

#include "hasplib.h"
#include "lv_freetype.h"

#include "hasp_mem.h"
#include "font/hasp_font_loader.h"

static lv_ll_t hasp_fonts_ll;

typedef struct
{
    const char* name; /* The name of the font file */
    lv_font_t* font;  /* point to lvgl font */
} hasp_font_info_t;

void font_setup()
{
    _lv_ll_init(&hasp_fonts_ll, sizeof(lv_ft_info_t));
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
        if(strcmp(font_p->name, payload) == 0) {
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

#if defined(ARDUINO_ARCH_ESP32)
    if(!font) {
        // Try .ttf file

        size_t pos = font_split_payload(payload);
        if(pos > 0 && pos < 56) {
            uint16_t size = atoi(payload + pos);

            char fontname[64];
            memset(fontname, 0, sizeof(fontname));
            strncpy(fontname, payload, pos);
            snprintf_P(filename, sizeof(filename), PSTR("L:\\%s.ttf"), fontname);

            lv_ft_info_t info;
            info.name   = filename;
            info.weight = size;
            info.style  = FT_FONT_STYLE_NORMAL;
            if(lv_ft_font_init(&info)) {
                font = info.font;
            }
        }
    }

    if(!font) {
        // Try .otf file
        snprintf_P(filename, sizeof(filename), PSTR("L:\\%s.otf"), payload);

        lv_ft_info_t info;
        info.name   = filename;
        info.weight = 56;
        info.style  = FT_FONT_STYLE_NORMAL;
        if(lv_ft_font_init(&info)) {
            font = info.font;
        }
    }
#endif

    if(!font) return NULL;
    LOG_VERBOSE(TAG_FONT, F("Loaded font %s size %d"), filename, font->line_height);

    size_t len = strlen(payload);
    name_p     = (char*)calloc(sizeof(char), len + 1);
    if(!name_p) return NULL;
    strncpy(name_p, payload, len);

    hasp_font_info_t info;
    info.name = name_p;
    info.font = font;

    hasp_font_info_t* info_p;
    info_p  = (hasp_font_info_t*)_lv_ll_ins_tail(&hasp_fonts_ll);
    *info_p = info;

    return info.font;
}

// Convert the payload to a font pointer
lv_font_t* get_font(const char* payload)
{
    lv_font_t* font = font_find_in_list(payload);
    if(font) return font;

    return font_add_to_list(payload);
}
