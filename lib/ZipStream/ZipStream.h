/**************************************************************************//**
 * @file ZipStream.h
 * @brief This library pack and unpack a ZIP-archive without store the archive
 *      in file system. Only uncompressed ZIP archives are supported (Store 
 *      mode). ZIP and Unzip can not use the same instance on the same time.
 *
 *      Under linux the archive can be created with the following command :
 *        zip -0 foobar.zip *
 *      and unpacked with
 *        unzip foobar.zip
 * 
 *      ZIP : The ZIP archive is transferred to the library as a stream. 
 *      This can be a file or a web stream. 
 *      At the beginning, a file list in JSON format must be passed to the 
 *      "beginZip()" function. The file list must be in the following format:
 *          [{"name":"foo.txt"},{"name":"config.json"},{"name":"bar.png"}]
 * 
 *      Unzip : Existing files are overwritten.
 *      At the beginning, the "beginUnZip()" function must be passed.
 *  
 * 
 * @version 0.9.0
 * @date 2025-04-14
 * 
 * @copyright Copyright (c) 2025
 * 
 **************************************************************************  */

#ifndef _ZIP_STREAM_H_
#define _ZIP_STREAM_H_

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

#include "stdio.h"

#include <Arduino.h>
#include "ArduinoJson.h"
#include "FS.h"

#include <stddef.h>
#include <Stream.h>

enum zip_stream_error_t {
    // Errors, negative values
    ZIP_STREAM_ERROR_NO_MEMORY = -1,
    ZIP_STREAM_NOT_VALID = -2,
    ZIP_STREAM_MODE_WRONG = -3,     // Wrong mode Zip / UnZip
    ZIP_STREAM_UNKNOWN_ERROR = -4,
    // No error
    ZIP_STREAM_OK = 0,
    // Warnings, positive values
    ZIP_STREAM_COMPRESSION_ERROR,
    ZIP_STREAM_DATA_INVALID,
    ZIP_STREAM_CRC_ERROR,
    ZIP_STREAM_FILE_INVALID,
    ZIP_STREAM_FILE_NOT_FOUND,
    ZIP_STREAM_FILE_OPEN_FAILED,
    ZIP_STREAM_FILENAME_LENGTH,
    ZIP_STREAM_FILE_LIST_FAIL,
    ZIP_STREAM_OVERWRITE_FILE
};

/**************************************************************************//**
 * @brief class ZipStream
 * 
 **************************************************************************  */

class ZipStream : public Stream {
private:
    enum { ZIP_NO_COMPRESSION = 0, ZIP_DEFLTATE = 8 };
    typedef uint16_t zip_compression_method_t;

    enum zip_stream_state_t { 
        ZIP_STREAM_INACTIVE = 0,
        // UnZip, positive values
        ZIP_STREAM_EXPECT_SIGNATURE = 1,
        ZIP_STREAM_EXPECT_HEADER,
        ZIP_STREAM_HEADER_COMPLETE,
        ZIP_STREAM_GET_FILENAME,
        ZIP_STREAM_GET_EXTRA_FIELD,
        ZIP_STREAM_STORE_DATA,
        ZIP_STREAM_SKIP_DATA,
        // Zip, negative values
        ZIP_STREAM_LOCAL_HEADER = -1,
        ZIP_STREAM_FILE_DATA = -2,
        ZIP_STREAM_CENTRAL_DIR_HEADER = -3,
        ZIP_STREAM_END_OF_CENTRAL_DIR = -4,
    };

    typedef struct __attribute__((packed))
    {
        uint16_t min_version;
        uint16_t flags;
        zip_compression_method_t compression_method;
        uint16_t time_modified;
        uint16_t date_modified;
        uint32_t crc;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint16_t filename_length;
        uint16_t extra_length;
    } zip_local_file_header_t;

    typedef struct __attribute__((packed))
    {
        uint16_t version_made_by;
        uint16_t version_min;
        uint16_t flags;
        zip_compression_method_t compression_method;
        uint16_t time_modified;
        uint16_t date_modified;
        uint32_t crc;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint16_t filename_length;
        uint16_t extra_length;
        uint16_t comment_length;
        uint16_t disk_no;
        uint16_t internal_file_attr;
        uint32_t external_file_attr;
        uint32_t relative_offset;
    } zip_central_dir_file_header_t;

    typedef struct __attribute__((packed))
    {
        uint32_t crc;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
    } zip_data_discriptor_t;

    typedef struct __attribute__((packed))
    {
        uint16_t disk_no_this;
        uint16_t disk_dir_starts;
        uint16_t central_dir_records;
        uint16_t total_dir_records;
        uint32_t central_dir_size;
        uint32_t offset_central_dir_start;
        uint16_t comment_length;
    } zip_end_central_dir_record_t;

    typedef struct __attribute__((packed))
    {
        bool is_valid;
        uint32_t crc;
        uint32_t compressed_size;
        uint32_t relative_offset;
    } zip_file_discriptor_t;


    zip_stream_state_t _streamState;
    zip_stream_error_t _lastErrorCode;
    String _lastErrorString;

    uint8_t *_pBuffer; // Pointer to the internal buffer
    size_t _position; // Current read position in the buffer
    fs::File _File; // On zipping the ZIP-file; on unzipping the unpacked file

    // Unzip
    size_t _datalen;
    uint32_t _signature;
    zip_local_file_header_t _zipLocalHeader;
    uint32_t _fileCRC32;

    // Zip
    StaticJsonDocument<2048> _jsonDoc;
    DeserializationError _jsonError;
    JsonArray _jFileArray;

    uint16_t _fileIdx;  // Current file index
    size_t _content_length;
    zip_file_discriptor_t *_pFileDescriptorBuffer;
    zip_end_central_dir_record_t _pEndCentralDirRecord;

    void buildBufferLocalFileHeader();
    void buildBufferCentralDirFileHeader();
    void buildBufferEndCentralFileHeader();

protected :

public:
    ZipStream() : 
        _streamState(ZIP_STREAM_INACTIVE),
        _pBuffer(NULL),
        _position(0),

        _content_length(0), 
        _fileIdx(0),
        _pFileDescriptorBuffer(NULL),

        _signature(0),
        _datalen(0),
        _fileCRC32(0),

        _lastErrorCode(ZIP_STREAM_OK),
        _lastErrorString("")
        { }

    ~ZipStream() { flush(); }

    zip_stream_error_t getLastError() { return _lastErrorCode; }
    String getLastErrorString( ) { return _lastErrorString; }
    void resetLastError() { _lastErrorCode = ZIP_STREAM_OK; _lastErrorString = ""; }

    zip_stream_error_t beginZip(String *fileList);
    zip_stream_error_t beginUnZip();

    // Read packed data chunk (only for zipping)
    size_t readBytes(uint8_t *buffer, size_t length);
    size_t readBytes(char *buffer, size_t length) { return readBytes((uint8_t *) buffer, length); };

    // Check how many bytes are available to read (only for zipping)
    int available() override { 
        if (!_pBuffer || _streamState >= ZIP_STREAM_INACTIVE) {
            return 0; // No buffer available
        }
        return _content_length - _position; 
    }

    // Read a single byte from the buffer (only for zipping)
    // not implemented
    int read() override { return -1; }

    // Peek at the next byte in the buffer without advancing the position (only for zipping)
    // not implemented
    int peek() override { return -1; }

    // Write a single byte to the buffer (only for unzipping)
    // not implemented
    size_t write(uint8_t byte) override { return 0; }

    // Unzip buffer (only for unzipping)
    size_t write(const uint8_t *pInData, size_t size) override;

    // Unzip stream (only for unzipping)
    size_t write(Stream &zipStream);

    // Flush the buffer (clear all data)
    void flush() override;

    // Reset the read position to the beginning
//    void reset() { _position = 0; }
};
    
#endif // HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

#endif // _ZIP_STREAM_H_