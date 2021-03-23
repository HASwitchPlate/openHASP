/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_UTILITIES_H
#define HASP_UTILITIES_H

#include <string>

class Utilities {

  public:
    static uint16_t get_sdbm(const char* str);
    static bool is_true(const char* s);
    static bool is_only_digits(const char* s);
    static int format_bytes(size_t filesize, char* buf, size_t len);
};

#ifndef ARDUINO
long map(long x, long in_min, long in_max, long out_min, long out_max);
#endif

#endif