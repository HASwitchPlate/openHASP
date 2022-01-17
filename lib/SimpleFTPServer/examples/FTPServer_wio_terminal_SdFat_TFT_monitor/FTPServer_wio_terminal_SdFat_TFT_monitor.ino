/*
 * FtpServer Wio Terminal with SdFat library
 * and with callbacks to the main actions of FTP server
 * and a monitor on TFT
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/
 *
 */

#include "SdFat.h"

#include <rpcWiFi.h>


#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

#include <FtpServer.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

#define DEG2RAD 0.0174532925

byte inc = 0;
unsigned int col = 0;

#define SD_CONFIG SdSpiConfig(SDCARD_SS_PIN, 2)
SdFs sd;

FtpServer ftpSrv;

const char *ssid = "reef-casa-sopra";
const char *password = "aabbccdd77";

#define MAIN_TOP 110

#define FREE_SPACE_PIE_X 80
#define FREE_SPACE_PIE_Y MAIN_TOP+40
#define FREE_SPACE_PIE_RADIUS 50

void freeSpacePieData(unsigned int freeSpace, unsigned int totalSpace) {
	int pieFree = 360 - (freeSpace * 360 / totalSpace);

    fillSegment(FREE_SPACE_PIE_X, FREE_SPACE_PIE_Y, 0, pieFree, FREE_SPACE_PIE_RADIUS, TFT_RED);
    fillSegment(FREE_SPACE_PIE_X, FREE_SPACE_PIE_Y, pieFree, 360 - pieFree, FREE_SPACE_PIE_RADIUS, TFT_BLUE);

    // Set "cursor" at top left corner of display (0,0) and select font 2
    // (cursor will move to next line automatically during printing with 'tft.println'
    //  or stay on the line is there is room for the text with tft.print)
    tft.setCursor(FREE_SPACE_PIE_X + 80, MAIN_TOP, 2);
    // Set the font colour to be white with a black background, set text size multiplier to 1
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
    // We can now plot text on screen using the "print" class
    Serial.print(freeSpace/1000);Serial.print("Mb/");Serial.print(String(totalSpace/1000));Serial.println("Mb");
    tft.print(freeSpace/1000);tft.print("Mb/");tft.print(String(totalSpace/1000));tft.println("Mb");
}

void connectedDisconnected(bool connected) {
    tft.fillCircle(FREE_SPACE_PIE_X + 80 + 10, MAIN_TOP+25+7, 10, (connected)?TFT_GREEN:TFT_RED);

    tft.setCursor(FREE_SPACE_PIE_X + 80 + 25, MAIN_TOP+25, 2);
    tft.println("              ");

    tft.setCursor(FREE_SPACE_PIE_X + 80 + 25, MAIN_TOP+25, 2);
    (connected)?tft.println("Connected!"):tft.println("Disconnected!");
}

void transfer(bool transfer, bool upload) {
    tft.fillCircle(FREE_SPACE_PIE_X + 80 + 10, MAIN_TOP+25+25+7, 10, (transfer)?(upload)?TFT_GREEN:TFT_BLUE:TFT_RED);

    tft.setCursor(FREE_SPACE_PIE_X + 80 + 25, MAIN_TOP+25+25, 2);
    tft.println("              ");

    tft.setCursor(FREE_SPACE_PIE_X + 80 + 25, MAIN_TOP+25+25, 2);
    (transfer)?tft.println((upload)?"Upload!":"Download!"):tft.println("Idle!");
}

//index - starting at, n- how many chars
char* subString(const char *s, int index, int n){
	char* b = (char*) malloc((strlen(s) + 1) * sizeof(char));
	strcpy(b,s);

	Serial.println("--------------------------------------");
	Serial.println(s);
	Serial.println(index);
	Serial.println(n);
    char *res = new char[n + 1];
    Serial.println(res);
    sprintf(res, "%.*s", n, b + index);
    Serial.println(res);
    free(b);
    return res;
}


void fileTransfer(FtpTransferOperation ftpOperation, const char* filename, unsigned int transferredSize) {
	int yoffset = 2;

    tft.setCursor(20, MAIN_TOP+(FREE_SPACE_PIE_RADIUS*2)+yoffset, 2);
    tft.println(F("                          "));

	tft.setCursor(20, MAIN_TOP+(FREE_SPACE_PIE_RADIUS*2)+yoffset, 2);
	int lenfile = strlen(filename);
	Serial.println(lenfile);
    if (lenfile>22) {

		tft.print(subString(filename, 0, 16));tft.print(F("~"));
		tft.print( subString(filename, (lenfile-4), 4) );
    } else {
		tft.print(filename);
    }
	tft.setCursor(20+160, MAIN_TOP+(FREE_SPACE_PIE_RADIUS*2)+yoffset, 2);
	tft.print(F("                         "));
	tft.setCursor(20+160, MAIN_TOP+(FREE_SPACE_PIE_RADIUS*2)+yoffset, 2);
	tft.print(transferredSize);tft.print("Kb");

	tft.setCursor(320-55, MAIN_TOP+(FREE_SPACE_PIE_RADIUS*2)+yoffset, 2);
	switch (ftpOperation) {
	case FTP_UPLOAD:
	    		tft.setTextColor(TFT_GREEN, TFT_BLACK);
				tft.print(F("Upload"));
				tft.setTextColor(TFT_WHITE, TFT_BLACK);
				break;
	case FTP_DOWNLOAD:
				tft.setTextColor(TFT_BLUE, TFT_BLACK);
				tft.print(F("Down"));
				tft.setTextColor(TFT_WHITE, TFT_BLACK);

				break;
	case FTP_TRANSFER_STOP:
				tft.setTextColor(TFT_GREEN, TFT_BLACK);
				tft.print(F("OK"));
				tft.setTextColor(TFT_WHITE, TFT_BLACK);

				break;
	case FTP_TRANSFER_ERROR:
				tft.setTextColor(TFT_RED, TFT_BLACK);
				tft.print(F("Error"));
				tft.setTextColor(TFT_WHITE, TFT_BLACK);

				break;

		default:
			break;
	}

}

void wifiStrenght (int8_t RSSI, bool connection = false) {
	Serial.print("RSSI --> ");Serial.println(RSSI);
	int marginX = 30;

	int startX = 90;
	int widthW = 320-(startX+marginX);

	int startY = 60;
	int heightY = 10;

	tft.setCursor(marginX, startY - 5, 2);
	tft.print(F("                                         "));
	tft.setCursor(marginX, startY - 5, 2);

	if (connection) {
		tft.print(F("Connectint to: "));
		tft.print(ssid);
	}else{
		tft.println("WiFi str: ");

		// 120 : 120-RSSI = 300 : x

		tft.drawRoundRect(startX, startY, widthW, heightY, 5, TFT_WHITE);

		uint32_t colorRSSI = TFT_GREEN;
		if (abs(RSSI)<55) {
			colorRSSI = TFT_GREEN;
		} else if (abs(RSSI)<75) {
			colorRSSI = TFT_YELLOW;
		} else if (abs(RSSI)<75) {
			colorRSSI = TFT_RED;
		}

		tft.fillRoundRect(startX+1, startY+1, (120+RSSI)*widthW/120, heightY-2, 5, colorRSSI);

		tft.setCursor(marginX, startY + 15, 2);

		tft.print("IP: ");
		tft.println(WiFi.localIP());
	}
}

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace){
	Serial.print(">>>>>>>>>>>>>>> _callback " );
	Serial.print(ftpOperation);
	/* FTP_CONNECT,
	 * FTP_DISCONNECT,
	 * FTP_FREE_SPACE_CHANGE
	 */
	Serial.print(" ");
	Serial.print(freeSpace);
	Serial.print(" ");
	Serial.println(totalSpace);

	// freeSpace : totalSpace = x : 360

	freeSpacePieData(freeSpace, totalSpace);

	if (ftpOperation == FTP_CONNECT) connectedDisconnected(true);
	if (ftpOperation == FTP_DISCONNECT) connectedDisconnected(false);
};
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize){
	Serial.print(">>>>>>>>>>>>>>> _transferCallback " );
	Serial.print(ftpOperation);
	/* FTP_UPLOAD_START = 0,
	 * FTP_UPLOAD = 1,
	 *
	 * FTP_DOWNLOAD_START = 2,
	 * FTP_DOWNLOAD = 3,
	 *
	 * FTP_TRANSFER_STOP = 4,
	 * FTP_DOWNLOAD_STOP = 4,
	 * FTP_UPLOAD_STOP = 4,
	 *
	 * FTP_TRANSFER_ERROR = 5,
	 * FTP_DOWNLOAD_ERROR = 5,
	 * FTP_UPLOAD_ERROR = 5
	 */
	Serial.print(" ");
	Serial.print(name);
	Serial.print(" ");
	Serial.println(transferredSize);

	(ftpOperation==FTP_UPLOAD || ftpOperation==FTP_DOWNLOAD)?transfer(true, ftpOperation==FTP_UPLOAD):transfer(false, false);

	fileTransfer(ftpOperation, name, transferredSize);
};


void setup()
{
	ftpSrv.setCallback(_callback);
	ftpSrv.setTransferCallback(_transferCallback);

	Serial.begin(115200);
    delay(1000);

    tft.init();

    tft.begin();

    tft.setRotation(3);

    tft.fillScreen(TFT_BLACK);

	tft.setCursor(0, 0);

    tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(2);

    tft.fillRoundRect(3, 3, 320-6, 40, 5, TFT_WHITE);

    tft.drawCentreString("www.mischianti.org", 160, 14,1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);


	freeSpacePieData(0, 0);
	connectedDisconnected(false);
	transfer(false, false);

	wifiStrenght(0, true);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.mode(WIFI_STA);


    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        WiFi.begin(ssid, password);
        Serial.print(".");
        tft.print(F("."));
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    wifiStrenght(WiFi.RSSI());

    delay(1000);

    if (!sd.begin(SD_CONFIG)) {
      sd.initErrorHalt(&Serial);
    }
    FsFile dir;
    FsFile file;

    // Open root directory
    if (!dir.open("/")){
      Serial.println("dir.open failed");
    }

	ftpSrv.begin("wioterminal","wioterminal");    //username, password for ftp.

}

void loop() {
    ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}


// #########################################################################
// Draw circle segments
// #########################################################################

// x,y == coords of centre of circle
// start_angle = 0 - 359
// sub_angle   = 0 - 360 = subtended angle
// r = radius
// colour = 16 bit colour value

int fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour) {
    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Draw colour blocks every inc degrees
    for (int i = start_angle; i < start_angle + sub_angle; i++) {

        // Calculate pair of coordinates for segment end
        int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
        int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;

        tft.fillTriangle(x1, y1, x2, y2, x, y, colour);

        // Copy segment end to sgement start for next segment
        x1 = x2;
        y1 = y2;
    }
}


// #########################################################################
// Return the 16 bit colour with brightness 0-100%
// #########################################################################
unsigned int brightness(unsigned int colour, int brightness) {
    byte red   = colour >> 11;
    byte green = (colour & 0x7E0) >> 5;
    byte blue  = colour & 0x1F;

    blue = (blue * brightness) / 100;
    green = (green * brightness) / 100;
    red = (red * brightness) / 100;

    return (red << 11) + (green << 5) + blue;
}

