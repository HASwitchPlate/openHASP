/*
 *  This sketch sends data via HTTP GET requests to examle.com service.
 */

#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

//	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_SAMD NETWORK_SEEED_RTL8720DN
//	#define DEFAULT_STORAGE_TYPE_SAMD STORAGE_SEEED_SD

#include <rpcWiFi.h>

#include <FtpServer.h>

FtpServer ftpSrv;

const char *ssid = "<YOUR-SSID>";
const char *password = "<YOUR-PASSWD>";

void listDir(const char* dirname, uint8_t levels) {
    Serial.print("Listing directory: ");
    Serial.println(dirname);

    File root = SD.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);

    while (!SD.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)) {
        Serial.println("Card Mount Failed");
        return;
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

    listDir("/", 0);

    ftpSrv.begin("esp8266","esp8266");    //username, password for ftp.
  }

  void loop(void) {
  	  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
  }
