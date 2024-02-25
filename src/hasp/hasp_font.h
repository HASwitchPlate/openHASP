/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FONT_H
#define HASP_FONT_H

void font_setup();
lv_font_t* get_font(const char* payload);
void font_clear_list(const char* payload);

#endif