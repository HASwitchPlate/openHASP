#include <cctype>

#ifdef ARDUINO
    #include "Arduino.h"
#endif

#include "hasp_conf.h"

#include "hasp_utilities.h"

/* 16-bit hashing function http://www.cse.yorku.ca/~oz/hash.html */
/* all possible attributes are hashed and checked if they are unique */
uint16_t Utilities::get_sdbm(const char * str)
{
    uint16_t hash = 0;
    char c;

    // while(c = *str++) hash = c + (hash << 6) + (hash << 16) - hash;
    while((c = *str++)) {
        hash = tolower(c) + (hash << 6) - hash;
    }

    return hash;
}

bool Utilities::is_true(const char * s)
{
    return (!strcasecmp_P(s, PSTR("true")) || !strcasecmp_P(s, PSTR("on")) || !strcasecmp_P(s, PSTR("yes")) ||
            !strcmp_P(s, PSTR("1")));
}

bool Utilities::is_only_digits(const char * s)
{
    size_t digits = 0;
    while(*(s + digits) != '\0' && isdigit(*(s + digits))) {
        digits++;
    }
    return strlen(s) == digits;
}

int Utilities::format_bytes(size_t filesize, char * buf, size_t len)
{
    if(filesize < 1024) return snprintf_P(buf, len, PSTR("%d B"), filesize);

    char labels[] = "kMGT";
    filesize      = filesize * 10 / 1024; // multiply by 10 for 1 decimal place
    int unit      = 0;

    while(filesize >= 10240 && unit < sizeof(labels) - 1) { // it is multiplied by 10
        unit++;
        filesize = filesize / 1024;
    }

    return snprintf_P(buf, len, PSTR("%d.%d %ciB"), filesize / 10, filesize % 10, labels[unit]);
}

#ifndef ARDUINO
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif