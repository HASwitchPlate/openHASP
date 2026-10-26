/* MIT License - Copyright (c) 2023 Jaroslav Kysela
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_WIREGUARD_H
#define HASP_WIREGUARD_H

void wg_setup();
int wg_config_valid();
void wg_network_disconnected();
void wg_network_connected();
void wg_get_statusupdate(char* buffer, size_t len);
int wg_get_ipaddress(char* buffer, size_t len);
void wg_get_info(JsonDocument& doc);

#if HASP_USE_CONFIG > 0
bool wgGetConfig(const JsonObject& settings);
bool wgSetConfig(const JsonObject& settings);
#endif

#ifndef WIREGUARD_IP
#define WIREGUARD_IP ""
#endif

#ifndef WIREGUARD_PRIVATE_KEY
#define WIREGUARD_PRIVATE_KEY ""
#endif

#ifndef WIREGUARD_EP_IP
#define WIREGUARD_EP_IP ""
#endif

#ifndef WIREGUARD_EP_PORT
#define WIREGUARD_EP_PORT 51820
#endif

#ifndef WIREGUARD_EP_PUBLIC_KEY
#define WIREGUARD_EP_PUBLIC_KEY ""
#endif

#endif
