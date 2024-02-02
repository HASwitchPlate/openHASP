/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if HASP_USE_TELNET > 0

#include "ConsoleInput.h"
#include <StreamUtils.h>

#include "hasp_debug.h"
#include "hasp_telnet.h"
#include "hasp_http.h"

#include "../../hasp/hasp_dispatch.h"

#define IAC_SHOW_CHARACTERS "\xff\xfc\x01"
#define IAC_HIDE_CHARACTERS "\xff\xfb\x01"

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
WiFiClient telnetClient;
static WiFiServer* telnetServer;
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
WiFiClient telnetClient;
static WiFiServer* telnetServer;
#else
// #include <STM32Ethernet.h>
EthernetClient telnetClient;
static EthernetServer telnetServer(23);
#endif

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
extern hasp_http_config_t http_config;
#endif

// Create a new Stream that buffers all writes to telnetClient
WriteBufferingClient bufferedTelnetClient{telnetClient, HASP_CONSOLE_BUFFER};

uint8_t telnetLoginState   = TELNET_UNAUTHENTICATED;
uint16_t telnetPort        = 23;
uint8_t telnetEnabled      = true; // Enable telnet debug output
uint8_t telnetLoginAttempt = 0;    // Initial attempt
ConsoleInput* telnetConsole;

void telnet_update_prompt()
{
    if(telnetConsole) telnetConsole->update(__LINE__);
    bufferedTelnetClient.flush();
}

static inline void telnet_new_prompt(const char* prompt)
{
    if(!telnetConsole) return;

    telnetClient.println(IAC_SHOW_CHARACTERS);
    telnetConsole->setPrompt(prompt);
    telnetConsole->clearLine(); // avoid leaking information, updates the prompt
    telnet_update_prompt();
}

static inline void telnet_echo_on()
{
    telnetClient.print(IAC_SHOW_CHARACTERS);
}

static inline void telnet_password_prompt()
{
    telnetClient.print(IAC_SHOW_CHARACTERS D_PASSWORD " " IAC_HIDE_CHARACTERS);
}

static inline void telnet_username_prompt()
{
    telnetClient.print(IAC_SHOW_CHARACTERS D_USERNAME " ");
}

static void telnetClientDisconnect()
{
    if(telnetClient.connected())
        LOG_TRACE(TAG_TELN, F(D_TELNET_CLOSING_CONNECTION), telnetClient.remoteIP().toString().c_str());
    bufferedTelnetClient.flush(); // empty buffer
    telnetClient.flush();         // empty buffer

    Log.unregisterOutput(1); // telnetClient
    telnetClient.stop();

    telnetLoginState   = TELNET_UNAUTHENTICATED;
    telnetLoginAttempt = 0; // Initial attempt
    delete telnetConsole;
    telnetConsole = NULL;
}

void telnetStop(void)
{
    telnetClientDisconnect();
    delete telnetServer;
    telnetServer = NULL;
}

void telnetClientLogon()
{
    debugPrintHaspHeader(&bufferedTelnetClient);
    telnetLoginState   = TELNET_AUTHENTICATED; // User and Pass are correct
    telnetLoginAttempt = 0;                    // Reset attempt counter
                                               // telnetClient.setTimeout(10);

    // empty buffers
    bufferedTelnetClient.flush();
    telnet_new_prompt("Prompt > ");
    telnetClient.flush();

    /* Now register logger for telnet and switch to buffered stream */
    Log.registerOutput(1, &bufferedTelnetClient, LOG_LEVEL_VERBOSE, true);
    LOG_TRACE(TAG_TELN, F(D_TELNET_CLIENT_LOGIN_FROM), telnetClient.remoteIP().toString().c_str());
}

void telnetAcceptClient()
{
    bufferedTelnetClient.flush(); // empty buffer

    if(telnetClient) {
        telnetClient.stop();     // previous client has disconnected
        Log.unregisterOutput(1); // telnetClient
    }
    telnetClient = telnetServer->available(); // ready for new client
    if(!telnetClient) {
        LOG_VERBOSE(TAG_TELN, F(D_TELNET_CLIENT_NOT_CONNECTED));
        return;
    }
    LOG_INFO(TAG_TELN, F(D_TELNET_CLIENT_CONNECT_FROM), telnetClient.remoteIP().toString().c_str());
    telnetClient.setNoDelay(true);
    telnetClient.printf("\x1b]2;%s\x07" TERM_CLEAR_LINE, haspDevice.get_hostname());
    telnetClient.println("\r\nWelcome\r\n");

    /* Avoid a buffer here */
    // telnetClient.print((char)0xFF); // DO TERMINAL-TYPE
    // telnetClient.print((char)0xFD);
    // telnetClient.print((char)0x1B);

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
    if(strlen(http_config.username) != 0 || strlen(http_config.password) != 0) {
        telnetLoginState = TELNET_UNAUTHENTICATED;
        telnetClient.println();
        telnet_username_prompt();
    } else
#endif
    {
        telnetClientLogon();
    }

    telnetClient.flush();
    telnetLoginAttempt = 0; // Initial attempt
}

#if 0
static inline void telnetProcessLine()
{
    telnetInputBuffer[telnetInputIndex] = 0; // null terminate our char array

    switch(telnetLoginState) {
        case TELNET_UNAUTHENTICATED: {
            telnetClient.printf(PSTR(D_PASSWORD" %c%c%c"), 0xFF, 0xFB, 0x01); // Hide characters
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
            telnetLoginState = strcmp(telnetInputBuffer, http_config.username) == 0 ? TELNET_USERNAME_OK : TELNET_USERNAME_NOK;
            break;
        }
        case TELNET_USERNAME_OK:
        case TELNET_USERNAME_NOK: {
            telnetClient.printf(PSTR("%c%c%c\n"), 0xFF, 0xFC, 0x01); // Show characters
            if(telnetLoginState == TELNET_USERNAME_OK && strcmp(telnetInputBuffer, http_config.password) == 0) {
                telnetClientLogon();
            } else {
                telnetLoginState = TELNET_UNAUTHENTICATED;
                telnetLoginAttempt++; // Subsequent attempt
                telnetClient.println(F(D_NETWORK_CONNECTION_UNAUTHORIZED"\r\n"));
                LOG_WARNING(TAG_TELN, F(D_TELNET_INCORRECT_LOGIN_ATTEMPT), telnetClient.remoteIP().toString().c_str());
                if(telnetLoginAttempt >= 3) {
                    telnetClientDisconnect();
                } else {
                    telnetClient.print(F(D_USERNAME" "));
                }
            }
#else
            telnetClientLogon();
#endif
            break;
        }
        default:
            if(telnetInputIndex > 0) {
                if(strcasecmp_P(telnetInputBuffer, PSTR("exit")) == 0) {
                    telnetClientDisconnect();
                } else {
                    dispatchTextLine(telnetInputBuffer);
                }
            }
    }

    telnetInputIndex = 0; // reset input buffer index
}

static inline void telnetProcessData(char ch)
{

    switch(ch) {
        case 10: // Linefeed
            telnetInputIndex = 0;
            break;
        case 8: // Backspace
            if(telnetInputIndex > 0) telnetInputIndex--;
            break;
        case 13: // Carriage Return
            telnetProcessLine("");
            break;
        case 32 ... 250:
            if(!isprint(ch)) {
                telnetClient.printf(PSTR(" 0x%02x "), ch);
            }
            // If we have room left in our buffer add the current byte
            if(telnetInputIndex < sizeof(telnetInputBuffer) - 1) {
                telnetInputBuffer[telnetInputIndex++] = ch;
            }
    }

    // Properly terminate buffer string
    if(telnetInputIndex < sizeof(telnetInputBuffer)) {
        telnetInputBuffer[telnetInputIndex] = 0;
    }
}

static inline void telnetProcessCharacter(char ch)
{
    // if(ch == (char)0xff || telnetInCommandMode) {
    //    telnetProcessCharacter(ch);
    //} else {
    telnetProcessData(ch);
    //}
}

#endif

static void telnetProcessLine(const char* input)
{
    bufferedTelnetClient.flush();

    switch(telnetLoginState) {
        case TELNET_UNAUTHENTICATED: {

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
            bool username_is_valid = strcmp(input, http_config.username) == 0;
            telnetLoginState       = username_is_valid ? TELNET_USERNAME_OK : TELNET_USERNAME_NOK;
            telnet_password_prompt();
            break;
        }
        case TELNET_USERNAME_OK:
        case TELNET_USERNAME_NOK: {
            bool password_is_valid = strcmp(input, http_config.password) == 0;
            if(telnetConsole) telnetConsole->clearLine();
            telnet_echo_on(); // Show characters

            if(telnetLoginState == TELNET_USERNAME_OK && password_is_valid) {
                telnetClientLogon();
            } else {
                telnetLoginState = TELNET_UNAUTHENTICATED;
                telnetLoginAttempt++; // Subsequent attempt
                LOG_WARNING(TAG_TELN, F(D_TELNET_INCORRECT_LOGIN_ATTEMPT), telnetClient.remoteIP().toString().c_str());

                telnetClient.println(F(D_NETWORK_CONNECTION_UNAUTHORIZED));

                if(telnetLoginAttempt >= 3) {
                    telnetClientDisconnect();
                } else {
                    telnetClient.println(F("\r\n\r\n" D_USERNAME " "));
                }
            }
#else
            telnetClientLogon();
#endif
            break;
        }
        default:
            if(strcasecmp_P(input, PSTR("exit")) == 0 || strcasecmp_P(input, PSTR("quit")) == 0 ||
               strcasecmp_P(input, PSTR("bye")) == 0) {
                telnetClientDisconnect();
            } else if(strcasecmp_P(input, PSTR("logoff")) == 0) {
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
                if(strcmp(input, http_config.password) == 0) {
                    telnet_username_prompt();
                    telnetLoginState = TELNET_UNAUTHENTICATED;
                } else
#endif
                {
                    telnetClientDisconnect();
                }
            } else {
                dispatch_text_line(input, TAG_TELN);
            }
    }

    telnetClient.flush();
}

void telnetStart()
{
    // telnetSetConfig(settings);

    if(telnetEnabled) { // Setup telnet server for remote debug output
#if defined(STM32F4xx)
        // if(!telnetServer) telnetServer = new EthernetServer(telnetPort);
        // if(telnetServer) {
        telnetServer->begin();
        LOG_INFO(TAG_TELN, F(D_TELNET_STARTED));
        // } else {
        //    LOG_ERROR(TAG_TELN,F("Failed to start telnet server"));
        //}
#else
        if(!telnetServer) telnetServer = new WiFiServer(telnetPort);
        if(telnetServer) {
            telnetServer->setNoDelay(true);
            telnetServer->begin();

            // telnetConsole = new ConsoleInput(&telnetClient, HASP_CONSOLE_BUFFER);
            // if(telnetConsole != NULL) {
            //     telnetConsole->setLineCallback(telnetProcessLine);
            //     LOG_INFO(TAG_TELN, F(D_TELNET_STARTED));
            //     return;
            // }
            LOG_INFO(TAG_TELN, F(D_TELNET_STARTED));
        } else {
            LOG_ERROR(TAG_TELN, F(D_TELNET_FAILED));
        }
#endif
    }
}

void telnetSetup()
{
#if HASP_START_TELNET
    telnetStart();
#endif
}

IRAM_ATTR void telnetLoop()
{
    // Basic telnet client handling code from: https://gist.github.com/tablatronix/4793677ca748f5f584c95ec4a2b10303

#if defined(STM32F4xx)
    Ethernet.schedule();
    // if(telnetServer)
    { // client is connected
        EthernetClient client = telnetServer->available();
        if(client) {
            if(!telnetClient || !telnetClient.connected()) {
                // telnetAcceptClient(client);

                telnetClient = client; // ready for new client
                // LOG_TRACE(TAG_TELN,F("Client connected from %s"), telnetClient.remoteIP().toString().c_str());
                if(!telnetClient) {
                    LOG_WARNING(TAG_TELN, F(D_TELNET_CLIENT_NOT_CONNECTED));
                    return;
                }
                LOG_TRACE(TAG_TELN, F(D_TELNET_CLIENT_CONNECTED));

                /* Avoid a buffer here */
                // telnetClient.print(0xFF); // DO TERMINAL-TYPE
                // telnetClient.print(0xFD);
                // telnetClient.print(0x1B);

            } else {
                // client.stop(); // have client, block new connections
            }
        }
    }
#else

    /* Active Client: Process user input */
    if(telnetClient.connected()) {
        if(telnetConsole) {
            while(int16_t key = telnetConsole->readKey()) {
                switch(key) {
                    case 0xf8:
                    case KEY_CTRL('C'): // 0x03 = ^C = Cancel (Quit)
                        telnetClientDisconnect();
                        break;

                    case 0xec:
                    case KEY_CTRL('D'): // ^D = Disconnect (Logout)
                        LOG_DEBUG(TAG_TELN, "Ctrl-D");
                        break;
                }
                if(!telnetConsole) return; // the telnetConsole was destroyed
                if(bufferedTelnetClient.available() <= 0) bufferedTelnetClient.flush(); // flush pending updates
            };

        } else {
            telnetConsole = new ConsoleInput(&bufferedTelnetClient, HASP_CONSOLE_BUFFER);
            if(telnetConsole) {
                telnetConsole->setLineCallback(telnetProcessLine);
#ifdef HASP_DEBUG
                telnetConsole->setDebug(true);
#endif
            } else {
                telnetClientDisconnect();
                LOG_ERROR(TAG_TELN, F(D_TELNET_FAILED));
            }
        }
    }

#endif
}

void telnetEverySecond(void)
{
    if(!telnetClient.connected() && telnetLoginState != TELNET_UNAUTHENTICATED) {
        telnetClientDisconnect(); // active client disconnected
    }

    if(telnetServer && telnetServer->hasClient()) { // a new client has connected
        if(!telnetClient.connected()) {             // nobody is already connected
            telnetAcceptClient();                   // allow the new client
        } else {
            LOG_WARNING(TAG_TELN, F(D_TELNET_CLIENT_REJECTED));
            telnetServer->available().stop(); // already have a client, block new connections
        }
    }
}

#if HASP_USE_CONFIG > 0
bool telnetGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(telnetEnabled != settings[FPSTR(FP_CONFIG_ENABLE)].as<bool>()) changed = true;
    settings[FPSTR(FP_CONFIG_ENABLE)] = telnetEnabled;

    if(telnetPort != settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PORT)] = telnetPort;

    if(changed) configOutput(settings, TAG_TELN);
    return changed;
}

/** Set TELNET Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formatted to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool telnetSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_TELN);
    bool changed = false;

    changed |= configSet(telnetEnabled, settings[FPSTR(FP_CONFIG_ENABLE)], F("telnetEnabled"));
    changed |= configSet(telnetPort, settings[FPSTR(FP_CONFIG_PORT)], F("telnetPort"));

    return changed;
}
#endif // HASP_USE_CONFIG

#endif