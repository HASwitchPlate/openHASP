#include <Arduino.h>
#include "ArduinoJson.h"

#include "hasp_conf.h"

#include "hasp_wifi.h"
#include "hasp_mqtt.h"
#include "hasp_http.h"
#include "hasp_mdns.h"
#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_gui.h"
#include "hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#else
#include <ESP8266WiFi.h>

static WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

#endif
//#include "DNSserver.h"

#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h"
#endif

#ifdef WIFI_SSID
char wifiSsid[32] = WIFI_SSID;
#else
char wifiSsid[32]     = "";
#endif
#ifdef WIFI_PASSW
char wifiPassword[32] = WIFI_PASSW;
#else
char wifiPassword[32] = "";
#endif

// const byte DNS_PORT = 53;
// DNSServer dnsServer;

String wifiGetMacAddress(int start, const char * seperator)
{
    byte mac[6];
    WiFi.macAddress(mac);
    String cMac((char *)0);
    cMac.reserve(128);

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
    bool isConnected = WiFi.status() == WL_CONNECTED;
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI: Received IP address %s"), ipaddress.toString().c_str());
    debugPrintln(buffer);
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI: Connected = %s"), isConnected ? PSTR("yes") : PSTR("no"));
    debugPrintln(buffer);

    if(isConnected) {
        mqttReconnect();
        haspReconnect();
        httpReconnect();
        mdnsStart();
    }
}

void wifiDisconnected(const char * ssid, uint8_t reason)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI: Disconnected from %s (Reason: %d)"), ssid, reason);
    debugPrintln(buffer);
    WiFi.reconnect();
}

void wifiSsidConnected(const char * ssid)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("WIFI: Connected to SSID %s. Requesting IP..."), ssid);
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
    char buffer[128];

    wifiSetConfig(settings);

    if(strlen(wifiSsid) == 0) {
        String apSsdid = F("HASP-");
        apSsdid += wifiGetMacAddress(3, "");
        snprintf_P(buffer, sizeof(buffer), PSTR("haspadmin"));

        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSsdid.c_str(), buffer);
        haspDisplayAP(apSsdid.c_str(), buffer);

        /* Setup the DNS server redirecting all the domains to the apIP */
        // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        // dnsServer.start(DNS_PORT, "*", apIP);

        IPAddress IP = WiFi.softAPIP();
        sprintf_P(buffer, PSTR("WIFI: Temporary Access Point %s password: %s"), apSsdid.c_str(), PSTR("haspadmin"));
        debugPrintln(buffer);
        sprintf_P(buffer, PSTR("WIFI: AP IP address : %s"), IP.toString().c_str());
        debugPrintln(buffer);
        httpReconnect();
    } else {

        WiFi.mode(WIFI_STA);
        snprintf_P(buffer, sizeof(buffer), PSTR("WIFI: Connecting to : %s"), wifiSsid);
        debugPrintln(buffer);

#if defined(ARDUINO_ARCH_ESP8266)
        // wifiEventHandler[0]      = WiFi.onStationModeConnected(wifiSTAConnected);
        gotIpEventHandler        = WiFi.onStationModeGotIP(wifiSTAGotIP); // As soon WiFi is connected, start NTP Client
        disconnectedEventHandler = WiFi.onStationModeDisconnected(wifiSTADisconnected);
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
#endif
#if defined(ARDUINO_ARCH_ESP32)
        WiFi.onEvent(wifi_callback);
        WiFi.setSleep(false);
#endif

        WiFi.begin(wifiSsid, wifiPassword);
    }
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
    settings[FPSTR(F_CONFIG_SSID)] = wifiSsid;     // String(wifiSsid.c_str());
    settings[FPSTR(F_CONFIG_PASS)] = wifiPassword; // String(wifiPassword.c_str());

    configOutput(settings);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool wifiSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    if(!settings[FPSTR(F_CONFIG_SSID)].isNull()) {
        changed |= strcmp(wifiSsid, settings[FPSTR(F_CONFIG_SSID)]) != 0;
        strncpy(wifiSsid, settings[FPSTR(F_CONFIG_SSID)], sizeof(wifiSsid));
    }

    if(!settings[FPSTR(F_CONFIG_PASS)].isNull() &&
       settings[FPSTR(F_CONFIG_PASS)].as<String>() != String(FPSTR("********"))) {
        changed |= strcmp(wifiPassword, settings[FPSTR(F_CONFIG_PASS)]) != 0;
        strncpy(wifiPassword, settings[FPSTR(F_CONFIG_PASS)], sizeof(wifiPassword));
    }

    return changed;
}

void wifiStop()
{
    debugPrintln(F("WIFI: Stopped"));
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect();
}