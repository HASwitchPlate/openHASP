/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if HASP_USE_CONSOLE > 0

#include "ConsoleInput.h"
#include <StreamUtils.h>

#include "hasp_debug.h"
#include "hasp_console.h"

#include "../../hasp/hasp_dispatch.h"

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
extern hasp_http_config_t http_config;
#endif

// Create a new Stream that buffers all writes to serialClient
HardwareSerial* bufferedSerialClient = (HardwareSerial*)&HASP_SERIAL;

uint8_t consoleLoginState   = CONSOLE_UNAUTHENTICATED;
uint16_t serialPort         = 0;
uint8_t consoleEnabled      = true; // Enable serial debug output
uint8_t consoleLoginAttempt = 0;    // Initial attempt
ConsoleInput* console;

void console_update_prompt()
{
    if(console) console->update(__LINE__);
    bufferedSerialClient->flush();
}

static void console_timeout()
{
    // todo
}

static void console_logoff()
{
    consoleLoginState   = CONSOLE_UNAUTHENTICATED;
    consoleLoginAttempt = 0; // Reset attempt counter
}

static void console_logon()
{
    // bufferedSerialClient->println();
    // debugPrintHaspHeader(bufferedSerialClient);

    consoleLoginState   = CONSOLE_AUTHENTICATED; // User and Pass are correct
    consoleLoginAttempt = 0;                     // Reset attempt counter

    LOG_TRACE(TAG_CONS, F(D_TELNET_CLIENT_LOGIN_FROM), "serial");
}

static void console_process_line(const char* input)
{
    switch(consoleLoginState) {
        case CONSOLE_UNAUTHENTICATED: {
            char buffer[20];
            snprintf_P(buffer, sizeof(buffer), PSTR(D_PASSWORD " %c%c%c\n"), 0xFF, 0xFB,
                       0x01); // Hide characters
            bufferedSerialClient->print(buffer);
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
            consoleLoginState = strcmp(input, http_config.username) == 0 ? CONSOLE_USERNAME_OK : CONSOLE_USERNAME_NOK;
            break;
        }
        case CONSOLE_USERNAME_OK:
        case CONSOLE_USERNAME_NOK: {
            bufferedSerialClient->printf(PSTR("%c%c%c\n"), 0xFF, 0xFC, 0x01); // Show characters
            if(consoleLoginState == CONSOLE_USERNAME_OK && strcmp(input, http_config.password) == 0) {
                console_logon();
            } else {
                consoleLoginState = CONSOLE_UNAUTHENTICATED;
                consoleLoginAttempt++; // Subsequent attempt
                bufferedSerialClient->println(F(D_NETWORK_CONNECTION_UNAUTHORIZED "\r\n"));
                LOG_WARNING(TAG_CONS, F(D_TELNET_INCORRECT_LOGIN_ATTEMPT), "serial");
                if(consoleLoginAttempt >= 3) {
                    console_timeout();
                } else {
                    bufferedSerialClient->print(F(D_USERNAME " "));
                }
            }
#else
            console_logon();
#endif
            break;
        }
        default:
            if(strcasecmp_P(input, PSTR("logoff")) == 0) {
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
                if(strcmp(input, http_config.password) == 0) {
                    bufferedSerialClient->println(F("\r\n" D_USERNAME " "));
                    consoleLoginState = CONSOLE_UNAUTHENTICATED;
                } else {
                }
#endif
            } else {
                dispatch_text_line(input, TAG_CONS);
            }
    }
}

void consoleStart()
{
    LOG_TRACE(TAG_MSGR, F(D_SERVICE_STARTING));
    console = new ConsoleInput(bufferedSerialClient, HASP_CONSOLE_BUFFER);
    if(console) {
        if(!debugStartSerial()) { // failed to open Serial port
            LOG_INFO(TAG_CONS, F(D_SERVICE_DISABLED));
            return;
        }

        /* Now register logger for serial */
        Log.registerOutput(0, bufferedSerialClient, HASP_LOG_LEVEL, true);
        bufferedSerialClient->flush();

        LOG_INFO(TAG_CONS, F(D_SERVICE_STARTED));

        console->setLineCallback(console_process_line);
        console->setAutoUpdate(false);
        console_logon(); // todo: logon
        console->setPrompt("Prompt > ");
    } else {
        console_logoff();
        LOG_ERROR(TAG_CONS, F(D_SERVICE_START_FAILED));
    }
}

void consoleStop()
{
    console_logoff();
    Log.unregisterOutput(0); // serialClient
    HASP_SERIAL.end();

    delete console;
    console = NULL;
}

void consoleSetup()
{
#if HASP_START_CONSOLE
    consoleStart();
#endif
}

IRAM_ATTR void consoleLoop()
{
    if(!console) return;

    bool update = false;
    while(int16_t keypress = console->readKey()) {
        switch(keypress) {

            case ConsoleInput::KEY_PAGE_UP:
                dispatch_page_next(LV_SCR_LOAD_ANIM_NONE);
                break;

            case ConsoleInput::KEY_PAGE_DOWN:
                dispatch_page_prev(LV_SCR_LOAD_ANIM_NONE);
                break;

            case(ConsoleInput::KEY_FN)...(ConsoleInput::KEY_FN + 12):
                dispatch_set_page(keypress - ConsoleInput::KEY_FN, LV_SCR_LOAD_ANIM_NONE, 500, 0);
                break;

            case 0:
            case -1:
                break;
                
            default: {
                update = true;
            }
        }
    }
    if(update) console_update_prompt();
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