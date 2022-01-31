/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef ARDUINO
#include "pgmspace.h"
#include <Arduino.h>
#endif

#include "hasplib.h"

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
    if(Parser::is_only_digits(payload)) {
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
    uint16_t sdbm    = Parser::get_sdbm(payload);

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

// Map events to either ON or OFF (UP or DOWN)
bool Parser::get_event_state(uint8_t eventid)
{
    switch(eventid) {
        case HASP_EVENT_ON:
        case HASP_EVENT_DOWN:
        case HASP_EVENT_LONG:
        case HASP_EVENT_HOLD:
            return true;
        // case HASP_EVENT_OFF:
        // case HASP_EVENT_UP:
        // case HASP_EVENT_SHORT:
        // case HASP_EVENT_DOUBLE:
        // case HASP_EVENT_LOST:
        default:
            return false;
    }
}

// Map events to their description string
void Parser::get_event_name(uint8_t eventid, char* buffer, size_t size)
{
    switch(eventid) {
        case HASP_EVENT_ON:
            memcpy_P(buffer, PSTR("on"), size);
            break;
        case HASP_EVENT_OFF:
            memcpy_P(buffer, PSTR("off"), size);
            break;
        case HASP_EVENT_UP:
            memcpy_P(buffer, PSTR("up"), size);
            break;
        case HASP_EVENT_DOWN:
            memcpy_P(buffer, PSTR("down"), size);
            break;
        case HASP_EVENT_RELEASE:
            memcpy_P(buffer, PSTR("release"), size);
            break;
        case HASP_EVENT_LONG:
            memcpy_P(buffer, PSTR("long"), size);
            break;
        case HASP_EVENT_HOLD:
            memcpy_P(buffer, PSTR("hold"), size);
            break;
        case HASP_EVENT_LOST:
            memcpy_P(buffer, PSTR("lost"), size);
            break;
        case HASP_EVENT_CHANGED:
            memcpy_P(buffer, PSTR("changed"), size);
            break;
        default:
            memcpy_P(buffer, PSTR("unknown"), size);
    }
}

/* 16-bit hashing function http://www.cse.yorku.ca/~oz/hash.html */
/* all possible attributes are hashed and checked if they are unique */
uint16_t Parser::get_sdbm(const char* str)
{
    uint16_t hash = 0;
    while(char c = *str++) hash = tolower(c) + (hash << 6) - hash; // case insensitive
    return hash;
}

bool Parser::is_true(const char* s)
{
    return (!strcasecmp_P(s, PSTR("true")) || !strcasecmp_P(s, PSTR("on")) || !strcasecmp_P(s, PSTR("yes")) ||
            !strcmp_P(s, PSTR("1")));
}

bool Parser::is_true(JsonVariant json)
{
    return is_true(json.as<std::string>().c_str());
}

bool Parser::is_only_digits(const char* s)
{
    size_t digits = 0;
    while(*(s + digits) != '\0' && isdigit(*(s + digits))) {
        digits++;
    }
    return strlen(s) == digits;
}

int Parser::format_bytes(size_t filesize, char* buf, size_t len)
{
    if(filesize < D_FILE_SIZE_DIVIDER) return snprintf_P(buf, len, PSTR("%d " D_FILE_SIZE_BYTES), filesize);
    filesize = filesize * 100;

    filesize = filesize / D_FILE_SIZE_DIVIDER; // multiply by 100 for 2 decimal place
    if(filesize < D_FILE_SIZE_DIVIDER * 100)
        return snprintf_P(buf, len, PSTR("%d" D_DECIMAL_POINT "%02d " D_FILE_SIZE_KILOBYTES), filesize / 100,
                          filesize % 100);

    filesize = filesize / D_FILE_SIZE_DIVIDER; // multiply by 100 for 2 decimal place
    if(filesize < D_FILE_SIZE_DIVIDER * 100)
        return snprintf_P(buf, len, PSTR("%d" D_DECIMAL_POINT "%02d " D_FILE_SIZE_MEGABYTES), filesize / 100,
                          filesize % 100);

    return snprintf_P(buf, len, PSTR("%d" D_DECIMAL_POINT "%02d " D_FILE_SIZE_GIGABYTES), filesize / 100,
                      filesize % 100);
}

uint8_t Parser::get_action_id(const char* action)
{
    if(!strcasecmp_P(action, PSTR("prev"))) {
        return HASP_NUM_PAGE_PREV;
    } else if(!strcasecmp_P(action, PSTR("next"))) {
        return HASP_NUM_PAGE_NEXT;
    } else if(!strcasecmp_P(action, PSTR("back"))) {
        return HASP_NUM_PAGE_BACK;
    } else if(action[0] == 'p') {
        action++;
        if(is_only_digits(action)) {
            return atoi(action);
        }
    }
    return 0;
}

#ifndef ARDUINO
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif