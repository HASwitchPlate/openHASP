#ifndef HASP_ETHERNET_ESP32_H
#define HASP_ETHERNET_ESP32_H

static bool eth_connected = false;

void ethernetSetup();
void ethernetLoop(void);

bool ethernetEvery5Seconds();

#endif