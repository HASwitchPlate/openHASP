/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"
#include "ArduinoLog.h"

#define HTTP_LEGACY

#if defined(ARDUINO_ARCH_ESP32)
#include "Update.h"
#include "Preferences.h"
#include "sdkconfig.h" // for CONFIG_IDF_TARGET_ESP32* defines
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#endif

#include "hasp_conf.h"
#include "dev/device.h"
#include "hal/hasp_hal.h"

#include "hasp_gui.h"
#include "hasp_debug.h"

#if HASP_USE_HTTP > 0
#include "sys/net/hasp_network.h"
#include "sys/net/hasp_time.h"

#if(HASP_USE_CAPTIVE_PORTAL > 0) && (HASP_USE_WIFI > 0)
#include <DNSServer.h>
#endif

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

#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
extern const uint8_t EDIT_HTM_GZ_START[] asm("_binary_data_static_edit_htm_gz_start");
extern const uint8_t EDIT_HTM_GZ_END[] asm("_binary_data_static_edit_htm_gz_end");
extern const uint8_t STYLE_CSS_GZ_START[] asm("_binary_data_static_style_css_gz_start");
extern const uint8_t STYLE_CSS_GZ_END[] asm("_binary_data_static_style_css_gz_end");
extern const uint8_t SCRIPT_JS_GZ_START[] asm("_binary_data_static_script_js_gz_start");
extern const uint8_t SCRIPT_JS_GZ_END[] asm("_binary_data_static_script_js_gz_end");
extern const uint8_t LOGO_SVG_GZ_START[] asm("_binary_data_static_logo_svg_gz_start");
extern const uint8_t LOGO_SVG_GZ_END[] asm("_binary_data_static_logo_svg_gz_end");
extern const uint8_t ACE_JS_GZ_START[] asm("_binary_data_static_ace_1_9_6_min_js_gz_start");
extern const uint8_t ACE_JS_GZ_END[] asm("_binary_data_static_ace_1_9_6_min_js_gz_end");
extern const uint8_t PETITE_VUE_HASP_JS_GZ_START[] asm("_binary_data_static_petite_vue_hasp_js_gz_start");
extern const uint8_t PETITE_VUE_HASP_JS_GZ_END[] asm("_binary_data_static_petite_vue_hasp_js_gz_end");
extern const uint8_t MAIN_JS_GZ_START[] asm("_binary_data_static_main_js_gz_start");
extern const uint8_t MAIN_JS_GZ_END[] asm("_binary_data_static_main_js_gz_end");
extern const uint8_t EN_JSON_GZ_START[] asm("_binary_data_static_en_json_gz_start");
extern const uint8_t EN_JSON_GZ_END[] asm("_binary_data_static_en_json_gz_end");
extern const uint8_t HASP_HTM_GZ_START[] asm("_binary_data_static_hasp_htm_gz_start");
extern const uint8_t HASP_HTM_GZ_END[] asm("_binary_data_static_hasp_htm_gz_end");

#endif // CONFIG_IDF_TARGET_ESP32

#endif // ESP32

HTTPUpload* upload;

const char MAIN_MENU_BUTTON[] PROGMEM = "<a href='/'>" D_HTTP_MAIN_MENU "</a>";

const char HTTP_DOCTYPE[] PROGMEM =
    "<!DOCTYPE html><html lang=\"en\"><head>"
    //  "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'self';img-src "
    //  "'self' data:;style-src 'self' data:;\">"
    "<meta charset='utf-8'><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"/>";
const char HTTP_META_GO_BACK[] PROGMEM = "<meta http-equiv='refresh' content='%d;url=/'/>";
const char HTTP_STYLESHEET[] PROGMEM   = "<link rel=\"stylesheet\" href=\"/%s.css\">";
const char HTTP_HEADER[] PROGMEM       = "<title>%s</title>";
const char HTTP_HEADER_END[] PROGMEM   = "<script src=\"/script.js\"></script>"
                                         "<link rel=\"stylesheet\" href=\"/style.css\"></head><body><div id='doc'>";
const char HTTP_FOOTER[] PROGMEM       = "<div class='clear'><hr/><a class='foot' href='/about'>" D_MANUFACTURER " ";
const char HTTP_END[] PROGMEM          = " " D_HTTP_FOOTER "</div></body></html>";
const uint8_t HTTP_VARS_CSS[] PROGMEM  = ":root{"
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
                                         "}";

////////////////////////////////////////////////////////////////////////////////////////////////////

// URL for auto-update "version.json"
// const char UPDATE_URL[] PROGMEM = "http://haswitchplate.com/update/version.json";
// // Default link to compiled Arduino firmware image
// String espFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.ino.d1_mini.bin";
// // Default link to compiled Nextion firmware images
// String lcdFirmwareUrl = "http://haswitchplate.com/update/HASwitchPlate.tft";

////////////////////////////////////////////////////////////////////////////////////////////////////
String getOption(int value, String label, int current_value = INT_MIN)
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

static String http_get_content_type(const String& path)
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

// Check authentication but only create Log entry if it failed
bool http_is_authenticated()
{
    if(http_config.password[0] != '\0') { // Request HTTP auth if httpPassword is set
        if(!webServer.authenticate(http_config.username, http_config.password)) {
            webServer.requestAuthentication();
            LOG_WARNING(TAG_HTTP, F(D_TELNET_INCORRECT_LOGIN_ATTEMPT),
                        webServer.client().remoteIP().toString().c_str());
            return false;
        }
    }
    return true;
}

// Check authentication and create Log entry
bool http_is_authenticated(const __FlashStringHelper* notused)
{
    if(!http_is_authenticated()) return false;

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    LOG_VERBOSE(TAG_HTTP, F(D_HTTP_SENDING_PAGE), webServer.uri().c_str(),
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

static void http_send_cache_header(int size, int age = 3600)
{
    webServer.sendHeader(F("Content-Length"), (String)(size));
    webServer.sendHeader(F("Cache-Control"), (String)(F("public, max-age=")) + (String)(age));
}

static int http_send_cached(int statuscode, const char* contenttype, const char* data, size_t size, int age = 3600)
{
    http_send_cache_header(size, age);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    webServer.send_P(statuscode, contenttype, data, size);
#else
    webServer.send(statuscode, contenttype, data);
#endif
    return statuscode;
}

static int http_send_static_file(const uint8_t* start, const uint8_t* end, String& contentType)
{
    size_t size = end > start ? end - start : 0;
    int age     = 365 * 24 * 60 * 60;
    return http_send_cached(200, contentType.c_str(), (const char*)start, size, age);
}

static int http_send_static_gzip_file(const uint8_t* start, const uint8_t* end, String& contentType)
{
    webServer.sendHeader(F("Content-Encoding"), F("gzip"));
    return http_send_static_file(start, end, contentType);
}

static void webSendHtmlHeader(const char* nodename, uint32_t httpdatalength, uint8_t gohome = 0)
{
    {
        char buffer[64];

        /* Calculate Content Length upfront */
        uint32_t contentLength = strlen(haspDevice.get_version()); // version length
        contentLength += sizeof(HTTP_DOCTYPE) - 1;
        contentLength += sizeof(HTTP_HEADER) - 1 - 2 + strlen(nodename);   // -2 for %s
        contentLength += sizeof(HTTP_STYLESHEET) - 1 - 2 + strlen("vars"); // -2 for %s
        if(gohome > 0) {
            snprintf_P(buffer, sizeof(buffer), HTTP_META_GO_BACK, gohome);
            contentLength += strlen(buffer); // gohome
        } else {
            buffer[0] = '\0';
        }
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
        webServer.sendContent(buffer); // gohome

        snprintf_P(buffer, sizeof(buffer), HTTP_STYLESHEET, "vars");
        webServer.sendContent(buffer); // stylesheet

        snprintf_P(buffer, sizeof(buffer), HTTP_HEADER, nodename);
        webServer.sendContent(buffer); // 17-2+len
    }

#if defined(STM32F4xx)
    webServer.sendContent(HTTP_HEADER_END); // 80
#else
    webServer.sendContent_P(HTTP_HEADER_END);             // 80
#endif
}

// Allows caching of this file, BUT browser must validate Etag before using cached versions
static void http_send_etag(String& etag)
{
    String newTag((char*)0);
    newTag.reserve(64);
    newTag = "\"";
    newTag += etag;
    newTag += "\"";
    webServer.sendHeader(F("Cache-Control"), F("no-cache, must-revalidate, public"));
    webServer.sendHeader(F("Expires"), F("Fri, 30 Oct 1998 14:19:41 GMT"));
    webServer.sendHeader(F("ETag"), newTag);
}

bool http_save_config()
{
    bool updated = false;

    if(webServer.method() == HTTP_POST && webServer.hasArg(PSTR("save"))) {
        String save = webServer.arg(PSTR("save"));

        StaticJsonDocument<256> settings;
        for(int i = 0; i < webServer.args(); i++) settings[webServer.argName(i)] = webServer.arg(i);

        if(save == String(PSTR("hasp"))) {
            updated = haspSetConfig(settings.as<JsonObject>());

#if HASP_USE_MQTT > 0
        } else if(save == String(PSTR(FP_MQTT))) {
            updated = mqttSetConfig(settings.as<JsonObject>());
#endif
#if HASP_USE_FTP > 0
        } else if(save == String(PSTR(FP_FTP))) {
            updated = ftpSetConfig(settings.as<JsonObject>());
#endif

        } else if(save == String(PSTR("gui"))) {
            settings[FPSTR(FP_GUI_POINTER)]         = webServer.hasArg(PSTR("cursor"));
            settings[FPSTR(FP_GUI_INVERT)]          = webServer.hasArg(PSTR("invert"));
            settings[FPSTR(FP_GUI_BACKLIGHTINVERT)] = webServer.hasArg(PSTR("bcklinv"));
            updated                                 = guiSetConfig(settings.as<JsonObject>());

        } else if(save == String(PSTR("debug"))) {
            settings[FPSTR(FP_DEBUG_ANSI)] = webServer.hasArg(PSTR("ansi"));
            updated                        = debugSetConfig(settings.as<JsonObject>());

        } else if(save == String(PSTR(FP_HTTP))) {
            updated = httpSetConfig(settings.as<JsonObject>());

            // Password might have changed
            if(!http_is_authenticated(F("config"))) return updated;

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
    if(!http_is_authenticated(F("root"))) return;
    bool updated = http_save_config();

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

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void httpHandleReboot()
{ // http://plate01/reboot
    if(!http_is_authenticated(F("reboot"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage = F(D_DISPATCH_REBOOT);

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 6);
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
    if(!http_is_authenticated(F("screenshot"))) return;

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

        // Check if screenshot bitmap is dirty
        if(webServer.hasArg(F("d"))) {
            if(guiScreenshotIsDirty())
                webServer.send(200, F("text/text"), "1");
            else
                webServer.send(304, F("text/text"), "0");
            return;
        }

        uint32_t modified = guiScreenshotEtag();
        String etag((char*)0);
        etag.reserve(64);

        if(webServer.hasHeader("If-None-Match")) {
            etag = webServer.header("If-None-Match");
            etag.replace("\"", "");
            LOG_DEBUG(TAG_HTTP, F("If-None-Match: %s"), etag.c_str());
            if(modified > 0 && modified == atol(etag.c_str())) { // Not Changed
                http_send_etag(etag);                            // Reuse same ETag
                webServer.send_P(304, PSTR("image/bmp"), "");    // Use correct mimetype
                return;                                          // 304 ot Modified
            }
        }

        etag = (String)(modified);
        http_send_etag(etag); // Send new tag with modification version

        // Send actual bitmap
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

        httpMessage += F("<div class=\"dist\"><a href='#' onclick=\"return upd('prev')\">" D_HTTP_PREV_PAGE "</a>");
        httpMessage += F("<a href='#' onclick=\"return upd('')\">" D_HTTP_REFRESH "</a>");
        httpMessage += F("<a href='#' onclick=\"return upd('next')\">" D_HTTP_NEXT_PAGE "</a></div>");
        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
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

static void add_license(JsonObject& obj, const char* title, const char* year, const char* author, const char* license,
                        uint8_t allrightsreserved = 0)
{
    obj["t"] = title;
    obj["y"] = year;
    obj["a"] = author;
    obj["l"] = license;
    if(allrightsreserved) obj["r"] = allrightsreserved;
}

static void webHandleApi()
{ // http://plate01/api
    if(!http_is_authenticated(F("api"))) return;

    DynamicJsonDocument doc(2048);
    String contentType = http_get_content_type(F(".json"));
    String endpoint((char*)0);
    endpoint = webServer.pathArg(0);

    if(!strcasecmp_P(endpoint.c_str(), PSTR("files"))) {
        webServer.send(200, contentType.c_str(), filesystem_list(HASP_FS, "/", 5).c_str());

    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("info"))) {
        String jsondata((char*)0);
        jsondata.reserve(HTTP_PAGE_SIZE);
        jsondata = "{";

        hasp_get_info(doc);
        add_json(jsondata, doc);

#if HASP_USE_MQTT > 0
        mqtt_get_info(doc);
        add_json(jsondata, doc);
#endif

#if HASP_USE_WIFI > 0 || HASP_USE_EHTERNET > 0
        network_get_info(doc);
        add_json(jsondata, doc);
#endif

        haspDevice.get_info(doc);
        add_json(jsondata, doc);

        jsondata[jsondata.length() - 1] = '}'; // Replace last comma with a bracket

        webServer.send(200, contentType, jsondata);
        return;

    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("license"))) {

        {
            JsonObject obj;
            obj = doc.createNestedObject();
            add_license(obj, "HASwitchPlate", "2019", "Allen Derusha allen@derusha.org", "mit");
            obj = doc.createNestedObject();
            add_license(obj, "Tasmota Core", "2016", "Tasmota", "apache2");
            obj = doc.createNestedObject();
            add_license(obj, "LVGL", "2021", "LVGL Kft", "mit");
#if defined(LGFX_USE_V1)
            obj = doc.createNestedObject();
            add_license(obj, "LovyanGFX", "2020", "lovyan03 (https://github.com/lovyan03)", "freebsd", 1);
#endif
            obj = doc.createNestedObject();
            add_license(obj, "TFT_eSPI", "2020", "Bodmer (https://github.com/Bodmer)", "freebsd", 1);
            obj = doc.createNestedObject();
            add_license(obj, "Adafruit_GFX", "2021", "Adafruit Industries.", "bsd", 1);
#if defined(HASP_USE_ARDUINOGFX)
            obj = doc.createNestedObject();
            add_license(obj, "Arduino_GFX", "", "moononournation", "", 0);
#endif
#if HASP_USE_MQTT > 0 && defined(HASP_USE_PUBSUBCLIENT)
            obj = doc.createNestedObject();
            add_license(obj, "PubSubClient", "2008-2015", "Nicholas O'Leary", "mit");
#endif
            obj = doc.createNestedObject();
            add_license(obj, "ArduinoJson", "2014-2022", "Benoit BLANCHON", "mit");
            obj = doc.createNestedObject();
            add_license(obj, "ArduinoLog", "2017-2018",
                        "Thijs Elenbaas, MrRobot62, rahuldeo2047, NOX73, dhylands, Josha blemasle, mfalkvidd", "mit");
#if HASP_USE_FTP > 0
            obj = doc.createNestedObject();
            add_license(obj, "SimpleFTPServer", "2017", "Renzo Mischianti www.mischianti.org", "mit", 1);
#endif
            obj = doc.createNestedObject();
            add_license(obj, "AceButton", "2018", "Brian T. Park", "mit");
            obj = doc.createNestedObject();
            add_license(obj, "QR Code generator", "", "Project Nayuki", "mit");
        }
        {
            char output[HTTP_PAGE_SIZE];
            serializeJson(doc, output, sizeof(output));
            webServer.send(200, contentType.c_str(), output);
        }

    } else if(!strcasecmp_P(endpoint.c_str(), PSTR("config"))) {

        JsonObject settings;
        String postBody((char*)0);
        postBody = webServer.arg("plain");

        if(webServer.method() == HTTP_GET) {
            // Make sure we have a valid JsonObject to start from
            settings = doc.to<JsonObject>();

        } else if(webServer.method() == HTTP_POST || webServer.method() == HTTP_PUT) {
            DeserializationError jsonError = deserializeJson(doc, postBody);
            if(jsonError) { // Couldn't parse incoming JSON command
                dispatch_json_error(TAG_HTTP, jsonError);
                return;
            }
            settings = doc.as<JsonObject>();
        } else {
            webServer.send(400, contentType, "Bad Request");
            return;
        }

        settings = doc.to<JsonObject>();
        const __FlashStringHelper* module;

        module = FPSTR(FP_HASP);
        settings.createNestedObject(module);
        haspGetConfig(settings[module]);

        module = FPSTR(FP_GUI);
        settings.createNestedObject(module);
        guiGetConfig(settings[module]);

        module = FPSTR(FP_DEBUG);
        settings.createNestedObject(module);
        debugGetConfig(settings[module]);

#if HASP_USE_WIFI > 0
        module = FPSTR(FP_WIFI);
        settings.createNestedObject(module);
        wifiGetConfig(settings[module]);

        module = FPSTR(FP_TIME);
        settings.createNestedObject(module);
        timeGetConfig(settings[module]);
#endif
#if HASP_USE_MQTT > 0
        module = FPSTR(FP_MQTT);
        settings.createNestedObject(module);
        mqttGetConfig(settings[module]);
#endif
#if HASP_USE_FTP > 0
        module = FPSTR(FP_FTP);
        settings.createNestedObject(module);
        ftpGetConfig(settings[module]);
#endif
#if HASP_USE_HTTP > 0
        module = FPSTR(FP_HTTP);
        settings.createNestedObject(module);
        httpGetConfig(settings[module]);
#endif
#if HASP_USE_ARDUINOOTA > 0 || HASP_USE_HTTP_UPDATE > 0
        module = FPSTR(FP_OTA);
        settings.createNestedObject(module);
        otaGetConfig(settings[module]);
#endif
#if HASP_USE_GPIO > 0
        module = FPSTR(FP_GPIO);
        settings.createNestedObject(module);
        gpioGetConfig(settings[module]);
#endif
        configOutput(settings, TAG_HTTP); // Log current JSON config

        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        // Mask non-blank passwords
        // if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() && settings[FPSTR(FP_CONFIG_PASS)].as<String>().length() != 0) {
        //     settings[FPSTR(FP_CONFIG_PASS)] = D_PASSWORD_MASK;
        // }

        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        doc.shrinkToFit();
        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        const size_t size = measureJson(doc) + 1;
        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        char jsondata[size];
        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        memset(jsondata, 0, size);
        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        serializeJson(doc, jsondata, size);
        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
        webServer.send(200, contentType, jsondata);
        LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);

    } else {
        webServer.send(400, contentType, "Bad Request");
    }
}

static void webHandleApiConfig()
{ // http://plate01/about
    if(!http_is_authenticated(F("api"))) return;

    if(webServer.method() != HTTP_GET && webServer.method() != HTTP_POST) {
        return;
    }

    StaticJsonDocument<1024> doc;
    JsonObject settings;
    String contentType = http_get_content_type(F(".json"));
    String endpoint((char*)0);
    endpoint                 = webServer.pathArg(0);
    const char* endpoint_key = endpoint.c_str();

    String postBody = webServer.arg("plain");

    if(webServer.method() == HTTP_GET) {
        // Make sure we have a valid JsonObject to start from
        settings = doc.to<JsonObject>();

    } else if(webServer.method() == HTTP_POST || webServer.method() == HTTP_PUT) {
        DeserializationError jsonError = deserializeJson(doc, postBody);
        if(jsonError) { // Couldn't parse incoming JSON command
            dispatch_json_error(TAG_HTTP, jsonError);
            return;
        }
        settings = doc.as<JsonObject>();
    } else {
        webServer.send(400, contentType, "Bad Request");
        return;
    }

    if(webServer.method() == HTTP_POST || webServer.method() == HTTP_PUT) {
        configOutput(settings, TAG_HTTP); // Log input JSON config

        if(!strcasecmp_P(endpoint_key, PSTR("hasp"))) {
            haspSetConfig(settings);
        } else if(!strcasecmp_P(endpoint_key, PSTR("gui"))) {
            guiSetConfig(settings);
        } else if(!strcasecmp_P(endpoint_key, PSTR("debug"))) {
            debugSetConfig(settings);
        } else
#if HASP_USE_WIFI > 0
            if(!strcasecmp_P(endpoint_key, PSTR("wifi"))) {
            wifiSetConfig(settings);
        } else if(!strcasecmp_P(endpoint_key, PSTR("time"))) {
            timeSetConfig(settings);
        } else
#endif
#if HASP_USE_MQTT > 0
            if(!strcasecmp_P(endpoint_key, PSTR(FP_MQTT))) {
            mqttSetConfig(settings);
        } else
#endif
#if HASP_USE_FTP > 0
            if(!strcasecmp_P(endpoint_key, PSTR(FP_FTP))) {
            ftpSetConfig(settings);
        } else
#endif
#if HASP_USE_HTTP > 0
            if(!strcasecmp_P(endpoint_key, PSTR(FP_HTTP))) {
            httpSetConfig(settings);
        } else
#endif
#if HASP_USE_ARDUINOOTA > 0 || HASP_USE_HTTP_UPDATE > 0
            if(!strcasecmp_P(endpoint_key, PSTR("ota"))) {
            otaSetConfig(settings);
        } else
#endif
        {
            LOG_WARNING(TAG_HTTP, F("Invalid module %s"), endpoint_key);
            return;
        }
    }

    settings = doc.to<JsonObject>();
    if(!strcasecmp_P(endpoint_key, PSTR("hasp"))) {
        haspGetConfig(settings);
    } else if(!strcasecmp_P(endpoint_key, PSTR("gui"))) {
        guiGetConfig(settings);
    } else if(!strcasecmp_P(endpoint_key, PSTR("debug"))) {
        debugGetConfig(settings);
    } else
#if HASP_USE_WIFI > 0
        if(!strcasecmp_P(endpoint_key, PSTR("wifi"))) {
        wifiGetConfig(settings);
    } else if(!strcasecmp_P(endpoint_key, PSTR("time"))) {
        timeGetConfig(settings);
    } else
#endif
#if HASP_USE_MQTT > 0
        if(!strcasecmp_P(endpoint_key, PSTR(FP_MQTT))) {
        mqttGetConfig(settings);
    } else
#endif
#if HASP_USE_FTP > 0
        if(!strcasecmp_P(endpoint_key, PSTR(FP_FTP))) {
        ftpGetConfig(settings);
    } else
#endif
#if HASP_USE_HTTP > 0
        if(!strcasecmp_P(endpoint_key, PSTR(FP_HTTP))) {
        httpGetConfig(settings);
    } else
#endif
#if HASP_USE_ARDUINOOTA > 0 || HASP_USE_HTTP_UPDATE > 0
        if(!strcasecmp_P(endpoint_key, PSTR("ota"))) {
        otaGetConfig(settings);
    } else
#endif
    {
        webServer.send(400, contentType, "Bad Request");
        return;
    }
    configOutput(settings, TAG_HTTP); // Log current JSON config

    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    // Mask non-blank passwords
    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() && settings[FPSTR(FP_CONFIG_PASS)].as<String>().length() != 0) {
        settings[FPSTR(FP_CONFIG_PASS)] = D_PASSWORD_MASK;
    }

    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    // doc.shrinkToFit();
    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    const size_t size = measureJson(doc) + 1;
    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    char jsondata[size];
    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    memset(jsondata, 0, size);
    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    serializeJson(doc, jsondata, size);
    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
    webServer.send(200, contentType, jsondata);
    LOG_DEBUG(TAG_HTTP, "%s - %d", __FILE__, __LINE__);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleAbout()
{ // http://plate01/about
    if(!http_is_authenticated(F("about"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);

        httpMessage += "<div id='lic'></div>";
        httpMessage += FPSTR(MAIN_MENU_BUTTON);
        httpMessage += "<div id='pkg'></div>";
        // TOREMOVE httpMessage += "<script>window.addEventListener('load', about());</script>";

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleInfoJson()
{ // http://plate01/
    if(!http_is_authenticated(F("infojson"))) return;

    { // Send Content
        String htmldata((char*)0);
        htmldata.reserve(HTTP_PAGE_SIZE);

        htmldata += F("<h1>");
        htmldata += haspDevice.get_hostname();
        htmldata += F("</h1><hr>");

        htmldata += "<div id=\"info\"></div>";
        htmldata += FPSTR(MAIN_MENU_BUTTON);

        webSendHtmlHeader(haspDevice.get_hostname(), htmldata.length(), 0);
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
static void http_upload_progress()
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
    Update.abort();
    Update.end(false);
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

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 10);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
    dispatch_reboot(true); // Save the current config
}

static void webHandleFirmwareUpload()
{
    upload = &webServer.upload();

    switch(upload->status) {

        case UPLOAD_FILE_START: {
            if(!http_is_authenticated(F("update"))) return;

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
            if(!Update.isFinished()) {
                if(Update.write(upload->buf, upload->currentSize) != upload->currentSize) {
                    webUpdatePrintError();
                } else {
                    http_upload_progress();
                }
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

static inline int handleFilesystemFile(String path)
{
    if(!http_is_authenticated()) return false;

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    // path = webServer.urlDecode(path).substring(0, 31);
    if(path.endsWith("/")) {
        path += F("index.html");
    }
    String pathWithGz = path + F(".gz");
    // String pathWithBr = path + F(".br");
    String contentType((char*)0);
    contentType = http_get_content_type(path);

    if(HASP_FS.exists(pathWithGz) /* || HASP_FS.exists(pathWithBr) */ || HASP_FS.exists(path)) {

        if(webServer.hasArg(F("download"))) contentType = F("application/octet-stream");

        if(!HASP_FS.exists(path) && HASP_FS.exists(pathWithGz))
            path = pathWithGz; // Only use .gz if normal file doesn't exist
        // if(!HASP_FS.exists(path) && HASP_FS.exists(pathWithBr))
        //     path = pathWithBr; // Only use .gz if normal file doesn't exist

        LOG_TRACE(TAG_HTTP, F(D_HTTP_SENDING_PAGE), path.c_str(), webServer.client().remoteIP().toString().c_str());

        String configFile((char*)0); // Verify if the file is config.json
        configFile = FPSTR(FP_HASP_CONFIG_FILE);

        if(path.endsWith(configFile.c_str())) { // "//config.json" is also a valid path!
            DynamicJsonDocument settings(2048);
            DeserializationError error = configParseFile(configFile, settings);

            if(error) return 500; // Internal Server Error

            configMaskPasswords(settings); // Output settings to the client with masked passwords!
            char buffer[1024];
            size_t len = serializeJson(settings, buffer, sizeof(buffer));
            webServer.setContentLength(len);
            webServer.send(200, contentType, buffer);

        } else {
            File file       = HASP_FS.open(path, "r");
            time_t modified = file.getLastWrite();
            String etag((char*)0);
            etag.reserve(64);

            if(webServer.hasHeader("If-None-Match")) {
                etag = webServer.header("If-None-Match");
                etag.replace("\"", "");
                LOG_DEBUG(TAG_HTTP, F("If-None-Match: %s"), etag.c_str());
                if(modified > 0 && modified == atol(etag.c_str())) { // Not Changed
                    file.close();                                    // Skip reading the file contents
                    http_send_etag(etag);                            // Reuse same ETag
                    webServer.send(304, contentType, "");            // Use correct mimetype
                    return 304;                                      // Not Modified
                }
            }

            etag = (String)(modified);
            http_send_etag(etag); // Send new tag with modification datetime

            /* Only needed for brotli encoding. Gzip is handled automatically in streamfile() */
            /* Brotli is not supported over HTTP/1.1 */
            // if(path.endsWith(".br") && contentType != String(FPSTR(mime::mimeTable[mime::type::none].mimeType))) {
            //     webServer.sendHeader(F("Content-Encoding"), F("br"));
            //     webServer.streamFile(file, contentType);
            //     LOG_DEBUG(TAG_HTTP, F("Headers: OK"));

            // } else {
            // Stream other files directly from filesystem
            webServer.streamFile(file, contentType);
            LOG_DEBUG(TAG_HTTP, F("If-None-Match: %s"), etag.c_str());
            //}
            file.close();
        }

        return 200; // OK
    }

#endif // HASP_USE_SPIFFS || HASP_USE_LITTLEFS

    return 404; // File Not found on Flash
}

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
static void handleFileUpload()
{
    // if(!http_is_authenticated()) return;   // Moved to UPLOAD_FILE_START

    if(webServer.uri() != "/edit") {
        return;
    }
    upload = &webServer.upload();
    switch(upload->status) {
        case UPLOAD_FILE_START: {
            if(!http_is_authenticated(F("fileupload"))) return;
            String filename((char*)0);
            filename.reserve(64);
            filename = upload->filename;
            if(!filename.startsWith("/")) {
                filename = "/";
                filename += upload->filename;
            }
            fsUploadFile = HASP_FS.open(filename, "w");
            if(fsUploadFile) {
                if(!fsUploadFile || fsUploadFile.isDirectory()) {
                    LOG_WARNING(TAG_HTTP, F(D_FILE_SAVE_FAILED), filename.c_str());
                    webServer.send_P(400, PSTR("text/plain"), PSTR("Invalid filename"));
                    fsUploadFile.close();
                    fsUploadFile = File();
                } else {
                    LOG_TRACE(TAG_HTTP, F("handleFileUpload Name: %s"), filename.c_str());
                    haspProgressMsg(fsUploadFile.name());
                }
            } else {
                LOG_ERROR(TAG_HTTP, F("Could not open file %s for writing"), filename.c_str());
                webServer.send_P(400, PSTR("text/plain"), PSTR("Could not open file for writing"));
            }
            break;
        }
        case UPLOAD_FILE_WRITE: {
            if(fsUploadFile) {
                if(fsUploadFile.write(upload->buf, upload->currentSize) != upload->currentSize) {
                    LOG_ERROR(TAG_HTTP, F("Failed to write received data to file"));
                    webServer.send_P(400, PSTR("text/plain"), PSTR("Failed to write received data to file"));
                    fsUploadFile.close();
                    fsUploadFile = File();
                } else {
                    http_upload_progress(); // Moved to httpEverySecond Loop
                }
            }
            break;
        }
        case UPLOAD_FILE_END: {
            if(fsUploadFile) {
                LOG_INFO(TAG_HTTP, F("Uploaded %s (%u bytes)"), fsUploadFile.name(), upload->totalSize);
                fsUploadFile.close();

                // Redirect to /config/hasp page. This flushes the web buffer and frees the memory
                // webServer.sendHeader(String(F("Location")), String(F("/config/hasp")), true);
                webServer.send_P(200, PSTR("text/plain"), "Upload OK");
            }
            haspProgressVal(255);
            break;
        }
        default:
            LOG_WARNING(TAG_HTTP, F("File upload aborted"));
            webServer.send_P(400, PSTR("text/plain"), PSTR("File upload aborted"));
            fsUploadFile.close();
            fsUploadFile = File();
    }
}

static void handleFileDelete()
{
    if(!http_is_authenticated(F("filedelete"))) return;

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
}

static void handleFileCreate()
{
    if(!http_is_authenticated(F("filecreate"))) return;

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
        dispatch_wakeup();
        hasp_init();
    }
    if(webServer.hasArg(F("load"))) {
        dispatch_wakeup();
        hasp_load_json();
    }
    if(webServer.hasArg(F("page"))) {
        dispatch_wakeup();
        dispatch_page(NULL, webServer.arg(F("page")).c_str(), TAG_HTTP);
        // uint8_t pageid = atoi(webServer.arg(F("page")).c_str());
        // dispatch_set_page(pageid, LV_SCR_LOAD_ANIM_NONE);
    }
    webServer.send(200, PSTR("text/plain"), "");
}

static void handleFileList()
{
    if(!http_is_authenticated(F("filelist"))) return;

    if(!webServer.hasArg(F("dir"))) {
        webServer.send(500, PSTR("text/plain"), PSTR("BAD ARGS"));
        return;
    }

    String path = webServer.arg(F("dir"));
    // LOG_TRACE(TAG_HTTP, F("handleFileList: %s"), path.c_str());
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
        bool isDir = file.isDirectory();
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

#ifdef HTTP_LEGACY

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
static void webHandleConfig()
{ // http://plate01/config
    if(!http_is_authenticated(F("config"))) return;

    bool updated = http_save_config();

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
#if HASP_USE_FTP > 0
        httpMessage += F("<a href='/config/ftp'>" D_HTTP_FTP_SETTINGS "</a>");
#endif
        httpMessage += F("<a href='/config/gui'>" D_HTTP_GUI_SETTINGS "</a>");

#if HASP_USE_GPIO > 0
        httpMessage += F("<a href='/config/gpio'>" D_HTTP_GPIO_SETTINGS "</a>");
#endif

        httpMessage += F("<a href='/config/debug'>" D_HTTP_DEBUG_SETTINGS "</a>");
        httpMessage += F("<a href='/config/reset' class='red'>" D_HTTP_FACTORY_RESET "</a>");
        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_MQTT > 0
static void webHandleMqttConfig()
{ // http://plate01/config/mqtt
    if(!http_is_authenticated(F("config/mqtt"))) return;

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
        httpMessage += F("<div class='container'><form method='POST' action='/config' id='mqtt'>");

        // Node Name
        httpMessage +=
            F("<div class='row'><div class='col-25'><label class='required' for='name'>Plate Name</label></div>");
        httpMessage += F("<div class='col-75'><input required type='text' id='name' name='name' maxlength= ");
        httpMessage += STR_LEN_HOSTNAME - 1;
        httpMessage += F("pattern='[a-z0-9_]*' placeholder='Plate Name' value=''></div></div>");

        // Group Name
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='group'>Group Name</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='group' name='group' maxlength=15 "
                         "pattern='[a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9]' "
                         "placeholder='Group Name' value=''></div></div>");

        // Broker
        httpMessage += F("<div class='row'><div class='col-25'><label for='host'>Broker</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='host' name='host' maxlength=");
        httpMessage += MAX_HOSTNAME_LENGTH - 1;
        httpMessage += F(" placeholder='Server Name' value=''></div></div>");

        // Mqtt Port
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='port'>Port</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='port' name='port' min='0' max='65535' "
                         "placeholder='1883' value=''></div></div>");

        // Mqtt User
        httpMessage += F("<div class='row'><div class='col-25'><label for='user'>User</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='user' name='user' maxlength=");
        httpMessage += MAX_USERNAME_LENGTH - 1;
        httpMessage += F(" placeholder='MQTT User' value=''></div></div>");

        // Mqtt Password
        httpMessage += F("<div class='row'><div class='col-25'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input type='password' id='pass' name='pass' maxlength=");
        httpMessage += MAX_PASSWORD_LENGTH - 1;
        httpMessage += F(" placeholder='MQTT Password' value='");
        httpMessage += F("'></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='mqtt'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleGuiConfig()
{ // http://plate01/config/wifi
    if(!http_is_authenticated(F("config/gui"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_GUI_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config' id='gui'>");

        // Short Idle
        httpMessage += F("<div class='row'><div class='col-25'><label for='idle1'>Short Idle</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='idle1' name='idle1' min='0' max='32400' "
                         "value='");
        httpMessage += F("'></div></div>");

        // Long Idle
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='idle2'>Long Idle</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='idle2' name='idle2' min='0' max='32400' "
                         "value='");
        httpMessage += F("'></div></div>");

        // Rotation
        httpMessage += F("<div class='row'><div class='col-25'><label for='group'>Orientation</label></div>");
        httpMessage += F("<div class='col-75'><select id='rotate' name='rotate'>");
        httpMessage += getOption(0, F("0 degrees"));
        httpMessage += getOption(1, F("90 degrees"));
        httpMessage += getOption(2, F("180 degrees"));
        httpMessage += getOption(3, F("270 degrees"));
        httpMessage += getOption(6, F("0 degrees - mirrored"));
        httpMessage += getOption(7, F("90 degrees - mirrored"));
        httpMessage += getOption(4, F("180 degrees - mirrored"));
        httpMessage += getOption(5, F("270 degrees - mirrored"));
        httpMessage += F("</select></div></div>");

        // Invert
        httpMessage += F("<div class='row'><div class='col-25'><label for='invert'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='invert' name='invert' value='1'");
        httpMessage += F(">Invert Colors</div></div>");

        // Cursor
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='cursor'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='cursor' name='cursor' value='1'");
        httpMessage += F(">Show Pointer</div></div>");

        // Backlight Pin
        httpMessage += F("<div class='row'><div class='col-25'><label for='group'>Backlight Pin</label></div>");
        httpMessage += F("<div class='col-75'><select id='bckl' name='bckl'>");
        httpMessage += getOption(-1, F("None"));
#if defined(ARDUINO_ARCH_ESP32)
        char buffer[10];
        // uint8_t pins[] = {0, 5, 12, 13, 15, 16, 17, 18, 19, 21, 22, 23, 26, 27, 32};
        // for(uint8_t i = 0; i < sizeof(pins); i++) {
        for(uint8_t gpio = 0; gpio < NUM_DIGITAL_PINS; gpio++) {
            if(!gpioIsSystemPin(gpio)) {
                snprintf_P(buffer, sizeof(buffer), PSTR("GPIO %d"), gpio);
                httpMessage += getOption(gpio, buffer);
            } else {
                LOG_WARNING(TAG_HTTP, F("pin %d"), gpio);
            }
        }
#else
        httpMessage += getOption(5, F("D1 - GPIO 5"));
        httpMessage += getOption(4, F("D2 - GPIO 4"));
        httpMessage += getOption(0, F("D3 - GPIO 0"));
        httpMessage += getOption(2, F("D4 - GPIO 2"));
#endif
        httpMessage += F("</select></div></div>");

        // Backlight Invert
        httpMessage += F("<div class='row'><div class='col-25'><label for='bcklinv'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='bcklinv' name='bcklinv' value='1'");
        httpMessage += F(">Invert Backlight</div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='gui'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

#if TOUCH_DRIVER == 0x2046 && defined(TOUCH_CS)
        add_form_button(httpMessage, F(D_HTTP_CALIBRATE), F("/config/gui?cal=1"));
#endif

        add_form_button(httpMessage, F(D_HTTP_ANTIBURN), F("/config/gui?brn=1"));
        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    { // Execute Actions
        if(webServer.hasArg(F("cal"))) dispatch_calibrate(NULL, NULL, TAG_HTTP);
        if(webServer.hasArg(F("brn"))) dispatch_antiburn(NULL, "on", TAG_HTTP);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleHttpConfig()
{ // http://plate01/config/http
    if(!http_is_authenticated(F("config/http"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_HTTP_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config' id='http'>");

        // Username
        httpMessage += F("<div class='row'><div class='col-25'><label for='user'>Username</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='user' name='user' maxlength=31 "
                         "placeholder='Username' value='");
        httpMessage += F("'></div></div>");

        // Password
        httpMessage += F("<div class='row'><div class='col-25'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input type='password' id='pass' name='pass' maxlength=63 "
                         "placeholder='Password' value='");
        httpMessage += F("'></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='http'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        httpMessage += F("<a href='/config'>" D_HTTP_CONFIGURATION "</a>");

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleFtpConfig()
{ // http://plate01/config/http
    if(!http_is_authenticated(F("config/ftp"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_FTP_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config' id='ftp'>");

        // Username
        httpMessage += F("<div class='row'><div class='col-25'><label for='user'>Username</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='user' name='user' maxlength=31 "
                         "placeholder='Username' value='");
        httpMessage += F("'></div></div>");

        // Password
        httpMessage += F("<div class='row'><div class='col-25'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input type='password' id='pass' name='pass' maxlength=63 "
                         "placeholder='Password' value='");
        httpMessage += F("'></div></div>");

        // Ftp Port
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='port'>Port</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='port' name='port' min='0' max='65535' "
                         "placeholder='21' value=''></div></div>");

        // Passiv Port
        httpMessage += F("<div class='row'><div class='col-25'><label for='pasv'>Passif Port</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='pasv' name='pasv' min='0' max='65535' "
                         "placeholder='50009' value=''></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='ftp'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        httpMessage += F("<a href='/config'>" D_HTTP_CONFIGURATION "</a>");

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_GPIO > 0
static void webHandleGpioConfig()
{ // http://plate01/config/gpio
    if(!http_is_authenticated(F("config/gpio"))) return;
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
                        case hasp_gpio_type_t::SERIAL_DIMMER_L8_HD_INVERTED:
                            httpMessage += F("L8-HD (inv.)");
                            break;
                        case hasp_gpio_type_t::SERIAL_DIMMER_L8_HD:
                            httpMessage += F("L8-HD");
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

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleGpioOutput()
{ // http://plate01/config/gpio/options
    if(!http_is_authenticated(F("config/gpio/options"))) return;

    { // Send Content
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
        httpMessage += getOption(hasp_gpio_type_t::SERIAL_DIMMER_L8_HD, F("L8-HD"), conf.type);
        httpMessage += getOption(hasp_gpio_type_t::SERIAL_DIMMER_L8_HD_INVERTED, F("L8-HD (inv.)"), conf.type);
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

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    // if(webServer.hasArg(F("action"))) dispatch_text_line(webServer.arg(F("action")).c_str()); // Security check
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleGpioInput()
{ // http://plate01/config/gpio/options
    if(!http_is_authenticated(F("config/gpio/input"))) return;

    { // Send Content
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

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();

    // if(webServer.hasArg(F("action"))) dispatch_text_line(webServer.arg(F("action")).c_str()); // Security check
}
#endif // HASP_USE_GPIO

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleDebugConfig()
{ // http://plate01/config/debug
    if(!http_is_authenticated(F("config/debug"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_DEBUG_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config' id='debug'>");

        // Baudrate
        httpMessage += F("<div class='row'><div class='col-25'><label for='baud'>Serial Port</label></div>");
        httpMessage += F("<div class='col-75'><select id='baud' name='baud'>");
        httpMessage += getOption(-1, F(D_SETTING_DISABLED)); // Don't use 0 here which is default 115200
        httpMessage += getOption(0, F(D_SETTING_DEFAULT));   // Don't use 0 here which is default 115200
        httpMessage += getOption(9600, F("9600"));
        httpMessage += getOption(19200, F("19200"));
        httpMessage += getOption(38400, F("38400"));
        httpMessage += getOption(57600, F("57600"));
        httpMessage += getOption(74880, F("74880"));
        httpMessage += getOption(115200, F("115200"));
        httpMessage += getOption(230400, F("230400"));
        httpMessage += getOption(460800, F("460800"));
        httpMessage += getOption(921600, F("921600"));
        httpMessage += F("</select></div></div>");

        // Telemetry Period
        httpMessage += F("<div class='row'><div class='col-25'><label for='tele'>Telemetry Period</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='tele' name='tele' min='0' max='65535' "
                         "value='");
        httpMessage += F("'></div></div>");

        // Invert
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='ansi'></label></div>");
        httpMessage += F("<div class='col-75'><input type='checkbox' id='ansi' name='ansi' value='1'");
        httpMessage += F(">Use ANSI Colors</div></div>");

#if HASP_USE_SYSLOG > 0
        // Syslog host
        httpMessage += F("<div class='row'><div class='col-25'><label for='host'>Syslog Server</label></div>");
        httpMessage += F("<div class='col-75'><input type='text' id='host' name='host' maxlength=");
        httpMessage += MAX_HOSTNAME_LENGTH - 1;
        httpMessage += F(" value='");
        httpMessage += F("'></div></div>");

        // Syslog Port
        httpMessage += F("<div class='row'><div class='col-25'><label for='port'>Syslog Port</label></div>");
        httpMessage += F("<div class='col-75'><input type='number' id='port' name='port' min='0' max='65535' "
                         "value='");
        httpMessage += F("'></div></div>");

        // Syslog Facility
        httpMessage += F("<div class='row'><div class='col-25'><label for='log'>Syslog Facility</label></div>");
        httpMessage += F("<div class='col-75'><select id='log' name='log'>");
        for(int i = 0; i < 8; i++) {
            httpMessage += getOption(i, String(F("Local")) + i);
        }
        httpMessage += F("</select></div></div>");

        // Syslog Protocol
        httpMessage += F("<div class='row'><div class='col-25'><label for='proto'>Syslog Protocol</label></div>");
        httpMessage += F("<div class='col-75'><input id='proto' name='proto' type='radio' value='0'");
        httpMessage += F(">IETF (RFC 5424) &nbsp; <input id='proto' name='proto' type='radio' value='1'");
        httpMessage += F(">BSD (RFC 3164)");
        httpMessage += F("</div></div>");
#endif

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='debug'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        // *******************************************************************

        add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleHaspConfig()
{ // http://plate01/config/http
    if(!http_is_authenticated(F("config/hasp"))) return;

    { // Send Content
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
        httpMessage += F("<div class='col-75'><input required type='file' name='filename'  "
                         "accept='.jsonl,.png,.zi'></div></div>");

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
        httpMessage += F("<div class='container'><form method='POST' action='/' id='hasp'>");

        // Theme
        httpMessage += F("<div class='row'><div class='col-25'><label for='theme'>UI Theme</label></div>");
        httpMessage += F("<div class='col-75'><select id='theme' name='theme'>");
#if LV_USE_THEME_HASP == 1
        httpMessage += getOption(2, F("Hasp Dark"));
        httpMessage += getOption(1, F("Hasp Light"));
#endif
#if LV_USE_THEME_EMPTY == 1
        httpMessage += getOption(0, F("Empty"));
#endif
#if LV_USE_THEME_MONO == 1
        httpMessage += getOption(3, F("Mono"));
#endif
#if LV_USE_THEME_MATERIAL == 1
        httpMessage += getOption(5, F("Material Dark"));
        httpMessage += getOption(4, F("Material Light"));
#endif
#if LV_USE_THEME_TEMPLATE == 1
        httpMessage += getOption(7, F("Template"));
#endif
        httpMessage += F("</select></div></div>");

        // Primary Color
        httpMessage += F("<div class='row'><div class='col-25'><label for='color1'>Primary Color</label></div>");
        httpMessage += F("<div class='col-75'><input id='color1' name='color1' type='color'></div></div>");

        // Secondary Color
        httpMessage += F("<div class='row'><div class='col-25'><label for='color2'>Secondary Color</label></div>");
        httpMessage += F("<div class='col-75'><input id='color2' name='color2' type='color'></div></div>");

        // Font
        httpMessage += F("<div class='row gap'><div class='col-25'><label for='font'>Default Font</label></div>");
        httpMessage += F("<div class='col-75'><select id='font' name='font'><option value=''>None</option>");
#if defined(ARDUINO_ARCH_ESP32)
        File root = HASP_FS.open("/");
        File file = root.openNextFile();

        while(file) {
            String filename = file.name();
            file            = root.openNextFile();
        }
#elif defined(ARDUINO_ARCH_ESP8266)
        Dir dir = HASP_FS.openDir("/");

        while(dir.next()) {
            File file       = dir.openFile("r");
            String filename = file.name();
            file.close();
        }
#endif
        httpMessage += F("</select></div></div>");

        // Pages.jsonl
        httpMessage += F("<div class='row'><div class='col-25'><label for='pages'>Startup Layout</label></div>");
        httpMessage += F("<div class='col-75'><input id='pages' name='pages' maxlength=31 placeholder='/pages.jsonl' "
                         "value=''></div></div>");

        // Startup Page
        httpMessage += F("<div class='row'><div class='col-25'><label for='startpage'>Startup Page</label></div>");
        httpMessage += F("<div class='col-75'><input id='startpage' required "
                         "name='startpage' type='number' min='1' max='12' value=''></div></div>");

        // Startup Brightness
        httpMessage += F("<div class='row'><div class='col-25'><label for='startdim'>Startup Brightness</label></div>");
        httpMessage += F("<div class='col-75'><input id='startdim' required name='startdim' type='number' min='0' "
                         "max='255' value=''></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='hasp'>" D_HTTP_SAVE_SETTINGS "</button>");
        httpMessage += F("</form></div>");

        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}
#endif // HASP_USE_CONFIG

#endif // HTTP_LEGACY

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_WIFI > 0
static void webHandleWifiConfig()
{ // http://plate01/config/wifi
    if(!http_is_authenticated(F("config/wifi"))) return;

    { // Send Content
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_WIFI_SETTINGS "</h2>");

        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/config' id='wifi'>");

        // Wifi SSID
        httpMessage += F("<div class='row'><div class='col-25 required'><label for='ssid'>SSID</label></div>");
        httpMessage += F("<div class='col-75'><input required type='text' id='ssid' name='ssid' maxlength=");
        httpMessage += MAX_SSID_LEN - 1;
        httpMessage += F(" placeholder='SSID' value='");
        httpMessage += F("'></div></div>");

        // Wifi Password
        httpMessage += F("<div class='row'><div class='col-25 required'><label for='pass'>Password</label></div>");
        httpMessage += F("<div class='col-75'><input required type='password' id='pass' name='pass' maxlength=");
        httpMessage += MAX_PASSPHRASE_LEN - 1;
        httpMessage += F(" placeholder='Password' value='");
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

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

#endif // HASP_USE_WIFI

static inline int handleFirmwareFile(String path)
{
    String contentType((char*)0);
    contentType = http_get_content_type(path);
    if(path.startsWith("/static/")) {
        path = path.substring(7);
    }

#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
    if(path == F("/edit.htm")) {
        return http_send_static_gzip_file(EDIT_HTM_GZ_START, EDIT_HTM_GZ_END, contentType);
    } else if(path == F("/hasp.htm")) { // 39 kB
        return http_send_static_gzip_file(HASP_HTM_GZ_START, HASP_HTM_GZ_END, contentType);
    } else if(path == F("/logo.svg")) { // 300 bytes
        return http_send_static_gzip_file(LOGO_SVG_GZ_START, LOGO_SVG_GZ_END, contentType);
    } else if(path == F("/style.css")) { // 11 kB
        return http_send_static_gzip_file(STYLE_CSS_GZ_START, STYLE_CSS_GZ_END, contentType);
    } else if(path == F("/vars.css")) {
        return http_send_static_file(HTTP_VARS_CSS, HTTP_VARS_CSS + sizeof(HTTP_VARS_CSS) - 1, contentType);
    } else if(path == F("/script.js")) { // 3 kB
        return http_send_static_gzip_file(SCRIPT_JS_GZ_START, SCRIPT_JS_GZ_END, contentType);
    } else if(path == F("/en.json")) { // 2 kB
        return http_send_static_gzip_file(EN_JSON_GZ_START, EN_JSON_GZ_END, contentType);
    } else if(path == F("/main.js")) { // 9 kB
        return http_send_static_gzip_file(MAIN_JS_GZ_START, MAIN_JS_GZ_END, contentType);
    } else if(path == F("/petite-vue.hasp.js")) { // 9 kB
        return http_send_static_gzip_file(PETITE_VUE_HASP_JS_GZ_START, PETITE_VUE_HASP_JS_GZ_END, contentType);
#if ESP_FLASH_SIZE > 4
    } else if(path == F("/ace.js")) { // 96 kB
        return http_send_static_gzip_file(ACE_JS_GZ_START, ACE_JS_GZ_END, contentType);
#endif
    }
#endif // ARDUINO_ARCH_ESP32

    if(path == F("/favicon.ico")) {
        return http_send_cached(204, contentType.c_str(), "", 0, 365 * 24 * 60 * 60); // No content
    }

    return 404;
}

static inline void httpHandleInvalidRequest(int statuscode, String& path)
{
    String httpMessage((char*)0);
    httpMessage.reserve(HTTP_PAGE_SIZE);

    if(statuscode == 500)
        httpMessage += F("Internal Server Error");
    else
        httpMessage += F(D_FILE_NOT_FOUND);

    httpMessage += F("\n\nURI: ");
    httpMessage += path;
    httpMessage += F("\nMethod: ");
    httpMessage += http_method_str(webServer.method());
    httpMessage += F("\nArguments: ");
    httpMessage += webServer.args();
    httpMessage += "\n";
    for(int i = 0; i < webServer.args(); i++) {
        httpMessage += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }
    webServer.send(statuscode, PSTR("text/plain"), httpMessage.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void httpHandleFile(String path)
{ // webServer 404
    int statuscode = 404;

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    if(statuscode == 404) {
        statuscode = handleFilesystemFile(path);
    }
#endif

    if(statuscode == 404) {
        statuscode = handleFirmwareFile(path);
    }

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    LOG_TRACE(TAG_HTTP, F("Sending %d %s to client connected from: %s"), statuscode, path.c_str(),
              webServer.client().remoteIP().toString().c_str());
#else
    // LOG_TRACE(TAG_HTTP,F("Sending 404 to client connected from: %s"),
    // String(webServer.client().remoteIP()).c_str());
#endif

    if(statuscode < 300 || statuscode == 304) return; // OK or Not Modified

    httpHandleInvalidRequest(statuscode, path);
}

static void httpHandleFileUri()
{
    httpHandleFile(webServer.uri());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void webHandleFirmware()
{
    if(!http_is_authenticated(F("firmware"))) return;

    if(webServer.method() == HTTP_POST && webServer.hasArg(PSTR("url"))) {
        StaticJsonDocument<512> settings;
        for(int i = 0; i < webServer.args(); i++) settings[webServer.argName(i)] = webServer.arg(i);
        bool updated = otaSetConfig(settings.as<JsonObject>());

        String url = webServer.arg(PSTR("url"));
        {
            String httpMessage((char*)0);
            httpMessage.reserve(HTTP_PAGE_SIZE);
            httpMessage += F("<h1>");
            httpMessage += haspDevice.get_hostname();
            httpMessage += F("</h1><hr>");

            httpMessage += F("<h2>" D_HTTP_FIRMWARE_UPGRADE "</h2>");
            httpMessage += F("<p>Updating firmware from: ");
            httpMessage += url;
            httpMessage += F("</p><p>Please wait...</p>");

            httpMessage += FPSTR(MAIN_MENU_BUTTON);

            webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 60);
            webServer.sendContent(httpMessage);
        }

        webSendFooter();
        dispatch_web_update(NULL, url.c_str(), TAG_HTTP);
        return;
    } else {
        // Send Firmware page
        String httpMessage((char*)0);
        httpMessage.reserve(HTTP_PAGE_SIZE);
        httpMessage += F("<h1>");
        httpMessage += haspDevice.get_hostname();
        httpMessage += F("</h1><hr>");
        httpMessage += F("<h2>" D_HTTP_FIRMWARE_UPGRADE "</h2>");

        // Form
        httpMessage +=
            F("<div class='container'><form method='POST' action='/update' enctype='multipart/form-data' id='ota'>");

        // File
        httpMessage +=
            F("<div class='row'><div class='col-25'><label class='required' for='filename'>OTA File</label></div>");
        httpMessage += F("<div class='col-75'><input required type='file' name='filename' accept='.bin'></div></div>");

        // Destination
        httpMessage += F("<div class='row'><div class='col-25'><label for='file'>Target</label></div>");
        httpMessage +=
            F("<div class='col-75'><input id='cmd' name='cmd' type='radio' value='0' checked>Firmware &nbsp; "
              "<input id='cmd' name='cmd' type='radio' value='100'>Filesystem</div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='debug'>" D_HTTP_UPDATE_FIRMWARE "</button>");
        httpMessage += F("</form></div>");

        // Update from URL
        // Form
        httpMessage += F("<div class='container'><form method='POST' action='/firmware'>");

        // URL
        httpMessage +=
            F("<div class='row'><div class='col-25'><label class='required' for='url'>OTA URL</label></div>");
        httpMessage += F("<div class='col-75'><input required id='url' name='url' value=''></div></div>");

        // Redirect
        httpMessage += F("<div class='row'><div class='col-25'><label for='redirect'>Follow Redirects</label></div>");
        httpMessage += F("<div class='col-75'><select id='redirect' name='redirect'>");
        httpMessage += getOption(0, F("Disabled"));
        httpMessage += getOption(1, F("Strict"));
        httpMessage += getOption(2, F("Always"));
        httpMessage += F("</select></div></div>");

        // Submit & End Form
        httpMessage += F("<button type='submit' name='save' value='debug'>" D_HTTP_UPDATE_FIRMWARE "</button>");
        httpMessage += F("</form></div>");

        httpMessage += FPSTR(MAIN_MENU_BUTTON);

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), 0);
        webServer.sendContent(httpMessage);
    }
    webSendFooter();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0

static void httpHandleResetConfig()
{ // http://plate01/config/reset
    if(!http_is_authenticated(F("reset"))) return;

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
            httpMessage += F("<form method='POST' action='/config/reset'>");
            httpMessage +=
                F("<div class=\"warning\"><b>Warning</b><p>This process will reset all settings to the "
                  "default values. The internal flash will be erased and the device is restarted. You may need to "
                  "connect to the WiFi AP displayed on the panel to reconfigure the device before accessing it "
                  "again.</p>"
                  "<p>ALL FILES WILL BE LOST!</p></div>");
            httpMessage += F("<p><button class='red' type='submit' name='confirm' value='yes'>" D_HTTP_ERASE_DEVICE
                             "</button></p></form>");

            add_form_button(httpMessage, F(D_BACK_ICON D_HTTP_CONFIGURATION), F("/config"));
        }

        webSendHtmlHeader(haspDevice.get_hostname(), httpMessage.length(), resetConfirmed ? 10 : 0);
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
#endif
#if HASP_USE_ETHERNET > 0
    IPAddress ip;
#if defined(ARDUINO_ARCH_ESP32)
#if HASP_USE_ETHSPI > 0
    ip = ETHSPI.localIP();
#else
    ip = ETH.localIP();
#endif
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
/*
static void webSendCssVars()
{
    if(!http_is_authenticated(F("cssvars"))) return;

    char filename[32];
    strncpy(filename, webServer.uri().c_str(), sizeof(filename));

    if(HASP_FS.exists(filename)) {
        String contentType((char*)0);
        contentType = http_get_content_type(filename);
        File file   = HASP_FS.open(filename, "r");
        http_send_cache_header(file.size(), 3600);
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
    http_send_cached(200, PSTR("text/css"), HTTP_CSS.c_str(), HTTP_CSS.length());
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void webStartConfigPortal()
{
#if HASP_USE_CAPTIVE_PORTAL > 0
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", apIP);
#endif // HASP_USE_CAPTIVE_PORTAL

    webServer.on(F("/vars.css"), httpHandleFileUri);
    webServer.on(F("/style.css"), httpHandleFileUri);
    webServer.on(F("/script.js"), httpHandleFileUri);
    // reply to all requests with same HTML
#if HASP_USE_WIFI > 0
    webServer.onNotFound(webHandleWifiConfig);
#endif
    LOG_TRACE(TAG_HTTP, F("Wifi access point"));
}

void httpSetup()
{
    Preferences preferences;
    nvs_user_begin(preferences, FP_HTTP, true);
    String password = preferences.getString(FP_CONFIG_PASS, HTTP_PASSWORD);
    strncpy(http_config.password, password.c_str(), sizeof(http_config.password));
    LOG_DEBUG(TAG_HTTP, F(D_BULLET "Read %s => %s (%d bytes)"), FP_CONFIG_PASS, password.c_str(), password.length());

    // ask server to track these headers
    const char* headerkeys[] = {"Content-Length", "If-None-Match"}; // "Authentication" is automatically checked
    size_t headerkeyssize    = sizeof(headerkeys) / sizeof(char*);
    webServer.collectHeaders(headerkeys, headerkeyssize);

    // Shared pages between STA and AP
    webServer.on(F("/about"), webHandleAbout);
    // webServer.on(F("/vars.css"), webSendCssVars);
    // webServer.on(F("/js"), webSendJavascript);
    webServer.on(UriBraces(F("/api/{}/")), webHandleApi);
    webServer.on(UriBraces(F("/api/config/{}/")), webHandleApiConfig);
    webServer.on(UriBraces(F("/{}/")), HTTP_GET, []() { httpHandleFile(F("/hasp.htm")); });
    webServer.on(UriBraces(F("/config/{}/")), HTTP_GET, []() { httpHandleFile(F("/hasp.htm")); });

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    webServer.on(F("/firmware"), webHandleFirmware);
    webServer.on(
        F("/update"), HTTP_POST,
        []() {
            webServer.send(200, "text/plain", "");
            LOG_VERBOSE(TAG_HTTP, F("Total size: %s"), webServer.hostHeader().c_str());
        },
        webHandleFirmwareUpload);
#endif

#ifdef HTTP_LEGACY
    webServer.on(F("/config"), webHandleConfig);
#endif

#if HASP_USE_WIFI > 0
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
        dispatch_page(NULL, webServer.arg(F("page")).c_str(), TAG_HTTP);
        // dispatch_set_page(pageid.toInt(), LV_SCR_LOAD_ANIM_NONE);
    });

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    webServer.on(F("/list"), HTTP_GET, handleFileList);
    // load editor
    webServer.on(F("/edit"), HTTP_GET, []() { httpHandleFile(F("/edit.htm")); });
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
    webServer.on(F("/screenshot"), webHandleScreenshot);
#ifdef HTTP_LEGACY
    webServer.on(F("/info"), webHandleInfoJson);
    webServer.on(F("/reboot"), httpHandleReboot);
#endif

#if HASP_USE_CONFIG > 0
#ifdef HTTP_LEGACY
    webServer.on(F("/config/hasp"), webHandleHaspConfig);
    webServer.on(F("/config/http"), webHandleHttpConfig);
    webServer.on(F("/config/gui"), webHandleGuiConfig);
    webServer.on(F("/config/debug"), webHandleDebugConfig);
#if HASP_USE_MQTT > 0
    webServer.on(F("/config/mqtt"), webHandleMqttConfig);
#endif
#if HASP_USE_FTP > 0
    webServer.on(F("/config/ftp"), webHandleFtpConfig);
#endif
#if HASP_USE_WIFI > 0
    webServer.on(F("/config/wifi"), webHandleWifiConfig);
#endif
#if HASP_USE_GPIO > 0
    webServer.on(F("/config/gpio"), webHandleGpioConfig);
    webServer.on(F("/config/gpio/options"), webHandleGpioOutput);
    webServer.on(F("/config/gpio/input"), webHandleGpioInput);
#endif
#endif // HTTP_LEGACY
    webServer.on(F("/config/reset"), httpHandleResetConfig);
#endif // HASP_USE_CONFIG
    webServer.onNotFound(httpHandleFileUri);

    LOG_INFO(TAG_HTTP, F(D_SERVICE_STARTED));
    // webStart();  Wait for network connection
}

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

    // if(strcmp(http_config.password, settings[FPSTR(FP_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    // settings[FPSTR(FP_CONFIG_PASS)] = http_config.password;
    if(strcmp(D_PASSWORD_MASK, settings[FPSTR(FP_CONFIG_PASS)].as<String>().c_str()) != 0) changed = true;
    settings[FPSTR(FP_CONFIG_PASS)] = D_PASSWORD_MASK;

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
    Preferences preferences;
    nvs_user_begin(preferences, FP_HTTP, false);

    configOutput(settings, TAG_HTTP);
    bool changed = false;

    changed |= configSet(http_config.port, settings[FPSTR(FP_CONFIG_PORT)], F("httpPort"));

    if(!settings[FPSTR(FP_CONFIG_USER)].isNull()) {
        changed |= strcmp(http_config.username, settings[FPSTR(FP_CONFIG_USER)]) != 0;
        strncpy(http_config.username, settings[FPSTR(FP_CONFIG_USER)], sizeof(http_config.username));
    }

    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() &&
       settings[FPSTR(FP_CONFIG_PASS)].as<String>() != String(FPSTR(D_PASSWORD_MASK))) {
        changed |= strcmp(FP_CONFIG_PASS, settings[FPSTR(FP_CONFIG_PASS)]) != 0;
        strncpy(http_config.password, settings[FPSTR(FP_CONFIG_PASS)], sizeof(http_config.password));
        nvsUpdateString(preferences, FP_CONFIG_PASS, settings[FPSTR(FP_CONFIG_PASS)]);
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
