# SimpleFTPServer

[Instruction on FTP server on esp8266 and esp32](https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32)
[Simple FTP Server library now with support for Wio Terminal and SD](https://www.mischianti.org/2021/07/01/simple-ftp-server-library-now-with-support-for-wio-terminal-and-sd/)

#### Simple FTP Server for 
 - Raspberry Pi Pico W (Flash: LittleFS) (To test SD and SdFat)
 - esp8266 (Flash: SPIFFs, LittleFS. SD: SD, SdFat 2)
 - esp32 (SPIFFS, LITTLEFS, FFAT, SD: SD, SdFat)
 - stm32 (SdFat, SPI flash)
 - Arduino (SD with 8.3 file format, SD: SD, SdFat 2)
 - Wio Terminal (SdFat 2, Seed SD, and native FAT)

#### Changelog
- 2022-01-13 2.1.5 Fix SPIFM external SPI Flash date management (add SPIFM esp32 example)
- 2022-09-21 2.1.4 Add support for Raspberry Pi Pico W and rp2040 boards, Fix SD card config
- 2022-09-20 2.1.3 Soft AP IP management, more disconnect event and SD_MCC
- 2022-05-21 2.1.2 Fix SD path (#19)
- 2022-05-21 2.1.1 Minor fix
- 2022-03-30 2.1.0 Add UTF8 support and enabled It by default (Thanks to @plaber)
- 2022-03-30 2.0.0 Complete support for STM32 with SD and SPI Flash minor bux fix and HELP command support
- 2022-03-17 1.3.0 Fix enc28j60 and w5500 support and restructuring for local settings
- 2022-02-25 1.2.1 Fix anonymous user begin and fix SPIFFS wrong display
- 2022-02-22 1.2.0 Add anonymous user and implement correct RFC (#9 now work correctly with File Explorer)
- 2022-02-01 1.1.1 Add workaround to start FTP server before connection, add end and setLocalIP method.

<!-- wp:paragraph -->
<p>When I develop a new solution I'd like to divide the application in layer, and so I'd like focus my attention in only one aspect at time. </p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p> In detail I separate the REST layer (written inside the microcontroller) and the Front-End (written in Angular, React/Redux or vanilla JS), so I'd like to upload new web interface directly to the microcontroller via FTP. </p>
<!-- /wp:paragraph -->

<!-- wp:image {"align":"center","id":2155} -->
<div class="wp-block-image"><figure class="aligncenter"><img width="450px" src="https://www.mischianti.org/wp-content/uploads/2019/06/FTPTransferEsp8266-1024x662.jpg" alt="" class="wp-image-2155"/><figcaption></figcaption></figure></div>
<!-- /wp:image -->

<!-- wp:paragraph -->
<p>For static information (Web pages for examples), that not change frequently, esp8266 or esp32 have internal SPIFFS (SPI Flash File System) and you can upload data via Arduino IDE as explained in the article  "<a href="https://www.mischianti.org/2019/08/30/wemos-d1-mini-esp8266-integrated-spiffs-filesistem-part-2/">WeMos D1 mini (esp8266), integrated SPIFFS Filesystem</a>" for esp8266 or "<a rel="noreferrer noopener" href="https://www.mischianti.org/2020/06/04/esp32-integrated-spiffs-filesystem-part-2/" target="_blank">ESP32: integrated SPIFFS FileSystem</a>" for esp32 or with LittleFS "<a href="https://www.mischianti.org/2020/06/22/wemos-d1-mini-esp8266-integrated-littlefs-filesystem-part-5/">WeMos D1 mini (esp8266), integrated LittleFS Filesystem</a>" but for fast operation and future support It's usefully use FTP.</p>
<!-- /wp:paragraph -->



```cpp
/*
 * FtpServer esp8266 and esp32 with SPIFFS
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif

#include <SimpleFTPServer.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";


FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial


void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
  
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
#ifdef ESP32       //esp32 we send true to format spiffs if cannot mount
  if (SPIFFS.begin(true)) {
#elif defined ESP8266
  if (SPIFFS.begin()) {
#endif
      Serial.println("SPIFFS opened!");
      ftpSrv.begin("esp8266","esp8266");    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }    
}
void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
 // server.handleClient();   //example if running a webserver you still need to call .handleClient();
 
}
```

https://downloads.arduino.cc/libraries/logs/github.com/xreef/SimpleFTPServer/