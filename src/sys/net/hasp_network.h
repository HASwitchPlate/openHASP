/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_NETWORK_H
#define HASP_NETWORK_H

/* ===== Default Event Processors ===== */
void networkSetup();
IRAM_ATTR void networkLoop(void);
bool networkEvery5Seconds(void);
// bool networkEverySecond(void);
void networkStart(void);
void networkStop(void);

/* ===== Special Event Processors ===== */

/* ===== Getter and Setter Functions ===== */
void network_get_statusupdate(char* buffer, size_t len);
void network_get_ipaddress(char* buffer, size_t len);
void network_get_info(JsonDocument& doc);

/* ===== Read/Write Configuration ===== */

#endif