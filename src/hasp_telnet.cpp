#include "hasp_conf.h"
#if HASP_USE_TELNET > 0

#include "Arduino.h"
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_http.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_telnet.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>
#endif

#define TELNET_UNAUTHENTICATED 0
#define TELNET_USERNAME_OK 10
#define TELNET_USERNAME_NOK 99
#define TELNET_AUTHENTICATED 255

extern char httpUser[32];
extern char httpPassword[32];

uint8_t telnetLoginState = TELNET_UNAUTHENTICATED;
static WiFiServer * telnetServer; //= new WiFiServer(23);
static WiFiClient * telnetClient = new WiFiClient;
bool telnetInCommandMode         = false;
uint8_t telnetEnabled            = true; // Enable telnet debug output
uint8_t telnetLoginAttempt       = 0;    // Initial attempt
uint8_t telnetInputIndex         = 0;    // Empty buffer
char telnetInputBuffer[128];

bool telnetExitCommand()
{
    if(strcmp_P(telnetInputBuffer, PSTR("exit")) == 0 || telnetLoginAttempt >= 3) {
        Log.notice(F("TELNET: Closing session from %s"), telnetClient->remoteIP().toString().c_str());
        telnetClient->stop();
        Log.unregisterOutput(1); // telnetClient
        telnetLoginState   = TELNET_UNAUTHENTICATED;
        telnetInputIndex   = 0; // Empty buffer
        telnetLoginAttempt = 0; // Initial attempt
        return true;
    } else {
        return false;
    }
}

void telnetAcceptClient()
{
    if(telnetClient) {
        telnetClient->stop();    // client disconnected
        Log.unregisterOutput(1); // telnetClient
    }
    *telnetClient = telnetServer->available(); // ready for new client
    Log.notice(F("TELNET: Client connected from %s"), telnetClient->remoteIP().toString().c_str());

    /* Avoid a buffer here */
    telnetClient->print(0xFF); // DO TERMINAL-TYPE
    telnetClient->print(0xFD);
    telnetClient->print(0x1B);
    telnetClient->print(F("\r\nUsername: "));
    telnetLoginState   = TELNET_UNAUTHENTICATED;
    telnetInputIndex   = 0; // reset input buffer index
    telnetLoginAttempt = 0; // Initial attempt
}

void telnetProcessCommand(char ch)
{
    switch(ch) {
        case 255:
            if(telnetInCommandMode) {
                telnetInCommandMode = true;
            }
    }
}

void telnetProcessLine()
{
    telnetInputBuffer[telnetInputIndex] = 0; // null terminate our char array

    switch(telnetLoginState) {
        case TELNET_UNAUTHENTICATED: {
            telnetClient->printf(PSTR("Password: %c%c%c"), 0xFF, 0xFB, 0x01); // Hide characters
            telnetLoginState = strcmp(telnetInputBuffer, httpUser) == 0 ? TELNET_USERNAME_OK : TELNET_USERNAME_NOK;
            break;
        }
        case TELNET_USERNAME_OK:
        case TELNET_USERNAME_NOK: {
            telnetClient->printf(PSTR("%c%c%c\n"), 0xFF, 0xFC, 0x01); // Show characters
            if(telnetLoginState == TELNET_USERNAME_OK && strcmp(telnetInputBuffer, httpPassword) == 0) {
                telnetClient->println();
                telnetClient->println(debugHaspHeader().c_str()); // Send version header
                telnetLoginState   = TELNET_AUTHENTICATED;        // User and Pass are correct
                telnetLoginAttempt = 0;                           // Reset attempt counter
                Log.notice(F("TELNET: Client login from %s"),
                           telnetClient->remoteIP().toString().c_str()); // Serial Only

                /* Echo locally as separate string */
                telnetClient->print(F("TELNET: Client login from "));
                telnetClient->println(telnetClient->remoteIP().toString().c_str());

                /* Now register logger for telnet */
                Log.registerOutput(1, telnetClient, LOG_LEVEL_VERBOSE, true);
            } else {
                telnetLoginState = TELNET_UNAUTHENTICATED;
                telnetLoginAttempt++; // Subsequent attempt
                telnetClient->println(F("Authorization failed!\r\n"));
                Log.warning(F("TELNET: Incorrect login attempt from %s"), telnetClient->remoteIP().toString().c_str());
                if(telnetLoginAttempt >= 3) {
                    telnetExitCommand();
                } else {
                    telnetClient->print(F("Username: "));
                }
            }
            break;
        }
        default:
            if(telnetInputIndex > 0 && !telnetExitCommand()) {
                dispatchCommand(telnetInputBuffer);
            }
    }

    telnetInputIndex = 0; // reset input buffer index
}

void telnetProcessData(char ch)
{

    switch(ch) {
        case 0 ... 7:
        case 9:
        case 11 ... 12:
        case 14 ... 31:
        case 251 ... 255:
            break;
        case 10:
            telnetInputIndex = 0;
            break;
        case 8: // Backspace
            if(telnetInputIndex > 0) telnetInputIndex--;
            break;
        case 13:
            telnetProcessLine();
            break;
        default:
            if(!isprint(ch)) {
                telnetClient->printf(PSTR(" 0x%02x "), ch);
            }
            // If we have room left in our buffer add the current byte
            if(telnetInputIndex < sizeof(telnetInputBuffer) - 1) {
                telnetInputBuffer[telnetInputIndex++] = ch;
                // telnetInputIndex++;
            }
    }
}

void telnetProcessCharacter(char ch)
{
    // if(ch == (char)0xff || telnetInCommandMode) {
    //    telnetProcessCharacter(ch);
    //} else {
    telnetProcessData(ch);
    //}
}

void telnetSetup(const JsonObject & settings)
{
    // telnetSetConfig(settings);

    if(telnetEnabled) { // Setup telnet server for remote debug output
        if(!telnetServer) telnetServer = new WiFiServer(23);
        if(telnetServer) {
            telnetServer->setNoDelay(true);
            telnetServer->begin();

            if(!telnetClient) telnetClient = new WiFiClient;
            if(!telnetClient) {
                Log.error(F("TELNET: Failed to start telnet client"));
            } else {
                telnetClient->setNoDelay(true);
            }

            Log.notice(F("TELNET: Debug telnet console started"));
        } else {
            Log.error(F("TELNET: Failed to start telnet server"));
        }
    }
}

void IRAM_ATTR telnetLoop()
{ // Basic telnet client handling code from: https://gist.github.com/tablatronix/4793677ca748f5f584c95ec4a2b10303

    if(telnetServer && telnetServer->hasClient()) { // client is connected
        if(!*telnetClient || !telnetClient->connected()) {
            telnetAcceptClient();
        } else {
            telnetServer->available().stop(); // have client, block new connections
        }
    }

    // Handle client input from telnet connection.
    if(telnetClient && telnetClient->connected() && telnetClient->available()) {
        telnetProcessCharacter(telnetClient->read()); // client input processing
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void telnetPrintln(const char * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected() && telnetLoginState == TELNET_AUTHENTICATED) {
        telnetClient->println(msg);
    }
}
void telnetPrint(const char * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected() && telnetLoginState == TELNET_AUTHENTICATED) {
        telnetClient->print(msg);
    }
}
void telnetPrint(const __FlashStringHelper * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected() && telnetLoginState == TELNET_AUTHENTICATED) {
        telnetClient->print(msg);
    }
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool telnetGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_ENABLE)] = telnetEnabled;

    configOutput(settings);
    return true;
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
    configOutput(settings);
    bool changed = false;

    return changed;
}

#endif