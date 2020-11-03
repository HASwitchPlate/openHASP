#include "hasp_conf.h"
#if HASP_USE_TELNET > 0

#include "Arduino.h"
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_telnet.h"
#include "hasp_conf.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
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

#define TELNET_UNAUTHENTICATED 0
#define TELNET_USERNAME_OK 10
#define TELNET_USERNAME_NOK 99
#define TELNET_AUTHENTICATED 255

#if HASP_USE_HTTP > 0
extern char httpUser[32];
extern char httpPassword[32];
#endif

uint8_t telnetLoginState = TELNET_UNAUTHENTICATED;
// WiFiClient telnetClient;
// static WiFiServer * telnetServer;
uint16_t telnetPort        = 23;
bool telnetInCommandMode   = false;
uint8_t telnetEnabled      = true; // Enable telnet debug output
uint8_t telnetLoginAttempt = 0;    // Initial attempt
uint8_t telnetInputIndex   = 0;    // Empty buffer
char telnetInputBuffer[128];

void telnetClientDisconnect()
{
    // Log.notice(F("Closing session from %s"), telnetClient.remoteIP().toString().c_str());
    telnetClient.stop();
    Log.unregisterOutput(1); // telnetClient
    telnetLoginState   = TELNET_UNAUTHENTICATED;
    telnetInputIndex   = 0; // Empty buffer
    telnetLoginAttempt = 0; // Initial attempt
}

void telnetClientLogon()
{
    telnetClient.println();
    telnetClient.println(debugHaspHeader().c_str()); // Send version header
    telnetLoginState   = TELNET_AUTHENTICATED;       // User and Pass are correct
    telnetLoginAttempt = 0;                          // Reset attempt counter
    Log.registerOutput(1, &telnetClient, LOG_LEVEL_VERBOSE, true);
    // Log.notice(F("Client login from %s"), telnetClient.remoteIP().toString().c_str());
    telnetClient.flush();
    /* Echo locally as separate string */
    // telnetClient.print(F("TELNET: Client login from "));
    // telnetClient.println(telnetClient.remoteIP().toString().c_str());

    /* Now register logger for telnet */
}

void telnetAcceptClient()
{
    if(telnetClient) {
        telnetClient.stop();     // client disconnected
        Log.unregisterOutput(1); // telnetClient
    }
    telnetClient = telnetServer->available(); // ready for new client
    // Log.notice(F("Client connected from %s"), telnetClient.remoteIP().toString().c_str());
    if(!telnetClient) {
        Log.notice(F("Client NOT connected"));
        return;
    }
    Log.notice(F("Client connected"));

    /* Avoid a buffer here */
    telnetClient.print(0xFF); // DO TERMINAL-TYPE
    telnetClient.print(0xFD);
    telnetClient.print(0x1B);
#if HASP_USE_HTTP > 0
    if(strlen(httpUser) != 0 || strlen(httpPassword) != 0) {
        telnetClient.print(F("\r\nUsername: "));
        telnetLoginState = TELNET_UNAUTHENTICATED;
    } else
#endif
    {
        telnetClientLogon();
    }
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

static void telnetProcessLine()
{
    telnetInputBuffer[telnetInputIndex] = 0; // null terminate our char array

    switch(telnetLoginState) {
        case TELNET_UNAUTHENTICATED: {
            telnetClient.printf(PSTR("Password: %c%c%c"), 0xFF, 0xFB, 0x01); // Hide characters
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
                telnetClient.println(F("Authorization failed!\r\n"));
                // Log.warning(F("Incorrect login attempt from %s"), telnetClient.remoteIP().toString().c_str());
                if(telnetLoginAttempt >= 3) {
                    telnetClientDisconnect();
                } else {
                    telnetClient.print(F("Username: "));
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
                    dispatchCommand(telnetInputBuffer);
                }
            }
    }

    telnetInputIndex = 0; // reset input buffer index
}

static void telnetProcessData(char ch)
{

    switch(ch) {
        case 10:
            telnetInputIndex = 0;
            break;
        case 8: // Backspace
            if(telnetInputIndex > 0) telnetInputIndex--;
            break;
        case 13:
            telnetProcessLine();
            break;
        case 32 ... 250:
            if(!isprint(ch)) {
                telnetClient.printf(PSTR(" 0x%02x "), ch);
            }
            // If we have room left in our buffer add the current byte
            if(telnetInputIndex < sizeof(telnetInputBuffer) - 1) {
                telnetInputBuffer[telnetInputIndex++] = ch;
                // telnetInputIndex++;
            }
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

void telnetSetup()
{
    // telnetSetConfig(settings);

    if(telnetEnabled) { // Setup telnet server for remote debug output
#if defined(STM32F4xx)
        // if(!telnetServer) telnetServer = new EthernetServer(telnetPort);
        // if(telnetServer) {
        telnetServer->begin();
        Log.notice(F("Debug telnet console started"));
        // } else {
        //    Log.error(F("Failed to start telnet server"));
        //}
#else
        if(!telnetServer) telnetServer = new WiFiServer(telnetPort);
        if(telnetServer) {
            telnetServer->setNoDelay(true);
            telnetServer->begin();

            // if(!telnetClient) telnetClient = new WiFiClient;
            // if(!telnetClient) {
            //    Log.error(F("Failed to start telnet client"));
            //} else {
            telnetClient.setNoDelay(true);
            //}

            Log.notice(F("Debug telnet console started"));
        } else {
            Log.error(F("Failed to start telnet server"));
        }
#endif
    }
}

void IRAM_ATTR telnetLoop()
{ // Basic telnet client handling code from: https://gist.github.com/tablatronix/4793677ca748f5f584c95ec4a2b10303

#if defined(STM32F4xx)
Ethernet.schedule();
  // if(telnetServer)
    { // client is connected
        EthernetClient client = telnetServer->available();
        if(client) {
            if(!telnetClient || !telnetClient.connected()) {
                //telnetAcceptClient(client);

                telnetClient = client; // ready for new client
                // Log.notice(F("Client connected from %s"), telnetClient.remoteIP().toString().c_str());
                if(!telnetClient) {
                    Log.notice(F("Client NOT connected"));
                    return;
                }
                Log.notice(F("Client connected"));

                /* Avoid a buffer here */
                // telnetClient.print(0xFF); // DO TERMINAL-TYPE
                // telnetClient.print(0xFD);
                // telnetClient.print(0x1B);

            } else {
                //client.stop(); // have client, block new connections
            }
        }
    }
#else
    if(telnetServer && telnetServer->hasClient()) { // client is connected
        if(!telnetClient || !telnetClient.connected()) {
            telnetAcceptClient();
        } else {
            telnetServer->available().stop(); // have client, block new connections
        }
    }

    // Handle client input from telnet connection.
    if(telnetClient && telnetClient.connected()) {
        while(telnetClient.available()) {
            telnetProcessCharacter(telnetClient.read()); // client input processing
        }
    }
#endif
}

bool telnetGetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(telnetEnabled != settings[FPSTR(F_CONFIG_ENABLE)].as<bool>()) changed = true;
    settings[FPSTR(F_CONFIG_ENABLE)] = telnetEnabled;

    if(telnetPort != settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(F_CONFIG_PORT)] = telnetPort;

    if(changed) configOutput(settings);
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
    configOutput(settings);
    bool changed = false;

    changed |= configSet(telnetEnabled, settings[FPSTR(F_CONFIG_ENABLE)], PSTR("telnetEnabled"));
    changed |= configSet(telnetPort, settings[FPSTR(F_CONFIG_PORT)], PSTR("telnetPort"));

    return changed;
}

#endif