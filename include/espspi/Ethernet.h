/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef WIFISPI_H
#define WIFISPI_H

// Glue between STM32_EthernetWebserver and WiFiSpi library
#include <WiFiSpi.h>
using EthernetClient = WiFiSpiClient; // Alias
using EthernetServer = WiFiSpiServer;

#endif