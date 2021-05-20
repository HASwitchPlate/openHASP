/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if HASP_USE_CONSOLE > 0

#include "ConsoleInput.h"

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_console.h"

#include "../../hasp/hasp_dispatch.h"

uint8_t consoleInputEnabled = true;
ConsoleInput debugConsole(&Serial, HASP_CONSOLE_BUFFER);

void console_update_prompt()
{
    debugConsole.update();
}

void consoleSetup()
{
    LOG_TRACE(TAG_MSGR, F(D_SERVICE_STARTING));
    debugConsole.setLineCallback(dispatch_text_line);
    LOG_INFO(TAG_CONS, F(D_SERVICE_STARTED));
}

IRAM_ATTR void consoleLoop()
{
    if(!consoleInputEnabled) return;

    int16_t keypress;
    do {
        switch(keypress = debugConsole.readKey()) {

            case ConsoleInput::KEY_PAGE_UP:
                dispatch_page_next(LV_SCR_LOAD_ANIM_NONE);
                break;

            case ConsoleInput::KEY_PAGE_DOWN:
                dispatch_page_prev(LV_SCR_LOAD_ANIM_NONE);
                break;

            case(ConsoleInput::KEY_FN)...(ConsoleInput::KEY_FN + 12):
                dispatch_set_page(keypress - ConsoleInput::KEY_FN, LV_SCR_LOAD_ANIM_NONE);
                break;
        }
    } while(keypress != 0);
}

#if HASP_USE_CONFIG > 0
bool consoleGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(changed) configOutput(settings, TAG_CONS);
    return changed;
}

/** Set console Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool consoleSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_CONS);
    bool changed = false;

    return changed;
}
#endif // HASP_USE_CONFIG

#endif