#ifndef WIFISPI_H
#define WIFISPI_H

// Glue between STM32_EthernetWebserver and WiFiSpi library
#include <WiFiSpi.h>
using EthernetClient = WiFiSpiClient; // Alias
using EthernetServer = WiFiSpiServer;

#endif