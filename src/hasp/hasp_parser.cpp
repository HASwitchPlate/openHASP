/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <cstdlib>

#ifdef ARDUINO
#include "pgmspace.h"
#endif

#include "lvgl.h"
#include "hasp_conf.h"

#include "hasp_parser.h"
#include "hasp_utilities.h"

bool Parser::haspPayloadToColor(const char* payload, lv_color32_t& color)
{
    /* HEX format #rrggbb or #rgb */
    if(*payload == '#') {
        if(strlen(payload) >= 8) return false;

        char* pEnd;
        long color_int = strtol(payload + 1, &pEnd, HEX);

        if(pEnd - payload == 7) { // #rrbbgg
            color.ch.red   = color_int >> 16 & 0xff;
            color.ch.green = color_int >> 8 & 0xff;
            color.ch.blue  = color_int & 0xff;

        } else if(pEnd - payload == 4) { // #rgb
            color.ch.red   = color_int >> 8 & 0xf;
            color.ch.green = color_int >> 4 & 0xf;
            color.ch.blue  = color_int & 0xf;

            color.ch.red += color.ch.red * HEX;
            color.ch.green += color.ch.green * HEX;
            color.ch.blue += color.ch.blue * HEX;

        } else {
            return false; /* Invalid hex length */
        }

        return true; /* Color found */
    }

    /* 16-bit RGB565 Color Scheme*/
    if(Utilities::is_only_digits(payload)) {
        uint16_t c = atoi(payload);

        /* Initial colors */
        uint8_t R5 = ((c >> 11) & 0b11111);
        uint8_t G6 = ((c >> 5) & 0b111111);
        uint8_t B5 = (c & 0b11111);

        /* Remapped colors */
        color.ch.red   = (R5 * 527 + 23) >> 6;
        color.ch.green = (G6 * 259 + 33) >> 6;
        color.ch.blue  = (B5 * 527 + 23) >> 6;

        return true; /* Color found */
    }

    /* Named colors */
    size_t numColors = sizeof(haspNamedColors) / sizeof(haspNamedColors[0]);
    uint16_t sdbm    = Utilities::get_sdbm(payload);

#ifdef ARDUINO
    for(size_t i = 0; i < numColors; i++) {
        if(sdbm == (uint16_t)pgm_read_word_near(&(haspNamedColors[i].hash))) {
            color.ch.red   = (uint16_t)pgm_read_byte_near(&(haspNamedColors[i].r));
            color.ch.green = (uint16_t)pgm_read_byte_near(&(haspNamedColors[i].g));
            color.ch.blue  = (uint16_t)pgm_read_byte_near(&(haspNamedColors[i].b));

            return true; /* Color found */
        }
    }
#else
    for(size_t i = 0; i < numColors; i++) {
        if(sdbm == haspNamedColors[i].hash) {
            color.ch.red   = haspNamedColors[i].r;
            color.ch.green = haspNamedColors[i].g;
            color.ch.blue  = haspNamedColors[i].b;

            return true; /* Color found */
        }
    }
#endif

    return false; /* Color not found */
}