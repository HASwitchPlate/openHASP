#ifndef HASP_CONF_H
#define HASP_CONF_H

#define HASP_VERSION_MAJOR 0
#define HASP_VERSION_MINOR 1
#define HASP_VERSION_REVISION 0

#define HASP_USE_APP 1

/* Network */
#define HASP_HAS_NETWORK (ARDUINO_ARCH_ESP32>0 || ARDUINO_ARCH_ESP8266>0)

#define HASP_USE_OTA (HASP_HAS_NETWORK)
#define HASP_USE_WIFI (HASP_HAS_NETWORK)
#define HASP_USE_MQTT (HASP_HAS_NETWORK)
#define HASP_USE_HTTP (HASP_HAS_NETWORK)
#define HASP_USE_MDNS (HASP_HAS_NETWORK)
#define HASP_USE_SYSLOG (HASP_HAS_NETWORK)
#define HASP_USE_TELNET (HASP_HAS_NETWORK)

/* Filesystem */
#define HASP_HAS_FILESYSTEM (ARDUINO_ARCH_ESP32>0 || ARDUINO_ARCH_ESP8266>0)

#define HASP_USE_SPIFFS (HASP_HAS_FILESYSTEM)
#define HASP_USE_EEPROM 1
#define HASP_USE_SDCARD 0

#define HASP_USE_GPIO 1

#define HASP_USE_QRCODE 1
#define HASP_USE_PNGDECODE 0

#define HASP_NUM_INPUTS 3 // Buttons
#define HASP_NUM_OUTPUTS 3

#if defined(ARDUINO_ARCH_ESP32)
#define HASP_NUM_PAGES 12
#else
#define HASP_NUM_PAGES 4
#endif


#if HASP_USE_SPIFFS>0
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h> // Include the SPIFFS library
#include "hasp_spiffs.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#include "lv_zifont.h"
#endif
#endif

#if HASP_USE_EEPROM>0
#include "hasp_eeprom.h"
#endif

#if HASP_USE_WIFI>0
#include "hasp_wifi.h"
#endif

#if HASP_USE_MQTT>0
#include "hasp_mqtt.h"
#endif

#if HASP_USE_HTTP>0
#include "hasp_http.h"
#endif

#if HASP_USE_TELNET>0
#include "hasp_telnet.h"
#endif

#if HASP_USE_MDNS>0
#include "hasp_mdns.h"
#endif

#if HASP_USE_BUTTON>0
#include "hasp_button.h"
#endif

#if HASP_USE_OTA>0
#include "hasp_ota.h"
#endif

#if HASP_USE_TASMOTA_SLAVE>0
#include "hasp_slave.h"
#endif

#endif

#ifndef FPSTR
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif
