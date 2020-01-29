#include <Arduino.h>
#include "ArduinoJson.h"

#include "hasp_conf.h"

#include "hasp_wifi.h"
#include "hasp_mqtt.h"
#include "hasp_http.h"
#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>

static WiFiEventHandler wifiEventHandler[3];

#endif
#include "DNSserver.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#ifdef WIFI_SSID
std::string wifiSsid = WIFI_SSID;
#else
std::string wifiSsid     = "";
#endif
#ifdef WIFI_PASSW
std::string wifiPassword = WIFI_PASSW;
#else
std::string wifiPassword = "";
#endif

const byte DNS_PORT = 53;
DNSServer dnsServer;

// long wifiPrevMillis         = 0;
// bool wifiWasConnected       = false;
// int8_t wifiReconnectAttempt = -20;

String wifiGetMacAddress(int start, const char * seperator)
{
    byte mac[6];
    WiFi.macAddress(mac);
    String cMac = "";
    for(int i = start; i < 6; ++i) {
        if(mac[i] < 0x10) cMac += "0";
        cMac += String(mac[i], HEX);
        if(i < 5) cMac += seperator;
    }
    cMac.toUpperCase();
    return cMac;
}

void wifiConnected(IPAddress ipaddress)
{
    char buffer[64];
    sprintf_P(buffer, PSTR("WIFI: Received IP address %s"), ipaddress.toString().c_str());
    debugPrintln(buffer);
    sprintf_P(buffer, PSTR("WIFI: Connected = %s"), WiFi.status() == WL_CONNECTED ? PSTR("yes") : PSTR("no"));
    debugPrintln(buffer);

    httpReconnect();
    // mqttReconnect();
    haspReconnect();
}

void wifiDisconnected(const char * ssid, uint8_t reason)
{
    char buffer[64];
    sprintf_P(buffer, PSTR("WIFI: Disconnected from %s (Reason: %d)"), ssid, reason);
    debugPrintln(buffer);
    WiFi.reconnect();
}

void wifiSsidConnected(const char * ssid)
{
    char buffer[64];
    sprintf_P(buffer, PSTR("WIFI: Connected to SSID %s. Requesting IP..."), ssid);
    debugPrintln(buffer);
}

#if defined(ARDUINO_ARCH_ESP32)
void wifi_callback(system_event_id_t event, system_event_info_t info)
{
    switch(event) {
        case SYSTEM_EVENT_STA_CONNECTED:
            wifiSsidConnected((const char *)info.connected.ssid);
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            wifiConnected(IPAddress(info.got_ip.ip_info.ip.addr));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            wifiDisconnected((const char *)info.disconnected.ssid, info.disconnected.reason);
            // NTP.stop(); // NTP sync can be disabled to avoid sync errors
            break;
        default:
            break;
    }
}
#endif

#if defined(ARDUINO_ARCH_ESP8266)
void wifiSTAConnected(WiFiEventStationModeConnected info)
{
    wifiSsidConnected(info.ssid.c_str());
}

// Start NTP only after IP network is connected
void wifiSTAGotIP(WiFiEventStationModeGotIP info)
{
    wifiConnected(IPAddress(info.ip));
}

// Manage network disconnection
void wifiSTADisconnected(WiFiEventStationModeDisconnected info)
{
    wifiDisconnected(info.ssid.c_str(), info.reason);
}
#endif

void wifiSetup(JsonObject settings)
{
    char buffer[64];

    wifiSetConfig(settings);

    if(wifiSsid == "") {
        String apSsdid = F("HASP-");
        apSsdid += wifiGetMacAddress(3, "");

        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSsdid.c_str(), "haspadmin");
        IPAddress IP = WiFi.softAPIP();

        /* Setup the DNS server redirecting all the domains to the apIP */
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", IP);

        sprintf_P(buffer, PSTR("WIFI: Setting up temporary Access Point: %s"), apSsdid.c_str());
        debugPrintln(buffer);
        sprintf_P(buffer, PSTR("WIFI: AP IP address : %s"), IP.toString().c_str());
        debugPrintln(buffer);
        haspDisplayAP(apSsdid.c_str(), "haspadmin");
        httpReconnect();
        return;
    }

    WiFi.mode(WIFI_STA);
    sprintf_P(buffer, PSTR("WIFI: Connecting to : %s"), wifiSsid.c_str());
    debugPrintln(buffer);

#if defined(ARDUINO_ARCH_ESP8266)
    wifiEventHandler[0] = WiFi.onStationModeGotIP(wifiSTAGotIP); // As soon WiFi is connected, start NTP Client
    wifiEventHandler[1] = WiFi.onStationModeDisconnected(wifiSTADisconnected);
    wifiEventHandler[2] = WiFi.onStationModeConnected(wifiSTAConnected);
#endif
#if defined(ARDUINO_ARCH_ESP32)
    WiFi.onEvent(wifi_callback);
#endif

    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
}

bool wifiLoop()
{
    return WiFi.status() == WL_CONNECTED;

    /*
      if(WiFi.status() == WL_CONNECTED) {
          if(wifiWasConnected) return true;

          debugPrintln(F("WIFI: Reconnected"));
          wifiWasConnected     = true;
          wifiReconnectAttempt = 1;
          wifiPrevMillis       = millis();
          haspOnline();
          return true;

      } else if(millis() - wifiPrevMillis > 1000) {
          if(wifiReconnectAttempt < 20) {
              if(wifiReconnectAttempt == 1) { // <0 means we were never connected yet
                                              // haspOffline();
                  warningPrintln(String(F("WIFI: %sConnection lost. Reconnecting... #")) +
      String(wifiReconnectAttempt)); WiFi.reconnect(); } else { debugPrintln(F("WIFI: Waiting for connection..."));
              }
          } else {
              // haspOffline();
              debugPrintln(F("WIFI: Connection lost. Reconnecting..."));
              WiFi.reconnect();
          }
          wifiReconnectAttempt++;
          wifiPrevMillis = millis();
      }
      return false;*/
}

bool wifiGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_SSID)] = String(wifiSsid.c_str());
    settings[FPSTR(F_CONFIG_PASS)] = String(wifiPassword.c_str());

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool wifiSetConfig(const JsonObject & settings)
{
    /*    if(!settings.isNull() && settings[FPSTR(F_CONFIG_STARTPAGE)] == haspStartPage &&
           settings[FPSTR(F_CONFIG_THEME)] == haspThemeId && settings[FPSTR(F_CONFIG_HUE)] == haspThemeHue &&
           settings[FPSTR(F_CONFIG_ZIFONT)] == haspZiFontPath && settings[FPSTR(F_CONFIG_PAGES)] == haspPagesPath)
            return false;
    */
    bool changed = false;

    if(!settings[FPSTR(F_CONFIG_SSID)].isNull()) {
        if(wifiSsid != settings[FPSTR(F_CONFIG_SSID)].as<String>().c_str()) {
            debugPrintln(F("wifiSsid changed"));
        }
        changed |= wifiSsid != settings[FPSTR(F_CONFIG_SSID)].as<String>().c_str();

        wifiSsid = settings[FPSTR(F_CONFIG_SSID)].as<String>().c_str();
    }

    if(!settings[FPSTR(F_CONFIG_PASS)].isNull() && settings[FPSTR(F_CONFIG_PASS)].as<String>() != F("********")) {
        if(wifiPassword != settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str()) {
            debugPrintln(F("wifiPassword changed"));
        }
        changed |= wifiPassword != settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str();

        wifiPassword = settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str();
    }

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return changed;
}

void wifiStop()
{
    debugPrintln(F("WIFI: Stopped"));
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect();
}