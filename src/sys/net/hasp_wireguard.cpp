/* MIT License - Copyright (c) 2023 Jaroslav Kysela
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if HASP_USE_WIREGUARD > 0

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_network.h"
#include "WireGuard-ESP32.h"

char wg_ip[16] = WIREGUARD_IP;
char wg_private_key[45] = WIREGUARD_PRIVATE_KEY;
char wg_ep_ip[40] = WIREGUARD_EP_IP;
uint16_t wg_ep_port = WIREGUARD_EP_PORT;
char wg_ep_public_key[45] = WIREGUARD_EP_PUBLIC_KEY;
static WireGuard wg;

void wg_setup()
{
    Preferences preferences;
    nvs_user_begin(preferences, FP_WG, true);
    String privkey = preferences.getString(FP_CONFIG_PRIVATE_KEY, String(wg_private_key));   // Update from NVS if it exists
    strncpy(wg_private_key, privkey.c_str(), sizeof(wg_private_key)-1);
    wg_private_key[sizeof(wg_private_key)-1] = '\0';
}

int wg_config_valid()
{
    return strlen(wg_ip) > 7 && strlen(wg_ep_ip) > 7 &&
            strlen(wg_private_key) == 44 && strlen(wg_ep_public_key) == 44 &&
            wg_ep_port > 0;
}

void wg_network_disconnected()
{
    wg.end();
}

void wg_network_connected()
{
    IPAddress local_ip;
    LOG_VERBOSE(TAG_WG, F("WireGuard connected"));
    if (local_ip.fromString(wg_ip) && wg_config_valid()) {
        LOG_INFO(TAG_WG, F("WireGuard begin (%s -> %s:%u)"), wg_ip, wg_ep_ip, wg_ep_port);
        wg.begin(local_ip, wg_private_key, wg_ep_ip, wg_ep_public_key, wg_ep_port);
    }
}

void wg_get_statusupdate(char* buffer, size_t len)
{
    snprintf_P(buffer, len, PSTR("\"wg\":\"%s\","), wg.is_initialized() ? "on" : "off");
}

int wg_get_ipaddress(char* buffer, size_t len)
{
    if (wg.is_initialized()) {
        snprintf(buffer, len, "%s", wg_ip);
        return 1;
    }
    return 0;
}

void wg_get_info(JsonDocument& doc)
{
    JsonObject info = doc.createNestedObject(F(D_INFO_WIREGUARD));
    info[F(D_INFO_STATUS)] = wg.is_initialized() ? F(D_WG_INITIALIZED) : F(D_WG_BAD_CONFIG);
    info[F(D_INFO_IP_ADDRESS)] = String(wg_ip);
    info[F(D_INFO_ENDPOINT_IP)] = String(wg_ep_ip);
    info[F(D_INFO_ENDPOINT_PORT)] = String(wg_ep_port);
}

#if HASP_USE_CONFIG > 0
bool wgGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(strcmp(wg_ip, settings[FPSTR(FP_CONFIG_VPN_IP)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_VPN_IP)] = wg_ip;

    if(strcmp(wg_private_key, settings[FPSTR(FP_CONFIG_PRIVATE_KEY)].as<String>().c_str()) != 0) changed = true;
    //settings[FPSTR(FP_CONFIG_PRIVATE_KEY)] = wg_private_key;
    settings[FPSTR(FP_CONFIG_PRIVATE_KEY)] = D_PASSWORD_MASK;

    if(strcmp(wg_ep_ip, settings[FPSTR(FP_CONFIG_HOST)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_HOST)] = wg_ep_ip;

    if(wg_ep_port != settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PORT)] = wg_ep_port;

    if(strcmp(wg_ep_public_key, settings[FPSTR(FP_CONFIG_PUBLIC_KEY)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_PUBLIC_KEY)] = wg_ep_public_key;

    if(changed) configOutput(settings, TAG_WG);
    return changed;
}

bool wgSetConfig(const JsonObject& settings)
{
    Preferences preferences;
    nvs_user_begin(preferences, "wg", false);

    configOutput(settings, TAG_WG);
    bool changed = false;

    changed |= configSet((char *)wg_ip, sizeof(wg_ip), settings[FPSTR(FP_CONFIG_VPN_IP)], F("wgIp"));
    if(!settings[FPSTR(FP_CONFIG_PRIVATE_KEY)].isNull() &&
       settings[FPSTR(FP_CONFIG_PRIVATE_KEY)].as<String>() != String(FPSTR(D_PASSWORD_MASK))) {
        changed |= strcmp(wg_private_key, settings[FPSTR(FP_CONFIG_PRIVATE_KEY)]) != 0;
        strncpy(wg_private_key, settings[FPSTR(FP_CONFIG_PRIVATE_KEY)], sizeof(wg_private_key)-1);
        wg_private_key[sizeof(wg_private_key)-1] = '\0';
        nvsUpdateString(preferences, FP_CONFIG_PRIVATE_KEY, settings[FPSTR(FP_CONFIG_PRIVATE_KEY)]);
    }
    changed |= configSet((char *)wg_ep_ip, sizeof(wg_ep_ip), settings[FPSTR(FP_CONFIG_HOST)], F("wgEpIp"));
    changed |= configSet(wg_ep_port, settings[FPSTR(FP_CONFIG_PORT)], F("wgEpPort"));
    changed |= configSet((char *)wg_ep_public_key, sizeof(wg_ep_public_key), settings[FPSTR(FP_CONFIG_PUBLIC_KEY)], F("wgEpPubKey"));

// may reset device...
//    if (changed && network_is_connected()) {
//        wg.end();
//        wg_network_connected();
//    }

    return changed;
}
#endif


#endif /* WIREGUARD */
