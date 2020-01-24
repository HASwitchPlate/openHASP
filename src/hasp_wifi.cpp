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

#include "user_config_override.h"

std::string wifiSsid     = WIFI_SSID;
std::string wifiPassword = WIFI_PASSW;

// long wifiPrevMillis         = 0;
// bool wifiWasConnected       = false;
// int8_t wifiReconnectAttempt = -20;

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

    if(!settings[F_CONFIG_SSID].isNull()) {
        wifiSsid = settings[F_CONFIG_SSID].as<String>().c_str();

        // sprintf_P(buffer, PSTR("Wifi Ssid: %s"), wifiSsid.c_str());
        // debugPrintln(buffer);
    }
    if(!settings[F_CONFIG_PASS].isNull()) {
        wifiPassword = settings[F_CONFIG_PASS].as<String>().c_str();

        // sprintf_P(buffer, PSTR("Wifi Password: %s"), wifiPassword.c_str());
        // debugPrintln(buffer);
    }

    sprintf_P(buffer, PSTR("WIFI: Connecting to : %s"), wifiSsid.c_str());
    debugPrintln(buffer);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());

#if defined(ARDUINO_ARCH_ESP32)
    WiFi.onEvent(wifi_callback);
#endif
#if defined(ARDUINO_ARCH_ESP8266)
    wifiEventHandler[0] = WiFi.onStationModeGotIP(wifiSTAGotIP); // As soon WiFi is connected, start NTP Client
    wifiEventHandler[1] = WiFi.onStationModeDisconnected(wifiSTADisconnected);
    wifiEventHandler[2] = WiFi.onStationModeConnected(wifiSTAConnected);
#endif
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
    if(!settings.isNull() && settings[F_CONFIG_SSID] == String(wifiSsid.c_str()) &&
       settings[F_CONFIG_PASS] == String(wifiPassword.c_str()))
        return false;

    settings[F_CONFIG_SSID] = String(wifiSsid.c_str());
    settings[F_CONFIG_PASS] = String(wifiPassword.c_str());

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}