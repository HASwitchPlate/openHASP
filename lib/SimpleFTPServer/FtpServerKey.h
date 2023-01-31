/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form Jean-Michel Gallego version
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

/*******************************************************************************
 **                                                                            **
 **                         SETTINGS FOR FTP SERVER                            **
 **                                                                            **
 *******************************************************************************/

#ifndef FTP_SERVER_CONFIG_H
#define FTP_SERVER_CONFIG_H

// Uncomment to enable printing out nice debug messages.
// #define FTP_SERVER_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

#define STORAGE_SDFAT1 		1 	// Library SdFat version 1.4.x
#define STORAGE_SDFAT2 		2 	// Library SdFat version >= 2.0.2
#define STORAGE_SPIFM  		3 	// Libraries Adafruit_SPIFlash and SdFat-Adafruit-Fork
#define STORAGE_FATFS  		4 	// Library FatFs
#define STORAGE_SD 			5 	// Standard SD library (suitable for Arduino esp8266 and esp32
#define STORAGE_SPIFFS 		6 	// SPIFFS
#define STORAGE_LITTLEFS 	7 	// LITTLEFS
#define STORAGE_SEEED_SD 	8 	// Seeed_SD library
#define STORAGE_FFAT  		9 	// ESP32 FFAT
#define STORAGE_SD_MMC		10 	// SD_MMC library

#define NETWORK_ESP8266_ASYNC 	(1)
#define NETWORK_ESP8266 		(2) 	// Standard ESP8266WiFi
#define NETWORK_ESP8266_242 	(3) 	// ESP8266WiFi before 2.4.2 core
#define NETWORK_W5100 			(4)		// Standard Arduino Ethernet library
#define NETWORK_ENC28J60 		(5) 	// UIPEthernet library
#define NETWORK_ESP32 			(6) 	// Standard WiFi library
#define NETWORK_RP2040_WIFI		(6) 	// Raspberry Pi Pico W standard WiFi library
#define NETWORK_ESP32_ETH 		(7)		// Standard ETH library
#define NETWORK_WiFiNINA 		(8)		// Standard WiFiNINA library
#define NETWORK_SEEED_RTL8720DN (9) 	// Standard SEED WiFi library
#define NETWORK_ETHERNET_LARGE 	(10)
#define NETWORK_ETHERNET_ENC 	(11)	// EthernetENC library (evolution of UIPEthernet
#define NETWORK_ETHERNET_STM 	(12)
#define NETWORK_UIPETHERNET 	(13)	// UIPEthernet library same of NETWORK_ENC28J60

// esp8266 configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 	NETWORK_ESP8266
	#define DEFAULT_STORAGE_TYPE_ESP8266 				STORAGE_LITTLEFS
#endif
// esp32 configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32 		NETWORK_ESP32
	#define DEFAULT_STORAGE_TYPE_ESP32 					STORAGE_FFAT
#endif
// Standard AVR Arduino configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO 	NETWORK_W5100
	#define DEFAULT_STORAGE_TYPE_ARDUINO 				STORAGE_SD
#endif
// STM32 configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_STM32 		NETWORK_W5100
	#define DEFAULT_STORAGE_TYPE_STM32 					STORAGE_SDFAT2
#endif
// Raspberry Pi Pico (rp2040) configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_RP2040
    #define DEFAULT_FTP_SERVER_NETWORK_TYPE_RP2040 		NETWORK_RP2040_WIFI
	#define DEFAULT_STORAGE_TYPE_RP2040					STORAGE_LITTLEFS
#endif

// Arduino SAMD21 like Arduino MKR Nano 33 IoT or Wio Terminal
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO_SAMD
// Wio Terminal
//	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD NETWORK_SEEED_RTL8720DN
//	#define DEFAULT_STORAGE_TYPE_SAMD STORAGE_SEEED_SD

// Arduino SAMD
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD 		NETWORK_WiFiNINA
	#define DEFAULT_STORAGE_TYPE_SAMD 					STORAGE_SD
#endif

#define UTF8_SUPPORT

//#define SD_CS_PIN 4
// Disconnect client after 5 minutes of inactivity (expressed in seconds)
#define FTP_TIME_OUT  5 * 60 


// Wait for authentication for 10 seconds (expressed in seconds)
#define FTP_AUTH_TIME_OUT 10


// Size of file buffer for read/write
// Transfer speed depends of this value
// Best value depends on many factors: SD card, client side OS, ... 
// But it can be reduced to 512 if memory usage is critical.
#define FTP_BUF_SIZE 1024 //2048 //1024 // 512

#endif // FTP_SERVER_CONFIG_H
