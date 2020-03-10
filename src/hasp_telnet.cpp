#include "Arduino.h"
#include "ArduinoJson.h"

#include "hasp_conf.h"
#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_telnet.h"

#if HASP_USE_TELNET > 0

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>
#endif

//#define telnetInputMax 128; // Size of user input buffer for user telnet session
WiFiServer * telnetServer; //(23);
WiFiClient * telnetClient;
uint8_t telnetEnabled      = true; // Enable telnet debug output
uint8_t telnetLoginAttempt = 0;    // Initial attempt
uint8_t telnetLoginState   = 0;    // Unauthenticated
uint8_t telnetInputIndex   = 0;    // Empty buffer
char telnetInputBuffer[128];

bool telnetExitCommand()
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("exit"));
    if(strcmp(telnetInputBuffer, buffer) == 0 || telnetLoginAttempt >= 3) {
        snprintf_P(buffer, sizeof(buffer), PSTR("TELNET: Closing session from %s"),
                   telnetClient->remoteIP().toString().c_str());
        debugPrintln(buffer);
        telnetClient->stop();
        telnetLoginState   = 0; // Unauthenticated
        telnetInputIndex   = 0; // Empty buffer
        telnetLoginAttempt = 0; // Initial attempt
        return true;
    } else {
        return false;
    }
}

void telnetSetup(const JsonObject & settings)
{
    telnetSetConfig(settings);

    if(telnetEnabled) { // Setup telnet server for remote debug output
        telnetServer = new WiFiServer(23);
        if(telnetServer) {
            telnetClient = new WiFiClient;
            telnetServer->setNoDelay(true);
            telnetServer->begin();
            debugPrintln(String(F("TELNET: Debug console enabled at telnet://")) + WiFi.localIP().toString());
        } else {
            errorPrintln(F("TELNET: %sFailed to start telnet server"));
        }
    }
}

void telnetLoop()
{ // Basic telnet client handling code from: https://gist.github.com/tablatronix/4793677ca748f5f584c95ec4a2b10303

    if(telnetServer && telnetServer->hasClient()) { // client is connected
        if(!*telnetClient || !telnetClient->connected()) {
            if(telnetClient) {
                telnetClient->stop(); // client disconnected
            }
            *telnetClient = telnetServer->available(); // ready for new client
            char buffer[128];
            snprintf_P(buffer, sizeof(buffer), PSTR("TELNET: Client connected from %s"),
                       telnetClient->remoteIP().toString().c_str());
            debugPrintln(buffer);
            snprintf_P(buffer, sizeof(buffer), PSTR("\r\nUsername: "));
            telnetClient->print(buffer);
            telnetLoginState   = 0; // Unauthenticated
            telnetInputIndex   = 0; // reset input buffer index
            telnetLoginAttempt = 0; // Initial attempt
        } else {
            telnetServer->available().stop(); // have client, block new connections
        }
    }

    // Handle client input from telnet connection.
    if(telnetClient && telnetClient->connected() && telnetClient->available()) { // client input processing

        if(telnetClient->available()) {
            char telnetInputByte = telnetClient->read(); // Read client byte
            // debugPrintln(String("telnet in: 0x") + String(telnetInputByte, HEX));
            switch(telnetInputByte) {
                case 0x01:
                case 0x03:
                case 0x05:
                case 0xff:
                case 0xfe:
                case 0xfd:
                case 0xfc:
                case 0xfb:
                case 0xf1:
                case 0x1f:
                case 10:
                    telnetInputIndex = 0;
                    break;
                case 0x08: // Backspace
                    if(telnetInputIndex > 0) {
                        telnetInputIndex--;
                    }
                    break;
                case 13:
                    telnetInputBuffer[telnetInputIndex] = 0; // null terminate our char array
                    switch(telnetLoginState) {
                        case 0: {
                            char buffer[128];
                            snprintf_P(buffer, sizeof(buffer), PSTR("Password: %c%c%c"), 0xFF, 0xFB, 0x01);
                            telnetClient->print(buffer);
                            snprintf_P(buffer, sizeof(buffer), PSTR("admin"));
                            telnetLoginState =
                                strcmp(telnetInputBuffer, buffer) == 0 ? 10 : 99; // Username OK=1 or NOK=99
                            break;
                        }
                        case 10:
                        case 99: {
                            char buffer[128];
                            snprintf_P(buffer, sizeof(buffer), PSTR("%c%c%c"), 0xFF, 0xFC, 0x01);
                            telnetClient->println(buffer);
                            snprintf_P(buffer, sizeof(buffer), PSTR("haspadmin"));
                            if(telnetLoginState == 10 && strcmp(telnetInputBuffer, buffer) == 0) { // Authenticated
                                telnetLoginState   = 255;                                          // Authenticated
                                telnetLoginAttempt = 0;                                            // Initial attempt
                                telnetClient->println(debugHaspHeader());
                                snprintf_P(buffer, sizeof(buffer), PSTR("TELNET: Client login from %s"),
                                           telnetClient->remoteIP().toString().c_str());
                                debugPrintln(buffer);
                            } else {
                                telnetLoginState = 0; // Unauthorized
                                telnetLoginAttempt++; // Subsequent attempt
                                snprintf_P(buffer, sizeof(buffer), PSTR("Authorization failed!\r\n"));
                                telnetClient->println(buffer);
                                snprintf_P(buffer, sizeof(buffer), PSTR("TELNET: %%Incorrect login attempt from %s"),
                                           telnetClient->remoteIP().toString().c_str());
                                errorPrintln(buffer);
                                if(telnetLoginAttempt >= 3) {
                                    telnetExitCommand();
                                } else {
                                    snprintf_P(buffer, sizeof(buffer), PSTR("Username: "));
                                    telnetClient->print(buffer);
                                }
                            }
                            break;
                        }
                        default:
                            if(telnetInputIndex > 0 && !telnetExitCommand()) {
                                dispatchCommand(telnetInputBuffer);
                            }
                    }
                    telnetInputIndex = 0;
                    break;
                default:
                    // If we have room left in our buffer add the current byte
                    if(telnetInputIndex < sizeof(telnetInputBuffer) - 1 && telnetInputByte >= 0x20) {
                        telnetInputBuffer[telnetInputIndex] = telnetInputByte;
                        telnetInputIndex++;
                    }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void telnetPrintln(const char * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected() && telnetLoginState == 255) {
        telnetClient->println(msg);
    }
}
void telnetPrint(const char * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected() && telnetLoginState == 255) {
        telnetClient->print(msg);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool telnetGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_ENABLE)] = telnetEnabled;

    configOutput(settings);
    return true;
}

bool telnetSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    return changed;
}

#endif