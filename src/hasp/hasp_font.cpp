/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <string.h>

#include "hasplib.h"
#if HASP_USE_FREETYPE > 0

#ifndef LV_USE_FREETYPE
#define LV_USE_FREETYPE
#endif

#include "lv_freetype.h"
#include "ft2build.h"  // for FT_FREETYPE_H macro
#include FT_FREETYPE_H // for FREETYPE_VERSION macros
#else
typedef struct
{
    const char* name; /* The name of the font file */
    lv_font_t* font;  /* point to lvgl font */
    uint16_t weight;  /* font size */
    uint16_t style;   /* font style */
} lv_ft_info_t;
#endif // HASP_USE_FREETYPE

#include "hasp_mem.h"
#include "font/hasp_font_loader.h"

#if defined(ARDUINO_ARCH_ESP32) && (HASP_USE_FREETYPE > 0) // && defined(ESP32S3)
// #if ESP_FLASH_SIZE > 4
extern const uint8_t OPENHASP_TTF_START[] asm("_binary_data_openhasp_ttf_start");
extern const uint8_t OPENHASP_TTF_END[] asm("_binary_data_openhasp_ttf_end");
// #else
// extern const uint8_t OPENHASP_TTF_START[] asm("_binary_data_openhasplite_ttf_start");
// extern const uint8_t OPENHASP_TTF_END[] asm("_binary_data_openhasplite_ttf_end");
// #endif
#endif

static lv_ll_t hasp_fonts_ll;

typedef struct
{
    char* payload;   /* The payload with name and size */
    lv_font_t* font; /* point to lvgl font */
    uint8_t type;
} hasp_font_info_t;

bool font_dummy_glyph_dsc(const struct _lv_font_struct*, lv_font_glyph_dsc_t*, uint32_t letter, uint32_t letter_next)
{
    return false;
}

const uint8_t* font_dummy_glyph_bitmap(const struct _lv_font_struct*, uint32_t)
{
    return NULL;
}

void font_setup()
{
#if(HASP_USE_FREETYPE > 0) // initialize the FreeType renderer

#if defined(ARDUINO_ARCH_ESP32)
    if(lv_freetype_init(LVGL_FREETYPE_MAX_FACES, LVGL_FREETYPE_MAX_SIZES,
                        hasp_use_psram() ? LVGL_FREETYPE_MAX_BYTES_PSRAM : LVGL_FREETYPE_MAX_BYTES)) {
        LOG_VERBOSE(TAG_FONT, F("FreeType v%d.%d.%d " D_SERVICE_STARTED " = %d"), FREETYPE_MAJOR, FREETYPE_MINOR,
                    FREETYPE_PATCH, hasp_use_psram());
        LOG_DEBUG(TAG_FONT, F("FreeType High Watermark %u"), lv_ft_freetype_high_watermark());
    } else {
        LOG_ERROR(TAG_FONT, F("FreeType " D_SERVICE_START_FAILED));
    }
#elif HASP_TARGET_PC
#else
#endif

#else
    LOG_VERBOSE(TAG_FONT, F("FreeType " D_SERVICE_DISABLED));
#endif // HASP_USE_FREETYPE

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

static void font_release(void* node)
{
    hasp_font_info_t* font_p = (hasp_font_info_t*)node;
    if(font_p->font) {
        if(font_p->type = 1) { // It's a FreeType font
#if(HASP_USE_FREETYPE > 0)
            lv_ft_font_destroy(font_p->font);
#endif
        } else if(font_p->type = 0) { // It's a binary font
            hasp_font_free(font_p->font);
        }
    }

    /* Free the allocated font_name last */
    if(font_p->payload) {
        LOG_DEBUG(TAG_FONT, F("Released font %s"), font_p->payload);
        hasp_free(font_p->payload);
    }
}

void font_clear_list(const char* payload)
{
    if(_lv_ll_is_empty(&hasp_fonts_ll)) return;

    while(void* node = _lv_ll_get_head(&hasp_fonts_ll)) {
        font_release(node);
        _lv_ll_remove(&hasp_fonts_ll, node);
        lv_mem_free(node);
    }
}

// void font_clear_list2(const char* payload)
// {
//     hasp_font_info_t* font_p = (hasp_font_info_t*)_lv_ll_get_head(&hasp_fonts_ll);
//     while(font_p) {
//         if(font_p->type == 1) {
//             lv_font_fmt_ft_dsc_t* dsc = (lv_font_fmt_ft_dsc_t*)font_p->font->dsc;
//             if(strcmp(dsc->name, payload) == 0) { // name and size
//                 lv_ft_font_destroy(font_p->font);
//                 *font_p->font = HASP_FONT_1;
//                 font_p->type  = 255;
//             }
//         }
//         // if(strcmp(font_p->payload, payload) == 0) { // name and size
//         //     LOG_DEBUG(TAG_FONT, F("Payload %s found => line height = %d - base_line = %d"), payload,
//         //               font_p->font->line_height, font_p->font->base_line);
//         //     return font_p->font;
//         // }

//         font_p = (hasp_font_info_t*)_lv_ll_get_next(&hasp_fonts_ll, font_p);
//     }
// }

static lv_font_t* font_find_in_list(const char* payload)
{
    hasp_font_info_t* font_p = (hasp_font_info_t*)_lv_ll_get_head(&hasp_fonts_ll);
    while(font_p) {
        if(strcmp(font_p->payload, payload) == 0) { // name and size
            LOG_DEBUG(TAG_FONT, F("Payload %s found => line height = %d - base_line = %d"), payload,
                      font_p->font->line_height, font_p->font->base_line);
            return font_p->font;
        }
        font_p = (hasp_font_info_t*)_lv_ll_get_next(&hasp_fonts_ll, font_p);
    }

    return NULL;
}

static lv_font_t* font_add_to_list(const char* payload)
{
    char filename[256];

    // Try .bin file
    snprintf_P(filename, sizeof(filename), PSTR("L:\\%s.bin"), payload);
    lv_font_t* font   = hasp_font_load(filename);
    char* name_p      = NULL;
    uint8_t font_type = 0;

#if defined(ARDUINO_ARCH_ESP32) && (HASP_USE_FREETYPE > 0)
    char* ext[] = {"ttf", "otf"};
    for(size_t i = 0; i < 2; i++) {
        if(!font) {

            size_t pos = font_split_payload(payload);
            if(pos > 0 && pos < 56) {
                uint16_t size = atoi(payload + pos);
                if(payload[pos - 1] == '_') pos--; // truncate trailing underscore

                char fontname[256];
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
                info.name     = filename;
                info.weight   = size;
                info.mem      = NULL;
                info.mem_size = 0;
                info.style    = FT_FONT_STYLE_NORMAL;
                LOG_VERBOSE(TAG_FONT, F("Loading font %s size %d"), filename, size);
                if(lv_ft_font_init(&info)) {
                    font      = info.font;
                    font_type = 1;
                }
            }
        }
    }

#if 1 || defined(ESP32S3)
    if(!font) {
        strcpy(filename, "default");
        uint16_t size = atoi(payload);
        if(size > 8) {
            lv_ft_info_t info;
            info.name     = filename;
            info.weight   = size;
            info.mem      = (const void*)OPENHASP_TTF_START;
            info.mem_size = OPENHASP_TTF_END - OPENHASP_TTF_START;
            info.style    = FT_FONT_STYLE_NORMAL;
            LOG_VERBOSE(TAG_FONT, F("Loading font %s size %d"), filename, size);
            if(lv_ft_font_init(&info)) {
                font      = info.font;
                font_type = 2; // Flash embedded TTF
            }
        }
    }
#endif // ESP32S3

#endif // ESP32 && HASP_USE_FREETYPE

    if(!font) return NULL;
    LOG_VERBOSE(TAG_FONT, F("Loaded font %s line_height %d"), filename, font->line_height);

    /* alloc payload str */
    size_t len = strlen(payload);
    name_p     = (char*)hasp_calloc(sizeof(char), len + 1);
    if(!name_p) return NULL;
    strncpy(name_p, payload, len);

    hasp_font_info_t* new_font_item;
    new_font_item = (hasp_font_info_t*)_lv_ll_ins_tail(&hasp_fonts_ll);
    if(!new_font_item) return NULL;

    new_font_item->payload = name_p;
    new_font_item->font    = font;
    new_font_item->type    = font_type;
    return font;
}

// Convert the payload to a font pointer
lv_font_t* get_font(const char* payload)
{
#if HASP_USE_FREETYPE > 0
    LOG_DEBUG(TAG_FONT, F("FreeType High Watermark %u"), lv_ft_freetype_high_watermark());
#endif

    lv_font_t* font = font_find_in_list(payload);
    if(font) return font;

    return font_add_to_list(payload);
}
