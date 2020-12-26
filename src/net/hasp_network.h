/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_NETWORK_H
#define HASP_NETWORK_H

/* ===== Default Event Processors ===== */
void networkSetup();
void IRAM_ATTR networkLoop(void);
void networkEvery5Seconds(void);
void networkEverySecond(void);
void networkStart(void);
void networkStop(void);

/* ===== Special Event Processors ===== */

/* ===== Getter and Setter Functions ===== */
void network_get_statusupdate(char * buffer, size_t len);

/* ===== Read/Write Configuration ===== */

#endif