/* MIT License - Copyright (c) 2022 Ben Suffolk, ben@vanilla.net
   For full license information read the LICENSE file in the project folder */

#if defined(HASP_USE_ETHSPI)

#include "ETHSPI.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_eth_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/dns.h"
extern void tcpipInit();

ETHSPIClass::ETHSPIClass()
 :initialized(false)
 ,staticIP(false)
 ,eth_handle(NULL)
 ,eth_netif_spi(NULL)
{
}

ETHSPIClass::~ETHSPIClass()
{}


bool ETHSPIClass::begin(int mosi_io, int miso_io, int sclk_io, int cs_io, int int_io, spi_host_device_t spi_host)
{
 if(initialized) {
  return true;
 }

 tcpipInit();

 // Create instance(s) of esp-netif for SPI Ethernet(s)
 esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
 esp_netif_config.if_desc = "eth0";
 esp_netif_config.route_prio = 30;

 esp_netif_config_t cfg_spi = {
  .base = &esp_netif_config,
  .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
 };
 
 eth_netif_spi =  esp_netif_new(&cfg_spi);


 // Init MAC and PHY configs to default
 eth_mac_config_t mac_config_spi = ETH_MAC_DEFAULT_CONFIG();
 eth_phy_config_t phy_config_spi = ETH_PHY_DEFAULT_CONFIG();

 // Install GPIO ISR handler to be able to service SPI Eth modlues interrupts
 gpio_install_isr_service(0);

 // Init SPI bus
 spi_device_handle_t spi_handle = NULL;
 spi_bus_config_t buscfg = {
  .mosi_io_num = mosi_io,
  .miso_io_num = miso_io,
  .sclk_io_num = sclk_io,
  .quadwp_io_num = -1,
  .quadhd_io_num = -1,
 };

 if(spi_bus_initialize(ETHSPI_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
  return false;
 }

 // Configure SPI interface and Ethernet driver for specific SPI module
 esp_eth_mac_t *mac_spi;
 esp_eth_phy_t *phy_spi;

 spi_device_interface_config_t devcfg = {
 .command_bits = 16, // Actually it's the address phase in W5500 SPI frame
 .address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
 .mode = 0,
 .clock_speed_hz = ETHSPI_CLOCK_MHZ * 1000 * 1000,
 .queue_size = 20
 };

 // Set SPI module Chip Select GPIO
 devcfg.spics_io_num = cs_io;

 if(spi_bus_add_device(spi_host, &devcfg, &spi_handle) != ESP_OK) {
  return false;
 }

 // w5500 ethernet driver is based on spi driver
 eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);

 // Set remaining GPIO numbers and configuration used by the SPI module
 w5500_config.int_gpio_num = int_io;
 phy_config_spi.phy_addr = 1;
 phy_config_spi.reset_gpio_num = -1;

 mac_spi = esp_eth_mac_new_w5500(&w5500_config, &mac_config_spi);
 phy_spi = esp_eth_phy_new_w5500(&phy_config_spi);

 esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac_spi, phy_spi);

 if(esp_eth_driver_install(&eth_config_spi, &eth_handle) != ESP_OK) {
  return false;
 }

 // W5500 Does not have a mac address on the module. So get the enternet mac address from ESP fuse
 uint8_t mac[6];
 esp_read_mac(mac, ESP_MAC_ETH);
 if(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, mac) != ESP_OK) {
  return false;
 }
 
 // attach Ethernet driver to TCP/IP stack
 if(esp_netif_attach(eth_netif_spi, esp_eth_new_netif_glue(eth_handle)) != ESP_OK) {
  return false;
 }

 if(esp_eth_start(eth_handle) != ESP_OK) {
  return false;
 }

 initialized = true;
 
 return initialized;
}


bool ETHSPIClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
 esp_netif_ip_info_t ip_info;

 if(local_ip != (uint32_t)0x00000000 && local_ip != INADDR_NONE) {
  ip_info.ip.addr = static_cast<uint32_t>(local_ip);
  ip_info.gw.addr = static_cast<uint32_t>(gateway);
  ip_info.netmask.addr = static_cast<uint32_t>(subnet);
 } else {
  ip_info.ip.addr = 0;
  ip_info.gw.addr = 0;
  ip_info.netmask.addr = 0;
	}

 // Stop DHCP client
 esp_netif_dhcp_status_t status;
 if(esp_netif_dhcpc_get_status(eth_netif_spi, &status) != ESP_OK) {
  log_e("could not get DHCP status");
  return false;
 }

 if(status == ESP_NETIF_DHCP_STARTED && esp_netif_dhcpc_stop(eth_netif_spi) != ESP_OK) {
  log_e("DHCP could not be stopped");
  return false;
 }

 // Set IP Details
 if(esp_netif_set_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  log_e("Unable to set IP address");
  return false;
 }


 // Start DHCP client is required
 if(ip_info.ip.addr){
  staticIP = true;
 } else {

  if(esp_netif_dhcpc_start(eth_netif_spi) != ESP_OK) {
   log_e("DHCP could not be started");
   return false;
  }
     
  staticIP = false;
 }


 // Set primary DNS
 if(dns1 != (uint32_t)0x00000000 && dns1 != INADDR_NONE) {
  
  esp_netif_dns_info_t dns_info;
  dns_info.ip.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
 
  if(esp_netif_set_dns_info(eth_netif_spi, ESP_NETIF_DNS_MAIN, &dns_info) != ESP_OK) {
   log_e("Unable to set DNS");
   return false;
  }
 }

 // Set secondary DNS
 if(dns2 != (uint32_t)0x00000000 && dns2 != INADDR_NONE) {
  
  esp_netif_dns_info_t dns_info;
  dns_info.ip.u_addr.ip4.addr = static_cast<uint32_t>(dns2);
 
  if(esp_netif_set_dns_info(eth_netif_spi, ESP_NETIF_DNS_FALLBACK, &dns_info) != ESP_OK) {
   log_e("Unable to set DNS");
   return false;
  }
 }

 return true;
}

IPAddress ETHSPIClass::localIP()
{
 esp_netif_ip_info_t ip_info;

 if(esp_netif_get_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  return IPAddress();
 }

 return IPAddress(ip_info.ip.addr);
}

IPAddress ETHSPIClass::subnetMask()
{
 esp_netif_ip_info_t ip_info;

 if(esp_netif_get_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  return IPAddress();
 }

 return IPAddress(ip_info.netmask.addr);
}

IPAddress ETHSPIClass::gatewayIP()
{
 esp_netif_ip_info_t ip_info;

 if(esp_netif_get_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  return IPAddress();
 }

 return IPAddress(ip_info.gw.addr);
}

IPAddress ETHSPIClass::dnsIP(esp_netif_dns_type_t dns_type)
{
 esp_netif_dns_info_t dns_info;

 if(esp_netif_get_dns_info(eth_netif_spi, dns_type, &dns_info) != ESP_OK) {
  return IPAddress();
 }

 return IPAddress(dns_info.ip.u_addr.ip4.addr);
}

IPAddress ETHSPIClass::broadcastIP()
{
 esp_netif_ip_info_t ip_info;

 if(esp_netif_get_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  return IPAddress();
 }

 return WiFiGenericClass::calculateBroadcast(IPAddress(ip_info.gw.addr), IPAddress(ip_info.netmask.addr));
}

IPAddress ETHSPIClass::networkID()
{
 esp_netif_ip_info_t ip_info;

 if(esp_netif_get_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  return IPAddress();
 }

 return WiFiGenericClass::calculateNetworkID(IPAddress(ip_info.gw.addr), IPAddress(ip_info.netmask.addr));
}

uint8_t ETHSPIClass::subnetCIDR()
{
 esp_netif_ip_info_t ip_info;

 if(esp_netif_get_ip_info(eth_netif_spi, &ip_info) != ESP_OK) {
  return IPAddress();
 }
 
 return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip_info.netmask.addr));
}

const char * ETHSPIClass::getHostname()
{
 const char *hostname;

 if(esp_netif_get_hostname(eth_netif_spi, &hostname) != ESP_OK) {
  return NULL;
 }

 return hostname;
}

bool ETHSPIClass::setHostname(const char *hostname)
{
 return(esp_netif_set_hostname(eth_netif_spi, hostname) == ESP_OK);
}

bool ETHSPIClass::fullDuplex()
{
 eth_duplex_t duplex;
 
 esp_eth_ioctl(eth_handle, ETH_CMD_G_DUPLEX_MODE, &duplex);
 return (duplex == ETH_DUPLEX_FULL);
}

bool ETHSPIClass::linkUp()
{
 return esp_netif_is_netif_up(eth_netif_spi);
}


uint8_t ETHSPIClass::linkSpeed()
{
 eth_speed_t link_speed;
 esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, &link_speed);
 return (link_speed == ETH_SPEED_10M)?10:100;
}


uint8_t * ETHSPIClass::macAddress(uint8_t* mac)
{
 if(!mac) {
  return NULL;
 }
 
 esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac);

 return mac;
}

String ETHSPIClass::macAddress(void)
{
 uint8_t mac[6];
 char macStr[18];
 
 macAddress(mac);
 
 sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
 return String(macStr);
}

ETHSPIClass ETHSPI;

#endif