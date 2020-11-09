#ifndef HASP_CONF_H
#define HASP_CONF_H

#define HASP_USE_APP 1

/* Network Services */
#define HASP_HAS_NETWORK (ARDUINO_ARCH_ESP32 > 0 || ARDUINO_ARCH_ESP8266 > 0)

#ifndef HASP_USE_OTA
#define HASP_USE_OTA (HASP_HAS_NETWORK)
#endif

#ifndef HASP_USE_WIFI
#define HASP_USE_WIFI (HASP_HAS_NETWORK)
#endif

#ifndef HASP_USE_ETHERNET
#define HASP_USE_ETHERNET 0
#endif

#ifndef HASP_USE_MQTT
#define HASP_USE_MQTT 0
#endif

#ifndef HASP_USE_HTTP
#define HASP_USE_HTTP 0
#endif

#ifndef HASP_USE_MDNS
#define HASP_USE_MDNS (HASP_HAS_NETWORK)
#endif

#ifndef HASP_USE_SYSLOG
#define HASP_USE_SYSLOG 1
#endif

#ifndef HASP_USE_TELNET
#define HASP_USE_TELNET 0
#endif

/* Filesystem */
#define HASP_HAS_FILESYSTEM (ARDUINO_ARCH_ESP32 > 0 || ARDUINO_ARCH_ESP8266 > 0)

#ifndef HASP_USE_SPIFFS
#define HASP_USE_SPIFFS (HASP_HAS_FILESYSTEM)
#endif

#ifndef HASP_USE_EEPROM
#define HASP_USE_EEPROM 1
#endif

#ifndef HASP_USE_SDCARD
#define HASP_USE_SDCARD 0
#endif

#ifndef HASP_USE_GPIO
#define HASP_USE_GPIO 0
#endif

#ifndef HASP_USE_QRCODE
#define HASP_USE_QRCODE 1
#endif

#ifndef HASP_USE_PNGDECODE
#define HASP_USE_PNGDECODE 0
#endif

#ifndef HASP_NUM_GPIO_CONFIG
#define HASP_NUM_GPIO_CONFIG 5
#endif

#ifndef HASP_NUM_INPUTS
#define HASP_NUM_INPUTS 3 // Buttons
#endif

#ifndef HASP_NUM_OUTPUTS
#define HASP_NUM_OUTPUTS 3
#endif

#ifndef HASP_NUM_PAGES
#if defined(ARDUINO_ARCH_ESP8266)
#define HASP_NUM_PAGES 4
#else
#define HASP_NUM_PAGES 12
#endif
#endif

/* Includes */
#if HASP_USE_SPIFFS > 0
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#include "hasp_spiffs.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#include "lv_zifont.h"
#endif
#endif // SPIFFS

#if HASP_USE_EEPROM > 0
#include "hasp_eeprom.h"
#endif

#if HASP_USE_WIFI > 0
#include "hasp_wifi.h"
#endif

#if HASP_USE_ETHERNET > 0
#if defined(ARDUINO_ARCH_ESP32)
#include <ETH.h>

#define ETH_ADDR        0
#define ETH_POWER_PIN   -1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define NRST            5
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_CLKMODE     ETH_CLOCK_GPIO17_OUT

#include "hasp_ethernet_esp32.h"
#warning Using ESP32 Ethernet LAN8720

#else
#if USE_BUILTIN_ETHERNET > 0
#include <LwIP.h>
#include <STM32Ethernet.h>
#warning Use built-in STM32 Ethernet
#elif USE_UIP_ETHERNET
#include <UIPEthernet.h>
#include <utility/logging.h>
#warning Use ENC28J60 Ethernet shield
#else
#include "Ethernet.h"
#warning Use W5x00 Ethernet shield
#endif
#include "hasp_ethernet.h"
#endif
#endif

#if HASP_USE_MQTT > 0
#include "hasp_mqtt.h"
#endif

#if HASP_USE_HTTP > 0
#include "hasp_http.h"
#endif

#if HASP_USE_TELNET > 0
#include "hasp_telnet.h"
#endif

#if HASP_USE_MDNS > 0
#include "hasp_mdns.h"
#endif

#if HASP_USE_BUTTON > 0
#include "hasp_button.h"
#endif

#if HASP_USE_OTA > 0
#include "hasp_ota.h"
#ifndef HASP_OTA_PORT
#if defined(ARDUINO_ARCH_ESP32)
#define HASP_OTA_PORT 3232
#elif defined(ARDUINO_ARCH_ESP8266)
#define HASP_OTA_PORT 8266
#endif
#endif
#endif

#if HASP_USE_TASMOTA_SLAVE > 0
#include "hasp_slave.h"
#endif

#if HASP_USE_ETHERNET > 0
#include "hasp_ethernet.h"
#endif

#if HASP_USE_WIFI > 0 && defined(STM32F4xx)
#include "WiFiSpi.h"
static WiFiSpiClass WiFi;
#endif

#ifndef FPSTR
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif

#ifndef PGM_P
#define PGM_P const char *
#endif

#endif // HASP_CONF_H