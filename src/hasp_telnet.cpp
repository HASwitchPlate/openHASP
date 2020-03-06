#include "Arduino.h"
#include "ArduinoJson.h"

#include "hasp_conf.h"
#include "hasp_log.h"
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
static char telnetInputBuffer[128];

uint8_t telnetEnabled = true; // Enable telnet debug output
WiFiServer * telnetServer;    //(23);
WiFiClient * telnetClient;

void telnetSetup(const JsonObject & settings)
{
    telnetSetConfig(settings);

    if(telnetEnabled) { // Setup telnet server for remote debug output
        telnetServer = new WiFiServer(23);
        telnetClient = new WiFiClient;
        if(telnetServer) {
            telnetServer->setNoDelay(true);
            telnetServer->begin();
            debugPrintln(String(F("TELNET: debug server enabled at telnet:")) + WiFi.localIP().toString());
        } else {
            errorPrintln(F("TELNET: %sFailed to create telnet server"));
        }
    }
}

void telnetLoop(bool isConnected)
{ // Basic telnet client handling code from: https://gist.github.com/tablatronix/4793677ca748f5f584c95ec4a2b10303
    if(!isConnected) return;

    static unsigned long telnetInputIndex = 0;
    if(telnetServer && telnetServer->hasClient()) { // client is connected
        if(!*telnetClient || !telnetClient->connected()) {
            if(telnetClient) {
                telnetClient->stop(); // client disconnected
            }
            *telnetClient    = telnetServer->available(); // ready for new client
            telnetInputIndex = 0;                         // reset input buffer index
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
                case 0x3:
                case 0x5:
                case 0xff:
                case 0xf1:
                    telnetInputIndex = 0;
                    break;
                case 10:
                case 13:
                    telnetInputBuffer[telnetInputIndex] = 0; // null terminate our char array
                    if(telnetInputIndex > 0) dispatchCommand(telnetInputBuffer);
                    telnetInputIndex = 0;
                    break;

                default:
                    if(telnetInputIndex <
                       sizeof(telnetInputBuffer)) { // If we have room left in our buffer add the current byte
                        telnetInputBuffer[telnetInputIndex] = telnetInputByte;
                        telnetInputIndex++;
                    }
            }
        }
    }
}

void telnetPrintln(const char * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected()) {
        telnetClient->println(msg);
    }
}
void telnetPrint(const char * msg)
{
    if(telnetEnabled && telnetClient && telnetClient->connected()) {
        telnetClient->print(msg);
    }
}

bool telnetGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_ENABLE)] = telnetEnabled;

    configOutput(settings);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool telnetSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    return changed;
}

#endif