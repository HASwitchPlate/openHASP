/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h" // include first

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

#ifndef HASP_ONLINE_CMD
#define HASP_ONLINE_CMD "jsonl {\"page\":0,\"id\":239,\"obj\":\"msgbox\",\"text\":\"%ip%\", \"auto_close\":20000}"
#endif

#ifndef HASP_OFFLINE_CMD
#define HASP_OFFLINE_CMD                                                                                               \
    "jsonl {\"page\":0,\"id\":239,\"obj\":\"msgbox\",\"text\":\"" D_NETWORK_OFFLINE "\", \"auto_close\":20000}"
#endif

#ifndef HASP_PAGES_JSONL
#define HASP_PAGES_JSONL "{\"page\":1,\"id\":10,\"w\":240,\"obj\":\"label\",\"text\":\"%hostname%\"}"
#endif

#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "hasp_debug.h"
#include "hasp_filesystem.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "rom/crc.h"

void filesystemUnzip(const char*, const char* filename, uint8_t source)
{
    File zipfile = HASP_FS.open(filename, FILE_READ);
    if(!zipfile) {
        return;
    }

    int32_t head;
    size_t len;
    bool done = false;

    zipfile.seek(0);
    while(!done) {
        len = zipfile.read((uint8_t*)&head, sizeof(head));
        if(len != sizeof(head)) {
            done = true;
            continue;
        }

        switch(head) {
            case 0x04034b50: {
                zip_file_header_t fh;
                zipfile.seek(zipfile.position() - 2, SeekSet); // rewind for struct alignment (26-28)
                len = zipfile.read((uint8_t*)(&fh), sizeof(zip_file_header_t));
                if(len != sizeof(zip_file_header_t)) {
                    done = true;
                    continue;
                }

                if(fh.filename_length >= 31) {
                    LOG_WARNING(TAG_FILE, F("filename length too long %d"), fh.filename_length);
                    zipfile.seek(fh.filename_length + fh.extra_length, SeekCur); // skip extra field
                    continue;
                    // } else {
                    //     LOG_WARNING(TAG_FILE, F("min %d - flag %d - len %d - xtra %d"), fh.min_version, fh.flags,
                    //                 fh.filename_length, fh.extra_length);
                }
                char name[32] = {0};
                name[0]       = '/';

                len = zipfile.read((uint8_t*)&name[1], fh.filename_length);
                if(len != fh.filename_length) {
                    LOG_WARNING(TAG_FILE, F("filename read failed %d != %d"), fh.filename_length, len);
                    done = true;
                    continue;
                }
                zipfile.seek(fh.extra_length, SeekCur); // skip extra field

                if(fh.compression_method != ZIP_NO_COMPRESSION) {
                    LOG_WARNING(TAG_FILE, F("Compression is not supported %d"), fh.compression_method);
                    zipfile.seek(fh.compressed_size, SeekCur); // skip compressed file
                } else {

                    if(HASP_FS.exists(name)) HASP_FS.remove(name);

                    File f = HASP_FS.open(name, FILE_WRITE);
                    if(f) {
                        uint8_t buffer[512];
                        uint32_t crc32 = 0;

                        while(!done && fh.compressed_size >= 512) {
                            len = zipfile.readBytes((char*)&buffer, 512);
                            if(len != 512) done = true;
                            fh.compressed_size -= len;
                            crc32 = crc32_le(crc32, buffer, len);
                            f.write(buffer, len);
                        }

                        if(!done && fh.compressed_size > 0) {
                            len = zipfile.readBytes((char*)&buffer, fh.compressed_size);
                            if(len != fh.compressed_size) done = true;
                            fh.compressed_size -= len;
                            crc32 = crc32_le(crc32, buffer, len);
                            f.write(buffer, len);
                        }

                        if(crc32 != fh.crc) done = true;

                        if(!done) {
                            Parser::format_bytes(fh.uncompressed_size, (char*)buffer, sizeof(buffer));
                            LOG_VERBOSE(TAG_FILE, F(D_BULLET "%s (%s)"), name, buffer);
                        } else {
                            LOG_ERROR(TAG_FILE, F(D_FILE_SAVE_FAILED), name);
                        }

                        f.close();
                    }
                }

                break;
            }
            case 0x02014b50:
                done = true;
                break;
            case 0x06054b50:
                // end of file
                done = true;
                break;
            default: {
                char outputString[9];
                itoa(head, outputString, 16);
                LOG_WARNING(TAG_FILE, F("invalid %s"), outputString);
                done = true;
            }
        }
    }
    zipfile.close();
    LOG_VERBOSE(TAG_FILE, F("extracting %s complete"), filename);
}
#endif

void filesystemInfo()
{ // Get all information of your SPIFFS
#ifdef ESP8266
    FSInfo fs_info;
    HASP_FS.info(fs_info);
    Log.verbose(TAG_FILE, "Partition size: total: %d, used: %d", fs_info.totalBytes, fs_info.usedBytes);
#endif

#ifdef ESP32
    Log.verbose(TAG_FILE, "Partition size: total: %d, used: %d", HASP_FS.totalBytes(), HASP_FS.usedBytes());
#endif
}

void filesystemList()
{
#if HASP_USE_SPIFFS > 0
#if defined(ARDUINO_ARCH_ESP8266)
    if(!HASP_FS.begin()) {
#else
    if(!HASP_FS.begin(true)) {            // default vfs path: /littlefs
#endif
        LOG_ERROR(TAG_FILE, F("Flash file system not mouted."));
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

static void filesystem_write_file(const char* filename, const char* data)
{
    if(HASP_FS.exists(filename)) return;

    LOG_TRACE(TAG_CONF, F(D_FILE_SAVING), filename);
    File file = HASP_FS.open(filename, "w");

    if(file) {
        file.print(data);
        file.close();
        LOG_INFO(TAG_CONF, F(D_FILE_SAVED), filename);
    } else {
        LOG_ERROR(TAG_FILE, D_FILE_SAVE_FAILED, filename);
    }
}

void filesystemSetupFiles()
{
    filesystem_write_file("/pages.jsonl", HASP_PAGES_JSONL);
    filesystem_write_file("/online.cmd", HASP_ONLINE_CMD);
    filesystem_write_file("/offline.cmd", HASP_OFFLINE_CMD);
}

bool filesystemSetup(void)
{
    // no SPIFFS settings, as settings depend on SPIFFS
    // no Logging, because it depends on the configuration file

    // Logging is defered until debugging has started
    // FS success or failure is printed at that time !

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#if defined(ARDUINO_ARCH_ESP8266)
    if(!HASP_FS.begin()) {
#else
    if(HASP_FS.begin(false)) return true; // already formated

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