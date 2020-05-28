#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_conf.h"

#if HASP_USE_WIFI > 0

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_gui.h"
#include "hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Wifi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>

static WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

#elif defined(STM32F4xx)
// #include <WiFi.h>
// #include "WiFiSpi.h"
// extern WiFiSpiClass WiFi;
SPIClass spi2(ESPSPI_MOSI, ESPSPI_MISO, ESPSPI_SCLK);    // SPI port where esp is connected

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
uint8_t wifiReconnectCounter = 0;

// const byte DNS_PORT = 53;
// DNSServer dnsServer;

void wifiConnected(IPAddress ipaddress)
{
#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    Log.notice(F("WIFI: Received IP address %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
#else
    Log.notice(F("WIFI: Received IP address %s"), ipaddress.toString().c_str());
#endif
    Log.verbose(F("WIFI: Connected = %s"), WiFi.status() == WL_CONNECTED ? PSTR("yes") : PSTR("no"));
    haspProgressVal(255);

    // if(isConnected) {
    // mqttReconnect();
    // haspReconnect();
    // httpReconnect();
    // mdnsStart();
    //}
}

void wifiDisconnected(const char * ssid, uint8_t reason)
{
    wifiReconnectCounter++;
    haspProgressVal(wifiReconnectCounter * 3);
    haspProgressMsg(F("Wifi Disconnected"));
    if(wifiReconnectCounter > 33) {
        Log.error(F("WIFI: Retries exceed %u: Rebooting..."), wifiReconnectCounter);
        dispatchReboot(false);
    }
    Log.warning(F("WIFI: Disconnected from %s (Reason: %d)"), ssid, reason);
}

void wifiSsidConnected(const char * ssid)
{
    Log.notice(F("WIFI: Connected to SSID %s. Requesting IP..."), ssid);
    wifiReconnectCounter = 0;
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

bool wifiShowAP()
{
    return (strlen(wifiSsid) == 0);
}

bool wifiShowAP(char * ssid, char * pass)
{
    if(strlen(wifiSsid) != 0) return false;

    byte mac[6];
    WiFi.macAddress(mac);
    sprintf_P(ssid, PSTR("HASP-%02x%02x%02x"), mac[3], mac[4], mac[5]);
    sprintf_P(pass, PSTR("haspadmin"));
#if defined(STM32F4xx)
    Log.warning(F("WIFI: We should setup Temporary Access Point %s password: %s"), ssid, pass);
#else
    WiFi.softAP(ssid, pass);

    /* Setup the DNS server redirecting all the domains to the apIP */
    // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    // dnsServer.start(DNS_PORT, "*", apIP);

    Log.warning(F("WIFI: Temporary Access Point %s password: %s"), ssid, pass);
    Log.warning(F("WIFI: AP IP address : %s"), WiFi.softAPIP().toString().c_str());
    // httpReconnect();}
#endif
    return true;
}

void wifiSetup()
{
#if defined(STM32F4xx)
    // Temp ESP reset function
    pinMode(ESPSPI_RST, OUTPUT);
    digitalWrite(ESPSPI_RST, 0);
    delay(150);
    digitalWrite(ESPSPI_RST, 1);
    delay(150);
    //

    // Initialize the WifiSpi library
    WiFiSpi.init(ESPSPI_CS, 8000000, &spi2);

    // check for the presence of the shield:
    if (WiFiSpi.status() == WL_NO_SHIELD) {
        Log.notice(F("WIFI: WiFi shield not present"));
        // don't continue:
        while (true);
    }

    if (!WiFiSpi.checkProtocolVersion()) {
        Log.notice(F("WIFI: Protocol version mismatch. Please upgrade the firmware"));
        // don't continue:
        while (true);
    }

    // attempt to connect to Wifi network
    int status = WL_IDLE_STATUS;     // the Wifi radio's status

    // while (status != WL_CONNECTED) {
        Log.notice(F("WIFI: Connecting to : %s"), wifiSsid);
        // Connect to WPA/WPA2 network
        status = WiFi.begin(wifiSsid, wifiPassword);
    // }

#else
    if(wifiShowAP()) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        WiFi.mode(WIFI_STA);

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
        Log.notice(F("WIFI: Connecting to : %s"), wifiSsid);
    }
#endif
}

bool wifiEvery5Seconds()
{
#if defined(STM32F4xx)
    if(WiFi.status() == WL_CONNECTED) {
#else
    if(WiFi.getMode() != WIFI_STA) {
        return false;
    } else if(WiFi.status() == WL_CONNECTED) {
#endif
        return true;
    } else {
        wifiReconnectCounter++;
        if(wifiReconnectCounter > 45) {
            Log.error(F("WIFI: Retries exceed %u: Rebooting..."), wifiReconnectCounter);
            dispatchReboot(false);
        }
        Log.warning(F("WIFI: No Connection... retry %u"), wifiReconnectCounter);
        if(wifiReconnectCounter % 6 == 0) WiFi.begin(wifiSsid, wifiPassword);
        return false;
    }
}

bool wifiGetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(strcmp(wifiSsid, settings[FPSTR(F_CONFIG_SSID)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_SSID)] = wifiSsid;

    if(strcmp(wifiPassword, settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_PASS)] = wifiPassword;

    if(changed) configOutput(settings);
    return changed;
}

/** Set WIFI Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
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

bool wifiTestConnection()
{
    uint8_t attempt = 0;
    WiFi.begin(wifiSsid, wifiPassword);
#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    char espIp[16];
    memset(espIp, 0 ,sizeof(espIp));
    snprintf_P(espIp, sizeof(espIp), PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    while(attempt < 10 && (WiFi.status() != WL_CONNECTED || String(espIp) == F("0.0.0.0"))) {
#else
    while(attempt < 10 && (WiFi.status() != WL_CONNECTED || WiFi.localIP().toString() == F("0.0.0.0"))) {
#endif
        attempt++;
        Log.verbose(F("WIFI: Trying to connect to %s... %u"), wifiSsid, attempt);
        delay(1000);
    }
#if defined(STM32F4xx)
    Log.verbose(F("WIFI: Received IP addres %s"), espIp);
    if((WiFi.status() == WL_CONNECTED && String(espIp) != F("0.0.0.0"))) return true;
#else
    Log.verbose(F("WIFI: Received IP addres %s"), WiFi.localIP().toString().c_str());
    if((WiFi.status() == WL_CONNECTED && WiFi.localIP().toString() != F("0.0.0.0"))) return true;
#endif
    WiFi.disconnect();
    return false;
}

void wifiStop()
{
    wifiReconnectCounter = 0; // Prevent endless loop in wifiDisconnected
    WiFi.disconnect();
#if !defined(STM32F4xx)
    WiFi.mode(WIFI_OFF);
#endif
    Log.warning(F("WIFI: Stopped"));
}

#endif