/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_conf.h"

#if HASP_USE_WIFI > 0

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp_network.h"
#include "hasp_gui.h"
#include "hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include "user_interface.h" // Wifi Reasons

static WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

#elif defined(STM32F4xx)
// #include <WiFi.h>
// #include "WiFiSpi.h"
// extern WiFiSpiClass WiFi;
SPIClass espSPI(ESPSPI_MOSI, ESPSPI_MISO, ESPSPI_SCLK); // SPI port where esp is connected

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

/* ============ Connection Event Handlers =============================================================== */

static void wifiConnected(IPAddress ipaddress)
{
#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    Log.notice(TAG_WIFI, F("Received IP address %d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
#else
    Log.notice(TAG_WIFI, F("Received IP address %s"), ipaddress.toString().c_str());
#endif

    Log.verbose(TAG_WIFI, F("Connected = %s"), WiFi.status() == WL_CONNECTED ? PSTR("yes") : PSTR("no"));
    networkEvent(true);
}

static void wifiDisconnected(const char * ssid, uint8_t reason)
{
    wifiReconnectCounter++;

    haspProgressVal(wifiReconnectCounter * 3);
    networkEvent(false);

    if(wifiReconnectCounter > 33) {
        Log.error(TAG_WIFI, F("Retries exceed %u: Rebooting..."), wifiReconnectCounter);
        dispatchReboot(false);
    }

    char buffer[64];

    switch(reason) {
#if defined(ARDUINO_ARCH_ESP8266)
        case REASON_UNSPECIFIED:
            snprintf_P(buffer, sizeof(buffer), PSTR("unspecified"));
            break;
        case REASON_AUTH_EXPIRE:
            snprintf_P(buffer, sizeof(buffer), PSTR("authentication expired"));
            break;
        case REASON_AUTH_LEAVE:
            snprintf_P(buffer, sizeof(buffer), PSTR("authentication leave"));
            break;
        case REASON_ASSOC_EXPIRE:
            snprintf_P(buffer, sizeof(buffer), PSTR("association expired"));
            break;
        case REASON_ASSOC_TOOMANY:
            snprintf_P(buffer, sizeof(buffer), PSTR("too many associations"));
            break;
        case REASON_NOT_AUTHED:
            snprintf_P(buffer, sizeof(buffer), PSTR("not authenticated"));
            break;
        case REASON_NOT_ASSOCED:
            snprintf_P(buffer, sizeof(buffer), PSTR("not associated"));
            break;
        case REASON_ASSOC_LEAVE:
            snprintf_P(buffer, sizeof(buffer), PSTR("association leave"));
            break;
        case REASON_ASSOC_NOT_AUTHED:
            snprintf_P(buffer, sizeof(buffer), PSTR("association not authenticated"));
            break;
        case REASON_DISASSOC_PWRCAP_BAD:
            snprintf_P(buffer, sizeof(buffer), PSTR("bad powercap"));
            break;
        case REASON_DISASSOC_SUPCHAN_BAD:
            snprintf_P(buffer, sizeof(buffer), PSTR("bad supchan"));
            break;
        case REASON_IE_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("ie invalid"));
            break;
        case REASON_MIC_FAILURE:
            snprintf_P(buffer, sizeof(buffer), PSTR("mic failure"));
            break;
        case REASON_4WAY_HANDSHAKE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("handshake timeout"));
            break;
        case REASON_GROUP_KEY_UPDATE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("key update timeout"));
            break;
        case REASON_IE_IN_4WAY_DIFFERS:
            snprintf_P(buffer, sizeof(buffer), PSTR("ie handshake differs"));
            break;
        case REASON_GROUP_CIPHER_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("group cipher invalid"));
            break;
        case REASON_PAIRWISE_CIPHER_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("pairwise cipher invalid"));
            break;
        case REASON_AKMP_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("akmp invalid"));
            break;
        case REASON_UNSUPP_RSN_IE_VERSION:
            snprintf_P(buffer, sizeof(buffer), PSTR("bad powercap"));
            break;
        case REASON_INVALID_RSN_IE_CAP:
            snprintf_P(buffer, sizeof(buffer), PSTR("INVALID_RSN_IE_CAP"));
            break;
        case REASON_802_1X_AUTH_FAILED:
            snprintf_P(buffer, sizeof(buffer), PSTR("802.1x auth failed"));
            break;
        case REASON_CIPHER_SUITE_REJECTED:
            snprintf_P(buffer, sizeof(buffer), PSTR("cipher suite rejected"));
            break;

        case REASON_BEACON_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("beacon timeout"));
            break;
        case REASON_NO_AP_FOUND:
            snprintf_P(buffer, sizeof(buffer), PSTR("no AP found"));
            break;
        case REASON_AUTH_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("auth failed"));
            break;
        case REASON_ASSOC_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("assoc failed"));
            break;
        case REASON_HANDSHAKE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("handshake timeout"));
            break;
#endif

#if defined(ARDUINO_ARCH_ESP32)
        case WIFI_REASON_UNSPECIFIED:
            snprintf_P(buffer, sizeof(buffer), PSTR("unspecified"));
            break;
        case WIFI_REASON_AUTH_EXPIRE:
            snprintf_P(buffer, sizeof(buffer), PSTR("authentication expired"));
            break;
        case WIFI_REASON_AUTH_LEAVE:
            snprintf_P(buffer, sizeof(buffer), PSTR("authentication leave"));
            break;
        case WIFI_REASON_ASSOC_EXPIRE:
            snprintf_P(buffer, sizeof(buffer), PSTR("association expired"));
            break;
        case WIFI_REASON_ASSOC_TOOMANY:
            snprintf_P(buffer, sizeof(buffer), PSTR("too many associations"));
            break;
        case WIFI_REASON_NOT_AUTHED:
            snprintf_P(buffer, sizeof(buffer), PSTR("not authenticated"));
            break;
        case WIFI_REASON_NOT_ASSOCED:
            snprintf_P(buffer, sizeof(buffer), PSTR("not associated"));
            break;
        case WIFI_REASON_ASSOC_LEAVE:
            snprintf_P(buffer, sizeof(buffer), PSTR("association leave"));
            break;
        case WIFI_REASON_ASSOC_NOT_AUTHED:
            snprintf_P(buffer, sizeof(buffer), PSTR("association not authenticated"));
            break;
        case WIFI_REASON_DISASSOC_PWRCAP_BAD:
            snprintf_P(buffer, sizeof(buffer), PSTR("bad powercap"));
            break;
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
            snprintf_P(buffer, sizeof(buffer), PSTR("bad supchan"));
            break;
        case WIFI_REASON_IE_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("ie invalid"));
            break;
        case WIFI_REASON_MIC_FAILURE:
            snprintf_P(buffer, sizeof(buffer), PSTR("mic failure"));
            break;
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("handshake timeout"));
            break;
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("key update timeout"));
            break;
        case WIFI_REASON_IE_IN_4WAY_DIFFERS:
            snprintf_P(buffer, sizeof(buffer), PSTR("ie handshake differs"));
            break;
        case WIFI_REASON_GROUP_CIPHER_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("group cipher invalid"));
            break;
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("pairwise cipher invalid"));
            break;
        case WIFI_REASON_AKMP_INVALID:
            snprintf_P(buffer, sizeof(buffer), PSTR("akmp invalid"));
            break;
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
            snprintf_P(buffer, sizeof(buffer), PSTR("bad powercap"));
            break;
        case WIFI_REASON_INVALID_RSN_IE_CAP:
            snprintf_P(buffer, sizeof(buffer), PSTR("INVALID_RSN_IE_CAP"));
            break;
        case WIFI_REASON_802_1X_AUTH_FAILED:
            snprintf_P(buffer, sizeof(buffer), PSTR("802.1x auth failed"));
            break;
        case WIFI_REASON_CIPHER_SUITE_REJECTED:
            snprintf_P(buffer, sizeof(buffer), PSTR("cipher suite rejected"));
            break;

        case WIFI_REASON_BEACON_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("beacon timeout"));
            break;
        case WIFI_REASON_NO_AP_FOUND:
            snprintf_P(buffer, sizeof(buffer), PSTR("no AP found"));
            break;
        case WIFI_REASON_AUTH_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("auth powercap"));
            break;
        case WIFI_REASON_ASSOC_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("assoc failed"));
            break;
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("handshake failed"));
            break;
        case WIFI_REASON_CONNECTION_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("connection failed"));
            break;
#endif

        default:
            snprintf_P(buffer, sizeof(buffer), PSTR("unknown"));
    }

    Log.warning(TAG_WIFI, F("Disconnected from %s (Reason: %s [%d])"), ssid, buffer, reason);
}

static void wifiSsidConnected(const char * ssid)
{
    Log.notice(TAG_WIFI, F("Connected to SSID %s. Requesting IP..."), ssid);
    wifiReconnectCounter = 0;
}

#if defined(ARDUINO_ARCH_ESP32)
static void wifi_callback(system_event_id_t event, system_event_info_t info)
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
static void wifiSTAConnected(WiFiEventStationModeConnected info)
{
    wifiSsidConnected(info.ssid.c_str());
}

// Start NTP only after IP network is connected
static void wifiSTAGotIP(WiFiEventStationModeGotIP info)
{
    wifiConnected(IPAddress(info.ip));
}

// Manage network disconnection
static void wifiSTADisconnected(WiFiEventStationModeDisconnected info)
{
    wifiDisconnected(info.ssid.c_str(), info.reason);
}
#endif

/* ================================================================================================ */

bool wifiShowAP()
{
    if(strlen(wifiSsid) != 0)
        return false;
    else
        return true;
}

bool wifiShowAP(char * ssid, char * pass)
{
    if(strlen(wifiSsid) != 0) return false;

    byte mac[6];
    WiFi.macAddress(mac);
    sprintf_P(ssid, PSTR("HASP-%02x%02x%02x"), mac[3], mac[4], mac[5]);
    sprintf_P(pass, PSTR("haspadmin"));
#if defined(STM32F4xx)
    Log.warning(TAG_WIFI, F("We should setup Temporary Access Point %s password: %s"), ssid, pass);
#else
    WiFi.softAP(ssid, pass);

    /* Setup the DNS server redirecting all the domains to the apIP */
    // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    // dnsServer.start(DNS_PORT, "*", apIP);

    Log.warning(TAG_WIFI, F("Temporary Access Point %s password: %s"), ssid, pass);
    Log.warning(TAG_WIFI, F("AP IP address : %s"), WiFi.softAPIP().toString().c_str());
// httpReconnect();}
#endif
    return true;
}

static void wifiReconnect(void)
{
    WiFi.disconnect(true);
#if defined(ARDUINO_ARCH_ESP8266)
    WiFi.hostname(mqttGetNodename().c_str());
#elif defined(ARDUINO_ARCH_ESP32)
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(mqttGetNodename().c_str());
#endif
    WiFi.begin(wifiSsid, wifiPassword);
}

/* ============ Setup, Loop, Start, Stop =================================================== */

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
    WiFiSpi.init(ESPSPI_CS, 8000000, &espSPI);

    // check for the presence of the shield:
    if(WiFiSpi.status() == WL_NO_SHIELD) {
        Log.fatal(TAG_WIFI, F("WiFi shield not present"));
        // don't continue:
        while(true)
            ;
    }

    if(!WiFiSpi.checkProtocolVersion()) {
        Log.fatal(TAG_WIFI, F("Protocol version mismatch. Please upgrade the firmware"));
        // don't continue:
        while(true)
            ;
    }

    // attempt to connect to Wifi network
    // int status = WL_IDLE_STATUS;     // the Wifi radio's status
    if(!wifiShowAP()) {
        // while (status != WL_CONNECTED) {
        Log.notice(TAG_WIFI, F("Connecting to : %s"), wifiSsid);
        // Connect to WPA/WPA2 network
        // status = WiFi.begin(wifiSsid, wifiPassword);
        WiFi.begin(wifiSsid, wifiPassword);
    }
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
#elif defined(ARDUINO_ARCH_ESP32)
        WiFi.onEvent(wifi_callback);
        WiFi.setSleep(false);
#endif

        wifiReconnect();
        Log.notice(TAG_WIFI, F("Connecting to : %s"), wifiSsid);
    }
#endif
}

bool wifiEvery5Seconds()
{
#if defined(STM32F4xx)
    if(wifiShowAP()) { // no ssid is set yet wait for user on-screen input
        return false;
    } else if(WiFi.status() == WL_CONNECTED) {
#else
    if(WiFi.getMode() != WIFI_STA) {
        return false;
    } else if(WiFi.status() == WL_CONNECTED) {
#endif
        return true;
    } else {
        wifiReconnectCounter++;
        if(wifiReconnectCounter > 45) {
            Log.error(TAG_WIFI, F("Retries exceed %u: Rebooting..."), wifiReconnectCounter);
            dispatchReboot(false);
        }
        Log.warning(TAG_WIFI, F("No Connection... retry %u"), wifiReconnectCounter);
        if(wifiReconnectCounter % 6 == 0) {
            wifiReconnect();
        }
        return false;
    }
}

bool wifiValidateSsid(const char * ssid, const char * pass)
{
    uint8_t attempt = 0;
    WiFi.begin(ssid, pass);

#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    char espIp[16];
    memset(espIp, 0, sizeof(espIp));
    snprintf_P(espIp, sizeof(espIp), PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    while(attempt < 15 && (WiFi.status() != WL_CONNECTED || String(espIp) == F("0.0.0.0"))) {
#else
    while(attempt < 15 && (WiFi.status() != WL_CONNECTED || WiFi.localIP().toString() == F("0.0.0.0"))) {
#endif
        attempt++;
        Log.trace(TAG_WIFI, F("Trying to connect to %s... %u"), wifiSsid, attempt);
        delay(500);
    }
#if defined(STM32F4xx)
    Log.trace(TAG_WIFI, F("Received IP addres %s"), espIp);
    if((WiFi.status() == WL_CONNECTED && String(espIp) != F("0.0.0.0"))) return true;
#else
    Log.trace(TAG_WIFI, F("Received IP addres %s"), WiFi.localIP().toString().c_str());
    if((WiFi.status() == WL_CONNECTED && WiFi.localIP().toString() != F("0.0.0.0"))) return true;
#endif

    Log.warning(TAG_WIFI, F("Received IP addres %s"), WiFi.localIP().toString().c_str());
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
    Log.warning(TAG_WIFI, F("Stopped"));
}

/* ============ Confiuration =============================================================== */

bool wifiGetConfig(const JsonObject & settings)
{
    bool changed = false;

    if(strcmp(wifiSsid, settings[FPSTR(F_CONFIG_SSID)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_SSID)] = wifiSsid;

    if(strcmp(wifiPassword, settings[FPSTR(F_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(F_CONFIG_PASS)] = wifiPassword;

    if(changed) configOutput(settings, TAG_WIFI);
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
    configOutput(settings, TAG_WIFI);
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

#endif