/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_ETHERNET_ESP32_H
#define HASP_ETHERNET_ESP32_H

static bool eth_connected = false;

void ethernetSetup();
void IRAM_ATTR ethernetLoop(void);

bool ethernetEvery5Seconds();

#endif