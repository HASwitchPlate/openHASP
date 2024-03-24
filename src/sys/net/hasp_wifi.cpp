/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_conf.h"

#if HASP_USE_WIFI > 0

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_network.h"
#include "hasp_gui.h"

#include "hasp/hasp_dispatch.h"
#include "hasp/hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#ifndef ESP_ARDUINO_VERSION_VAL
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#endif

#include <WiFi.h>
#include "Preferences.h"
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
// #include "DNSserver.h"

char wifiSsid[MAX_SSID_LEN]           = WIFI_SSID;
char wifiPassword[MAX_PASSPHRASE_LEN] = WIFI_PASSWORD;
char wifiIpAddress[16]                = "";
bool wifiEnabled                      = true;
extern uint16_t network_reconnect_counter;

// const byte DNS_PORT = 53;
// DNSServer dnsServer;

/* ============ Connection Event Handlers =============================================================== */

static void wifiConnected(IPAddress ipaddress)
{
#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    snprintf_P(wifiIpAddress, sizeof(wifiIpAddress), PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
#else
    strncpy(wifiIpAddress, ipaddress.toString().c_str(), sizeof(wifiIpAddress));
#endif

    if((uint32_t)ipaddress == 0) {
        LOG_ERROR(TAG_WIFI, F(D_NETWORK_IP_ADDRESS_RECEIVED), wifiIpAddress);
        network_disconnected();
        return;
    } else {
        LOG_TRACE(TAG_WIFI, F(D_NETWORK_IP_ADDRESS_RECEIVED), wifiIpAddress);
    }

    network_connected();
}

static void wifiDisconnected(const char* ssid, uint8_t reason)
{
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
        case WIFI_REASON_BSS_TRANSITION_DISASSOC:
            snprintf_P(buffer, sizeof(buffer), PSTR("bss transition disassoc"));
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

        case WIFI_REASON_INVALID_PMKID:
            snprintf_P(buffer, sizeof(buffer), PSTR("invalid pmkid"));
            break;

        case WIFI_REASON_BEACON_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("beacon timeout"));
            break;
        case WIFI_REASON_NO_AP_FOUND:
            snprintf_P(buffer, sizeof(buffer), PSTR("no AP found"));
            break;
        case WIFI_REASON_AUTH_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("auth failed"));
            break;
        case WIFI_REASON_ASSOC_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR("assoc failed"));
            break;
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            snprintf_P(buffer, sizeof(buffer), PSTR("handshake failed"));
            break;
        case WIFI_REASON_CONNECTION_FAIL:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_NETWORK_CONNECTION_FAILED));
            break;
        case WIFI_REASON_AP_TSF_RESET:
        case WIFI_REASON_ROAMING:
#endif

        default:
            snprintf_P(buffer, sizeof(buffer), PSTR(D_ERROR_UNKNOWN " (%d)"), reason);
    }

    LOG_WARNING(TAG_WIFI, buffer);
    network_disconnected();
}

static void wifiSsidConnected(const char* ssid)
{
    LOG_TRACE(TAG_WIFI, F(D_WIFI_CONNECTED_TO), ssid);
}

#if defined(ARDUINO_ARCH_ESP32)
static void wifi_callback(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch(event) {
        case SYSTEM_EVENT_WIFI_READY: /*!< ESP32 WiFi ready */
            LOG_VERBOSE(TAG_WIFI, F("ready"));
            break;
        case SYSTEM_EVENT_STA_START: /*!< ESP32 station start */
            LOG_VERBOSE(TAG_WIFI, F("station start"));
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:    /*!< the auth mode of AP connected by ESP32 station changed */
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:     /*!< ESP32 station wps succeeds in enrollee mode */
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:      /*!< ESP32 station wps fails in enrollee mode */
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:     /*!< ESP32 station wps timeout in enrollee mode */
        case SYSTEM_EVENT_STA_WPS_ER_PIN:         /*!< ESP32 station wps pin code in enrollee mode */
        case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP: /*!< ESP32 station wps overlap in enrollee mode */
        case SYSTEM_EVENT_AP_START:               /*!< ESP32 soft-AP start */
        case SYSTEM_EVENT_AP_STACONNECTED:        /*!< a station connected to ESP32 soft-AP */
        case SYSTEM_EVENT_AP_STADISCONNECTED:     /*!< a station disconnected from ESP32 soft-AP */
        case SYSTEM_EVENT_AP_STAIPASSIGNED:       /*!< ESP32 soft-AP assign an IP to a connected station */
        case SYSTEM_EVENT_AP_PROBEREQRECVED:      /*!< Receive probe request packet in soft-AP interface */
        case SYSTEM_EVENT_ACTION_TX_STATUS:       /*!< Receive status of Action frame transmitted */
        case SYSTEM_EVENT_ROC_DONE:               /*!< Indicates the completion of Remain-on-Channel operation status */
        case SYSTEM_EVENT_STA_BEACON_TIMEOUT:     /*!< ESP32 station beacon timeout */
        case SYSTEM_EVENT_FTM_REPORT:             /*!< Receive report of FTM procedure */
        case SYSTEM_EVENT_GOT_IP6:          /*!< ESP32 station or ap or ethernet interface v6IP addr is preferred */
        case SYSTEM_EVENT_ETH_START:        /*!< ESP32 ethernet start */
        case SYSTEM_EVENT_ETH_STOP:         /*!< ESP32 ethernet stop */
        case SYSTEM_EVENT_ETH_CONNECTED:    /*!< ESP32 ethernet phy link up */
        case SYSTEM_EVENT_ETH_DISCONNECTED: /*!< ESP32 ethernet phy link down */
        case SYSTEM_EVENT_ETH_GOT_IP:       /*!< ESP32 ethernet got IP from connected AP */
        case SYSTEM_EVENT_ETH_LOST_IP:      /*!< ESP32 ethernet lost IP and the IP is reset to 0 */
        case SYSTEM_EVENT_MAX:              /*!< Number of members in this enum */
            LOG_DEBUG(TAG_WIFI, F("Other Event: %d"), event);
            break;

        case SYSTEM_EVENT_STA_CONNECTED: /*!< ESP32 station connected to AP */
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0)
            wifiSsidConnected((const char*)info.wifi_sta_connected.ssid);
#else
            wifiSsidConnected((const char*)info.connected.ssid);
#endif
            break;

        case SYSTEM_EVENT_STA_BSS_RSSI_LOW:
            LOG_WARNING(TAG_WIFI, F("BSS rssi goes below threshold"));
            break;

        case SYSTEM_EVENT_AP_STOP:          /*!< ESP32 soft-AP stop */
        case SYSTEM_EVENT_STA_STOP:         /*!< ESP32 station stop */
                                            // wifiSetup();
                                            // break;
        case SYSTEM_EVENT_STA_LOST_IP:      /*!< ESP32 station lost IP and the IP is reset to 0 */
        case SYSTEM_EVENT_STA_DISCONNECTED: /*!< ESP32 station disconnected from AP */
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0)
            wifiDisconnected((const char*)info.wifi_sta_disconnected.ssid, info.wifi_sta_disconnected.reason);
#else
            wifiDisconnected((const char*)info.disconnected.ssid, info.disconnected.reason);
#endif
            // NTP.stop(); // NTP sync can be disabled to avoid sync errors
            break;

        case SYSTEM_EVENT_STA_GOT_IP: /*!< ESP32 station got IP from connected AP */
            wifiConnected(IPAddress(info.got_ip.ip_info.ip.addr));
            break;

        case SYSTEM_EVENT_SCAN_DONE: { /*!< ESP32 finish scanning AP */
            uint16_t count = WiFi.scanComplete();
            for(int i = 0; i < count; ++i) {
                // Print SSID and RSSI for each network found
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(")");
                Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            }
            break;
        }

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
    return wifiEnabled && strlen(wifiSsid) == 0;
}

bool wifiShowAP(char* ssid, char* pass)
{
    if(strlen(wifiSsid) != 0) return false;

    byte mac[6];
    WiFi.macAddress(mac);
    sprintf_P(ssid, PSTR("HASP-%02x%02x%02x"), mac[3], mac[4], mac[5]);
    sprintf_P(pass, PSTR("haspadmin"));
#if defined(STM32F4xx)
    LOG_WARNING(TAG_WIFI, F("We should setup Temporary Access Point %s password: %s"), ssid, pass);
#else
    WiFi.softAP(ssid, pass);

    /* Setup the DNS server redirecting all the domains to the apIP */
    // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    // dnsServer.start(DNS_PORT, "*", apIP);

    LOG_WARNING(TAG_WIFI, F("Temporary Access Point %s password: %s"), ssid, pass);
    LOG_WARNING(TAG_WIFI, F("AP IP address : %s"), WiFi.softAPIP().toString().c_str());
    networkStart();
// httpReconnect();}
#endif
    return true;
}

static void wifiReconnect(void)
{
#if defined(ARDUINO_ARCH_ESP8266)
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.begin(wifiSsid, wifiPassword);
    WiFi.hostname(haspDevice.get_hostname());

#elif defined(ARDUINO_ARCH_ESP32)
    // https://github.com/espressif/arduino-esp32/issues/3438#issuecomment-721428310
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.setHostname(haspDevice.get_hostname());
    WiFi.setSleep(false);

    IPAddress ip(INADDR_NONE);
    // IPAddress ip(192, 168, 0, 60);
    IPAddress net(INADDR_NONE);
    // IPAddress net(255, 255, 255, 0);
    IPAddress gw(INADDR_NONE);
    IPAddress dns1(INADDR_NONE);
    IPAddress dns2(INADDR_NONE);

    if(ip && net)
        WiFi.config(ip, gw, net, dns1, dns2);
    else
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword);
#endif
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
        LOG_FATAL(TAG_WIFI, F("WiFi shield not present")); // Needs to be in curly braces
    }

    if(!WiFiSpi.checkProtocolVersion()) {
        LOG_FATAL(TAG_WIFI, F("Protocol version mismatch. Please upgrade the firmware")); // Needs to be in curly braces
    }

    // attempt to connect to Wifi network
    // int status = WL_IDLE_STATUS;     // the Wifi radio's status
    if(!wifiShowAP()) {
        // while (status != WL_CONNECTED) {
        LOG_TRACE(TAG_WIFI, F(D_WIFI_CONNECTING_TO), wifiSsid);
        // Connect to WPA/WPA2 network
        // status = WiFi.begin(wifiSsid, wifiPassword);
        WiFi.begin(wifiSsid, wifiPassword);
    }
    // }

#else
    if(wifiShowAP()) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        //   WiFi.mode(WIFI_STA);

#if defined(ARDUINO_ARCH_ESP8266)
        // wifiEventHandler[0]      = WiFi.onStationModeConnected(wifiSTAConnected);
        gotIpEventHandler        = WiFi.onStationModeGotIP(wifiSTAGotIP); // As soon WiFi is connected, start NTP Client
        disconnectedEventHandler = WiFi.onStationModeDisconnected(wifiSTADisconnected);
#elif defined(ARDUINO_ARCH_ESP32)
        WiFi.onEvent(wifi_callback);

        Preferences preferences;
        nvs_user_begin(preferences, "wifi", true);
        String password = preferences.getString(FP_CONFIG_PASS, WIFI_PASSWORD);
        strncpy(wifiPassword, password.c_str(), sizeof(wifiPassword));
        LOG_DEBUG(TAG_WIFI, F(D_BULLET "Read %s => %s (%d bytes)"), FP_CONFIG_PASS, password.c_str(),
                  password.length());
        preferences.end();
#endif

        WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
        wifiReconnect();
        WiFi.setAutoReconnect(false); // done in wifiEvery5Seconds
        LOG_TRACE(TAG_WIFI, F(D_WIFI_CONNECTING_TO), wifiSsid);
    }
#endif
}

bool wifiEvery5Seconds()
{
#if defined(STM32F4xx)
    if(wifiShowAP()) { // no ssid is set yet wait for user on-screen input
        return false;
    }
#else
    if(WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        LOG_DEBUG(TAG_WIFI, F("5sec mode AP %d"), WiFi.getMode());
        return false;
    }
#endif

    if(WiFi.status() == WL_CONNECTED && WiFi.localIP() > 0) {
        return true;
    }

    if(wifiEnabled) {
        LOG_WARNING(TAG_WIFI, F("No Connection... retry %d"), network_reconnect_counter);
        wifiReconnect();
    }

    return false;
}

bool wifiValidateSsid(const char* ssid, const char* pass)
{
#ifdef ARDUINO_ARCH_ESP32
    WiFi.begin(ssid, pass, WIFI_ALL_CHANNEL_SCAN);
#else
    WiFi.begin(ssid, pass);
#endif

    uint8_t attempt = 0;

#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    char espIp[16];
    memset(espIp, 0, sizeof(espIp));
    snprintf_P(espIp, sizeof(espIp), PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    while(attempt < 30 && (WiFi.status() != WL_CONNECTED || String(espIp) == F("0.0.0.0"))) {
#else
    while(attempt < 30 && (WiFi.status() != WL_CONNECTED || WiFi.localIP().toString() == F("0.0.0.0"))) {
#endif
        attempt++;
        LOG_INFO(TAG_WIFI, F(D_WIFI_CONNECTING_TO "... %u"), ssid, attempt);
        delay(250);
    }
#if defined(STM32F4xx)
    LOG_INFO(TAG_WIFI, F(D_NETWORK_IP_ADDRESS_RECEIVED), espIp);
    if((WiFi.status() == WL_CONNECTED && String(espIp) != F("0.0.0.0"))) return true;
#else
    LOG_INFO(TAG_WIFI, F(D_NETWORK_IP_ADDRESS_RECEIVED), WiFi.localIP().toString().c_str());
    if((WiFi.status() == WL_CONNECTED && WiFi.localIP().toString() != F("0.0.0.0"))) return true;
#endif

    LOG_WARNING(TAG_WIFI, F(D_NETWORK_IP_ADDRESS_RECEIVED), WiFi.localIP().toString().c_str());
    WiFi.disconnect(true);
    return false;
}

void wifiStop()
{
    WiFi.disconnect(true);
#if !defined(STM32F4xx)
    WiFi.mode(WIFI_OFF);
#endif
    LOG_WARNING(TAG_WIFI, F(D_SERVICE_STOPPED));
}

void wifi_get_statusupdate(char* buffer, size_t len)
{
#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    char espIp[16];
    memset(espIp, 0, sizeof(espIp));
    snprintf_P(buffer, len, PSTR("\"ssid\":\"%s\",\"rssi\":%i,\"ip\":\"%d.%d.%d.%d\",\"mac\":\"%s\","), WiFi.SSID(),
               WiFi.RSSI(), ip[0], ip[1], ip[2], ip[3], "TODO");
#else
    strncpy(wifiIpAddress, WiFi.localIP().toString().c_str(), sizeof(wifiIpAddress));
    snprintf_P(buffer, len, PSTR("\"ssid\":\"%s\",\"rssi\":%i,\"ip\":\"%s\",\"mac\":\"%s\","), WiFi.SSID().c_str(),
               WiFi.RSSI(), wifiIpAddress, WiFi.macAddress().c_str());
#endif
}

const char* wifi_get_ssid()
{
    return wifiSsid;
}

const char* wifi_get_ip_address()
{
    return wifiIpAddress;
}

void wifi_get_info(JsonDocument& doc)
{
    String buffer((char*)0);
    buffer.reserve(64);

    JsonObject info       = doc.createNestedObject(F(D_INFO_WIFI));
    info[F(D_INFO_BSSID)] = WiFi.BSSIDstr();

    int8_t rssi = WiFi.RSSI();
    buffer += String(rssi);
    buffer += F("dBm (");

    if(rssi >= -50) {
        buffer += F(D_WIFI_RSSI_EXCELLENT ")");
    } else if(rssi >= -59) {
        buffer += F(D_WIFI_RSSI_GOOD ")");
    } else if(rssi >= -68) {
        buffer += F(D_WIFI_RSSI_FAIR ")");
    } else if(rssi >= -77) {
        buffer += F(D_WIFI_RSSI_WEAK ")");
    } else {
        buffer += F(D_WIFI_RSSI_BAD ")");
    }

    info[F(D_INFO_SSID)] = String(WiFi.SSID());
    info[F(D_INFO_RSSI)] = buffer;

#if defined(STM32F4xx)
    byte mac[6];
    WiFi.macAddress(mac);
    char macAddress[16];
    snprintf_P(macAddress, sizeof(macAddress), PSTR("%02x%02x%02x"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    info[F(D_INFO_IP_ADDRESS)]  = String(WiFi.localIP());
    info[F(D_INFO_GATEWAY)]     = String(WiFi.gatewayIP());
    info[F(D_INFO_MAC_ADDRESS)] = String(macAddress);
#else
    info[F(D_INFO_IP_ADDRESS)]  = WiFi.localIP().toString();
    info[F(D_INFO_GATEWAY)]     = WiFi.gatewayIP().toString();
    info[F(D_INFO_DNS_SERVER)]  = WiFi.dnsIP().toString();
    info[F(D_INFO_MAC_ADDRESS)] = WiFi.macAddress();
#endif
}

/* ============ Confiuration =============================================================== */
#if HASP_USE_CONFIG > 0
bool wifiGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(strcmp(wifiSsid, settings[FPSTR(FP_CONFIG_SSID)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_SSID)] = wifiSsid;

    // if(strcmp(wifiPassword, settings[FPSTR(FP_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    // settings[FPSTR(FP_CONFIG_PASS)] = wifiPassword;
    if(strcmp(D_PASSWORD_MASK, settings[FPSTR(FP_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_PASS)] = D_PASSWORD_MASK;

    if(changed) configOutput(settings, TAG_WIFI);
    return changed;
}

/** Set WIFI Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formatted to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool wifiSetConfig(const JsonObject& settings)
{
    Preferences preferences;
    nvs_user_begin(preferences, "wifi", false);

    configOutput(settings, TAG_WIFI);
    bool changed = false;

    if(!settings[FPSTR(FP_CONFIG_SSID)].isNull()) {
        changed |= strcmp(wifiSsid, settings[FPSTR(FP_CONFIG_SSID)]) != 0;
        strncpy(wifiSsid, settings[FPSTR(FP_CONFIG_SSID)], sizeof(wifiSsid));
    }

    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() &&
       settings[FPSTR(FP_CONFIG_PASS)].as<String>() != String(FPSTR(D_PASSWORD_MASK))) {
        changed |= strcmp(wifiPassword, settings[FPSTR(FP_CONFIG_PASS)]) != 0;
        strncpy(wifiPassword, settings[FPSTR(FP_CONFIG_PASS)], sizeof(wifiPassword));
        nvsUpdateString(preferences, FP_CONFIG_PASS, settings[FPSTR(FP_CONFIG_PASS)]);
    }

    return changed;
}
#endif // HASP_USE_CONFIG

#endif
