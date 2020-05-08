#ifndef HASP_ETHERNET_H
#define HASP_ETHERNET_H

void ethernetSetup();
void ethernetLoop(void);

bool ethernetEvery5Seconds();
#endif