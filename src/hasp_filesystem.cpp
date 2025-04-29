/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef ARDUINO
#include <Arduino.h>
#include "ArduinoLog.h"
#include "FS.h"
#endif

#include "hasp_conf.h" // include first

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

#ifndef HASP_ONLINE_CMD
#define HASP_ONLINE_CMD "jsonl {\"page\":0,\"id\":239,\"obj\":\"msgbox\",\"text\":\"%ip%\",\"auto_close\":20000}"
#endif

#ifndef HASP_OFFLINE_CMD
#define HASP_OFFLINE_CMD                                                                                               \
    "jsonl {\"page\":0,\"id\":239,\"obj\":\"msgbox\",\"text\":\"" D_NETWORK_OFFLINE "\",\"auto_close\":20000}"
#endif

#ifndef HASP_PAGES_JSONL
// #define HASP_PAGES_JSONL "{\"page\":1,\"id\":10,\"w\":240,\"obj\":\"label\",\"text\":\"%hostname%\"}"
#if defined(PAGES_JSONL)
extern const uint8_t PAGES_JSONL_START[] asm(QUOTE(PAGES_JSONL)"_start");
extern const uint8_t PAGES_JSONL_END[] asm(QUOTE(PAGES_JSONL)"_end");
#else
extern const uint8_t PAGES_JSONL_START[] asm("_binary_data_pages_pages_jsonl_start");
extern const uint8_t PAGES_JSONL_END[] asm("_binary_data_pages_pages_jsonl_end");
#endif
#endif

#include "ArduinoJson.h"
#include "hasp_debug.h"
#include "hasp_filesystem.h"
#include "ZipStream.h"


#if defined(ARDUINO_ARCH_ESP32)
#include "rom/crc.h"

void filesystemUnzip(const char*, const char* filename, uint8_t source)
{
    if (strlen(filename) < 1) {
        LOG_ERROR(TAG_FILE, F("File name not available"));
        return;
    }
    
    File zipfile = HASP_FS.open(filename, FILE_READ);
    if (!zipfile) {
        LOG_ERROR(TAG_FILE, F("File %s not found"), filename);
        return;
    }

    ZipStream unzipStream;
    if ( unzipStream.beginUnZip() == ZIP_STREAM_OK) {
        unzipStream.write(zipfile);
    }

    zipfile.close();

    if (unzipStream.getLastError() != ZIP_STREAM_OK) {
        LOG_ERROR(TAG_FILE, F("Unpacking error %d - %s"), unzipStream.getLastError(), unzipStream.getLastErrorString().c_str() );
    }
}
#endif

void filesystemInfo()
{ // Get all information of your SPIFFS
    char used[16]  = "";
    char total[16] = "";

#ifdef ESP8266
    FSInfo fs_info;
    HASP_FS.info(fs_info);
    Parser::format_bytes(fs_info.usedBytes(), used, sizeof(used));
    Parser::format_bytes(fs_info.totalBytes(), total, sizeof(total));
#endif

#ifdef ESP32
    Parser::format_bytes(HASP_FS.usedBytes(), used, sizeof(used));
    Parser::format_bytes(HASP_FS.totalBytes(), total, sizeof(total));
#endif

    Log.verbose(TAG_FILE, "Partition size: used: %s / total: %s", used, total);
}

void filesystemList()
{
#if HASP_USE_SPIFFS > 0
#if defined(ARDUINO_ARCH_ESP8266)
    if(!HASP_FS.begin()) {
#else
    if(!HASP_FS.begin(true)) {            // default vfs path: /littlefs
#endif
        LOG_ERROR(TAG_FILE, F("Flash file system not mounted."));
    } else {

        LOG_VERBOSE(TAG_FILE, F("Listing files on the internal flash:"));

#if defined(ARDUINO_ARCH_ESP32)
        File root = HASP_FS.open("/");
        File file = root.openNextFile();
        while(file) {
            LOG_VERBOSE(TAG_FILE, F("   * %s  (%u bytes)"), file.name(), (uint32_t)file.size());
            file = root.openNextFile();
        }
#endif
#if defined(ARDUINO_ARCH_ESP8266)
        Dir dir = HASP_FS.openDir("/");
        while(dir.next()) {
            LOG_VERBOSE(TAG_FILE, F("   * %s  (%u bytes)"), dir.fileName().c_str(), (uint32_t)dir.fileSize());
        }
#endif
    }
#endif
}

#if defined(ARDUINO_ARCH_ESP32)
String filesystem_list(fs::FS& fs, const char* dirname, uint8_t levels)
{
    LOG_VERBOSE(TAG_FILE, "Listing directory: %s\n", dirname);
    String data = "[";

    File root = fs.open(dirname);
    if(!root) {
        LOG_WARNING(TAG_FILE, "Failed to open directory");
    } else if(!root.isDirectory()) {
        LOG_WARNING(TAG_FILE, "Not a directory");
    } else {
        File file = root.openNextFile();
        while(file) {

            if(data != "[") {
                data += ",";
            }
            data += "{\"name\":\"";
            data += file.name();
            data += "\"";

            if(file.isDirectory()) {
                data += ",\"children\":";
                if(levels) {
                    String dir = dirname;
                    dir += file.name();
                    dir += '/';
                    data += filesystem_list(fs, dir.c_str(), levels - 1);
                } else {
                    data += "[]";
                }
            }
            data += "}";
            file = root.openNextFile();
        }
        root.close();
    }

    data += "]";
    return data;
}
#endif

static void filesystem_write_file(const char* filename, const char* data, size_t len)
{
    if(HASP_FS.exists(filename)) return;

    LOG_TRACE(TAG_CONF, F(D_FILE_SAVING), filename);
    File file = HASP_FS.open(filename, "w");

    if(file) {
        file.write((const uint8_t*)data, len);
        file.close();
        LOG_INFO(TAG_CONF, F(D_FILE_SAVED), filename);
    } else {
        LOG_ERROR(TAG_FILE, D_FILE_SAVE_FAILED, filename);
    }
}

void filesystemSetupFiles()
{
#ifdef HASP_PAGES_JSONL
    filesystem_write_file("/pages.jsonl", HASP_PAGES_JSONL, strlen(HASP_PAGES_JSONL));
#else
    filesystem_write_file("/pages.jsonl", (const char*)PAGES_JSONL_START, PAGES_JSONL_END - PAGES_JSONL_START);
#endif
    filesystem_write_file("/online.cmd", HASP_ONLINE_CMD, strlen(HASP_ONLINE_CMD));
    filesystem_write_file("/offline.cmd", HASP_OFFLINE_CMD, strlen(HASP_OFFLINE_CMD));
#ifdef HASP_BOOT_CMD
    filesystem_write_file("/boot.cmd", HASP_BOOT_CMD, strlen(HASP_BOOT_CMD));
#endif
#ifdef HASP_MQTT_ON_CMD
    filesystem_write_file("/mqtt_on.cmd", HASP_MQTT_ON_CMD, strlen(HASP_MQTT_ON_CMD));
#endif
#ifdef HASP_MQTT_OFF_CMD
    filesystem_write_file("/mqtt_off.cmd", HASP_MQTT_OFF_CMD, strlen(HASP_MQTT_OFF_CMD));
#endif
}

bool filesystemSetup(void)
{
    // no SPIFFS settings, as settings depend on SPIFFS
    // no Logging, because it depends on the configuration file

    // Logging is deferred until debugging has started
    // FS success or failure is printed at that time !

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#if defined(ARDUINO_ARCH_ESP8266)
    if(!HASP_FS.begin()) {
#else
    if(HASP_FS.begin(false)) return true; // already formatted

    if(!HASP_FS.begin(true)) { // format partition
#endif
        // LOG_ERROR(TAG_FILE, F("SPI flash init failed. Unable to mount FS."));
        // return false;
    } else {
        filesystemSetupFiles();
        // LOG_INFO(TAG_FILE, F("SPI Flash FS mounted"));
        return true;
    }
#endif

    return false;
}

#endif
