/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"
#if HASP_USE_TELNET > 0

    #include "ArduinoJson.h"
    #include "ConsoleInput.h"

    #include "hasp_debug.h"
    #include "hasp_config.h"
    #include "hasp_telnet.h"

    #include "../hasp/hasp_dispatch.h"

    #if defined(ARDUINO_ARCH_ESP32)
        #include <WiFi.h>
WiFiClient telnetClient;
static WiFiServer * telnetServer;
    #elif defined(ARDUINO_ARCH_ESP8266)
        #include <ESP8266WiFi.h>
WiFiClient telnetClient;
static WiFiServer * telnetServer;
    #else
//#include <STM32Ethernet.h>
EthernetClient telnetClient;
static EthernetServer telnetServer(23);
    #endif

    #if HASP_USE_HTTP > 0
extern char httpUser[32];
extern char httpPassword[32];
    #endif

uint8_t telnetLoginState   = TELNET_UNAUTHENTICATED;
uint16_t telnetPort        = 23;
uint8_t telnetEnabled      = true; // Enable telnet debug output
uint8_t telnetLoginAttempt = 0;    // Initial attempt
ConsoleInput * telnetConsole;

void telnetClientDisconnect()
{
    Log.unregisterOutput(1); // telnetClient
    LOG_TRACE(TAG_TELN, F(D_TELNET_CLOSING_CONNECTION), telnetClient.remoteIP().toString().c_str());
    telnetLoginState   = TELNET_UNAUTHENTICATED;
    telnetLoginAttempt = 0; // Initial attempt
    // delete telnetConsole;
    // telnetConsole = NULL;
    telnetClient.stop();
}

void telnetClientLogon()
{
    telnetClient.println();
    debugPrintHaspHeader(&telnetClient);
    telnetLoginState   = TELNET_AUTHENTICATED; // User and Pass are correct
    telnetLoginAttempt = 0;                    // Reset attempt counter

    /* Now register logger for telnet */
    Log.registerOutput(1, &telnetClient, LOG_LEVEL_VERBOSE, true);
    telnetClient.flush();
    // telnetClient.setTimeout(10);

    LOG_TRACE(TAG_TELN, F(D_TELNET_CLIENT_LOGIN_FROM), telnetClient.remoteIP().toString().c_str());
}

void telnetAcceptClient()
{
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

    /* Avoid a buffer here */
    // telnetClient.print((char)0xFF); // DO TERMINAL-TYPE
    // telnetClient.print((char)0xFD);
    // telnetClient.print((char)0x1B);

    #if HASP_USE_HTTP > 0
    if(strlen(httpUser) != 0 || strlen(httpPassword) != 0) {
        telnetClient.println(F("\r\n" D_USERNAME " "));
        telnetLoginState = TELNET_UNAUTHENTICATED;
    } else
    #endif
    {
        telnetClientLogon();
    }
    telnetLoginAttempt = 0; // Initial attempt
}

    #if 0
static inline void telnetProcessLine()
{
    telnetInputBuffer[telnetInputIndex] = 0; // null terminate our char array

    switch(telnetLoginState) {
        case TELNET_UNAUTHENTICATED: {
            telnetClient.printf(PSTR(D_TELNET_PASSWORD" %c%c%c"), 0xFF, 0xFB, 0x01); // Hide characters
        #if HASP_USE_HTTP > 0
            telnetLoginState = strcmp(telnetInputBuffer, httpUser) == 0 ? TELNET_USERNAME_OK : TELNET_USERNAME_NOK;
            break;
        }
        case TELNET_USERNAME_OK:
        case TELNET_USERNAME_NOK: {
            telnetClient.printf(PSTR("%c%c%c\n"), 0xFF, 0xFC, 0x01); // Show characters
            if(telnetLoginState == TELNET_USERNAME_OK && strcmp(telnetInputBuffer, httpPassword) == 0) {
                telnetClientLogon();
            } else {
                telnetLoginState = TELNET_UNAUTHENTICATED;
                telnetLoginAttempt++; // Subsequent attempt
                telnetClient.println(F(D_TELNET_AUTHENTICATION_FAILED"\r\n"));
                LOG_WARNING(TAG_TELN, F(D_TELNET_INCORRECT_LOGIN_ATTEMPT), telnetClient.remoteIP().toString().c_str());
                if(telnetLoginAttempt >= 3) {
                    telnetClientDisconnect();
                } else {
                    telnetClient.print(F(D_TELNET_USERNAME" "));
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
        case 13: // Cariage Return
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

static inline void telnetProcessLine(const char * input)
{
    switch(telnetLoginState) {
        case TELNET_UNAUTHENTICATED: {
            char buffer[20];
            snprintf_P(buffer, sizeof(buffer), PSTR(D_PASSWORD " %c%c%c\n"), 0xFF, 0xFB,
                       0x01); // Hide characters
            telnetClient.print(buffer);
    #if HASP_USE_HTTP > 0
            telnetLoginState = strcmp(input, httpUser) == 0 ? TELNET_USERNAME_OK : TELNET_USERNAME_NOK;
            break;
        }
        case TELNET_USERNAME_OK:
        case TELNET_USERNAME_NOK: {
            telnetClient.printf(PSTR("%c%c%c\n"), 0xFF, 0xFC, 0x01); // Show characters
            if(telnetLoginState == TELNET_USERNAME_OK && strcmp(input, httpPassword) == 0) {
                telnetClientLogon();
            } else {
                telnetLoginState = TELNET_UNAUTHENTICATED;
                telnetLoginAttempt++; // Subsequent attempt
                telnetClient.println(F(D_TELNET_AUTHENTICATION_FAILED "\r\n"));
                LOG_WARNING(TAG_TELN, F(D_TELNET_INCORRECT_LOGIN_ATTEMPT), telnetClient.remoteIP().toString().c_str());
                if(telnetLoginAttempt >= 3) {
                    telnetClientDisconnect();
                } else {
                    telnetClient.print(F(D_USERNAME " "));
                }
            }
    #else
            telnetClientLogon();
    #endif
            break;
        }
        default:
            if(strcasecmp_P(input, PSTR("exit")) == 0) {
                telnetClientDisconnect();
            } else if(strcasecmp_P(input, PSTR("logoff")) == 0) {
                telnetClient.println(F("\r\n" D_USERNAME " "));
                telnetLoginState = TELNET_UNAUTHENTICATED;
            } else {
                dispatch_text_line(input);
            }
    }
}

void telnetSetup()
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

            telnetConsole = new ConsoleInput(&telnetClient, HASP_CONSOLE_BUFFER);
            if(telnetConsole != NULL) {
                telnetConsole->setLineCallback(telnetProcessLine);
                LOG_INFO(TAG_TELN, F(D_TELNET_STARTED));
                return;
            }
        }

        LOG_ERROR(TAG_TELN, F(D_TELNET_FAILED));
    #endif
    }
}

void telnetLoop()
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
    if(telnetServer && telnetServer->hasClient()) { // a new client has connected
        if(!telnetClient.connected()) {             // nobody is already connected
            telnetAcceptClient();                   // allow the new client
        } else {
            LOG_WARNING(TAG_TELN, F(D_TELNET_CLIENT_REJECTED));
            telnetServer->available().stop(); // already have a client, block new connections
        }
    } else {
        if(!telnetClient.connected() && telnetLoginState != TELNET_UNAUTHENTICATED) {
            telnetClientDisconnect(); // active client disconnected
        } else {

            /* Active Client: Process user input */
            if(telnetConsole && telnetClient.connected()) {
                int16_t keypress = telnetConsole->readKey();
                switch(keypress) {
                    case ConsoleInput::KEY_PAUSE:
                        break;
                }
            }
        }
    }
    #endif
}

    #if HASP_USE_CONFIG > 0
bool telnetGetConfig(const JsonObject & settings)
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
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool telnetSetConfig(const JsonObject & settings)
{
    configOutput(settings, TAG_TELN);
    bool changed = false;

    changed |= configSet(telnetEnabled, settings[FPSTR(FP_CONFIG_ENABLE)], F("telnetEnabled"));
    changed |= configSet(telnetPort, settings[FPSTR(FP_CONFIG_PORT)], F("telnetPort"));

    return changed;
}
    #endif // HASP_USE_CONFIG

#endif