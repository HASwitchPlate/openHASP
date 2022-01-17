/*
 *  This sketch sends data via HTTP GET requests to examle.com service.
 */

#include "SdFat.h"

#include <rpcWiFi.h>

#include <FtpServer.h>

//	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD NETWORK_SEEED_RTL8720DN
//	#define DEFAULT_STORAGE_TYPE_SAMD STORAGE_SDFAT2

#define SD_CONFIG SdSpiConfig(SDCARD_SS_PIN, 2)
SdFs sd;

FtpServer ftpSrv;

const char *ssid = "<YOUR-SSID>";
const char *password = "<YOUR-PASSWD>";

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);

    // Initialize the SD.
    if (!sd.begin(SD_CONFIG)) {
      sd.initErrorHalt(&Serial);
    }
    FsFile dir;
    FsFile file;

    // Open root directory
    if (!dir.open("/")){
      Serial.println("dir.open failed");
    }

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.mode(WIFI_STA);


    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(1000);

    Serial.print("Starting SD.");

    Serial.println("finish!");

    while (file.openNext(&dir, O_RDONLY)) {
      file.printFileSize(&Serial);
      Serial.write(' ');
      file.printModifyDateTime(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if (file.isDir()) {
        // Indicate a directory.
        Serial.write('/');
      }
      Serial.println();
      file.close();
    }
    if (dir.getError()) {
      Serial.println("openNext failed");
    } else {
      Serial.println("Done!");
    }


    ftpSrv.begin("esp8266","esp8266");    //username, password for ftp.
  }

  void loop(void) {
  	  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
  }
