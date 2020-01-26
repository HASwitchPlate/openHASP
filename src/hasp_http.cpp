//#include "webServer.h"
#include <Arduino.h>
#include "ArduinoJson.h"

#include "hasp_log.h"
#include "hasp_debug.h"
#include "hasp_http.h"
#include "hasp_mqtt.h"
#include "hasp_wifi.h"
#include "hasp_config.h"
#include "hasp.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include <FS.h>
#include <ESP.h>

#define F_HTTP_ENABLE F("enable")
#define F_HTTP_PORT F("port")
#define F_HTTP_USER F("user")
#define F_HTTP_PASS F("pass")

bool httpEnable       = true;
bool webServerStarted = false;
uint16_t httpPort     = 80;
FS * filesystem       = &SPIFFS;
File fsUploadFile;
String httpUser     = "admin";
String httpPassword = "";

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
ESP8266WebServer webServer(80);
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <rom/rtc.h> // needed to get the ResetInfo
#include <WebServer.h>
WebServer webServer(80);

// Compatibility function for ESP8266 getRestInfo
String esp32ResetReason(uint8_t cpuid)
{
    if(cpuid > 1) {
        return F("Invalid CPU id");
    }
    RESET_REASON reason = rtc_get_reset_reason(cpuid);

    switch(reason) {
        case 1:
            return F("POWERON_RESET");
            break; /**<1, Vbat power on reset*/
        case 3:
            return F("SW_RESET");
            break; /**<3, Software reset digital core*/
        case 4:
            return F("OWDT_RESET");
            break; /**<4, Legacy watch dog reset digital core*/
        case 5:
            return F("DEEPSLEEP_RESET");
            break; /**<5, Deep Sleep reset digital core*/
        case 6:
            return F("SDIO_RESET");
            break; /**<6, Reset by SLC module, reset digital core*/
        case 7:
            return F("TG0WDT_SYS_RESET");
            break; /**<7, Timer Group0 Watch dog reset digital core*/
        case 8:
            return F("TG1WDT_SYS_RESET");
            break; /**<8, Timer Group1 Watch dog reset digital core*/
        case 9:
            return F("RTCWDT_SYS_RESET");
            break; /**<9, RTC Watch dog Reset digital core*/
        case 10:
            return F("INTRUSION_RESET");
            break; /**<10, Instrusion tested to reset CPU*/
        case 11:
            return F("TGWDT_CPU_RESET");
            break; /**<11, Time Group reset CPU*/
        case 12:
            return F("SW_CPU_RESET");
            break; /**<12, Software reset CPU*/
        case 13:
            return F("RTCWDT_CPU_RESET");
            break; /**<13, RTC Watch dog Reset CPU*/
        case 14:
            return F("EXT_CPU_RESET");
            break; /**<14, for APP CPU, reseted by PRO CPU*/
        case 15:
            return F("RTCWDT_BROWN_OUT_RESET");
            break; /**<15, Reset when the vdd voltage is not stable*/
        case 16:
            return F("RTCWDT_RTC_RESET");
            break; /**<16, RTC Watch dog reset digital core and rtc module*/
        default:
            return F("NO_MEAN");
    }
}

#endif // ESP32

static const char HTTP_DOCTYPE[] PROGMEM =
    "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,"
    "user-scalable=no\"/>";
static const char HTTP_META_GO_BACK[] PROGMEM = "<meta http-equiv='refresh' content='10;url=/'/>";
static const char HTTP_HEADER[] PROGMEM       = "<title>%s</title>";
static const char HTTP_STYLE[] PROGMEM =
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
static const char HTTP_SCRIPT[] PROGMEM = "<script>function "
                                          "c(l){document.getElementById('s').value=l.innerText||l.textContent;document."
                                          "getElementById('p').focus();}</script>";
static const char HTTP_HEADER_END[] PROGMEM =
    "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
static const char HTTP_END[] PROGMEM = "<div style='text-align:right;font-size:11px;'><hr/><a href='/about' "
                                       "style='color:#aaa;'>HASP 0.0.0 by Francis Van Roie</div></body></html>";
// Additional CSS style to match Hass theme
static const char HASP_STYLE[] PROGMEM =
    "<style>button{background-color:#03A9F4;}body{width:60%;margin:auto;}input:invalid{border:"
    "1px solid red;}input[type=checkbox]{width:20px;}</style>";

// these need to be removed
uint8_t motionPin       = 0;     // GPIO input pin for motion sensor if connected and enabled
bool debugSerialEnabled = true;  // Enable USB serial debug output
bool debugTelnetEnabled = false; // Enable telnet debug output

////////////////////////////////////////////////////////////////////////////////////////////////////
// These defaults may be overwritten with values saved by the web interface
char motionPinConfig[3] = "0";
////////////////////////////////////////////////////////////////////////////////////////////////////

// URL for auto-update "version.json"
const char UPDATE_URL[] = "http://haswitchplate.com/update/version.json";
// Default link to compiled Arduino firmware image
String espFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.ino.d1_mini.bin";
// Default link to compiled Nextion firmware images
String lcdFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.tft";

////////////////////////////////////////////////////////////////////////////////////////////////////
String formatBytes(size_t bytes);
void webHandleHaspConfig();

////////////////////////////////////////////////////////////////////////////////////////////////////
bool httpIsAuthenticated(const String & page)
{
    if(httpPassword[0] != '\0') { // Request HTTP auth if httpPassword is set
        if(!webServer.authenticate(httpUser.c_str(), httpPassword.c_str())) {
            webServer.requestAuthentication();
            return false;
        }
    }
    char buffer[128];
    sprintf(buffer, PSTR("HTTP: Sending %s page to client connected from: %s"), page.c_str(),
            webServer.client().remoteIP().toString().c_str());
    debugPrintln(buffer);
    return true;
}

String getOption(uint8_t value, String label, bool selected)
{
    char buffer[128];
    sprintf_P(buffer, PSTR("<option value='%u' %s>%s</option>"), value, (selected ? PSTR("selected") : ""),
              label.c_str());
    return buffer;
}
String getOption(String value, String label, bool selected)
{
    char buffer[128];
    sprintf_P(buffer, PSTR("<option value='%s' %s>%s</option>"), value.c_str(), (selected ? PSTR("selected") : ""),
              label.c_str());
    return buffer;
}

void webSendPage(String & nodename, uint32_t httpdatalength, bool gohome = false)
{
    char buffer[64];

    /* Calculate Content Length upfront */
    uint16_t contentLength = 0;
    contentLength += sizeof(HTTP_DOCTYPE) - 1;
    contentLength += sizeof(HTTP_HEADER) - 1 - 2 + nodename.length();
    contentLength += sizeof(HTTP_SCRIPT) - 1;
    contentLength += sizeof(HTTP_STYLE) - 1;
    contentLength += sizeof(HASP_STYLE) - 1;
    if(gohome) contentLength += sizeof(HTTP_META_GO_BACK) - 1;
    contentLength += sizeof(HTTP_HEADER_END) - 1;
    contentLength += sizeof(HTTP_END) - 1;

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

void webHandleRoot()
{
    if(!httpIsAuthenticated(F("root"))) return;

    char buffer[64];
    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1024);

    httpMessage += String(F("<h1>"));
    httpMessage += String(nodename);
    httpMessage += String(F("</h1>"));

    httpMessage += F("<p><form method='get' action='info'><button type='submit'>Information</button></form></p>");
    httpMessage += F("<p><form method='get' action='config'><button type='submit'>Configuration</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='firmware'><button type='submit'>Firmware Upgrade</button></form></p>");

    if(SPIFFS.exists(F("/edit.htm.gz"))) {
        httpMessage += F(
            "<p><form method='get' action='edit.htm.gz?path=/'><button type='submit'>File Browser</button></form></p>");
    }

    httpMessage +=
        F("<p><form method='get' action='reboot'><button class='red' type='submit'>Restart</button></form></p>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleReboot()
{ // http://plate01/reboot
    if(!httpIsAuthenticated(F("/reboot"))) return;

    String nodename    = haspGetNodename();
    String httpMessage = F("Rebooting Device");
    webSendPage(nodename, httpMessage.length(), true);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
    delay(500);

    debugPrintln(PSTR("HTTP: Reboot device"));
    haspSetPage(0);
    haspSetAttr(F("p[0].b[1].txt"), F("\"Rebooting...\""));

    delay(500);
    haspReset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleAbout()
{ // http://plate01/about
    if(!httpIsAuthenticated(F("/about"))) return;

    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1250);

    httpMessage += F("<p><h3>HASP OpenHardware edition</h3>Copyright&copy; 2020 Francis Van Roie "
                     "</br>MIT License</p>");
    httpMessage += F("<p>Based on the previous work of the following open source developers.</p><hr>");
    httpMessage +=
        F("<p><h3>HASwitchPlate</h3>Copyright&copy; 2019 Allen Derusha allen@derusha.org</b></br>MIT License</p>");
    httpMessage +=
        F("<p><h3>LittlevGL</h3>Copyright&copy; 2016 G&aacute;bor Kiss-V&aacute;mosi</br>Copyright&copy; 2019 "
          "LittlevGL</br>MIT License</p>");
    httpMessage += F("<p><h3>Lvgl ziFont Font Engine</h3>Copyright&copy; 2020 Francis Van Roie</br>MIT License</p>");
    httpMessage += F("<p><h3>TFT_eSPI Library</h3>Copyright&copy; 2017 Bodmer (https://github.com/Bodmer) All "
                     "rights reserved.</br>FreeBSD License</br>");
    httpMessage +=
        F("<i>includes parts from the Adafruit_GFX library - Copyright&copy; 2012 Adafruit Industries. All rights "
          "reserved. BSD License</i></p>");
    httpMessage += F("<p><h3>ArduinoJson</h3>Copyright&copy; 2014-2019 Benoit BLANCHON</br>MIT License</p>");
    httpMessage += F("<p><h3>PubSubClient</h3>Copyright&copy; 2008-2015 Nicholas O'Leary</br>MIT License</p>");
    httpMessage += F("<p><h3>Syslog</h3>Copyright&copy; 2016 Martin Sloup</br>MIT License</p>");

    httpMessage += F("</p><p><form method='get' action='/'><button type='submit'>Main Menu</button></form>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleInfo()
{ // http://plate01/
    if(!httpIsAuthenticated(F("/info"))) return;

    char buffer[64];
    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1024);

    httpMessage += F("<hr><b>MQTT Status: </b>");
    if(mqttIsConnected()) { // Check MQTT connection
        httpMessage += String(F("Connected"));
    } else {
        httpMessage += String(F("<font color='red'><b>Disconnected</b></font>, return code: "));
        //     +String(mqttClient.returnCode());
    }
    httpMessage += String(F("<br/><b>MQTT ClientID: </b>"));
    //   +String(mqttClientId);
    httpMessage += F("<br/><b>HASP Version: </b>");
    httpMessage += String(haspGetVersion());
    httpMessage += F("<br/><b>Uptime: </b>");
    httpMessage += String(long(millis() / 1000));

    // httpMessage += String(F("<br/><b>LCD Model: </b>")) + String(LV_HASP_HOR_RES_MAX) + " x " +
    // String(LV_HASP_VER_RES_MAX); httpMessage += String(F("<br/><b>LCD Version: </b>")) + String(lcdVersion);
    httpMessage += F("</p/><p><b>LCD Active Page: </b>");
    httpMessage += String(haspGetPage());
    httpMessage += F("<br/><b>CPU Frequency: </b>");
    httpMessage += String(ESP.getCpuFreqMHz());

    httpMessage += F("MHz</p/><p><b>SSID: </b>");
    httpMessage += String(WiFi.SSID());
    httpMessage += F("</br><b>Signal Strength: </b>");
    httpMessage += String(WiFi.RSSI());
    httpMessage += F("</br><b>IP Address: </b>");
    httpMessage += String(WiFi.localIP().toString());
    httpMessage += F("</br><b>Gateway: </b>");
    httpMessage += String(WiFi.gatewayIP().toString());
    httpMessage += F("</br><b>DNS Server: </b>");
    httpMessage += String(WiFi.dnsIP().toString());
    httpMessage += F("</br><b>MAC Aress: </b>");
    httpMessage += String(WiFi.macAddress());

    httpMessage += F("</p/><p><b>ESP Chip Id: </b>");
#if defined(ARDUINO_ARCH_ESP32)
    httpMessage += String(ESP.getChipRevision());
#else
    httpMessage += String(ESP.getChipId());
#endif
    httpMessage += F("<br/><b>Flash Chip Size: </b>");
    httpMessage += formatBytes(ESP.getFlashChipSize());
    httpMessage += F("</br><b>Program Size: </b>");
    httpMessage += formatBytes(ESP.getSketchSize());
    httpMessage += F(" bytes<br/><b>Free Program Space: </b>");
    httpMessage += formatBytes(ESP.getFreeSketchSpace());
    httpMessage += F(" bytes<br/><b>Free Memory: </b>");
    httpMessage += formatBytes(ESP.getFreeHeap());

#if defined(ARDUINO_ARCH_ESP32)
    // httpMessage += F("<br/><b>Heap Max Alloc: </b>");
    // httpMessage += String(ESP.getMaxAllocHeap());
    httpMessage += F("<br/><b>Memory Fragmentation: </b>");
    httpMessage += String((int16_t)(100.00f - (float)ESP.getMaxAllocHeap() / (float)ESP.getFreeHeap() * 100.00f));
    httpMessage += F("<br/><b>ESP SDK version: </b>");
    httpMessage += String(ESP.getSdkVersion());
    httpMessage += F("<br/><b>Last Reset: </b> CPU0: ");
    httpMessage += String(esp32ResetReason(0));
    httpMessage += F(" / CPU1: ");
    httpMessage += String(esp32ResetReason(1));
#else
    httpMessage += F("<br/><b>Memory Fragmentation: </b>");
    httpMessage += String(ESP.getHeapFragmentation());
    httpMessage += F("<br/><b>ESP Core version: </b>");
    httpMessage += String(ESP.getCoreVersion());
    httpMessage += F("<br/><b>Last Reset: </b>");
    httpMessage += String(ESP.getResetInfo());
#endif

    httpMessage += F("</p><p><form method='get' action='/'><button type='submit'>Main Menu</button></form>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}

String formatBytes(size_t bytes)
{
    if(bytes < 1024) {
        return String(bytes) + "B";
    } else if(bytes < (1024 * 1024)) {
        return String(bytes / 1024.0) + "KB";
    } else if(bytes < (1024 * 1024 * 1024)) {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    } else {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

String getContentType(String filename)
{
    if(webServer.hasArg(F("download"))) {
        return F("application/octet-stream");
    } else if(filename.endsWith(F(".htm"))) {
        return F("text/html");
    } else if(filename.endsWith(F(".html"))) {
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

String urldecode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for(int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if(c == '+') {
            encodedString += ' ';
        } else if(c == '%') {
            char buffer[3];
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
}

bool handleFileRead(String path)
{
    path = urldecode(path).substring(0, 31);
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

void handleFileUpload()
{
    if(webServer.uri() != "/edit") {
        return;
    }
    HTTPUpload & upload = webServer.upload();
    if(upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if(!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        debugPrintln(String(F("handleFileUpload Name: ")) + filename);
        fsUploadFile = filesystem->open(filename, "w");
        filename.clear();
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        // DBG_OUTPUT_PORT.print("handleFileUpload Data: "); debugPrintln(upload.currentSize);
        if(fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if(upload.status == UPLOAD_FILE_END) {
        if(fsUploadFile) {
            fsUploadFile.close();
        }
        debugPrintln(String(F("handleFileUpload Size: ")) + String(upload.totalSize));
        String filename = upload.filename;
        if(filename.endsWith(".zi")) webHandleHaspConfig();
    }
}

void handleFileDelete()
{
    if(webServer.args() == 0) {
        return webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
    }
    String path = webServer.arg(0);
    debugPrintln(String(F("handleFileDelete: ")) + path);
    if(path == "/") {
        return webServer.send(500, PSTR("text/plain"), PSTR("BAD PATH"));
    }
    if(!filesystem->exists(path)) {
        return webServer.send(404, PSTR("text/plain"), PSTR("FileNotFound"));
    }
    filesystem->remove(path);
    webServer.send(200, PSTR("text/plain"), "");
    path.clear();
}

void handleFileCreate()
{
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
    if(!webServer.hasArg(F("dir"))) {
        webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
        return;
    }

    String path = webServer.arg(F("dir"));
    debugPrintln(String(F("handleFileList: ")) + path);
    path.clear();

#if defined(ARDUINO_ARCH_ESP32)
    debugPrintln(PSTR("HTTP: Listing files on the internal flash:"));
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

        char msg[64];
        sprintf(msg, PSTR("HTTP:    * %s  (%u bytes)"), file.name(), (uint32_t)file.size());
        debugPrintln(msg);

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
    if(!httpIsAuthenticated(F("/config"))) return;

    if(webServer.method() == HTTP_POST) {
        if(webServer.hasArg(F("save"))) {

            DynamicJsonDocument settings(256);
            for(int i = 0; i < webServer.args(); i++) settings[webServer.argName(i)] = webServer.arg(i);

            if(webServer.arg(F("save")) == String(F("hasp"))) {
                haspSetConfig(settings.as<JsonObject>());

            } else if(webServer.arg(F("save")) == String(F("mqtt"))) {
                mqttSetConfig(settings.as<JsonObject>());

            } else if(webServer.arg(F("save")) == String(F("http"))) {

            } else if(webServer.arg(F("save")) == String(F("wifi"))) {
                wifiSetConfig(settings.as<JsonObject>());
            }
        }
    }

    if(WiFi.getMode() == WIFI_AP) {
        httpHandleReboot();
    }

    char buffer[64];
    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1024);

    httpMessage += F("<hr>");
    httpMessage +=
        F("<p><form method='get' action='/config/wifi'><button type='submit'>Wifi Settings</button></form></p>");

#if LV_USE_HASP_MQTT > 0
    httpMessage +=
        F("<p><form method='get' action='/config/mqtt'><button type='submit'>MQTT Settings</button></form></p>");
#endif

    httpMessage +=
        F("<p><form method='get' action='/config/http'><button type='submit'>HTTP Settings</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='/config/tft'><button type='submit'>Display Settings</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='/config/hasp'><button type='submit'>HASP Settings</button></form></p>");

    httpMessage +=
        F("<p><form method='get' action='/config/debug'><button type='submit'>Debug Settings</button></form></p>");

    httpMessage += F("<p><form method='get' action='resetConfig'><button class='red' type='submit'>Factory Reset "
                     "All Settings</button></form></p>");

    httpMessage += F("<form method='get' action='/'><button type='submit'>Main Menu</button></form>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if LV_USE_HASP_MQTT > 0
void webHandleMqttConfig()
{ // http://plate01/config/mqtt
    if(!httpIsAuthenticated(F("/config/mqtt"))) return;

    DynamicJsonDocument settings(256);
    mqttGetConfig(settings.to<JsonObject>());

    char buffer[64];
    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1024);

    httpMessage += String(F("<form method='POST' action='/config'>"));
    httpMessage += F("<b>HASP Node Name</b> <i><small>(required. lowercase letters, numbers, and _ only)</small>"
                     "</i><input id='haspGetNodename()' required name='haspGetNodename()' maxlength=15 "
                     "placeholder='HASP Node Name' pattern='[a-z0-9_]*' value='");
    httpMessage += nodename;
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

    httpMessage += F("'><button type='submit' name='save' value='mqtt'>Save Settings</button></form>");
    httpMessage += F("<form method='get' action='/config'><button type='submit'>Configuration</button></form>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#if LV_USE_HASP_WIFI > 0
void webHandleWifiConfig()
{ // http://plate01/config/wifi
    if(!httpIsAuthenticated(F("/config/wifi"))) return;

    DynamicJsonDocument settings(256);
    wifiGetConfig(settings.to<JsonObject>());

    char buffer[64];
    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1024);

    httpMessage += String(F("<form method='POST' action='/config'>"));
    httpMessage += String(F("<b>WiFi SSID</b> <i><small>(required)</small></i><input id='ssid' required "
                            "name='ssid' maxlength=32 placeholder='WiFi SSID' value='"));
    httpMessage += settings[FPSTR(F_CONFIG_SSID)].as<String>();
    httpMessage += String(F("'><br/><b>WiFi Password</b> <i><small>(required)</small></i><input id='pass' required "
                            "name='pass' type='password' maxlength=64 placeholder='WiFi Password' value='"));
    if(settings[FPSTR(F_CONFIG_PASS)].as<String>() != "") {
        httpMessage += F("********");
    }
    httpMessage += F("'><button type='submit' name='save' value='wifi'>Save Settings</button></form>");
    httpMessage += F("<form method='get' action='/config'><button type='submit'>Configuration</button></form>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleHaspConfig()
{ // http://plate01/config/http
    if(!httpIsAuthenticated(F("/config/hasp"))) return;

    DynamicJsonDocument settings(256);
    haspGetConfig(settings.to<JsonObject>());

    char buffer[64];
    String nodename = haspGetNodename();
    String httpMessage((char *)0);
    httpMessage.reserve(1024);

    httpMessage += String(F("<p><form action='/edit' method='post' enctype='multipart/form-data'><input type='file' "
                            "name='filename' accept='.zi'>"));
    httpMessage += F("<hr><button type='submit'>Upload Font</button></form></p>");

    httpMessage += String(F("<form method='POST' action='/config'>"));
    httpMessage += String(F("<p><b>UI Theme</b> <i><small>(required)</small></i><select id='theme' name='theme'>"));

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
    httpMessage += String(F("</select></br>"));
    httpMessage +=
        "<b>Hue</b><div style='width:100%;background-image:linear-gradient(to "
        "right,red,orange,yellow,green,blue,indigo,violet);'><input style='align:center;padding:0px' id='hue' "
        "name='hue' type='range' "
        "min='0' max='360' value='" +
        settings[FPSTR(F_CONFIG_HUE)].as<String>() + "'></div></p>";

    httpMessage += String(F("<p><b>Default Font</b><select id='font' name='font'><option value=''>None</option>"));
#if defined(ARDUINO_ARCH_ESP32)
    debugPrintln(PSTR("HTTP: Listing files on the internal flash:"));
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
    httpMessage += String(F("</select></p>"));

    httpMessage += String(F("<p><b>Pages File</b> <i><small>(required)</small></i><input id='pages' required "
                            "name='pages' maxlength=32 placeholder='/pages.jsonl' value='")) +
                   settings[FPSTR(F_CONFIG_PAGES)].as<String>() + "'>";
    httpMessage += String(F("</br><b>Startup Page</b> <i><small>(required)</small></i><input id='startpage' required "
                            "name='startpage' type='number' min='0' max='3' value='")) +
                   settings[FPSTR(F_CONFIG_STARTPAGE)].as<String>() + "'></p>";

    httpMessage += F("<button type='submit' name='save' value='hasp'>Save Settings</button></form>");

    httpMessage += F("<form method='get' action='/config'><button type='submit'>Configuration</button></form>");

    webSendPage(nodename, httpMessage.length(), false);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleNotFound()
{ // webServer 404
    if(handleFileRead(webServer.uri())) return;

    debugPrintln(String(F("HTTP: Sending 404 to client connected from: ")) + webServer.client().remoteIP().toString());

    String httpMessage((char *)0);
    httpMessage.reserve(128);
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
    if(!httpIsAuthenticated(F("/saveConfig"))) return;

    configWriteConfig();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleFirmware()
{
    if(!httpIsAuthenticated(F("/firmware"))) return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleEspFirmware()
{ // http://plate01/espfirmware
    if(!httpIsAuthenticated(F("/espfirmware"))) return;

    String nodename = haspGetNodename();
    char buffer[64];
    String httpMessage((char *)0);
    httpMessage.reserve(128);
    httpMessage += String(F("<h1>"));
    httpMessage += String(haspGetNodename());
    httpMessage += String(F(" ESP update</h1><br/>Updating ESP firmware from: "));
    httpMessage += String(webServer.arg("espFirmware"));

    webSendPage(nodename, httpMessage.length(), true);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20

    debugPrintln(String(F("HTTP: Attempting ESP firmware update from: ")) + String(webServer.arg("espFirmware")));
    // espStartOta(webServer.arg("espFirmware"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpHandleResetConfig()
{ // http://plate01/resetConfig
    if(!httpIsAuthenticated(F("/resetConfig"))) return;

    bool resetConfirmed = webServer.arg(F("confirm")) == F("yes");
    String nodename     = haspGetNodename();
    char buffer[64];
    String httpMessage((char *)0);
    httpMessage.reserve(128);

    if(resetConfirmed) { // User has confirmed, so reset everything
        httpMessage += F("<h1>");
        httpMessage += haspGetNodename();
        bool formatted = SPIFFS.format();
        if(formatted) {
            httpMessage += F("</h1><b>Resetting all saved settings and restarting device into WiFi AP mode</b>");
        } else {
            httpMessage += F("</h1><b>Failed to format the internal flash partition</b>");
            resetConfirmed = false;
        }
    } else {
        httpMessage +=
            F("<h1>Warning</h1><b>This process will reset all settings to the default values. The internal flash will "
              "be erased and the device is restarted. You may need to connect to the WiFi AP displayed on the panel to "
              "re-configure the device before accessing it again. ALL FILES WILL BE LOST!"
              "<br/><hr><br/><form method='get' action='resetConfig'>"
              "<br/><br/><button type='submit' name='confirm' value='yes'>Reset All Settings</button></form>"
              "<br/><hr><br/><form method='get' action='/config'>"
              "<button type='submit'>Configuration</button></form>");
    }

    webSendPage(nodename, httpMessage.length(), resetConfirmed);
    webServer.sendContent(httpMessage); // len
    webServer.sendContent_P(HTTP_END);  // 20

    if(resetConfirmed) {
        delay(1000);
        // configClearSaved();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpSetup(const JsonObject & settings)
{
    webServer.on(F("/page/"), []() {
        String pageid = webServer.arg(F("page"));
        webServer.send(200, PSTR("text/plain"), "Page: '" + pageid + "'");
        haspSetPage(pageid.toInt());
    });

    webServer.on("/list", HTTP_GET, handleFileList);
    // load editor
    webServer.on("/edit", HTTP_GET, []() {
        if(!handleFileRead("/edit.htm")) {
            webServer.send(404, "text/plain", "FileNotFound");
        }
    });
    webServer.on("/edit", HTTP_PUT, handleFileCreate);
    webServer.on("/edit", HTTP_DELETE, handleFileDelete);
    // first callback is called after the request has ended with all parsed arguments
    // second callback handles file uploads at that location
    webServer.on("/edit", HTTP_POST, []() { webServer.send(200, "text/plain", ""); }, handleFileUpload);
    // get heap status, analog input value and all GPIO statuses in one json call
    webServer.on("/all", HTTP_GET, []() {
        String json('{');
        json += "\"heap\":" + String(ESP.getFreeHeap());
        json += ", \"analog\":" + String(analogRead(A0));
        json += "}";
        webServer.send(200, "text/json", json);
        json.clear();
    });

    webServer.on(F("/"), webHandleRoot);
    webServer.on(F("/about"), webHandleAbout);
    webServer.on(F("/info"), webHandleInfo);
    webServer.on(F("/config"), webHandleConfig);
    webServer.on(F("/config/hasp"), webHandleHaspConfig);
#if LV_USE_HASP_MQTT > 0
    webServer.on(F("/config/mqtt"), webHandleMqttConfig);
#endif
#if LV_USE_HASP_WIFI > 0
    webServer.on(F("/config/wifi"), webHandleWifiConfig);
#endif
    webServer.on(F("/saveConfig"), webHandleSaveConfig);
    webServer.on(F("/resetConfig"), httpHandleResetConfig);
    webServer.on(F("/firmware"), webHandleFirmware);
    webServer.on(F("/espfirmware"), httpHandleEspFirmware);
    webServer.on(F("/reboot"), httpHandleReboot);
    webServer.onNotFound(httpHandleNotFound);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpReconnect()
{
    if(!httpEnable) return;

    if(WiFi.getMode() == WIFI_AP) {
        webServer.on(F("/"), webHandleWifiConfig);
        webServer.on(F("/config"), webHandleConfig);
        webServer.onNotFound(webHandleWifiConfig);
    }

    webServer.stop();
    webServer.begin();
    debugPrintln(String(F("HTTP: Server started @ http://")) +
                 (WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void httpLoop(bool wifiIsConnected)
{
    if(httpEnable) webServer.handleClient();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool httpGetConfig(const JsonObject & settings)
{
    settings[FPSTR(F_HTTP_ENABLE)] = httpEnable;
    settings[FPSTR(F_HTTP_PORT)]   = httpPort;
    settings[FPSTR(F_HTTP_USER)]   = httpUser;
    settings[FPSTR(F_HTTP_PASS)]   = httpPassword;

    size_t size = serializeJson(settings, Serial);
    Serial.println();

    return true;
}