/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

//#include "webServer.h"
#include "hasplib.h"
#include "ArduinoLog.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "Update.h"
#include "sdkconfig.h" // for CONFIG_IDF_TARGET_ESP32* defines
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#endif

#include "hasp_conf.h"
#include "dev/device.h"
#include "hal/hasp_hal.h"

#include "hasp_gui.h"
#include "hasp_debug.h"
#include "hasp_config.h"

#if HASP_USE_HTTP > 0
#include "sys/net/hasp_network.h"

#if(HASP_USE_CAPTIVE_PORTAL > 0) && (HASP_USE_WIFI > 0)
#include <DNSServer.h>
#endif

// #ifdef USE_CONFIG_OVERRIDE
// #include "user_config_override.h"
// #endif

/* clang-format off */
//default theme
#ifndef D_HTTP_COLOR_TEXT
#define D_HTTP_COLOR_TEXT               "#000"       // Global text color - Black
#endif
#ifndef D_HTTP_COLOR_BACKGROUND
#define D_HTTP_COLOR_BACKGROUND         "#fff"       // Global background color - White
#endif
#ifndef D_HTTP_COLOR_INPUT_TEXT
#define D_HTTP_COLOR_INPUT_TEXT         "#000"       // Input text color - Black
#endif
#ifndef D_HTTP_COLOR_INPUT
#define D_HTTP_COLOR_INPUT              "#fff"       // Input background color - White
#endif
#ifndef D_HTTP_COLOR_INPUT_WARNING
#define D_HTTP_COLOR_INPUT_WARNING      "#f00"       // Input warning border color - Red
#endif
#ifndef D_HTTP_COLOR_BUTTON_TEXT
#define D_HTTP_COLOR_BUTTON_TEXT        "#fff"       // Button text color - White
#endif
#ifndef D_HTTP_COLOR_BUTTON
#define D_HTTP_COLOR_BUTTON             "#1fa3ec"    // Button color - Vivid blue
#endif
#ifndef D_HTTP_COLOR_BUTTON_HOVER
#define D_HTTP_COLOR_BUTTON_HOVER       "#0083cc"    // Button color - Olympic blue
#endif
#ifndef D_HTTP_COLOR_BUTTON_RESET
#define D_HTTP_COLOR_BUTTON_RESET       "#f00"       // Restart/Reset button color - red
#endif
#ifndef D_HTTP_COLOR_BUTTON_RESET_HOVER
#define D_HTTP_COLOR_BUTTON_RESET_HOVER "#b00"       // Restart/Reset button color - Dark red
#endif
#ifndef D_HTTP_COLOR_GROUP_TEXT
#define D_HTTP_COLOR_GROUP_TEXT         "#000"       // Container text color - Black
#endif
#ifndef D_HTTP_COLOR_GROUP
#define D_HTTP_COLOR_GROUP              "#f3f3f3"    // Container color - Light gray
#endif
#ifndef D_HTTP_COLOR_FOOTER_TEXT
#define D_HTTP_COLOR_FOOTER_TEXT        "#0083cc"    // Text color of the page footer
#endif
/* clang-format on */

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
File fsUploadFile;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
bool webServerStarted = false;

#if(HASP_USE_CAPTIVE_PORTAL > 0) && (HASP_USE_WIFI > 0)
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);
#ifndef DNS_PORT
#define DNS_PORT 53
#endif // DNS_PORT
#endif // HASP_USE_CAPTIVE_PORTAL

hasp_http_config_t http_config;

#define HTTP_PAGE_SIZE (6 * 256)

#if(defined(STM32F4xx) || defined(STM32F7xx)) && HASP_USE_ETHERNET > 0
#include <EthernetWebServer_STM32.h>
EthernetWebServer webServer(80);
#endif

#if defined(STM32F4xx) && HASP_USE_WIFI > 0
#include <EthernetWebServer_STM32.h>
// #include <WiFi.h>
EthernetWebServer webServer(80);
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include "StringStream.h"
#include <ESP8266WebServer.h>
#include <detail/mimetable.h>
ESP8266WebServer webServer(80);
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#include <detail/mimetable.h>
WebServer webServer(80);

#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2)
extern const uint8_t EDIT_HTM_GZ_START[] asm("_binary_data_edit_htm_gz_start");
extern const uint8_t EDIT_HTM_GZ_END[] asm("_binary_data_edit_htm_gz_end");
extern const uint8_t STYLE_CSS_GZ_START[] asm("_binary_data_style_css_gz_start");
extern const uint8_t STYLE_CSS_GZ_END[] asm("_binary_data_style_css_gz_end");
extern const uint8_t SCRIPT_JS_GZ_START[] asm("_binary_data_script_js_gz_start");
extern const uint8_t SCRIPT_JS_GZ_END[] asm("_binary_data_script_js_gz_end");
#endif // CONFIG_IDF_TARGET_ESP32

#endif // ESP32

HTTPUpload* upload;

static const char HTTP_MENU_BUTTON[] PROGMEM =
    "<p><form method='GET' action='%s'><button type='submit'>%s</button></form></p>";

const char MAIN_MENU_BUTTON[] PROGMEM = "<a href='/'>" D_HTTP_MAIN_MENU "</a>";
const char MIT_LICENSE[] PROGMEM      = "</br>MIT License</p>";

const char HTTP_DOCTYPE[] PROGMEM      = "<!DOCTYPE html><html lang=\"en\"><head><meta charset='utf-8'><meta "
                                         "name=\"viewport\" content=\"width=device-width,initial-scale=1\"/>";
const char HTTP_META_GO_BACK[] PROGMEM = "<meta http-equiv='refresh' content='15;url=/'/>";
const char HTTP_HEADER[] PROGMEM       = "<title>%s</title>";
const char HTTP_HEADER_END[] PROGMEM =
    "<script src=\"/script.js\"></script><link rel=\"stylesheet\" href=\"/vars.css\">"
    "<link rel=\"stylesheet\" href=\"/style.css\"></head><body><div id='doc'>";
const char HTTP_FOOTER[] PROGMEM = "<div class='clear'><hr/><a class='foot' href='/about'>" D_MANUFACTURER " ";
const char HTTP_END[] PROGMEM    = " " D_HTTP_FOOTER "</div></body></html>";

////////////////////////////////////////////////////////////////////////////////////////////////////

// URL for auto-update "version.json"
// const char UPDATE_URL[] PROGMEM = "http://haswitchplate.com/update/version.json";
// // Default link to compiled Arduino firmware image
// String espFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.ino.d1_mini.bin";
// // Default link to compiled Nextion firmware images
// String lcdFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.tft";

////////////////////////////////////////////////////////////////////////////////////////////////////
String getOption(int value, String label, int current_value)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("<option value='%d'%s>%s</option>"), value,
               (value == current_value ? PSTR(" selected") : ""), label.c_str());
    return buffer;
}

String getOption(String& value, String& label, String& current_value)
{
    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("<option value='%s'%s>%s</option>"), value.c_str(),
               (value == current_value ? PSTR(" selected") : ""), label.c_str());
    return buffer;
}

static void add_form_button(String& str, const __FlashStringHelper* label, const __FlashStringHelper* action)
{
    str += F("<a href='");
    str += action;
    str += F("'>");
    str += label;
    str += F("</a>");
}

static String getContentType(const String& path)
{
    char buff[sizeof(mime::mimeTable[0].mimeType)];
    // Check all entries but last one for match, return if found
    for(size_t i = 0; i < sizeof(mime::mimeTable) / sizeof(mime::mimeTable[0]) - 1; i++) {
        strcpy_P(buff, mime::mimeTable[i].endsWith);
        if(path.endsWith(buff)) {
            strcpy_P(buff, mime::mimeTable[i].mimeType);
            return String(buff);
        }
    }
    // Fall-through and just return default type
    strcpy_P(buff, mime::mimeTable[sizeof(mime::mimeTable) / sizeof(mime::mimeTable[0]) - 1].mimeType);
    return String(buff);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void webHandleHaspConfig();

////////////////////////////////////////////////////////////////////////////////////////////////////

bool httpIsAuthenticated()
{
    if(http_config.password[0] != '\0') { // Request HTTP auth if httpPassword is set
        if(!webServer.authenticate(http_config.username, http_config.password)) {
            webServer.requestAuthentication();
            return false;
        }
    }
    return true;
}

bool httpIsAuthenticated(const __FlashStringHelper* notused)
{
    if(!httpIsAuthenticated()) return false;

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    LOG_TRACE(TAG_HTTP, F(D_HTTP_SENDING_PAGE), webServer.uri().c_str(),
              webServer.client().remoteIP().toString().c_str());
#else
        // LOG_INFO(TAG_HTTP,F(D_HTTP_SENDING_PAGE), page,
        //             String(webServer.client().remoteIP()).c_str());
#endif

    return true;
}

static void webSendFooter()
{
#if defined(STM32F4xx)
    webServer.sendContent(HTTP_FOOTER);
    webServer.sendContent(haspDevice.get_version());
    webServer.sendContent(HTTP_END);
#else
    webServer.sendContent_P(HTTP_FOOTER);
    webServer.sendContent(haspDevice.get_version());
    webServer.sendContent_P(HTTP_END);
#endif
}

static void webSendCacheHeader(int size, int age)
{
    webServer.sendHeader(F("Content-Length"), (String)(size));
    webServer.sendHeader(F("Cache-Control"), F("max-age=3600, public"));
}

static int webSendCached(int statuscode, const char* contenttype, const char* data, size_t size)
{
    webSendCacheHeader(size, 3600);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    webServer.send_P(statuscode, contenttype, data, size);
#else
    webServer.send(statuscode, contenttype, data);
#endif
    return statuscode;
}

static void webSendHeader(const char* nodename, uint32_t httpdatalength, bool gohome = false)
{
    {
        char buffer[64];

        /* Calculate Content Length upfront */
        uint32_t contentLength = strlen(haspDevice.get_version()); // version length
        contentLength += sizeof(HTTP_DOCTYPE) - 1;
        contentLength += sizeof(HTTP_HEADER) - 1 - 2 + strlen(nodename); // -2 for %s
                                                                         //    contentLength += sizeof(HTTP_SCRIPT) - 1;
        if(gohome) contentLength += sizeof(HTTP_META_GO_BACK) - 1;
        contentLength += sizeof(HTTP_HEADER_END) - 1;
        contentLength += sizeof(HTTP_FOOTER) - 1;
        contentLength += sizeof(HTTP_END) - 1;

        if(httpdatalength > HTTP_PAGE_SIZE) {
            LOG_WARNING(TAG_HTTP, F("Sending page with %u static and %u dynamic bytes"), contentLength, httpdatalength);
        }

        webServer.setContentLength(contentLength + httpdatalength);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
        webServer.send_P(200, PSTR("text/html"), HTTP_DOCTYPE); // 122
#else
        webServer.send(200, ("text/html"), HTTP_DOCTYPE); // 122
#endif

        snprintf_P(buffer, sizeof(buffer), HTTP_HEADER, nodename);
        webServer.sendContent(buffer); // 17-2+len
    }

#if defined(STM32F4xx)
    // webServer.sendContent(HTTP_SCRIPT); // 131
    // webServer.sendContent(HTTP_STYLE); // 487
    // webServer.sendContent(HASP_STYLE);                   // 145
    if(gohome) webServer.sendContent(HTTP_META_GO_BACK); // 47
    webServer.sendContent(HTTP_HEADER_END);              // 80
#else
    // webServer.sendContent_P(HTTP_SCRIPT);                 // 131
    // webServer.sendContent_P(HTTP_STYLE); // 487
    // webServer.sendContent_P(HASP_STYLE);                   // 145
    if(gohome) webServer.sendContent_P(HTTP_META_GO_BACK); // 47
    webServer.sendContent_P(HTTP_HEADER_END);              // 80
#endif
}

bool saveConfig()
{
    bool updated = false;

    if(webServer.method() == HTTP_POST && webServer.hasArg(PSTR("save"))) {
        String save = webServer.arg(PSTR("save"));

        StaticJsonDocument<256> settings;
        for(int i = 0; i < webServer.args(); i++) settings[webServer.argName(i)] = webServer.arg(i);

        if(save == String(PSTR("hasp"))) {
            updated = haspSetConfig(settings.as<JsonObject>());

#if HASP_USE_MQTT > 0
        } else if(save == String(PSTR("mqtt"))) {
            updated = mqttSetConfig(settings.as<JsonObject>());
#endif

        } else if(save == String(PSTR("gui"))) {
            settings[FPSTR(FP_GUI_POINTER)] = webServer.hasArg(PSTR("cursor"));
            settings[FPSTR(FP_GUI_INVERT)]  = webServer.hasArg(PSTR("invert"));
            updated                         = guiSetConfig(settings.as<JsonObject>());

        } else if(save == String(PSTR("debug"))) {
            settings[FPSTR(FP_DEBUG_ANSI)] = webServer.hasArg(PSTR("ansi"));
            updated                        = debugSetConfig(settings.as<JsonObject>());

        } else if(save == String(PSTR("http"))) {
            updated = httpSetConfig(settings.as<JsonObject>());

            // Password might have changed
            if(!httpIsAuthenticated(F("config"))) return updated;

#if HASP_USE_WIFI > 0
        } else if(save == String(PSTR("wifi"))) {
            updated = wifiSetConfig(settings.as<JsonObject>());
#endif
        }
    }

    return updated;
}

static void webHandleRoot()
{
    if(!httpIsAuthenticated(F("root"))) return;
    bool updated = saveConfig();

    {
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");

        if(updated) {
            httpMessage += F("<p class='info'>" D_HTTP_CONFIG_CHANGED "</p>");
        }

        httpMessage += F("<a href='/config/hasp'>" D_HTTP_HASP_DESIGN "</a>");
        httpMessage += F("<a href='/screenshot'>" D_HTTP_SCREENSHOT "</a>");
        httpMessage += F("<a href='/info'>" D_HTTP_INFORMATION "</a>");
        httpMessage += F("<a href='/config'>" D_HTTP_CONFIGURATION "</a>");
        httpMessage += F("<a href='/firmware'>" D_HTTP_FIRMWARE_UPGRADE "</a>");

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#ifdef ARDUINO_ARCH_ESP32
        bool flashfile = true;
#else
        bool flashfile = false;
#endif
        if(flashfile || HASP_FS.exists(F("/edit.htm.gz")) || HASP_FS.exists(F("/edit.htm"))) {
            httpMessage += F("<a href='/edit.htm?file=/'>" D_HTTP_FILE_BROWSER "</a>");
        }
#endif

        httpMessage += F("<a href='/reboot' class='red'>" D_HTTP_REBOOT "</a>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void httpHandleReboot()
{ // http://plate01/reboot
    if(!httpIsAuthenticated(F("reboot"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage = F(D_DISPATCH_REBOOT);

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), true);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    { // Execute Actions
        // delay(200);
        dispatch_reboot(true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleScreenshot()
{ // http://plate01/screenshot
    if(!httpIsAuthenticated(F("screenshot"))) return;

    { // Execute actions
        if(webServer.hasArg(F("a"))) {
            if(webServer.arg(F("a")) == F("next")) {
                dispatch_page_next(LV_SCR_LOAD_ANIM_NONE);
            } else if(webServer.arg(F("a")) == F("prev")) {
                dispatch_page_prev(LV_SCR_LOAD_ANIM_NONE);
            } else if(webServer.arg(F("a")) == F("back")) {
                dispatch_page_back(LV_SCR_LOAD_ANIM_NONE);
            }
        }

        if(webServer.hasArg(F("q"))) {
            lv_disp_t* disp = lv_disp_get_default();
            webServer.setContentLength(66 + disp->driver.hor_res * disp->driver.ver_res * sizeof(lv_color_t));
            webServer.send_P(200, PSTR("image/bmp"), "");
            guiTakeScreenshot();
            webServer.client().stop();
            return;
        }
    }

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");

        httpMessage += F("<p class='c'><img id='bmp' src='?q=0'");
        httpMessage += F(" onload=\"aref(5)\" onerror=\"aref(15)\"/></p>"); // Automatic refresh

        httpMessage += F("<div class=\"dist\"><a href='#' onclick=\"return ref('prev')\">" D_HTTP_PREV_PAGE "</a>");
        httpMessage += F("<a href='#' onclick=\"return ref('')\">" D_HTTP_REFRESH "</a>");
        httpMessage += F("<a href='#' onclick=\"return ref('next')\">" D_HTTP_NEXT_PAGE "</a></div>");
        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void add_json(String& data, JsonDocument& doc)
{
    char buffer[800];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    if(doc.isNull()) return; // empty document

    buffer[len - 1] = ',';
    char* start     = buffer + 1;
    data += String(start);
    doc.clear();
}

static void webHandleApi()
{ // http://plate01/about
    if(!httpIsAuthenticated(F("api"))) return;

    DynamicJsonDocument doc(800);
    String contentType = getContentType(F(".json"));
    String endpoint((char*)0);
    endpoint = webServer.pathArg(0);

    if(!strcasecmp_P(endpoint.c_str(), PSTR("info"))) {
        String jsondata((char*)0);
        jsondata.reserve(HTTP_PAGE_SIZE);
        jsondata = "{";

        hasp_get_info(doc);
        add_json(jsondata, doc);

#if HASP_USE_MQTT > 0
        mqtt_get_info(doc);
        add_json(jsondata, doc);
#endif

        network_get_info(doc);
        add_json(jsondata, doc);

        haspDevice.get_info(doc);
        add_json(jsondata, doc);

        jsondata[jsondata.length() - 1] = '}'; // Replace last comma with a bracket

        webServer.send(200, contentType, jsondata);

    } else {
        webServer.send(400, contentType, "Bad Request");
    }
}

static void webHandleApiConfig()
{ // http://plate01/about
    if(!httpIsAuthenticated(F("api"))) return;

    DynamicJsonDocument doc(800);
    String contentType = getContentType(F(".json"));
    String endpoint((char*)0);
    endpoint = webServer.pathArg(0);

    JsonObject settings = doc.to<JsonObject>(); // Settings are invalid, force creation of an empty JsonObject

    if(!strcasecmp_P(endpoint.c_str(), PSTR("wifi"))) {
        wifiGetConfig(settings);
    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("mqtt"))) {
        mqttGetConfig(settings);
    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("http"))) {
        httpGetConfig(settings);
    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("gui"))) {
        guiGetConfig(settings);
    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("debug"))) {
        debugGetConfig(settings);
    } else {
        webServer.send(400, contentType, "Bad Request");
        return;
    }

    // Mask non-blank passwords
    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() && settings[FPSTR(FP_CONFIG_PASS)].as<String>().length() != 0) {
        settings[FPSTR(FP_CONFIG_PASS)] = D_PASSWORD_MASK;
    }

    doc.shrinkToFit();
    const size_t size = measureJson(doc) + 1;
    char jsondata[size];
    memset(jsondata, 0, size);
    serializeJson(doc, jsondata, size);
    webServer.send(200, contentType, jsondata);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleAbout()
{ // http://plate01/about
    if(!httpIsAuthenticated(F("about"))) return;

    { // Send Content
        String mitLicense((char*)0);
        mitLicense = FPSTR(MIT_LICENSE);

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);

        httpMessage += "<div id='doc'></div><script>window.addEventListener('load', about());</script>";
        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleInfoJson()
{ // http://plate01/
    if(!httpIsAuthenticated(F("infojson"))) return;

    { // Send Content
        String htmldata((char*)0);
        htmldata.reserve(HTTP_PAGE_SIZE);
        DynamicJsonDocument doc(512);

        htmldata += F("<h1>");
        htmldata += haspDevice.get_hostname();
        htmldata += F("</h1><hr>");

        htmldata += "<div id=\"info\"></div><script>loader(\"GET\", \"/api/info/\", info)</script>";
        htmldata += FPSTR(MAIN_MENU_BUTTON);

        webSendHeader(haspDevice.get_hostname(), htmldata.length(), false);
        webServer.sendContent(htmldata);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
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

static unsigned long htppLastLoopTime = 0;
static void webUploadProgress()
{
    long t = webServer.header("Content-Length").toInt();
    if(millis() - htppLastLoopTime >= 1250) {
        LOG_VERBOSE(TAG_HTTP, F(D_BULLET "Uploaded %u / %d bytes"), upload->totalSize + upload->currentSize, t);
        htppLastLoopTime = millis();
    }
    if(t > 0) t = (upload->totalSize + upload->currentSize) * 100 / t;
    haspProgressVal(t);
}

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
static inline void webUpdatePrintError()
{
#if defined(ARDUINO_ARCH_ESP8266)
    String output((char*)0);
    output.reserve(128);
    StringStream stream((String&)output);
    Update.printError(stream); // ESP8266 only has printError()
    LOG_ERROR(TAG_HTTP, output.c_str());
    haspProgressMsg(output.c_str());
#elif defined(ARDUINO_ARCH_ESP32)
    LOG_ERROR(TAG_HTTP, Update.errorString()); // ESP32 has errorString()
    haspProgressMsg(Update.errorString());
#endif
}

static void webUpdateReboot()
{
    LOG_INFO(TAG_HTTP, F("Update Success: %u bytes received. Rebooting..."), upload->totalSize);

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<b>Upload complete. Rebooting device, please wait...</b>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), true);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    // Exectute Actions
    {
        // delay(250);
        dispatch_reboot(true); // Save the current config
    }
}

static void webHandleFirmwareUpload()
{
    upload = &webServer.upload();

    switch(upload->status) {

        case UPLOAD_FILE_START: {
            if(!httpIsAuthenticated(F("update"))) return;

            // WiFiUDP::stopAll();

            int command = webServer.arg(F("cmd")).toInt();
            size_t size = 0;
            if(command == U_FLASH) {
                LOG_TRACE(TAG_HTTP, F("Update flash: %s"), upload->filename.c_str());
                size = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#ifdef ESP32
            } else if(command == U_SPIFFS) {
                LOG_TRACE(TAG_HTTP, F("Update filesystem: %s"), upload->filename.c_str());
                size = UPDATE_SIZE_UNKNOWN;
#endif
            }
            haspProgressMsg(upload->filename.c_str());

            // if(!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
            //  const char label[] = "spiffs";
            if(!Update.begin(size, command, -1, 0U)) { // start with max available size
                webUpdatePrintError();
            }
            break;
        }

        case UPLOAD_FILE_WRITE: // flashing firmware to ESP
            if(Update.write(upload->buf, upload->currentSize) != upload->currentSize) {
                webUpdatePrintError();
            } else {
                webUploadProgress();
            }
            break;

        case UPLOAD_FILE_END:
            haspProgressVal(100);
            if(Update.end(true)) { // true to set the size to the current progress
                haspProgressMsg(F(D_OTA_UPDATE_APPLY));
                webUpdateReboot();
            } else {
                webUpdatePrintError();
            }
            break;

        default:;
    }
}
#endif

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
static int handleFileRead(String path)
{
    // if(!httpIsAuthenticated(F("fileread"))) return false;
    if(!httpIsAuthenticated()) return false;

    path = webServer.urlDecode(path).substring(0, 31);
    if(path.endsWith("/")) {
        path += F("index.htm");
    }

    String style_css = F("/style.css");
    String script_js = F("/script.js");
    bool is_cached   = (path == style_css) || (path == script_js);

    String pathWithGz = path + F(".gz");
    if(HASP_FS.exists(pathWithGz) || HASP_FS.exists(path)) {

        String contentType((char*)0);
        if(webServer.hasArg(F("download")))
            contentType = F("application/octet-stream");
        else
            contentType = getContentType(path);

        if(!HASP_FS.exists(path) && HASP_FS.exists(pathWithGz))
            path = pathWithGz; // Only use .gz if normal file doesn't exist

        LOG_TRACE(TAG_HTTP, F(D_HTTP_SENDING_PAGE), path.c_str(), webServer.client().remoteIP().toString().c_str());

        String configFile((char*)0); // Verify if the file is config.json
        configFile = FPSTR(FP_HASP_CONFIG_FILE);

        if(path.endsWith(configFile.c_str())) { // "//config.json" is also a valid path!
            DynamicJsonDocument settings(8 * 256);
            DeserializationError error = configParseFile(configFile, settings);

            if(error) return 500; // Internal Server Error

            configMaskPasswords(settings); // Output settings to the client with masked passwords!
            char buffer[1024];
            size_t len = serializeJson(settings, buffer, sizeof(buffer));
            webServer.setContentLength(len);
            webServer.send(200, contentType, buffer);

        } else {
            File file = HASP_FS.open(path, "r");

            // script.js and styles.css can be cached
            if(is_cached) webSendCacheHeader(file.size(), 3600);

            // Stream other files directly from filesystem
            webServer.streamFile(file, contentType);
            file.close();
        }

        return 200; // OK
    }

#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2)
    if(path == F("/edit.htm")) {
        size_t size = EDIT_HTM_GZ_END - EDIT_HTM_GZ_START;
        webServer.sendHeader(F("Content-Encoding"), F("gzip"));
        return webSendCached(200, PSTR("text/html"), (const char*)EDIT_HTM_GZ_START, size);
    }

    if(path == style_css) {
        size_t size = STYLE_CSS_GZ_END - STYLE_CSS_GZ_START;
        webServer.sendHeader(F("Content-Encoding"), F("gzip"));
        return webSendCached(200, PSTR("text/css"), (const char*)STYLE_CSS_GZ_START, size);
    }

    if(path == script_js) {
        size_t size = SCRIPT_JS_GZ_END - SCRIPT_JS_GZ_START;
        webServer.sendHeader(F("Content-Encoding"), F("gzip"));
        return webSendCached(200, PSTR("text/javascript"), (const char*)SCRIPT_JS_GZ_START, size);
    }
#endif // ARDUINO_ARCH_ESP32

    if(!strcasecmp_P(path.c_str(), PSTR("/favicon.ico")))
        return webSendCached(204, PSTR("image/bmp"), "", 0); // No content

    return 404; // Not found
}

static void handleFileUpload()
{
    if(webServer.uri() != "/edit") {
        return;
    }
    upload = &webServer.upload();
    if(upload->status == UPLOAD_FILE_START) {
        if(!httpIsAuthenticated(F("fileupload"))) return;
        LOG_INFO(TAG_HTTP, F("Total size: %s"), webServer.headerName(0).c_str());
        String filename((char*)0);
        filename.reserve(64);
        filename = upload->filename;
        if(!filename.startsWith("/")) {
            filename = "/";
            filename += upload->filename;
        }
        if(filename.length() < 32) {
            fsUploadFile = HASP_FS.open(filename, "w");
            if(!fsUploadFile || fsUploadFile.isDirectory()) {
                LOG_WARNING(TAG_HTTP, F(D_FILE_SAVE_FAILED), filename.c_str());
                fsUploadFile.close();
            } else {
                LOG_TRACE(TAG_HTTP, F("handleFileUpload Name: %s"), filename.c_str());
                haspProgressMsg(fsUploadFile.name());
            }
        } else {
            LOG_ERROR(TAG_HTTP, F("Filename %s is too long"), filename.c_str());
        }
    } else if(upload->status == UPLOAD_FILE_WRITE) {
        // DBG_OUTPUT_PORT.print("handleFileUpload Data: "); debugPrintln(upload.currentSize);
        if(fsUploadFile) {
            if(fsUploadFile.write(upload->buf, upload->currentSize) != upload->currentSize) {
                LOG_ERROR(TAG_HTTP, F("Failed to write received data to file"));
            } else {
                webUploadProgress(); // Moved to httpEverySecond Loop
            }
        }
    } else if(upload->status == UPLOAD_FILE_END) {
        if(fsUploadFile) {
            LOG_INFO(TAG_HTTP, F("Uploaded %s (%u bytes)"), fsUploadFile.name(), upload->totalSize);
            fsUploadFile.close();

            // Redirect to /config/hasp page. This flushes the web buffer and frees the memory
            webServer.sendHeader(String(F("Location")), String(F("/config/hasp")), true);
            webServer.send_P(302, PSTR("text/plain"), "");
        } else {
            webServer.send_P(400, PSTR("text/plain"), "Bad Request");
        }
        haspProgressVal(255);

        // httpReconnect();
    }
}

static void handleFileDelete()
{
    if(!httpIsAuthenticated(F("filedelete"))) return;

    char mimetype[16];
    snprintf_P(mimetype, sizeof(mimetype), PSTR("text/plain"));

    if(webServer.args() == 0) {
        return webServer.send_P(500, mimetype, PSTR("BAD ARGS"));
    }
    String path = webServer.arg(0);
    LOG_TRACE(TAG_HTTP, F("handleFileDelete: %s"), path.c_str());
    if(path == "/") {
        return webServer.send_P(500, mimetype, PSTR("BAD PATH"));
    }
    if(!HASP_FS.exists(path)) {
        return webServer.send_P(404, mimetype, PSTR("FileNotFound"));
    }
    HASP_FS.remove(path);
    webServer.send_P(200, mimetype, PSTR(""));
    // path.clear();
}

static void handleFileCreate()
{
    if(!httpIsAuthenticated(F("filecreate"))) return;

    if(webServer.args() == 0) {
        return webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
    }

    if(webServer.hasArg(F("path"))) {
        String path = webServer.arg(F("path"));
        LOG_TRACE(TAG_HTTP, F("handleFileCreate: %s"), path.c_str());
        if(path == "/") {
            return webServer.send(500, PSTR("text/plain"), PSTR("BAD PATH"));
        }
        if(HASP_FS.exists(path)) {
            return webServer.send(500, PSTR("text/plain"), PSTR("FILE EXISTS"));
        }
        File file = HASP_FS.open(path, "w");
        if(file) {
            file.close();
        } else {
            return webServer.send(500, PSTR("text/plain"), PSTR("CREATE FAILED"));
        }
    }
    if(webServer.hasArg(F("init"))) {
        dispatch_idle(NULL, "0", TAG_HTTP);
        hasp_init();
    }
    if(webServer.hasArg(F("load"))) {
        dispatch_idle(NULL, "0", TAG_HTTP);
        hasp_load_json();
    }
    if(webServer.hasArg(F("page"))) {
        uint8_t pageid = atoi(webServer.arg(F("page")).c_str());
        dispatch_idle(NULL, "0", TAG_HTTP);
        dispatch_set_page(pageid, LV_SCR_LOAD_ANIM_NONE);
    }
    webServer.send(200, PSTR("text/plain"), "");
}

static void handleFileList()
{
    if(!httpIsAuthenticated(F("filelist"))) return;

    if(!webServer.hasArg(F("dir"))) {
        webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
        return;
    }

    String path = webServer.arg(F("dir"));
    //  LOG_TRACE(TAG_HTTP, F("handleFileList: %s"), path.c_str());
    path.clear();

#if defined(ARDUINO_ARCH_ESP32)
    File root = HASP_FS.open("/", FILE_READ);
    File file = root.openNextFile();
    String output((char*)0);
    output.reserve(HTTP_PAGE_SIZE);
    output = "[";

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
    webServer.send(200, PSTR("text/json"), output);
#elif defined(ARDUINO_ARCH_ESP8266)
    Dir dir = HASP_FS.openDir(path);
    String output((char*)0);
    output.reserve(HTTP_PAGE_SIZE);
    output = "[";

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
    webServer.send(200, PSTR("text/json"), output);
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
static void webHandleConfig()
{ // http://plate01/config
    if(!httpIsAuthenticated(F("config"))) return;

    bool updated = saveConfig();

// Reboot after saving wifi config in AP mode
#if HASP_USE_WIFI > 0 && !defined(STM32F4xx)
    if(WiFi.getMode() != WIFI_STA) {
        httpHandleReboot();
    }
#endif

    {
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");

        if(updated) {
            httpMessage += F("<p class='info'>" D_HTTP_CONFIG_CHANGED "</p>");
        }

#if HASP_USE_WIFI > 0
        httpMessage += F("<a href='/config/wifi'>" D_HTTP_WIFI_SETTINGS "</a>");
#endif
#if HASP_USE_MQTT > 0
        httpMessage += F("<a href='/config/mqtt'>" D_HTTP_MQTT_SETTINGS "</a>");
#endif
        httpMessage += F("<a href='/config/http'>" D_HTTP_HTTP_SETTINGS "</a>");
        httpMessage += F("<a href='/config/gui'>" D_HTTP_GUI_SETTINGS "</a>");

#if HASP_USE_GPIO > 0
        httpMessage += F("<a href='/config/gpio'>" D_HTTP_GPIO_SETTINGS "</a>");
#endif

        httpMessage += F("<a href='/config/debug'>" D_HTTP_DEBUG_SETTINGS "</a>");
        httpMessage += F("<a href='/resetConfig' class='red'>" D_HTTP_FACTORY_RESET "</a>");
        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_MQTT > 0
static void webHandleMqttConfig()
{ // http://plate01/config/mqtt
    if(!httpIsAuthenticated(F("config/mqtt"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        mqttGetConfig(settings.to<JsonObject>());

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_MQTT_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config'>");

        // Node Name
        httpMessage +=
            F("<div class='row'><div class='col-25'><label class='required' for='name'>Plate Name</label></div>");
        httpMessage +=
            F("<div class='col-75'><input required type='text' id='name' name='name' maxlength=15 pattern='[a-z0-9_]*' "
              "placeholder='Plate Name' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_NAME)].as<String>();
        httpMessage += F("'></div></div>");

        // Group Name
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='group'>Group Name</label></div>");
        httpMessage +=
            F("<div class='col-75'><input type='text' id='group' name='group' maxlength=15 pattern='[a-z0-9_]*' "
              "placeholder='Group Name' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_GROUP)].as<String>();
        httpMessage += F("'></div></div>");

        // Broker
        httpMessage += F("<div class='row'><div class='col-25'><label for='host'>Broker</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='host' name='host' maxlength=");
        httpMessage += MAX_HOSTNAME_LENGTH - 1;
        httpMessage += F(" placeholder='Server Name' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_HOST)].as<String>();
        httpMessage += F("'></div></div>");

        // Mqtt Port
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='port'>Port</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='port' name='port' min='0' max='65535' "
                         "placeholder='1883' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>();
        httpMessage += F("'></div></div>");

        // Mqtt User
        httpMessage += F("<div class='row'><div class='col-25'><label for='user'>User</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='user' name='user' maxlength=");
        httpMessage += MAX_USERNAME_LENGTH - 1;
        httpMessage += F(" placeholder='MQTT User' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_USER)].as<String>();
        httpMessage += F("'></div></div>");

        // Mqtt Password
        httpMessage += F("<div class='row'><div class='col-25'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input type='password' id='pass' name='pass' maxlength=");
        httpMessage += MAX_PASSWORD_LENGTH - 1;
        httpMessage += F(" placeholder='MQTT Password' value='");
        // if(settings[FPSTR(FP_CONFIG_PASS)].as<String>() != "") httpMessage += F(D_PASSWORD_MASK);
        httpMessage += F("'></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='mqtt'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));
        httpMessage += "<script>filler(\"GET\", \"/api/config/mqtt/\")</script>";

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleGuiConfig()
{ // http://plate01/config/wifi
    if(!httpIsAuthenticated(F("config/gui"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        guiGetConfig(settings.to<JsonObject>());

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_GUI_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config'>");

        // Short Idle
        httpMessage += F("<div class='row'><div class='col-25'><label for='idle1'>Short Idle</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='idle1' name='idle1' min='0' max='32400' "
                         "value='");
        // httpMessage += settings[FPSTR(FP_GUI_IDLEPERIOD1)].as<String>();
        httpMessage += F("'></div></div>");

        // Long Idle
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='idle2'>Long Idle</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='idle2' name='idle2' min='0' max='32400' "
                         "value='");
        // httpMessage += settings[FPSTR(FP_GUI_IDLEPERIOD2)].as<String>();
        httpMessage += F("'></div></div>");

        // Rotation
        int8_t rotation = -1; // settings[FPSTR(FP_GUI_ROTATION)].as<int8_t>();
        httpMessage += F("<div class='row'><div class='col-25'><label for='group'>Orientation</label></div>");
        httpMessage += F("<div class='col-75'><select id='rotate' name='rotate'>");
        httpMessage += getOption(0, F("0 degrees"), rotation);
        httpMessage += getOption(1, F("90 degrees"), rotation);
        httpMessage += getOption(2, F("180 degrees"), rotation);
        httpMessage += getOption(3, F("270 degrees"), rotation);
        httpMessage += getOption(6, F("0 degrees - mirrored"), rotation);
        httpMessage += getOption(7, F("90 degrees - mirrored"), rotation);
        httpMessage += getOption(4, F("180 degrees - mirrored"), rotation);
        httpMessage += getOption(5, F("270 degrees - mirrored"), rotation);
        httpMessage += F("</select></div></div>");

        // Invert
        httpMessage += F("<div class='row'><div class='col-25'><label for='invert'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='invert' name='invert' value='1'");
        // if(settings[FPSTR(FP_GUI_INVERT)].as<bool>()) httpMessage += F(" checked");
        httpMessage += F(">Invert Colors</div></div>");

        // Cursor
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='cursor'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='cursor' name='cursor' value='1'");
        // if(settings[FPSTR(FP_GUI_POINTER)].as<bool>()) httpMessage += F(" checked");
        httpMessage += F(">Show Pointer</div></div>");

        // Backlight
        int8_t bcklpin = settings[FPSTR(FP_GUI_BACKLIGHTPIN)].as<int8_t>();
        httpMessage += F("<div class='row'><div class='col-25'><label for='group'>Backlight Control</label></div>");
        httpMessage += F("<div class='col-75'><select id='bckl' name='bckl'>");
        httpMessage += getOption(-1, F("None"), bcklpin);
#if defined(ARDUINO_ARCH_ESP32)
        char buffer[10];
        uint8_t pins[] = {0, 5, 12, 13, 15, 16, 17, 18, 19, 21, 22, 23, 27, 32};
        for(uint8_t i = 0; i < sizeof(pins); i++) {
            // if(!gpioIsSystemPin(pins[i])) {
            snprintf_P(buffer, sizeof(buffer), PSTR("GPIO %d"), pins[i]);
            httpMessage += getOption(pins[i], buffer, bcklpin);
            // }
        }
#else
        httpMessage += getOption(5, F("D1 - GPIO 5"), bcklpin);
        httpMessage += getOption(4, F("D2 - GPIO 4"), bcklpin);
        httpMessage += getOption(0, F("D3 - GPIO 0"), bcklpin);
        httpMessage += getOption(2, F("D4 - GPIO 2"), bcklpin);
#endif
        httpMessage += F("</select></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='gui'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

#if TOUCH_DRIVER == 0x2046 && defined(TOUCH_CS)
        add_form_button(httpMessage, F(D_HTTP_CALIBRATE), F("/config/gui?cal=1"));
#endif

        add_form_button(httpMessage, F(D_HTTP_ANTIBURN), F("/config/gui?brn=1"));
        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));
        httpMessage += F("<script>filler(\"GET\",\"/api/config/gui/\")</script>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    { // Execute Actions
        if(webServer.hasArg(F("cal"))) dispatch_calibrate(NULL, NULL, TAG_HTTP);
        if(webServer.hasArg(F("brn"))) dispatch_antiburn(NULL, "on", TAG_HTTP);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_WIFI > 0
static void webHandleWifiConfig()
{ // http://plate01/config/wifi
    if(!httpIsAuthenticated(F("config/wifi"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        wifiGetConfig(settings.to<JsonObject>());

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_WIFI_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config'>");

        // Wifi SSID
        httpMessage += F("<div class='row'><div class='col-25 required'><label for='ssid'>SSID</label></div>");
        httpMessage += F("<div class='col-75'><input required type='text' id='ssid' name='ssid' maxlength=");
        httpMessage += MAX_USERNAME_LENGTH - 1;
        httpMessage += F(" placeholder='SSID' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_SSID)].as<String>();
        httpMessage += F("'></div></div>");

        // Wifi Password
        httpMessage += F("<div class='row'><div class='col-25 required'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input required type='password' id='pass' name='pass' maxlength=");
        httpMessage += MAX_PASSWORD_LENGTH - 1;
        httpMessage += F(" placeholder='Password' value='");
        // if(settings[FPSTR(FP_CONFIG_PASS)].as<String>() != "") {
        //     httpMessage += F(D_PASSWORD_MASK);
        // }
        httpMessage += F("'></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='wifi'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

#if HASP_USE_WIFI > 0 && !defined(STM32F4xx)
        if(WiFi.getMode() == WIFI_STA) {
            add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
        } else {
            add_form_button(httpMessage, F(D_HTTP_FIRMWARE_UPGRADE), F("/firmware"));
#endif // ARDUINO_ARCH_ESP
        }
#endif // HASP_USE_WIFI

        httpMessage += F("<script>filler(\"GET\",\"/api/config/wifi/\")</script>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

#endif // HASP_USE_WIFI

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleHttpConfig()
{ // http://plate01/config/http
    if(!httpIsAuthenticated(F("config/http"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        httpGetConfig(settings.to<JsonObject>());

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_HTTP_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config'>");

        // Username
        httpMessage += F("<div class='row'><div class='col-25'><label for='user'>Username</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='user' name='user' maxlength=31 "
                         "placeholder='Username' value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_USER)].as<String>();
        httpMessage += F("'></div></div>");

        // Password
        httpMessage += F("<div class='row'><div class='col-25'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input type='password' id='pass' name='pass' maxlength=63 "
                         "placeholder='Password' value='");
        // if(settings[FPSTR(FP_CONFIG_PASS)].as<String>() != "") {
        //     httpMessage += F(D_PASSWORD_MASK);
        // }
        httpMessage += F("'></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='http'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        httpMessage += F("<a href='/config'>" D_HTTP_CONFIGURATION "</a>");
        httpMessage += F("<script>filler(\"GET\",\"/api/config/http/\")</script>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_GPIO > 0
static void webHandleGpioConfig()
{ // http://plate01/config/gpio
    if(!httpIsAuthenticated(F("config/gpio"))) return;
    uint8_t configCount = 0;

    { // Execute Actions
        uint8_t id  = webServer.arg(F("id")).toInt();
        uint8_t pin = webServer.arg(F("pin")).toInt();

        if(webServer.hasArg(PSTR("save"))) {
            uint8_t type    = webServer.arg(F("type")).toInt();
            uint8_t group   = webServer.arg(F("group")).toInt();
            uint8_t pinfunc = webServer.arg(F("func")).toInt();
            bool inverted   = webServer.arg(F("state")).toInt();
            gpioSavePinConfig(id, pin, type, group, pinfunc, inverted);
        }

        if(webServer.hasArg(PSTR("del"))) {
            gpioSavePinConfig(id, pin, hasp_gpio_type_t::FREE, 0, 0, false);
        }
    }

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_GPIO_SETTINGS "</h2>");

        httpMessage += F("<form method='POST' action='/config'>");

        httpMessage += F("<table><tr><th>" D_GPIO_PIN "</th><th>Type</th><th>" D_GPIO_GROUP
                         "</th><th>Default</th><th>Action</th></tr>");

        for(uint8_t gpio = 0; gpio < NUM_DIGITAL_PINS; gpio++) {
            for(uint8_t id = 0; id < HASP_NUM_GPIO_CONFIG; id++) {
                hasp_gpio_config_t conf = gpioGetPinConfig(id);
                if((conf.pin == gpio) && gpioConfigInUse(id) && !gpioIsSystemPin(gpio)) {
                    httpMessage += F("<tr><td>");
                    // httpMessage += halGpioName(gpio);
                    httpMessage += haspDevice.gpio_name(gpio).c_str();
                    if(conf.type >= 0x80) {
                        httpMessage += F("</td><td><a href='/config/gpio/input?id=");
                    } else {
                        httpMessage += F("</td><td><a href='/config/gpio/options?id=");
                    }
                    httpMessage += id;
                    httpMessage += F("'>");

                    switch(conf.type) {

                        case hasp_gpio_type_t::BUTTON:
                            httpMessage += F(D_GPIO_BUTTON);
                            break;
                        case hasp_gpio_type_t::SWITCH:
                            httpMessage += F(D_GPIO_SWITCH);
                            break;
                        case hasp_gpio_type_t::DOOR:
                            httpMessage += F("door");
                            break;
                        case hasp_gpio_type_t::GARAGE_DOOR:
                            httpMessage += F("garage_door");
                            break;
                        case hasp_gpio_type_t::GAS:
                            httpMessage += F("gas");
                            break;
                        case hasp_gpio_type_t::LIGHT:
                            httpMessage += F("light");
                            break;
                        case hasp_gpio_type_t::LOCK:
                            httpMessage += F("lock");
                            break;
                        case hasp_gpio_type_t::MOISTURE:
                            httpMessage += F("moisture");
                            break;
                        case hasp_gpio_type_t::MOTION:
                            httpMessage += F("motion");
                            break;
                        case hasp_gpio_type_t::OCCUPANCY:
                            httpMessage += F("occupancy");
                            break;
                        case hasp_gpio_type_t::OPENING:
                            httpMessage += F("opening");
                            break;
                        case hasp_gpio_type_t::PRESENCE:
                            httpMessage += F("presence");
                            break;
                        case hasp_gpio_type_t::PROBLEM:
                            httpMessage += F("problem");
                            break;
                        case hasp_gpio_type_t::SAFETY:
                            httpMessage += F("Safety");
                            break;
                        case hasp_gpio_type_t::SMOKE:
                            httpMessage += F("Smoke");
                            break;
                        case hasp_gpio_type_t::VIBRATION:
                            httpMessage += F("Vibration");
                            break;
                        case hasp_gpio_type_t::WINDOW:
                            httpMessage += F("Window");
                            break;

                        case hasp_gpio_type_t::TOUCH:
                            httpMessage += F(D_GPIO_TOUCH);
                            break;
                        case hasp_gpio_type_t::LED:
                            httpMessage += F(D_GPIO_LED);
                            break;
                        case hasp_gpio_type_t::LED_R:
                            httpMessage += F(D_GPIO_LED_R);
                            break;
                        case hasp_gpio_type_t::LED_G:
                            httpMessage += F(D_GPIO_LED_G);
                            break;
                        case hasp_gpio_type_t::LED_B:
                            httpMessage += F(D_GPIO_LED_B);
                            break;
                        case hasp_gpio_type_t::LIGHT_RELAY:
                            httpMessage += F(D_GPIO_LIGHT_RELAY);
                            break;
                        case hasp_gpio_type_t::POWER_RELAY:
                            httpMessage += F(D_GPIO_POWER_RELAY);
                            break;
                        case hasp_gpio_type_t::SHUTTER_RELAY:
                            httpMessage += F("SHUTTER_RELAY");
                            break;
                        case hasp_gpio_type_t::PWM:
                            httpMessage += F(D_GPIO_PWM);
                            break;
                        case hasp_gpio_type_t::HASP_DAC:
                            httpMessage += F(D_GPIO_DAC);
                            break;

#if defined(LANBONL8)
                            // case hasp_gpio_type_t::SERIAL_DIMMER:
                            //     httpMessage += F(D_GPIO_SERIAL_DIMMER);
                            //     break;
                        case hasp_gpio_type_t::SERIAL_DIMMER_EU:
                            httpMessage += F("L8-HD (EU)");
                            break;
                        case hasp_gpio_type_t::SERIAL_DIMMER_AU:
                            httpMessage += F("L8-HD (AU)");
                            break;
#endif
                        default:
                            httpMessage += F(D_GPIO_UNKNOWN);
                    }

                    httpMessage += F("</a></td><td>");
                    httpMessage += conf.group;
                    httpMessage += F("</td><td>");
                    httpMessage += (conf.inverted) ? F(D_GPIO_STATE_INVERTED) : F(D_GPIO_STATE_NORMAL);

                    httpMessage += ("</td><td><a href='/config/gpio?del=&id=");
                    httpMessage += id;
                    httpMessage += ("&pin=");
                    httpMessage += conf.pin;
                    httpMessage += ("' class='trash'></a></td><tr>");
                    configCount++;
                }
            }
        }

        httpMessage += F("</table></form>");

        if(configCount < HASP_NUM_GPIO_CONFIG) {
            httpMessage += F("<a href='gpio/input?id=");
            httpMessage += gpioGetFreeConfigId();
            httpMessage += F("'>" D_HTTP_ADD_GPIO " Input</a>");

            httpMessage += F("<a href='gpio/options?id=");
            httpMessage += gpioGetFreeConfigId();
            httpMessage += F("'>" D_HTTP_ADD_GPIO " Output</a>");
        }

        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleGpioOutput()
{ // http://plate01/config/gpio/options
    if(!httpIsAuthenticated(F("config/gpio/options"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        guiGetConfig(settings.to<JsonObject>());

        uint8_t config_id = webServer.arg(F("id")).toInt();

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");

        httpMessage += F("<form method='GET' action='/config/gpio'>");
        httpMessage += F("<input type='hidden' name='id' value='");
        httpMessage += config_id;
        httpMessage += F("'>");

        httpMessage += F("<p><b>GPIO Output</b></p>");

        httpMessage += F("<p><b>" D_GPIO_PIN "</b> <select id='pin' name='pin'>");
        hasp_gpio_config_t conf = gpioGetPinConfig(config_id);

        for(uint8_t io = 0; io < NUM_DIGITAL_PINS; io++) {
            if(((conf.pin == io) || !gpioInUse(io)) && !gpioIsSystemPin(io)) {
                httpMessage += getOption(io, haspDevice.gpio_name(io).c_str(), conf.pin);
            }
        }
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>Type</b> <select id='type' name='type'>");
        httpMessage += getOption(hasp_gpio_type_t::LED, F(D_GPIO_LED), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::LED_R, F(D_GPIO_LED_R), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::LED_G, F(D_GPIO_LED_G), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::LED_B, F(D_GPIO_LED_B), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::LIGHT_RELAY, F(D_GPIO_LIGHT_RELAY), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::POWER_RELAY, F(D_GPIO_POWER_RELAY), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::SHUTTER_RELAY, F("Shutter Relay"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::HASP_DAC, F(D_GPIO_DAC), conf.type);
        // httpMessage += getOption(hasp_gpio_type_t::SERIAL_DIMMER, F(D_GPIO_SERIAL_DIMMER), conf.type);
#if defined(LANBONL8)
        httpMessage += getOption(hasp_gpio_type_t::SERIAL_DIMMER_AU, F("L8-HD (AU)"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::SERIAL_DIMMER_EU, F("L8-HD (EU)"), conf.type);
#endif
        if(digitalPinHasPWM(webServer.arg(0).toInt())) {
            httpMessage += getOption(hasp_gpio_type_t::PWM, F(D_GPIO_PWM), conf.type);
        }
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>" D_GPIO_GROUP "</b> <select id='group' name='group'>");
        httpMessage += getOption(0, F(D_GPIO_GROUP_NONE), conf.group);
        String group((char*)0);
        group.reserve(10);
        for(int i = 1; i < 15; i++) {
            group = F(D_GPIO_GROUP " ");
            group += i;
            httpMessage += getOption(i, group, conf.group);
        }
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>Value</b> <select id='state' name='state'>");
        httpMessage += getOption(0, F(D_GPIO_STATE_NORMAL), conf.inverted);
        httpMessage += getOption(1, F(D_GPIO_STATE_INVERTED), conf.inverted);
        httpMessage += F("</select></p>");

        httpMessage +=
            F("<p><button type='submit' name='save' value='gpio'>" D_HTTP_SAVE_SETTINGS "</button></p></form>");

        httpMessage += PSTR("<p><form method='GET' action='/config/gpio'><button type='submit'>&#8617; " D_HTTP_BACK
                            "</button></form></p>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    // if(webServer.hasArg(F("action"))) dispatch_text_line(webServer.arg(F("action")).c_str()); // Security check
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleGpioInput()
{ // http://plate01/config/gpio/options
    if(!httpIsAuthenticated(F("config/gpio/input"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        guiGetConfig(settings.to<JsonObject>());

        uint8_t config_id = webServer.arg(F("id")).toInt();

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");

        httpMessage += F("<form method='GET' action='/config/gpio'>");
        httpMessage += F("<input type='hidden' name='id' value='");
        httpMessage += config_id;
        httpMessage += F("'>");

        httpMessage += F("<p><b>GPIO Input</b></p>");

        httpMessage += F("<p><b>" D_GPIO_PIN "</b> <select id='pin' name='pin'>");
        hasp_gpio_config_t conf = gpioGetPinConfig(config_id);

        for(uint8_t io = 0; io < NUM_DIGITAL_PINS; io++) {
            if(((conf.pin == io) || !gpioInUse(io)) && !gpioIsSystemPin(io)) {
                httpMessage += getOption(io, haspDevice.gpio_name(io).c_str(), conf.pin);
            }
        }
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>Type</b> <select id='type' name='type'>");
        httpMessage += getOption(hasp_gpio_type_t::BUTTON, F(D_GPIO_BUTTON), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::SWITCH, F(D_GPIO_SWITCH), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::DOOR, F("door"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::GARAGE_DOOR, F("garage_door"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::GAS, F("gas"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::LIGHT, F("light"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::LOCK, F("lock"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::MOISTURE, F("moisture"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::MOTION, F("motion"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::OCCUPANCY, F("occupancy"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::OPENING, F("opening"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::PRESENCE, F("presence"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::PROBLEM, F("problem"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::SAFETY, F("Safety"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::SMOKE, F("Smoke"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::VIBRATION, F("Vibration"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::WINDOW, F("Window"), conf.type);
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>" D_GPIO_GROUP "</b> <select id='group' name='group'>");
        httpMessage += getOption(0, F(D_GPIO_GROUP_NONE), conf.group);
        String group((char*)0);
        group.reserve(10);
        for(int i = 1; i < 15; i++) {
            group = F(D_GPIO_GROUP " ");
            group += i;
            httpMessage += getOption(i, group, conf.group);
        }
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>Default State</b> <select id='state' name='state'>");
        httpMessage += getOption(0, F("Normally Open"), conf.inverted);
        httpMessage += getOption(1, F("Normally Closed"), conf.inverted);
        httpMessage += F("</select></p>");

        httpMessage += F("<p><b>Resistor</b> <select id='func' name='func'>");
        httpMessage += getOption(hasp_gpio_function_t::INTERNAL_PULLUP, F("Internal Pullup"), conf.gpio_function);
        httpMessage += getOption(hasp_gpio_function_t::INTERNAL_PULLDOWN, F("Internal Pulldown"), conf.gpio_function);
        httpMessage += getOption(hasp_gpio_function_t::EXTERNAL_PULLUP, F("External Pullup"), conf.gpio_function);
        httpMessage += getOption(hasp_gpio_function_t::EXTERNAL_PULLDOWN, F("External Pulldown"), conf.gpio_function);
        httpMessage += F("</select></p>");

        httpMessage +=
            F("<p><button type='submit' name='save' value='gpio'>" D_HTTP_SAVE_SETTINGS "</button></p></form>");

        httpMessage += PSTR("<p><form method='GET' action='/config/gpio'><button type='submit'>&#8617; " D_HTTP_BACK
                            "</button></form></p>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    // if(webServer.hasArg(F("action"))) dispatch_text_line(webServer.arg(F("action")).c_str()); // Security check
}
#endif // HASP_USE_GPIO

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleDebugConfig()
{ // http://plate01/config/debug
    if(!httpIsAuthenticated(F("config/debug"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        debugGetConfig(settings.to<JsonObject>());

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_DEBUG_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config'>");

        // Baudrate
        uint16_t baudrate = settings[FPSTR(FP_CONFIG_BAUD)].as<uint16_t>();
        httpMessage += F("<div class='row'><div class='col-25'><label for='baud'>Serial Port</label></div>");
        httpMessage += F("<div class='col-75'><select id='baud' name='baud'>");
        httpMessage += getOption(1, F(D_SETTING_DISABLED), baudrate); // Don't use 0 here which is default 115200
        httpMessage += getOption(960, F("9600"), baudrate);
        httpMessage += getOption(1920, F("19200"), baudrate);
        httpMessage += getOption(3840, F("38400"), baudrate);
        httpMessage += getOption(5760, F("57600"), baudrate);
        httpMessage += getOption(7488, F("74880"), baudrate);
        httpMessage += getOption(11520, F("115200"), baudrate);
        httpMessage += F("</select></div></div>");

        // Telemetry Period
        httpMessage += F("<div class='row'><div class='col-25'><label for='tele'>Telemetry Period</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='tele' name='tele' min='0' max='65535' "
                         "value='");
        // httpMessage += settings[FPSTR(FP_DEBUG_TELEPERIOD)].as<String>();
        httpMessage += F("'></div></div>");

        // Invert
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='ansi'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='ansi' name='ansi' value='1'");
        // if(settings[FPSTR(FP_DEBUG_ANSI)].as<bool>()) httpMessage += F(" checked");
        httpMessage += F(">Use ANSI Colors</div></div>");

#if HASP_USE_SYSLOG > 0
        // Syslog host
        httpMessage += F("<div class='row'><div class='col-25'><label for='host'>Syslog Server</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='host' name='host' maxlength=");
        httpMessage += MAX_HOSTNAME_LENGTH - 1;
        httpMessage += F(" value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_HOST)].as<String>();
        httpMessage += F("'></div></div>");

        // Syslog Port
        httpMessage += F("<div class='row'><div class='col-25'><label for='port'>Syslog Port</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='port' name='port' min='0' max='65535' "
                         "value='");
        // httpMessage += settings[FPSTR(FP_CONFIG_PORT)].as<String>();
        httpMessage += F("'></div></div>");

        // Syslog Facility
        uint8_t logid = settings[FPSTR(FP_CONFIG_LOG)].as<uint8_t>();
        httpMessage += F("<div class='row'><div class='col-25'><label for='log'>Syslog Facility</label></div>");
        httpMessage += F("<div class='col-75'><select id='log' name='log'>");
        for(int i = 0; i < 8; i++) {
            httpMessage += getOption(i, String(F("Local")) + i, logid);
        }
        httpMessage += F("</select></div></div>");

        // Syslog Protocol
        uint8_t proto = settings[FPSTR(FP_CONFIG_PROTOCOL)].as<uint8_t>();
        httpMessage += F("<div class='row'><div class='col-25'><label for='proto'>Syslog Protocol</label></div>");
        httpMessage += F("<div class='col-75'><input id='proto' name='proto' type='radio' value='0'");
        // if(proto == 0) httpMessage += F(" checked");
        httpMessage += F(">IETF (RFC 5424) &nbsp; <input id='proto' name='proto' type='radio' value='1'");
        // if(proto == 1) httpMessage += F(" checked");
        httpMessage += F(">BSD (RFC 3164)");
        httpMessage += F("</div></div>");
#endif

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='debug'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        // *******************************************************************

        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));
        httpMessage += F("<script>filler(\"GET\",\"/api/config/debug/\")</script>");

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleHaspConfig()
{ // http://plate01/config/http
    if(!httpIsAuthenticated(F("config/hasp"))) return;

    { // Send Content
        StaticJsonDocument<256> settings;
        haspGetConfig(settings.to<JsonObject>());

        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_HASP_DESIGN "</h2>");

#if defined(ARDUINO_ARCH_ESP8266)
        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/edit' enctype='multipart/form-data'>");

        // File
        httpMessage +=
            F("<div class='row'><div class='col-25'><label class='required' for='filename'>From File</label></div>");
        httpMessage +=
            F("<div class='col-75'><input required type='file' name='filename'  accept='.jsonl,.png,.zi'></div></div>");

        // Destination
        httpMessage += F("<div class='row'><div class='col-25'><label for='file'>Target</label></div>");
        httpMessage +=
            F("<div class='col-75'><input id='cmd' name='cmd' type='radio' value='0' checked>Firmware &nbsp; "
              "<input id='cmd' name='cmd' type='radio' value='100'>Filesystem</div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='debug'>" D_HTTP_UPLOAD_FILE "</button>");
        httpMessage += F("</form></div>");
#endif

        // Form
        // httpMessage += F("<div class='container'><form method='POST' action='/config'>");
        httpMessage += F("<div class='container'><form method='POST' action='/'>");

        // Theme
        httpMessage += F("<div class='row'><div class='col-25'><label for='theme'>UI Theme</label></div>");
        httpMessage += F("<div class='col-75'><select id='theme' name='theme'>");
        uint8_t themeid = settings[FPSTR(FP_CONFIG_THEME)].as<uint8_t>();
#if LV_USE_THEME_HASP == 1
        httpMessage += getOption(2, F("Hasp Dark"), themeid);
        httpMessage += getOption(1, F("Hasp Light"), themeid);
#endif
#if LV_USE_THEME_EMPTY == 1
        httpMessage += getOption(0, F("Empty"), themeid);
#endif
#if LV_USE_THEME_MONO == 1
        httpMessage += getOption(3, F("Mono"), themeid);
#endif
#if LV_USE_THEME_MATERIAL == 1
        httpMessage += getOption(5, F("Material Dark"), themeid);
        httpMessage += getOption(4, F("Material Light"), themeid);
#endif
#if LV_USE_THEME_TEMPLATE == 1
        httpMessage += getOption(7, F("Template"), themeid);
#endif
        httpMessage += F("</select></div></div>");

        // Hue
        httpMessage += F("<div class='row'><div class='col-25'><label for='hue'>Hue</label></div>");
        httpMessage += F("<div class='col-75'><div style='width:100%;background-image:linear-gradient(to "
                         "right,red,orange,yellow,green,blue,indigo,violet,red);'><input "
                         "style='align:center;padding:0px;width:100%;' "
                         "name='hue' type='range' min='0' max='360' value='");
        httpMessage += settings[FPSTR(FP_CONFIG_HUE)].as<String>();
        httpMessage += F("'></div></div></div>");

        // Font
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='font'>Default Font</label></div>");
        httpMessage += F("<div class='col-75'><select id='font' name='font'><option value=''>None</option>");
#if defined(ARDUINO_ARCH_ESP32)
        File root        = HASP_FS.open("/");
        File file        = root.openNextFile();
        String main_font = settings[FPSTR(FP_CONFIG_ZIFONT)].as<String>();

        while(file) {
            String filename = file.name();
            // if(filename.endsWith(".zi")) httpMessage += getOption(filename, filename, main_font);
            file = root.openNextFile();
        }
#elif defined(ARDUINO_ARCH_ESP8266)
        Dir dir = HASP_FS.openDir("/");
        String main_font = settings[FPSTR(FP_CONFIG_ZIFONT)].as<String>();

        while(dir.next()) {
            File file = dir.openFile("r");
            String filename = file.name();
            if(filename.endsWith(".zi")) httpMessage += getOption(filename, filename, main_font);
            file.close();
        }
#endif
        httpMessage += F("</select></div></div>");

        // Pages.jsonl
        httpMessage += F("<div class='row'><div class='col-25'><label for='pages'>Startup Layout</label></div>");
        httpMessage +=
            F("<div class='col-75'><input id='pages' name='pages' maxlength=31 placeholder='/pages.jsonl' value='");
        httpMessage += settings[FPSTR(FP_CONFIG_PAGES)].as<String>();
        httpMessage += F("'></div></div>");

        // Startup Page
        httpMessage += F("<div class='row'><div class='col-25'><label for='startpage'>Startup Page</label></div>");
        httpMessage += F("<div class='col-75'><input id='startpage' required "
                         "name='startpage' type='number' min='1' max='12' value='");
        httpMessage += settings[FPSTR(FP_CONFIG_STARTPAGE)].as<String>();
        httpMessage += F("'></div></div>");

        // Startup Brightness
        httpMessage += F("<div class='row'><div class='col-25'><label for='startdim'>Startup Brightness</label></div>");
        httpMessage += F("<div class='col-75'><input id='startpage' required name='startdim' type='number' min='0' "
                         "max='255' value='");
        httpMessage += settings[FPSTR(FP_CONFIG_STARTDIM)].as<String>();
        httpMessage += F("'></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='hasp'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}
#endif // HASP_USE_CONFIG

////////////////////////////////////////////////////////////////////////////////////////////////////
static void httpHandleFileFromFlash()
{ // webServer 404
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    int statuscode = handleFileRead(webServer.uri());
#else
    int statuscode = 404;
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    LOG_TRACE(TAG_HTTP, F("Sending %d %s to client connected from: %s"), statuscode, webServer.uri().c_str(),
              webServer.client().remoteIP().toString().c_str());
#else
    // LOG_TRACE(TAG_HTTP,F("Sending 404 to client connected from: %s"),
    // String(webServer.client().remoteIP()).c_str());
#endif

    if(statuscode < 300) return; // OK

    String httpMessage((char*)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);

    if(statuscode == 500)
        httpMessage += F("Internal Server Error");
    else
        httpMessage += F(D_FILE_NOT_FOUND);

    httpMessage += F("\n\nURI: ");
    httpMessage += webServer.uri();
    httpMessage += F("\nMethod: ");
    httpMessage += (webServer.method() == HTTP_GET) ? F("GET") : F("POST");
    httpMessage += F("\nArguments: ");
    httpMessage += webServer.args();
    httpMessage += "\n";
    for(int i = 0; i < webServer.args(); i++) {
        httpMessage += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }
    webServer.send(statuscode, PSTR("text/plain"), httpMessage.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleFirmware()
{
    if(!httpIsAuthenticated(F("firmware"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_FIRMWARE_UPGRADE "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/update' enctype='multipart/form-data'>");

        // File
        httpMessage +=
            F("<div class='row'><div class='col-25'><label class='required' for='filename'>From File</label></div>");
        httpMessage += F("<div class='col-75'><input required type='file' name='filename' accept='.bin'></div></div>");

        // Destination
        httpMessage += F("<div class='row'><div class='col-25'><label for='file'>Target</label></div>");
        httpMessage +=
            F("<div class='col-75'><input id='cmd' name='cmd' type='radio' value='0' checked>Firmware &nbsp; "
              "<input id='cmd' name='cmd' type='radio' value='100'>Filesystem</div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='debug'>" D_HTTP_UPDATE_FIRMWARE "</button>");
        httpMessage += F("</form></div>");

        /* Update from URL
            // Form
            httpMessage += F("<div class='container'><form method='POST' action='/espfirmware'>");

            // URL
            httpMessage +=
                F("<div class='row'><div class='col-25'><label class='required' for='url'>From URL</label></div>");
            httpMessage += F("<div class='col-75'><input required id='url' name='url' value=''></div></div>");

            // Submit & End Form
            httpMessage += F("<button type='submit' name='save' value='debug'>Update from URL</button>");
            httpMessage += F("</form></div>");
        */

        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), false);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void httpHandleEspFirmware()
{ // http://plate01/espfirmware
    if(!httpIsAuthenticated(F("espfirmware"))) return;
    if(!webServer.hasArg(F("url"))) return;

    const char* url = webServer.arg(F("url")).c_str();
    {
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");

        httpMessage += F("<p><b>ESP update</b></p>Updating ESP firmware from: ");
        httpMessage += url;

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), true);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    LOG_TRACE(TAG_HTTP, F("Updating ESP firmware from: %s"), url);
    dispatch_web_update(NULL, url, TAG_HTTP);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
static void webHandleSaveConfig()
{
    if(!httpIsAuthenticated(F("saveConfig"))) return;

    configWrite();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void httpHandleResetConfig()
{ // http://plate01/resetConfig
    if(!httpIsAuthenticated(F("resetConfig"))) return;

    bool resetConfirmed = webServer.arg(F("confirm")) == F("yes");

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_FACTORY_RESET "</h2>");

        if(resetConfirmed) {                           // User has confirmed, so reset everything
            bool formatted = dispatch_factory_reset(); // configClearEeprom();
            if(formatted) {
                httpMessage += F("<div class=\"success\">Reset all saved settings. Restarting device...</div>");
            } else {
                httpMessage +=
                    F("<div class=\"error\">Failed to reset the internal storage to factory settings!</div>");
                resetConfirmed = false;
            }
        } else {
            // Form
            httpMessage += F("<form method='POST' action='/resetConfig'>");
            httpMessage +=
                F("<div class=\"warning\"><b>Warning</b><p>This process will reset all settings to the default values. "
                  "The internal flash will be erased and the device is restarted. You may need to connect to the WiFi "
                  "AP displayed on the panel to reconfigure the device before accessing it again.</p>"
                  "<p>ALL FILES WILL BE LOST!</p></div>");
            httpMessage += F("<p><button class='red' type='submit' name='confirm' value='yes'>" D_HTTP_ERASE_DEVICE
                             "</button></p></form>");

            add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));
        }

        webSendHeader(haspDevice.get_hostname(), httpMessage.length(), resetConfirmed);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    { // Execute Actions
        if(resetConfirmed) {
            // delay(250);
            dispatch_reboot(false); // Do NOT save the current config
        }
    }
}
#endif // HASP_USE_CONFIG

void httpStart()
{
    webServer.begin();
    webServerStarted = true;
#if HASP_USE_WIFI > 0
#if defined(STM32F4xx)
    IPAddress ip;
    ip = WiFi.localIP();
    LOG_INFO(TAG_HTTP, F(D_SERVICE_STARTED " @ http://%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
#else
    LOG_INFO(TAG_HTTP, F(D_SERVICE_STARTED " @ http://%s"),
             (WiFi.getMode() != WIFI_STA ? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str()));
#endif
#else
    IPAddress ip;
#if defined(ARDUINO_ARCH_ESP32)
    ip = ETH.localIP();
#else
    ip = Ethernet.localIP();
#endif
    LOG_INFO(TAG_HTTP, F(D_SERVICE_STARTED " @ http://%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
#endif
}

void httpStop()
{
    webServer.stop();
    webServerStarted = false;
    LOG_WARNING(TAG_HTTP, F(D_SERVICE_STOPPED));
}

// Do not keep CSS in memory because it is cached in the browser
static void webSendCssVars()
{
    char filename[32];
    strncpy(filename, webServer.uri().c_str(), sizeof(filename));

    if(HASP_FS.exists(filename)) {
        String contentType((char*)0);
        contentType = getContentType(filename);
        File file   = HASP_FS.open(filename, "r");
        webSendCacheHeader(file.size(), 3600);
        webServer.streamFile(file, contentType);
        file.close();
        return;
    }

    String HTTP_CSS = F(":root{"
                        "--txt:" D_HTTP_COLOR_TEXT ";"
                        "--bg:" D_HTTP_COLOR_BACKGROUND ";"
                        "--btnfg:" D_HTTP_COLOR_BUTTON_TEXT ";"
                        "--btnbg:" D_HTTP_COLOR_BUTTON ";"
                        "--btnbghi:" D_HTTP_COLOR_BUTTON_HOVER ";"
                        "--btnred:" D_HTTP_COLOR_BUTTON_RESET ";"
                        "--btnredhi:" D_HTTP_COLOR_BUTTON_RESET_HOVER ";"
                        "--btnbrd: transparent;"
                        "--grpfg:" D_HTTP_COLOR_GROUP_TEXT ";"
                        "--grpbg:" D_HTTP_COLOR_GROUP ";"
                        "--fldbg:" D_HTTP_COLOR_INPUT ";"
                        "--fldfg:" D_HTTP_COLOR_INPUT_TEXT ";"
                        "--fldred:" D_HTTP_COLOR_INPUT_WARNING ";"
                        "--footfg:" D_HTTP_COLOR_FOOTER_TEXT ";"
                        "}");
    webSendCached(200, PSTR("text/css"), HTTP_CSS.c_str(), HTTP_CSS.length());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void webStartConfigPortal()
{
#if HASP_USE_CAPTIVE_PORTAL > 0
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", apIP);
#endif // HASP_USE_CAPTIVE_PORTAL

    // replay to all requests with same HTML
    webServer.onNotFound([]() { webHandleWifiConfig(); });

    webServer.on(F("/style.css"), httpHandleFileFromFlash);
    webServer.on(F("/script.js"), httpHandleFileFromFlash);

    LOG_TRACE(TAG_HTTP, F("Wifi access point"));
}

void httpSetup()
{
    // httpSetConfig(settings);

    // ask server to track these headers
    const char* headerkeys[] = {"Content-Length"}; // "Authentication"
    size_t headerkeyssize    = sizeof(headerkeys) / sizeof(char*);
    webServer.collectHeaders(headerkeys, headerkeyssize);

    // Shared pages between STA and AP
    webServer.on(F("/about"), webHandleAbout);
    webServer.on(F("/vars.css"), webSendCssVars);
    // webServer.on(F("/js"), webSendJavascript);
    webServer.on(UriBraces(F("/api/{}/")), webHandleApi);
    webServer.on(UriBraces(F("/api/config/{}/")), webHandleApiConfig);

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    webServer.on(F("/firmware"), webHandleFirmware);
    webServer.on(
        F("/update"), HTTP_POST,
        []() {
            webServer.send(200, "text/plain", "");
            LOG_VERBOSE(TAG_HTTP, F("Total size: %s"), webServer.hostHeader().c_str());
        },
        webHandleFirmwareUpload);
    // webServer.on(F("/espfirmware"), httpHandleEspFirmware);
#endif

#if HASP_USE_WIFI > 0
    // These two endpoints are needed in STA and AP mode
    webServer.on(F("/config"), webHandleConfig);

#if !defined(STM32F4xx)

#if HASP_USE_CONFIG > 0
    if(WiFi.getMode() != WIFI_STA) {
        webStartConfigPortal();
        return;
    }

#endif // HASP_USE_CONFIG
#endif // !STM32F4xx
#endif // HASP_USE_WIFI

    // The following endpoints are only needed in STA mode
    webServer.on(F("/page/"), []() {
        String pageid = webServer.arg(F("page"));
        webServer.send(200, PSTR("text/plain"), "Page: '" + pageid + "'");
        dispatch_set_page(pageid.toInt(), LV_SCR_LOAD_ANIM_NONE);
    });

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    webServer.on(F("/list"), HTTP_GET, handleFileList);
    // load editor
    webServer.on(F("/edit"), HTTP_GET, []() {
        if(handleFileRead("/edit.htm") != 200) {
            char mimetype[16];
            snprintf_P(mimetype, sizeof(mimetype), PSTR("text/plain"));
            webServer.send_P(404, mimetype, PSTR("FileNotFound"));
        }
    });
    webServer.on(F("/edit"), HTTP_PUT, handleFileCreate);
    webServer.on(F("/edit"), HTTP_DELETE, handleFileDelete);
    // first callback is called after the request has ended with all parsed arguments
    // second callback handles file uploads at that location
    webServer.on(
        F("/edit"), HTTP_POST,
        []() {
            webServer.send(200, "text/plain", "");
            LOG_VERBOSE(TAG_HTTP, F("Headers: %d"), webServer.headers());
        },
        handleFileUpload);
#endif

    webServer.on(F("/"), webHandleRoot);
    webServer.on(F("/info"), webHandleInfoJson);
    // webServer.on(F("/info"), webHandleInfo);
    webServer.on(F("/screenshot"), webHandleScreenshot);
    webServer.on(F("/reboot"), httpHandleReboot);

#if HASP_USE_CONFIG > 0
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
#if HASP_USE_GPIO > 0
    webServer.on(F("/config/gpio"), webHandleGpioConfig);
    webServer.on(F("/config/gpio/options"), webHandleGpioOutput);
    webServer.on(F("/config/gpio/input"), webHandleGpioInput);
#endif
    webServer.on(F("/saveConfig"), webHandleSaveConfig);
    webServer.on(F("/resetConfig"), httpHandleResetConfig);
#endif // HASP_USE_CONFIG
    webServer.onNotFound(httpHandleFileFromFlash);

    LOG_INFO(TAG_HTTP, F(D_SERVICE_STARTED));
    // webStart();  Wait for network connection
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
static void httpReconnect()
{
    if(!http_config.enable) return;

    if(webServerStarted) {
        httpStop();
    } else
#if HASP_USE_WIFI > 0 && !defined(STM32F4xx)
        if(WiFi.status() == WL_CONNECTED || WiFi.getMode() != WIFI_STA)
#endif
    {
        httpStart();
    }
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
IRAM_ATTR void httpLoop(void)
{
#if(HASP_USE_CAPTIVE_PORTAL > 0) && (HASP_USE_WIFI > 0)
    dnsServer.processNextRequest();
#endif
    webServer.handleClient();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
bool httpGetConfig(const JsonObject& settings)
{
    bool changed = false;

    settings[FPSTR(FP_CONFIG_ENABLE)] = http_config.enable;

    if(http_config.port != settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PORT)] = http_config.port;

    if(strcmp(http_config.username, settings[FPSTR(FP_CONFIG_USER)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_USER)] = http_config.username;

    if(strcmp(http_config.password, settings[FPSTR(FP_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_PASS)] = http_config.password;

    if(changed) configOutput(settings, TAG_HTTP);
    return changed;
}

/** Set HTTP Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool httpSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_HTTP);
    bool changed = false;

    changed |= configSet(http_config.port, settings[FPSTR(FP_CONFIG_PORT)], F("httpPort"));

    if(!settings[FPSTR(FP_CONFIG_USER)].isNull()) {
        changed |= strcmp(http_config.username, settings[FPSTR(FP_CONFIG_USER)]) != 0;
        strncpy(http_config.username, settings[FPSTR(FP_CONFIG_USER)], sizeof(http_config.username));
    }

    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() &&
       settings[FPSTR(FP_CONFIG_PASS)].as<String>() != String(FPSTR(D_PASSWORD_MASK))) {
        changed |= strcmp(http_config.password, settings[FPSTR(FP_CONFIG_PASS)]) != 0;
        strncpy(http_config.password, settings[FPSTR(FP_CONFIG_PASS)], sizeof(http_config.password));
    }

    return changed;
}
#endif // HASP_USE_CONFIG

size_t httpClientWrite(const uint8_t* buf, size_t size)
{
    /***** Sending 16Kb at once freezes on STM32 EthernetClient *****/
    size_t bytes_sent = 0;
    while(bytes_sent < size) {
        if(!webServer.client()) return bytes_sent;
        if(size - bytes_sent >= 20480) {
            bytes_sent += webServer.client().write(buf + bytes_sent, 20480); // 2048
            delay(1);                                                        // Fixes the freeze
        } else {
            bytes_sent += webServer.client().write(buf + bytes_sent, size - bytes_sent);
            return bytes_sent;
        }
        // Serial.println(bytes_sent);

        // stm32_eth_scheduler(); // already in write
        // webServer.client().flush();
    }
    return bytes_sent;
}

#endif
