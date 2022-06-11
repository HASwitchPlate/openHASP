/* MIT License - Copyright (c) 2022 Ben Suffolk, ben@vanilla.net
   For full license information read the LICENSE file in the project folder */

#ifndef _ETHSPI_H_
#define _ETHSPI_H_

#include "WiFi.h"
#include "esp_system.h"
#include "esp_eth.h"
#include "driver/spi_master.h"

#ifndef ETHSPI_HOST
#define ETHSPI_HOST HSPI_HOST
#endif

#ifndef ETHSPI_CLOCK_MHZ
#define ETHSPI_CLOCK_MHZ 12
#endif

#ifndef ETHSPI_INT_GPIO
#define ETHSPI_INT_GPIO 4
#endif

#ifndef ETHSPI_MOSI_GPIO
#define ETHSPI_MOSI_GPIO 13
#endif

#ifndef ETHSPI_MISO_GPIO
#define ETHSPI_MISO_GPIO 12
#endif

#ifndef ETHSPI_SCLK_GPIO
#define ETHSPI_SCLK_GPIO 14
#endif

#ifndef ETHSPI_CS_GPIO
#define ETHSPI_CS_GPIO 15
#endif

class ETHSPIClass {
 private:
  bool initialized;
  bool staticIP;
  esp_eth_handle_t eth_handle;
  esp_netif_t *eth_netif_spi;

 public:
  ETHSPIClass();
  ~ETHSPIClass();

  bool begin(int mosi_io = ETHSPI_MOSI_GPIO, int miso_io = ETHSPI_MISO_GPIO, int sclk_io = ETHSPI_SCLK_GPIO, int cs_io = ETHSPI_CS_GPIO, int int_io = ETHSPI_INT_GPIO, spi_host_device_t spi_host = ETHSPI_HOST);

  bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);

  const char * getHostname();
  bool setHostname(const char * hostname);

  bool fullDuplex();
  bool linkUp();
  uint8_t linkSpeed();

  IPAddress localIP();
  IPAddress subnetMask();
  IPAddress gatewayIP();
  IPAddress dnsIP(esp_netif_dns_type_t dns_type = ESP_NETIF_DNS_MAIN);

  IPAddress broadcastIP();
  IPAddress networkID();
  uint8_t subnetCIDR();

  uint8_t * macAddress(uint8_t* mac);
  String macAddress();

  friend class WiFiClient;
  friend class WiFiServer;
};

extern ETHSPIClass ETHSPI;

#endif /* _ETH_H_ */
