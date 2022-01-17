/*
 * FtpServer Arduino with Ethernet library and w5100 shield
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include <SPI.h>
#include <Ethernet.h>
#include "SD.h"

#include <SimpleFtpServer.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE1 };

// Set the static IP address to use if the DHCP fails to assign
byte macAddr[] = {0x5e, 0xa4, 0x18, 0xf0, 0x8a, 0xf2};
IPAddress arduinoIP(192, 168, 1, 177);
IPAddress dnsIP(192, 168, 1, 1);
IPAddress gatewayIP(192, 168, 1, 1);
IPAddress subnetIP(255, 255, 255, 0);

FtpServer ftpSrv;

void setup(void){
  Serial.begin(115200);
  delay(2000);
  // If other chips are connected to SPI bus, set to high the pin connected
  // to their CS before initializing Flash memory
  pinMode( 4, OUTPUT );
  digitalWrite( 4, HIGH );
  pinMode( 10, OUTPUT );
  digitalWrite( 10, HIGH );

  Serial.print("Starting SD.");
  while (!SD.begin(4)) {
	  Serial.print(".");
  }
  Serial.println("finish!");

  // start the Ethernet connection:
  Serial.print("Starting ethernet.");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(macAddr, arduinoIP, dnsIP, gatewayIP, subnetIP);
  }else{
	Serial.println("ok to configure Ethernet using DHCP");
  }

  Serial.print("IP address ");
  Serial.println(Ethernet.localIP());

  Serial.println("SPIFFS opened!");
  ftpSrv.begin("esp8266","esp8266");    //username, password for ftp.
}
void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
}
