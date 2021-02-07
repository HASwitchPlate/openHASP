/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_UTILITIES_H
#define HASP_UTILITIES_H

uint16_t hasp_util_get_sdbm(const char * str);
bool hasp_util_is_true(const char * s);
bool hasp_util_is_only_digits(const char * s);
int hasp_util_format_bytes(size_t filesize, char * buf, size_t len);

#endif