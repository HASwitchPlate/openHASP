/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_HTTP_H
#define HASP_HTTP_H

#include "hasp_conf.h"

#ifndef HTTP_USERNAME
#define HTTP_USERNAME ""
#endif

#ifndef HTTP_PASSWORD
#define HTTP_PASSWORD ""
#endif

struct hasp_http_config_t
{
    bool enable   = true;
    uint16_t port = 80;

    char username[MAX_USERNAME_LENGTH] = HTTP_USERNAME;
    char password[MAX_PASSWORD_LENGTH] = HTTP_PASSWORD;
};

void httpSetup();
IRAM_ATTR void httpLoop(void);
// void httpReconnect(void);
void httpStart(void);
void httpStop(void);

size_t httpClientWrite(const uint8_t* buf, size_t size); // Screenshot Write Data

#if HASP_USE_CONFIG > 0
bool httpGetConfig(const JsonObject& settings);
bool httpSetConfig(const JsonObject& settings);
#endif // HASP_USE_CONFIG

/* clang-format off */
//default theme
#ifndef D_HTTP_COLOR_TEXT
#define D_HTTP_COLOR_TEXT               "#000"       // Global text color - Black
#endif
#ifndef D_HTTP_COLOR_BACKGROUND
#define D_HTTP_COLOR_BACKGROUND         "#fff"       // Global background color - White
#endif
#ifndef D_HTTP_COLOR_INPUT_TEXT
#define D_HTTP_COLOR_INPUT_TEXT         "#000"       // Input text color - Black
#endif
#ifndef D_HTTP_COLOR_INPUT
#define D_HTTP_COLOR_INPUT              "#fff"       // Input background color - White
#endif
#ifndef D_HTTP_COLOR_INPUT_WARNING
#define D_HTTP_COLOR_INPUT_WARNING      "#f00"       // Input warning border color - Red
#endif
#ifndef D_HTTP_COLOR_BUTTON_TEXT
#define D_HTTP_COLOR_BUTTON_TEXT        "#fff"       // Button text color - White
#endif
#ifndef D_HTTP_COLOR_BUTTON
#define D_HTTP_COLOR_BUTTON             "#1fa3ec"    // Button color - Vivid blue
#endif
#ifndef D_HTTP_COLOR_BUTTON_HOVER
#define D_HTTP_COLOR_BUTTON_HOVER       "#0083cc"    // Button color - Olympic blue
#endif
#ifndef D_HTTP_COLOR_BUTTON_RESET
#define D_HTTP_COLOR_BUTTON_RESET       "#f00"       // Restart/Reset button color - red
#endif
#ifndef D_HTTP_COLOR_BUTTON_RESET_HOVER
#define D_HTTP_COLOR_BUTTON_RESET_HOVER "#b00"       // Restart/Reset button color - Dark red
#endif
#ifndef D_HTTP_COLOR_GROUP_TEXT
#define D_HTTP_COLOR_GROUP_TEXT         "#000"       // Container text color - Black
#endif
#ifndef D_HTTP_COLOR_GROUP
#define D_HTTP_COLOR_GROUP              "#f3f3f3"    // Container color - Light gray
#endif
#ifndef D_HTTP_COLOR_FOOTER_TEXT
#define D_HTTP_COLOR_FOOTER_TEXT        "#0083cc"    // Text color of the page footer
#endif
/* clang-format on */

#endif