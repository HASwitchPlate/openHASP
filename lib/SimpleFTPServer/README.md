# SimpleFTPServer

[Instruction on FTP server on esp8266 and esp32](https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32)
[Simple FTP Server library now with support for Wio Terminal and SD](https://www.mischianti.org/2021/07/01/simple-ftp-server-library-now-with-support-for-wio-terminal-and-sd/)

Simple FTP Server for 
 - esp8266 (Flash: SPIFFs, LittleFS. SD: SD, SdFat 2)
 - esp32 (SPIFFS, LITTLEFS, FFAT, SdFat)
 - Arduino (SD with 8.3 file format, SdFat 2)
 - Wio Terminal (SdFat 2, and native FAT)

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