//#include "webServer.h"
#include "Arduino.h"
#include "ArduinoJson.h"
//#include "Update.h"
#include "lvgl.h"

#include "hasp_conf.h"

#include "hasp_log.h"
#include "hasp_gui.h"
#include "hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_http.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_spiffs.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h>
#include <ESP.h>

bool httpEnable       = true;
bool webServerStarted = false;
uint16_t httpPort     = 80;
FS * filesystem       = &SPIFFS;
File fsUploadFile;
char httpUser[32]     = "";
char httpPassword[32] = "";
HTTPUpload * upload;
#define HTTP_PAGE_SIZE (6 * 256)

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
ESP8266WebServer webServer(80);
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
WebServer webServer(80);

#endif // ESP32

const char MAIN_MENU_BUTTON[] PROGMEM =
    "</p><p><form method='get' action='/'><button type='submit'>Main Menu</button></form>";
const char MIT_LICENSE[] PROGMEM = "</br>MIT License</p>";

const char HTTP_DOCTYPE[] PROGMEM =
    "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,"
    "user-scalable=no\"/>";
const char HTTP_META_GO_BACK[] PROGMEM = "<meta http-equiv='refresh' content='10;url=/'/>";
const char HTTP_HEADER[] PROGMEM       = "<title>%s</title>";
const char HTTP_STYLE[] PROGMEM =
    "<style>.c{text-align:center;}"
    "div,input{padding:5px;font-size:1em;}"
    "input{width:90%;}"
    "#hue{width:100%;}"
    "body{text-align:center;font-family:verdana;}"
    "button{border:0;border-radius:0.6rem;background-color:#1fb3ec;color:#eee;line-height:2.4rem;font-size:1.2rem;"
    "width:100%;}"
    ".q{float:right;width:64px;text-align:right;}"
    ".red{background-color:#f33;}"
    ".button3{background-color:#f44336;}"
    ".button4{background-color:#e7e7e7;color:black;}"
    ".button5{background-color:#555555;}"
    ".button6{background-color:#4CAF50;}</style>";
const char HTTP_SCRIPT[] PROGMEM = "<script>function "
                                   "c(l){document.getElementById('s').value=l.innerText||l.textContent;document."
                                   "getElementById('p').focus();}</script>";
const char HTTP_HEADER_END[] PROGMEM =
    "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_END[] PROGMEM = "<div style='text-align:right;font-size:11px;'><hr/><a href='/about' "
                                "style='color:#aaa;'>HASP ";
const char HTTP_FOOTER[] PROGMEM = " by Francis Van Roie</div></body></html>";

// Additional CSS style to match Hass theme
const char HASP_STYLE[] PROGMEM =
    "<style>button{background-color:#03A9F4;}body{width:60%;margin:auto;}input:invalid{border:"
    "1px solid red;}input[type=checkbox]{width:20px;}</style>";

////////////////////////////////////////////////////////////////////////////////////////////////////

// URL for auto-update "version.json"
const char UPDATE_URL[] PROGMEM = "http://haswitchplate.com/update/version.json";
// Default link to compiled Arduino firmware image
String espFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.ino.d1_mini.bin";
// Default link to compiled Nextion firmware images
String lcdFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.tft";

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleHaspConfig();

////////////////////////////////////////////////////////////////////////////////////////////////////
bool httpIsAuthenticated(const String & page)
{
    if(httpPassword[0] != '\0') { // Request HTTP auth if httpPassword is set
        if(!webServer.authenticate(httpUser, httpPassword)) {
            webServer.requestAuthentication();
            return false;
        }
    }

    char buffer[128];
    snprintf(buffer, sizeof(buffer), PSTR("HTTP: Sending %s page to client connected from: %s"), page.c_str(),
             webServer.client().remoteIP().toString().c_str());
    debugPrintln(buffer);
    return true;
}

String getOption(int value, String label, bool selected)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("<option value='%d'%s>%s</option>"), value,
               (selected ? PSTR(" selected") : ""), label.c_str());
    return buffer;
}
String getOption(String value, String label, bool selected)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("<option value='%s'%s>%s</option>"), value.c_str(),
               (selected ? PSTR(" selected") : ""), label.c_str());
    return buffer;
}
void webSendFooter()
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%u.%u.%u"), HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);

    webServer.sendContent_P(HTTP_END);
    webServer.sendContent(buffer);
    webServer.sendContent_P(HTTP_FOOTER);
}

void webSendPage(String & nodename, uint32_t httpdatalength, bool gohome = false)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%u.%u.%u"), HASP_VERSION_MAJOR, HASP_VERSION_MINOR, HASP_VERSION_REVISION);

    /* Calculate Content Length upfront */
    uint16_t contentLength = strlen(buffer); // verion length
    contentLength += sizeof(HTTP_DOCTYPE) - 1;
    contentLength += sizeof(HTTP_HEADER) - 1 - 2 + nodename.length();
    contentLength += sizeof(HTTP_SCRIPT) - 1;
    contentLength += sizeof(HTTP_STYLE) - 1;
    contentLength += sizeof(HASP_STYLE) - 1;
    if(gohome) contentLength += sizeof(HTTP_META_GO_BACK) - 1;
    contentLength += sizeof(HTTP_HEADER_END) - 1;
    contentLength += sizeof(HTTP_END) - 1;
    contentLength += sizeof(HTTP_FOOTER) - 1;

    snprintf_P(buffer, sizeof(buffer), PSTR("HTTP: Sending page with %u static and %u dynamic bytes"), contentLength,
               httpdatalength);
    debugPrintln(buffer);

    webServer.setContentLength(contentLength + httpdatalength);
    webServer.send_P(200, PSTR("text/html"), HTTP_DOCTYPE); // 122
    sprintf_P(buffer, HTTP_HEADER, nodename.c_str());
    webServer.sendContent(buffer);                         // 17-2+len
    webServer.sendContent_P(HTTP_SCRIPT);                  // 131
    webServer.sendContent_P(HTTP_STYLE);                   // 487
    webServer.sendContent_P(HASP_STYLE);                   // 145
    if(gohome) webServer.sendContent_P(HTTP_META_GO_BACK); // 47
    webServer.sendContent_P(HTTP_HEADER_END);              // 80
}

void webSendPage(uint32_t httpdatalength, bool gohome = false)
{
    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();
    webSendPage(nodename, httpdatalength, gohome);
}

void webHandleRoot()
{
    if(!httpIsAuthenticated(F("root"))) return;

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<p><form method='get' action='info'><button type='submit'>Information</button></form></p>");
    httpMessage += F("<p><form method='get' action='screenshot'><button type='submit'>Screenshot</button></form></p>");
    httpMessage +=
        PSTR("<p><form method='get' action='config'><button type='submit'>Configuration</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='firmware'><button type='submit'>Firmware Upgrade</button></form></p>");

    if(SPIFFS.exists(F("/edit.htm.gz"))) {
        httpMessage += F(
            "<p><form method='get' action='edit.htm.gz?path=/'><button type='submit'>File Browser</button></form></p>");
    }

    httpMessage +=
        F("<p><form method='get' action='reboot'><button class='red' type='submit'>Restart</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleReboot()
{ // http://plate01/reboot
    if(!httpIsAuthenticated(F("reboot"))) return;

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");
    httpMessage = F("Rebooting Device");

    webSendPage(nodename, httpMessage.length(), true);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();

    delay(200);
    dispatchReboot(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleScreenshot()
{ // http://plate01/screenshot
    if(!httpIsAuthenticated(F("screenshot"))) return;

    if(webServer.hasArg(F("q"))) {
        webServer.setContentLength(122 + 320 * 240 * 2);
        webServer.send(200, PSTR("image/bmp"), "");

        guiTakeScreenshot(webServer);
    } else {

        String nodename((char *)0);
        nodename.reserve(128);
        nodename = mqttGetNodename();

        String httpMessage((char *)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += nodename;
        httpMessage += F("</h1><hr>");

        httpMessage += F("<p class='c'><img id='bmp' src='?q=0'></p>");
        httpMessage += F("<p><form method='get' onsubmit=\"var timestamp = new Date().getTime();var ");
        httpMessage += F("el=document.getElementById('bmp');el.src='?q='+timestamp;return false;\">");
        httpMessage += F("<button type='submit'>Refresh</button></form></p>");
        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendPage(nodename, httpMessage.length(), false);
        webServer.sendContent(httpMessage);
        httpMessage.clear();
        webSendFooter();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void webHandleAbout()
{ // http://plate01/about
    if(!httpIsAuthenticated(F("about"))) return;

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);

    httpMessage += F("<p><h3>HASP OpenHardware edition</h3>Copyright&copy; 2020 Francis Van Roie ");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p>Based on the previous work of the following open source developers.</p><hr>");
    httpMessage += F("<p><h3>HASwitchPlate</h3>Copyright&copy; 2019 Allen Derusha allen@derusha.org</b>");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage +=
        F("<p><h3>LittlevGL</h3>Copyright&copy; 2016 G&aacute;bor Kiss-V&aacute;mosi</br>Copyright&copy; 2019 "
          "LittlevGL");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p><h3>zi Font Engine</h3>Copyright&copy; 2020 Francis Van Roie");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p><h3>TFT_eSPI Library</h3>Copyright&copy; 2017 Bodmer (https://github.com/Bodmer) All "
                     "rights reserved.</br>FreeBSD License</p>");
    httpMessage +=
        F("<p><i>includes parts from the <b>Adafruit_GFX library</b></br>Copyright&copy; 2012 Adafruit Industries. "
          "All rights reserved</br>BSD License</i></p>");
    httpMessage += F("<p><h3>ArduinoJson</h3>Copyright&copy; 2014-2019 Benoit BLANCHON");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p><h3>PubSubClient</h3>Copyright&copy; 2008-2015 Nicholas O'Leary");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p><h3>Syslog</h3>Copyright&copy; 2016 Martin Sloup");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p><h3>QR Code generator</h3>Copyright&copy; Project Nayuki");
    httpMessage += FPSTR(MIT_LICENSE);
    httpMessage += F("<p><h3>AceButton</h3>Copyright&copy; 2018 Brian T. Park");
    httpMessage += FPSTR(MIT_LICENSE);

    httpMessage += FPSTR(MAIN_MENU_BUTTON);

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleInfo()
{ // http://plate01/
    if(!httpIsAuthenticated(F("info"))) return;

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    /* HASP Stats */
    httpMessage += F("<b>HASP Version: </b>");
    httpMessage += String(haspGetVersion());
    httpMessage += F("<br/><b>Build DateTime: </b>");
    httpMessage += __DATE__;
    httpMessage += F(" ");
    httpMessage += __TIME__;
    httpMessage += F(" CET<br/><b>Uptime: </b>");
    httpMessage += String(long(millis() / 1000));
    httpMessage += F("<br/><b>Free Memory: </b>");
    httpMessage += spiffsFormatBytes(ESP.getFreeHeap());
    httpMessage += F("<br/><b>Memory Fragmentation: </b>");
    httpMessage += String(halGetHeapFragmentation());

    /* LVGL Stats */
    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);
    httpMessage += F("</p><p><b>LVGL Memory: </b>");
    httpMessage += spiffsFormatBytes(mem_mon.total_size);
    httpMessage += F("<br/><b>LVGL Free: </b>");
    httpMessage += spiffsFormatBytes(mem_mon.free_size);
    httpMessage += F("<br/><b>LVGL Fragmentation: </b>");
    httpMessage += mem_mon.frag_pct;

    // httpMessage += F("<br/><b>LCD Model: </b>")) + String(LV_HASP_HOR_RES_MAX) + " x " +
    // String(LV_HASP_VER_RES_MAX); httpMessage += F("<br/><b>LCD Version: </b>")) + String(lcdVersion);
    httpMessage += F("</p/><p><b>LCD Active Page: </b>");
    httpMessage += String(haspGetPage());

    /* Wifi Stats */
    httpMessage += F("</p/><p><b>SSID: </b>");
    httpMessage += String(WiFi.SSID());
    httpMessage += F("</br><b>Signal Strength: </b>");
    httpMessage += String(WiFi.RSSI());
    httpMessage += F("</br><b>IP Address: </b>");
    httpMessage += String(WiFi.localIP().toString());
    httpMessage += F("</br><b>Gateway: </b>");
    httpMessage += String(WiFi.gatewayIP().toString());
    httpMessage += F("</br><b>DNS Server: </b>");
    httpMessage += String(WiFi.dnsIP().toString());
    httpMessage += F("</br><b>MAC Address: </b>");
    httpMessage += String(WiFi.macAddress());

    /* Mqtt Stats */
    httpMessage += F("</p/><p><b>MQTT Status: </b>");
    if(mqttIsConnected()) { // Check MQTT connection
        httpMessage += F("Connected");
    } else {
        httpMessage += F("<font color='red'><b>Disconnected</b></font>, return code: ");
        //     +String(mqttClient.returnCode());
    }
    httpMessage += F("<br/><b>MQTT ClientID: </b>");
    httpMessage += nodename;

    /* ESP Stats */
    httpMessage += F("</p/><p><b>ESP Chip Id: </b>");
#if defined(ARDUINO_ARCH_ESP32)
    httpMessage += String(ESP.getChipRevision());
#else
    httpMessage += String(ESP.getChipId());
#endif
    httpMessage += F("<br/><b>CPU Frequency: </b>");
    httpMessage += String(ESP.getCpuFreqMHz());
    httpMessage += F("MHz<br/><b>Flash Chip Size: </b>");
    httpMessage += spiffsFormatBytes(ESP.getFlashChipSize());
    httpMessage += F("</br><b>Program Size: </b>");
    httpMessage += spiffsFormatBytes(ESP.getSketchSize());
    httpMessage += F("<br/><b>Free Program Space: </b>");
    httpMessage += spiffsFormatBytes(ESP.getFreeSketchSpace());

#if defined(ARDUINO_ARCH_ESP32)
    httpMessage += F("<br/><b>ESP SDK version: </b>");
    httpMessage += String(ESP.getSdkVersion());
#else
    httpMessage += F("<br/><b>ESP Core version: </b>");
    httpMessage += String(ESP.getCoreVersion());
#endif
    httpMessage += F("<br/><b>Last Reset: </b>");
    httpMessage += halGetResetInfo();

    httpMessage += FPSTR(MAIN_MENU_BUTTON);

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}

String getContentType(String filename)
{
    if(webServer.hasArg(F("download"))) {
        return F("application/octet-stream");
    } else if(filename.endsWith(F(".htm")) || filename.endsWith(F(".html"))) {
        return F("text/html");
    } else if(filename.endsWith(F(".css"))) {
        return F("text/css");
    } else if(filename.endsWith(F(".js"))) {
        return F("application/javascript");
    } else if(filename.endsWith(F(".png"))) {
        return F("image/png");
    } else if(filename.endsWith(F(".gif"))) {
        return F("image/gif");
    } else if(filename.endsWith(F(".jpg"))) {
        return F("image/jpeg");
    } else if(filename.endsWith(F(".ico"))) {
        return F("image/x-icon");
    } else if(filename.endsWith(F(".xml"))) {
        return F("text/xml");
    } else if(filename.endsWith(F(".pdf"))) {
        return F("application/x-pdf");
    } else if(filename.endsWith(F(".zip"))) {
        return F("application/x-zip");
    } else if(filename.endsWith(F(".gz"))) {
        return F("application/x-gzip");
    }
    return F("text/plain");
}

/* String urldecode(String str)
{
    String encodedString = "";
    char c;
    for(unsigned int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if(c == '+') {
            encodedString += ' ';
        } else if(c == '%') {
            // char buffer[3];
            char buffer[128];
            i++;
            buffer[0] = str.charAt(i);
            i++;
            buffer[1] = str.charAt(i);
            buffer[2] = '\0';
            c         = (char)strtol((const char *)&buffer, NULL, 16);
            encodedString += c;
        } else {
            encodedString += c;
        }
        yield();
    }
    return encodedString;
} */

bool handleFileRead(String path)
{
    if(!httpIsAuthenticated(F("fileread"))) return false;

    // path = urldecode(path).substring(0, 31);
    path = webServer.urlDecode(path);
    if(!httpIsAuthenticated(path)) return false;

    if(path.endsWith("/")) {
        path += F("index.htm");
    }
    String pathWithGz = path + F(".gz");
    if(filesystem->exists(pathWithGz) || filesystem->exists(path)) {
        if(filesystem->exists(pathWithGz)) path += F(".gz");

        File file          = filesystem->open(path, "r");
        String contentType = getContentType(path);
        if(path == F("/edit.htm.gz")) {
            contentType = F("text/html");
        }
        webServer.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

/*
void handleFirmwareUpdate()
{
    upload = &webServer.upload();
    if(upload->status == UPLOAD_FILE_START) {
        if(!httpIsAuthenticated(F("firmwareupdate"))) return false;
        Serial.printf("Update: %s\n", upload->filename.c_str());
        if(!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
            Update.printError(Serial);
        }
    } else if(upload->status == UPLOAD_FILE_WRITE) {
        // flashing firmware to /
if(Update.write(upload->buf, upload->currentSize) != upload->currentSize) {
    Update.printError(Serial);
}
}
else if(upload->status == UPLOAD_FILE_END)
{
    if(Update.end(true)) { // true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload->totalSize);
    } else {
        Update.printError(Serial);
    }
}
}
*/

void handleFileUpload()
{
    if(webServer.uri() != "/edit") {
        return;
    }
    upload = &webServer.upload();
    if(upload->status == UPLOAD_FILE_START) {
        if(!httpIsAuthenticated(F("fileupload"))) return;
        String filename((char *)0);
        filename.reserve(128);
        filename = upload->filename;
        if(!filename.startsWith("/")) {
            filename = "/";
            filename += upload->filename;
        }
        if(filename.length() < 32) {
            fsUploadFile = filesystem->open(filename, "w");
            filename     = String(F("handleFileUpload Name: ")) + filename;
            debugPrintln(filename);
        } else {
            filename = String(F("%sFilename is too long: ")) + filename;
            errorPrintln(filename);
        }
    } else if(upload->status == UPLOAD_FILE_WRITE) {
        // DBG_OUTPUT_PORT.print("handleFileUpload Data: "); debugPrintln(upload.currentSize);
        if(fsUploadFile) {
            fsUploadFile.write(upload->buf, upload->currentSize);
            char buffer[128];
            sprintf_P(buffer, PSTR("    * Uploaded %u bytes"), upload->totalSize + upload->currentSize);
            debugPrintln(buffer);
        }
    } else if(upload->status == UPLOAD_FILE_END) {
        if(fsUploadFile) {
            char buffer[128];
            sprintf_P(buffer, PSTR("Uploaded %s (%u bytes)"), fsUploadFile.name(), upload->totalSize);
            debugPrintln(buffer);
            fsUploadFile.close();
        }

        // Redirect to /config/hasp page. This flushes the web buffer and frees the memory
        webServer.sendHeader(String(F("Location")), String(F("/config/hasp")), true);
        webServer.send_P(302, PSTR("text/plain"), "");
        // httpReconnect();
    }
}

void handleFileDelete()
{
    if(!httpIsAuthenticated(F("filedelete"))) return;

    char mimetype[128];
    sprintf(mimetype, PSTR("text/plain"));

    if(webServer.args() == 0) {
        return webServer.send_P(500, mimetype, PSTR("BAD ARGS"));
    }
    String path = webServer.arg(0);
    debugPrintln(String(F("handleFileDelete: ")) + path);
    if(path == "/") {
        return webServer.send_P(500, mimetype, PSTR("BAD PATH"));
    }
    if(!filesystem->exists(path)) {
        return webServer.send_P(404, mimetype, PSTR("FileNotFound"));
    }
    filesystem->remove(path);
    webServer.send_P(200, mimetype, PSTR(""));
    path.clear();
}

void handleFileCreate()
{
    if(!httpIsAuthenticated(F("filecreate"))) return;

    if(webServer.args() == 0) {
        return webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
    }
    String path = webServer.arg(0);
    debugPrintln(String(F("handleFileCreate: ")) + path);
    if(path == "/") {
        return webServer.send(500, PSTR("text/plain"), PSTR("BAD PATH"));
    }
    if(filesystem->exists(path)) {
        return webServer.send(500, PSTR("text/plain"), PSTR("FILE EXISTS"));
    }
    File file = filesystem->open(path, "w");
    if(file) {
        file.close();
    } else {
        return webServer.send(500, PSTR("text/plain"), PSTR("CREATE FAILED"));
    }
    webServer.send(200, PSTR("text/plain"), "");
    path.clear();
}

void handleFileList()
{
    if(!httpIsAuthenticated(F("filelist"))) return;

    if(!webServer.hasArg(F("dir"))) {
        webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
        return;
    }

    String path = webServer.arg(F("dir"));
    debugPrintln(String(F("handleFileList: ")) + path);
    path.clear();

#if defined(ARDUINO_ARCH_ESP32)
    File root     = SPIFFS.open("/");
    File file     = root.openNextFile();
    String output = "[";

    while(file) {
        if(output != "[") {
            output += ',';
        }
        bool isDir = false;
        output += F("{\"type\":\"");
        output += (isDir) ? F("dir") : F("file");
        output += F("\",\"name\":\"");
        if(file.name()[0] == '/') {
            output += &(file.name()[1]);
        } else {
            output += file.name();
        }
        output += F("\"}");

        // file.close();
        file = root.openNextFile();
    }
    output += "]";
#else
    Dir dir       = filesystem->openDir(path);
    String output = "[";
    while(dir.next()) {
        File entry = dir.openFile("r");
        if(output != "[") {
            output += ',';
        }
        bool isDir = false;
        output += F("{\"type\":\"");
        output += (isDir) ? F("dir") : F("file");
        output += F("\",\"name\":\"");
        if(entry.name()[0] == '/') {
            output += &(entry.name()[1]);
        } else {
            output += entry.name();
        }
        output += F("\"}");
        entry.close();
    }
    output += "]";
#endif
    webServer.send(200, PSTR("text/json"), output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleConfig()
{ // http://plate01/config
    if(!httpIsAuthenticated(F("config"))) return;

    if(webServer.method() == HTTP_POST) {
        if(webServer.hasArg(PSTR("save"))) {
            String save = webServer.arg(PSTR("save"));

            DynamicJsonDocument settings(256);
            for(int i = 0; i < webServer.args(); i++) settings[webServer.argName(i)] = webServer.arg(i);

            if(save == String(PSTR("hasp"))) {
                haspSetConfig(settings.as<JsonObject>());

            } else if(save == String(PSTR("mqtt"))) {
                mqttSetConfig(settings.as<JsonObject>());

            } else if(save == String(PSTR("gui"))) {
                guiSetConfig(settings.as<JsonObject>());

            } else if(save == String(PSTR("debug"))) {
                debugSetConfig(settings.as<JsonObject>());

            } else if(save == String(PSTR("http"))) {
                httpSetConfig(settings.as<JsonObject>());

                // Password might have changed
                if(!httpIsAuthenticated(F("config"))) return;

            } else if(save == String(PSTR("wifi"))) {
                wifiSetConfig(settings.as<JsonObject>());
            }
        }
    }

    if(WiFi.getMode() == WIFI_AP) {
        httpHandleReboot();
    }

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage +=
        F("<p><form method='get' action='/config/wifi'><button type='submit'>Wifi Settings</button></form></p>");

#if HASP_USE_MQTT > 0
    httpMessage +=
        F("<p><form method='get' action='/config/mqtt'><button type='submit'>MQTT Settings</button></form></p>");
#endif

    httpMessage +=
        F("<p><form method='get' action='/config/http'><button type='submit'>HTTP Settings</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='/config/gui'><button type='submit'>Display Settings</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='/config/hasp'><button type='submit'>HASP Settings</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='/config/debug'><button type='submit'>Debug Settings</button></form></p>");

    httpMessage += F("<p><form method='get' action='resetConfig'><button class='red' type='submit'>Factory Reset"
                     "</button></form>");

    httpMessage += FPSTR(MAIN_MENU_BUTTON);
    ;

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_MQTT > 0
void webHandleMqttConfig()
{ // http://plate01/config/mqtt
    if(!httpIsAuthenticated(F("config/mqtt"))) return;

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    DynamicJsonDocument settings(256);
    mqttGetConfig(settings.to<JsonObject>());

    // char buffer[128];
    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<form method='POST' action='/config'>");
    httpMessage += F("<b>HASP Node Name</b> <i><small>(required. lowercase letters, numbers, and _ only)</small>"
                     "</i><input id='name' required name='name' maxlength=15 "
                     "placeholder='HASP Node Name' pattern='[a-z0-9_]*' value='");
    httpMessage += settings[FPSTR(F_CONFIG_NAME)].as<String>();
    httpMessage += F("'><br/><br/><b>Group Name</b> <i><small>(required)</small></i><input id='group' required "
                     "name='group' maxlength=15 placeholder='Group Name' value='");
    httpMessage += settings[FPSTR(F_CONFIG_GROUP)].as<String>();
    httpMessage += F("'><br/><br/><b>MQTT Broker</b> <i><small>(required)</small></i><input id='host' required "
                     "name='host' maxlength=63 placeholder='mqttServer' value='");
    httpMessage += settings[FPSTR(F_CONFIG_HOST)].as<String>();
    httpMessage += F("'><br/><b>MQTT Port</b> <i><small>(required)</small></i><input id='port' required "
                     "name='port' type='number' maxlength=5 placeholder='mqttPort' value='");
    httpMessage += settings[FPSTR(F_CONFIG_PORT)].as<uint16_t>();
    httpMessage += F("'><br/><b>MQTT User</b> <i><small>(optional)</small></i><input id='mqttUser' name='user' "
                     "maxlength=31 placeholder='user' value='");
    httpMessage += settings[FPSTR(F_CONFIG_USER)].as<String>();
    httpMessage += F("'><br/><b>MQTT Password</b> <i><small>(optional)</small></i><input id='pass' "
                     "name='pass' type='password' maxlength=31 placeholder='mqttPassword' value='");
    if(settings[FPSTR(F_CONFIG_PASS)].as<String>() != "") httpMessage += F("********");

    httpMessage += F("'><p><button type='submit' name='save' value='mqtt'>Save Settings</button></form></p>");
    httpMessage +=
        PSTR("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleGuiConfig()
{ // http://plate01/config/wifi
    if(!httpIsAuthenticated(F("config/gui"))) return;

    DynamicJsonDocument settings(256);
    guiGetConfig(settings.to<JsonObject>());

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<form method='POST' action='/config'>");

    httpMessage += F("<p><b>Short Idle</b> <input id='idle1' required "
                     "name='idle1' type='number' min='0' max='32400' value='");
    httpMessage += settings[FPSTR(F_GUI_IDLEPERIOD1)].as<String>();
    httpMessage += F("'></p>");

    httpMessage += F("<p><b>Long Idle</b> <input id='idle2' required "
                     "name='idle2' type='number' min='0' max='32400' value='");
    httpMessage += settings[FPSTR(F_GUI_IDLEPERIOD2)].as<String>();
    httpMessage += F("'></p>");

    int8_t rotation = settings[FPSTR(F_GUI_ROTATION)].as<int8_t>();
    httpMessage += F("<p><b>Orientation</b> <select id='rotation' name='rotation'>");
    httpMessage += getOption(0, F("0 degrees"), rotation == 0);
    httpMessage += getOption(1, F("90 degrees"), rotation == 1);
    httpMessage += getOption(2, F("180 degrees"), rotation == 2);
    httpMessage += getOption(3, F("270 degrees"), rotation == 3);
    httpMessage += getOption(6, F("0 degrees - mirrored"), rotation == 6);
    httpMessage += getOption(7, F("90 degrees - mirrored"), rotation == 7);
    httpMessage += getOption(4, F("180 degrees - mirrored"), rotation == 4);
    httpMessage += getOption(5, F("270 degrees - mirrored"), rotation == 5);
    httpMessage += F("</select></p>");

    int8_t bcklpin = settings[FPSTR(F_GUI_BACKLIGHTPIN)].as<int8_t>();
    httpMessage += F("<p><b>Backlight Control</b> <select id='bcklpin' name='bcklpin'>");
    httpMessage += getOption(-1, F("None"), bcklpin == -1);
#if defined(ARDUINO_ARCH_ESP32)
    httpMessage += getOption(16, F("GPIO 16"), bcklpin == 16);
    httpMessage += getOption(17, F("GPIO 17"), bcklpin == 17);
    httpMessage += getOption(21, F("GPIO 21"), bcklpin == 21);
    httpMessage += getOption(22, F("GPIO 22"), bcklpin == 22);
#else
    httpMessage += getOption(5, F("D1 - GPIO 5"), bcklpin == 5);
    httpMessage += getOption(4, F("D2 - GPIO 4"), bcklpin == 4);
    httpMessage += getOption(0, F("D3 - GPIO 0"), bcklpin == 0);
    httpMessage += getOption(2, F("D4 - GPIO 2"), bcklpin == 2);
#endif
    httpMessage += F("</select></p>");

    httpMessage += F("<p><button type='submit' name='save' value='gui'>Save Settings</button></p></form>");

    httpMessage += PSTR("<p><form method='get' action='/config/gui'><button type='submit' name='action' "
                        "value='calibrate'>Calibrate</button></form></p>");

    httpMessage +=
        PSTR("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();

    if(webServer.hasArg(F("action"))) dispatchCommand(webServer.arg(F("action")));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_WIFI > 0
void webHandleWifiConfig()
{ // http://plate01/config/wifi
    if(!httpIsAuthenticated(F("config/wifi"))) return;

    DynamicJsonDocument settings(256);
    wifiGetConfig(settings.to<JsonObject>());

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<form method='POST' action='/config'>");
    httpMessage += F("<b>WiFi SSID</b> <i><small>(required)</small></i><input id='ssid' required "
                     "name='ssid' maxlength=31 placeholder='WiFi SSID' value='");
    httpMessage += settings[FPSTR(F_CONFIG_SSID)].as<String>();
    httpMessage += F("'><br/><b>WiFi Password</b> <i><small>(required)</small></i><input id='pass' required "
                     "name='pass' type='password' maxlength=63 placeholder='WiFi Password' value='");
    if(settings[FPSTR(F_CONFIG_PASS)].as<String>() != "") {
        httpMessage += F("********");
    }
    httpMessage += F("'><p><button type='submit' name='save' value='wifi'>Save Settings</button></p></form>");

    if(WiFi.getMode() == WIFI_STA) {
        httpMessage +=
            PSTR("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");
    }

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_HTTP > 0
void webHandleHttpConfig()
{ // http://plate01/config/http
    if(!httpIsAuthenticated(F("config/http"))) return;

    DynamicJsonDocument settings(256);
    httpGetConfig(settings.to<JsonObject>());

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<form method='POST' action='/config'>");
    httpMessage += F("<b>Web Username</b> <i><small>(optional)</small></i><input id='user' "
                     "name='user' maxlength=31 placeholder='admin' value='");
    httpMessage += settings[FPSTR(F_CONFIG_USER)].as<String>();
    httpMessage += F("'><br/><b>Web Password</b> <i><small>(optional)</small></i><input id='pass' "
                     "name='pass' type='password' maxlength=63 placeholder='Password' value='");
    if(settings[FPSTR(F_CONFIG_PASS)].as<String>() != "") {
        httpMessage += F("********");
    }
    httpMessage += F("'><p><button type='submit' name='save' value='http'>Save Settings</button></p></form>");

    httpMessage +=
        PSTR("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleDebugConfig()
{ // http://plate01/config/http
    if(!httpIsAuthenticated(F("config/debug"))) return;

    DynamicJsonDocument settings(256);
    debugGetConfig(settings.to<JsonObject>());

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<form method='POST' action='/config'>");
    httpMessage += F("<p><b>Telemetry Period</b> <input id='teleperiod' required "
                     "name='idle1' type='number' min='0' max='65535' value='");
    httpMessage += settings[FPSTR(F_DEBUG_TELEPERIOD)].as<String>();
    httpMessage += F("'></p>");
    httpMessage += F("<p><button type='submit' name='save' value='debug'>Save Settings</button></p></form>");

    httpMessage +=
        PSTR("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleHaspConfig()
{ // http://plate01/config/http
    if(!httpIsAuthenticated(F("config/hasp"))) return;

    DynamicJsonDocument settings(256);
    haspGetConfig(settings.to<JsonObject>());

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<p><form action='/edit' method='post' enctype='multipart/form-data'><input type='file' "
                     "name='filename' accept='.jsonl,.zi'>");
    httpMessage += F("<button type='submit'>Upload File</button></form></p><hr>");

    httpMessage += F("<form method='POST' action='/config'>");
    httpMessage += F("<p><b>UI Theme</b> <i><small>(required)</small></i><select id='theme' name='theme'>");

    uint8_t themeid = settings[FPSTR(F_CONFIG_THEME)].as<uint8_t>();
    httpMessage += getOption(0, F("Built-in"), themeid == 0);
    httpMessage += getOption(8, F("Hasp"), themeid == 8);
#if LV_USE_THEME_ALIEN == 1
    httpMessage += getOption(1, F("Alien"), themeid == 1);
#endif
#if LV_USE_THEME_NIGHT == 1
    httpMessage += getOption(2, F("Night"), themeid == 2);
#endif
#if LV_USE_THEME_MONO == 1
    httpMessage += getOption(3, F("Mono"), themeid == 3);
#endif
#if LV_USE_THEME_MATERIAL == 1
    httpMessage += getOption(4, F("Material"), themeid == 4);
#endif
#if LV_USE_THEME_ZEN == 1
    httpMessage += getOption(5, F("Zen"), themeid == 5);
#endif
#if LV_USE_THEME_NEMO == 1
    httpMessage += getOption(6, F("Nemo"), themeid == 6);
#endif
#if LV_USE_THEME_TEMPL == 1
    httpMessage += getOption(7, F("Template"), themeid == 7);
#endif
    httpMessage += F("</select></br>");
    httpMessage +=
        F("<b>Hue</b><div style='width:100%;background-image:linear-gradient(to "
          "right,red,orange,yellow,green,blue,indigo,violet);'><input style='align:center;padding:0px' id='hue' "
          "name='hue' type='range' min='0' max='360' value='");
    httpMessage += settings[FPSTR(F_CONFIG_HUE)].as<String>();
    httpMessage += F("'></div></p>");
    httpMessage += F("<p><b>Default Font</b><select id='font' name='font'><option value=''>None</option>");

#if defined(ARDUINO_ARCH_ESP32)
    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while(file) {
        String filename = file.name();
        if(filename.endsWith(".zi"))
            httpMessage +=
                getOption(file.name(), file.name(), filename == settings[FPSTR(F_CONFIG_ZIFONT)].as<String>());
        file = root.openNextFile();
    }
#else
    Dir dir = filesystem->openDir("/");
    while(dir.next()) {
        File file       = dir.openFile("r");
        String filename = file.name();
        if(filename.endsWith(".zi"))
            httpMessage +=
                getOption(file.name(), file.name(), filename == settings[FPSTR(F_CONFIG_ZIFONT)].as<String>());
        file.close();
    }
#endif
    httpMessage += F("</select></p>");

    httpMessage += F("<p><b>Startup Layout</b> <i><small>(optional)</small></i><input id='pages' "
                     "name='pages' maxlength=31 placeholder='/pages.jsonl' value='");

    httpMessage += settings[FPSTR(F_CONFIG_PAGES)].as<String>();
    httpMessage += F("'></br><b>Startup Page</b> <i><small>(required)</small></i><input id='startpage' required "
                     "name='startpage' type='number' min='0' max='3' value='");
    httpMessage += settings[FPSTR(F_CONFIG_STARTPAGE)].as<String>();
    httpMessage +=
        F("'></p><p><b>Startup Brightness</b> <i><small>(required)</small></i><input id='startpage' required "
          "name='startdim' type='number' min='0' max='100' value='");
    httpMessage += settings[FPSTR(F_CONFIG_STARTDIM)].as<String>();
    httpMessage += F("'></p>");

    httpMessage += F("<p><button type='submit' name='save' value='hasp'>Save Settings</button></form></p>");

    httpMessage += F("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleNotFound()
{ // webServer 404
    if(handleFileRead(webServer.uri())) return;

    debugPrintln(String(F("HTTP: Sending 404 to client connected from: ")) + webServer.client().remoteIP().toString());

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);

    httpMessage += F("File Not Found\n\nURI: ");
    httpMessage += webServer.uri();
    httpMessage += F("\nMethod: ");
    httpMessage += (webServer.method() == HTTP_GET) ? F("GET") : F("POST");
    httpMessage += F("\nArguments: ");
    httpMessage += webServer.args();
    httpMessage += "\n";
    for(uint8_t i = 0; i < webServer.args(); i++) {
        httpMessage += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }
    webServer.send(404, PSTR("text/plain"), httpMessage.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleSaveConfig()
{
    if(!httpIsAuthenticated(F("saveConfig"))) return;

    configWriteConfig();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleFirmware()
{
    if(!httpIsAuthenticated(F("firmware"))) return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleEspFirmware()
{ // http://plate01/espfirmware
    if(!httpIsAuthenticated(F("espfirmware"))) return;

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    httpMessage += F("<p><b>ESP update</b></p>Updating ESP firmware from: ");
    httpMessage += webServer.arg("espFirmware");

    webSendPage(nodename, httpMessage.length(), true);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();

    debugPrintln(String(F("HTTP: Attempting ESP firmware update from: ")) + String(webServer.arg("espFirmware")));
    // espStartOta(webServer.arg("espFirmware"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleResetConfig()
{ // http://plate01/resetConfig
    if(!httpIsAuthenticated(F("resetConfig"))) return;

    bool resetConfirmed = webServer.arg(F("confirm")) == F("yes");

    String nodename((char *)0);
    nodename.reserve(128);
    nodename = mqttGetNodename();

    String httpMessage((char *)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);
    httpMessage += F("<h1>");
    httpMessage += nodename;
    httpMessage += F("</h1><hr>");

    if(resetConfirmed) { // User has confirmed, so reset everything
        bool formatted = SPIFFS.format();
        if(formatted) {
            httpMessage += F("<b>Resetting all saved settings and restarting device into WiFi AP mode</b>");
        } else {
            httpMessage += F("<b>Failed to format the internal flash partition</b>");
            resetConfirmed = false;
        }
    } else {
        httpMessage +=
            F("<h2>Warning</h2><b>This process will reset all settings to the default values. The internal flash will "
              "be erased and the device is restarted. You may need to connect to the WiFi AP displayed on the panel to "
              "re-configure the device before accessing it again. ALL FILES WILL BE LOST!"
              "<br/><hr><br/><form method='get' action='resetConfig'>"
              "<br/><br/><button type='submit' name='confirm' value='yes'>Reset All Settings</button></form>"
              "<br/><hr><br/>");

        httpMessage +=
            PSTR("<p><form method='get' action='/config'><button type='submit'>Configuration</button></form></p>");
    }

    webSendPage(nodename, httpMessage.length(), resetConfirmed);
    webServer.sendContent(httpMessage);
    httpMessage.clear();
    webSendFooter();

    if(resetConfirmed) {
        delay(250);
        // configClearSaved();
        dispatchReboot(false); // Do not save the current config
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpSetup(const JsonObject & settings)
{
    if(WiFi.getMode() == WIFI_AP) {
        debugPrintln(F("HTTP: Wifi access point"));
        webServer.on(F("/"), webHandleWifiConfig);
    } else {

        webServer.on(F("/page/"), []() {
            String pageid = webServer.arg(F("page"));
            webServer.send(200, PSTR("text/plain"), "Page: '" + pageid + "'");
            haspSetPage(pageid.toInt());
        });

        webServer.on(F("/list"), HTTP_GET, handleFileList);
        // load editor
        webServer.on(F("/edit"), HTTP_GET, []() {
            if(!handleFileRead("/edit.htm")) {
                char mimetype[128];
                sprintf(mimetype, PSTR("text/plain"));
                webServer.send_P(404, mimetype, PSTR("FileNotFound"));
            }
        });
        webServer.on(F("/edit"), HTTP_PUT, handleFileCreate);
        webServer.on(F("/edit"), HTTP_DELETE, handleFileDelete);
        // first callback is called after the request has ended with all parsed arguments
        // second callback handles file uploads at that location
        webServer.on(F("/edit"), HTTP_POST, []() { webServer.send(200, "text/plain", ""); }, handleFileUpload);
        // get heap status, analog input value and all GPIO statuses in one json call
        /*webServer.on(F("/all"), HTTP_GET, []() {
            String json;
            json.reserve(128);
            json += F("{\"heap\":");
            json += String(ESP.getFreeHeap());
            json += F(", \"analog\":");
            json += String(analogRead(A0));
            json += F("}");

            char mimetype[128];
            sprintf(mimetype, PSTR("text/json"));
            webServer.send(200, mimetype, json);
            json.clear();
        });*/

        webServer.on(F("/"), webHandleRoot);
        webServer.on(F("/info"), webHandleInfo);

        webServer.on(F("/config/hasp"), webHandleHaspConfig);
        webServer.on(F("/config/http"), webHandleHttpConfig);
        webServer.on(F("/config/gui"), webHandleGuiConfig);
        webServer.on(F("/config/debug"), webHandleDebugConfig);
#if HASP_USE_MQTT > 0
        webServer.on(F("/config/mqtt"), webHandleMqttConfig);
#endif
#if HASP_USE_WIFI > 0
        webServer.on(F("/config/wifi"), webHandleWifiConfig);
#endif
        webServer.on(F("/screenshot"), webHandleScreenshot);
        webServer.on(F("/saveConfig"), webHandleSaveConfig);
        webServer.on(F("/resetConfig"), httpHandleResetConfig);
        webServer.on(F("/firmware"), webHandleFirmware);
        webServer.on(F("/espfirmware"), httpHandleEspFirmware);
        webServer.on(F("/reboot"), httpHandleReboot);
        webServer.onNotFound(httpHandleNotFound);
    }

    // Shared pages
    webServer.on(F("/about"), webHandleAbout);
    webServer.on(F("/config"), webHandleConfig);
    webServer.onNotFound(httpHandleNotFound);

    httpReconnect();
    debugPrintln(F("HTTP: Setup Complete"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpReconnect()
{
    if(!httpEnable) return;

    if(webServerStarted) {
        webServer.stop();
        webServerStarted = false;
        debugPrintln(F("HTTP: Server stoped"));
    } else if(WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP) {

        /*
            if(WiFi.getMode() == WIFI_AP) {
                webServer.on(F("/"), webHandleWifiConfig);
                webServer.on(F("/config"), webHandleConfig);
                webServer.onNotFound(httpHandleNotFound);
            } else {
            }
        */
        webServer.begin();
        webServerStarted = true;

        debugPrintln(String(F("HTTP: Server started @ http://")) +
                     (WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString()));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpLoop()
{
    if(httpEnable) webServer.handleClient();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpEverySecond()
{
    if(httpEnable && !webServerStarted) httpReconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool httpGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_CONFIG_ENABLE)] = httpEnable;
    settings[FPSTR(F_CONFIG_PORT)]   = httpPort;
    settings[FPSTR(F_CONFIG_USER)]   = httpUser;
    settings[FPSTR(F_CONFIG_PASS)]   = httpPassword;

    configOutput(settings);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool httpSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    changed |= configSet(httpPort, settings[FPSTR(F_CONFIG_PORT)], PSTR("httpPort"));

    if(!settings[FPSTR(F_CONFIG_USER)].isNull()) {
        changed |= strcmp(httpUser, settings[FPSTR(F_CONFIG_USER)]) != 0;
        strncpy(httpUser, settings[FPSTR(F_CONFIG_USER)], sizeof(httpUser));
    }

    if(!settings[FPSTR(F_CONFIG_PASS)].isNull()) {
        changed |= strcmp(httpPassword, settings[FPSTR(F_CONFIG_PASS)]) != 0;
        strncpy(httpPassword, settings[FPSTR(F_CONFIG_PASS)], sizeof(httpPassword));
    }

    return changed;
}