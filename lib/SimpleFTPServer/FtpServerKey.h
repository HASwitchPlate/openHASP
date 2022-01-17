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
//#define FTP_SERVER_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

#define STORAGE_SDFAT1 0 // Library SdFat version 1.4.x
#define STORAGE_SDFAT2 1 // Library SdFat version >= 2.0.2
#define STORAGE_SPIFM  2 // Libraries Adafruit_SPIFlash and SdFat-Adafruit-Fork
#define STORAGE_FATFS  3 // Library FatFs
#define STORAGE_SD 4	 // Standard SD library (suitable for Arduino esp8266 and esp32
#define STORAGE_SPIFFS 5 // SPIFFS
#define STORAGE_LITTLEFS 6 // LITTLEFS
#define STORAGE_SEEED_SD 7 // Seeed_SD library
#define STORAGE_FFAT  8 // ESP32 FFAT

#define NETWORK_ESP8266_ASYNC (0)
#define NETWORK_ESP8266 (1)
#define NETWORK_ESP8266_242 (6)
#define NETWORK_W5100 (2)
#define NETWORK_ENC28J60 (3)
#define NETWORK_ESP32 (4)
#define NETWORK_ESP32_ETH (5)
#define NETWORK_WiFiNINA (7)
#define NETWORK_SEEED_RTL8720DN (8)

#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP8266 	NETWORK_ESP8266
	#define DEFAULT_STORAGE_TYPE_ESP8266 STORAGE_LITTLEFS
#endif
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32 	NETWORK_ESP32
	#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SPIFFS
#endif
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO 	NETWORK_W5100
	#define DEFAULT_STORAGE_TYPE_ARDUINO STORAGE_SDFAT2
#endif
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ARDUINO_SAMD
// Wio Terminal
//	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD NETWORK_SEEED_RTL8720DN
//	#define DEFAULT_STORAGE_TYPE_SAMD STORAGE_SEEED_SD
// Arduino SAMD
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD NETWORK_WiFiNINA // NETWORK_SEEED_RTL8720DN	// NETWORK_WiFiNINA
	#define DEFAULT_STORAGE_TYPE_SAMD STORAGE_SD // STORAGE_SDFAT2  // STORAGE_SD
#endif

//#define SD_CS_PIN 4
// Disconnect client after 5 minutes of inactivity (expressed in seconds)
#define FTP_TIME_OUT  5 * 60 


// Wait for authentication for 10 seconds (expressed in seconds)
#define FTP_AUTH_TIME_OUT 10


// Size of file buffer for read/write
// Transfer speed depends of this value
// Best value depends on many factors: SD card, client side OS, ... 
// But it can be reduced to 512 if memory usage is critical.
#define FTP_BUF_SIZE 2048 //1024 // 512

#endif // FTP_SERVER_CONFIG_H
