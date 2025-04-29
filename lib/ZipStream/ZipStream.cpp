/**************************************************************************//**
 * @file ZipStream.cpp
 * @brief This library pack and unpack a ZIP-archive without store the archive
 *      in file system. Only uncompressed ZIP archives are supported (Store 
 *      mode). ZIP and Unzip can not use the same instance on the same time.
 *
 *      Under linux the archive can be created with the following command :
 *        zip -0 foobar.zip *
 *      and unpacked with
 *        unzip foobar.zip
 *
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


#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

#include "ZipStream.h"
#include "rom/crc.h"

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0)
#include <LittleFS.h>
#define HASP_FS LittleFS
#else
#include "LITTLEFS.h"
#include "esp_littlefs.h"
#define HASP_FS LITTLEFS
#endif // ESP_ARDUINO_VERSION

#define ZIP_LFH_SIGNATURE 0x04034b50
#define ZIP_CDFH_SIGNATURE 0x02014b50
#define ZIP_EOCD_SIGNATURE 0x06054b50
#define ZIP_DD_SIGNATURE 0x08074b50

#define TAG_ZIP "ZIP"
#define ZIP_STREAM_BUFFER_SIZE 512
#define ZIP_BUFFER_SIZE 512
#define CRC_BUFFER_SIZE 512

static const char zip_ok[] PROGMEM = "OK";
static const char zip_memory_error[] PROGMEM = "Failed to allocate memory";
static const char zip_compreession_unsupport[] PROGMEM = "Compression is unsupported";
static const char zip_invalid_file_list_error[] PROGMEM = "File list error";
static const char zip_file_not_found_error[] PROGMEM = "File not found";
static const char zip_file_open_failed_error[] PROGMEM = "File open failed";
static const char zip_filename_length_error[] PROGMEM = "Filename too long";
static const char zip_file_invalid_data_error[] PROGMEM = "File invalid";
static const char zip_file_crc_error[] PROGMEM = "File wrong CRC";
static const char zip_invalid_stream_error[] PROGMEM = "Stream invalid";
static const char zip_initialisation_error[] PROGMEM = "Initialisation wrong";
static const char zip_unknown_error[] PROGMEM = "Unknown internal error";

/**************************************************************************//**
 * @brief Initialises the class for a new unpacking process. Must always 
 *      be called before the first write to the stream.
 * 
 * @return zip_stream_error_t 
 **************************************************************************  */
zip_stream_error_t ZipStream::beginUnZip() 
{ 
    if (_streamState != ZIP_STREAM_INACTIVE) {
        _lastErrorCode = ZIP_STREAM_MODE_WRONG;
        _lastErrorString = String(zip_initialisation_error);
        return _lastErrorCode;
    }

    flush();

    _position = 0;
    _lastErrorCode = ZIP_STREAM_OK;
    _lastErrorString = "";

    _pBuffer = (uint8_t *)malloc(ZIP_BUFFER_SIZE);

    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
        return _lastErrorCode;
    }

    _streamState = ZIP_STREAM_EXPECT_SIGNATURE;

    return _lastErrorCode;
}

/**************************************************************************//**
 * @brief 
 * 
 * @param fileList 
 * @return zip_stream_error_t 
 **************************************************************************  */
zip_stream_error_t ZipStream::beginZip(String *fileList) 
{
    if (_streamState != ZIP_STREAM_INACTIVE) {
        _lastErrorCode = ZIP_STREAM_MODE_WRONG;
        _lastErrorString = String(zip_initialisation_error);
        return _lastErrorCode;
    }

    flush();

    _position = 0;
    _fileIdx = 0;
    _lastErrorCode = ZIP_STREAM_OK;
    _lastErrorString = "";

    memset(&_pEndCentralDirRecord, 0, sizeof(zip_end_central_dir_record_t));

    _jsonError = deserializeJson(_jsonDoc, *fileList);
    if (_jsonError != DeserializationError::Ok) {
        _lastErrorCode = ZIP_STREAM_FILE_LIST_FAIL;
        _lastErrorString = String("JSON parse error: ") + _jsonError.c_str();
    }
    if(!_jsonDoc.is<JsonArray>() ) { // Only JsonArray is valid
        _lastErrorCode = ZIP_STREAM_FILE_LIST_FAIL;
        _lastErrorString = String("JSON is not an array");
    }        
    _jFileArray = _jsonDoc.as<JsonArray>();
    if (!_jFileArray.size()) {
        _lastErrorCode = ZIP_STREAM_FILE_LIST_FAIL;
        _lastErrorString = String("No files in list");
    }

    if (_lastErrorCode != ZIP_STREAM_OK) {
        _lastErrorString = String("File list error");
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return _lastErrorCode;
    }

//    ESP_LOGI(TAG_FILE, F("File list size : %d | %s"), _jFileArray.size(), fileObj["name"].as<String>().c_str());

    _pBuffer = (uint8_t *)malloc(ZIP_BUFFER_SIZE);

    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
        return _lastErrorCode;
    }

    if (!_pFileDescriptorBuffer) {
        _pFileDescriptorBuffer = (zip_file_discriptor_t *)malloc(sizeof(zip_file_discriptor_t) * _jFileArray.size());
        if (!_pFileDescriptorBuffer) {
            _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
            _lastErrorString = String(zip_memory_error);
//            ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
            return _lastErrorCode;
        }
    }

    _streamState = ZIP_STREAM_LOCAL_HEADER;
    buildBufferLocalFileHeader();

    return ZIP_STREAM_OK;
}

/**************************************************************************//**
 * @brief Unzips a stream of data. 
 * 
 * @param zipStream - The stream to unzip e.g. a file or a web stream
 * @return size_t 
 **************************************************************************  */
size_t ZipStream::write(Stream &zipStream) 
{
    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
        return _lastErrorCode;
    }

    if (_streamState <= ZIP_STREAM_INACTIVE) {
        _lastErrorCode = ZIP_STREAM_MODE_WRONG;
        _lastErrorString = String(zip_initialisation_error);
        return 0;
    }

    if(!dynamic_cast<Stream*>(&zipStream)) {
        _lastErrorCode = ZIP_STREAM_NOT_VALID;
        _lastErrorString = String(zip_invalid_stream_error);
        return 0;
    }

    uint8_t * _pInputBuffer = (uint8_t *)malloc(ZIP_STREAM_BUFFER_SIZE);

    if (!_pInputBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
        return 1;
    }

    size_t toRead = 0;    
    size_t bytesRead = 0;
    size_t totalWritten = 0;

    while ( toRead = zipStream.available() ) {
        if (toRead > ZIP_STREAM_BUFFER_SIZE) { toRead = ZIP_STREAM_BUFFER_SIZE; }
        bytesRead = zipStream.readBytes(_pInputBuffer, toRead);
        totalWritten += write(_pInputBuffer, bytesRead);
        if (_lastErrorCode < ZIP_STREAM_OK) { // On error stop, on warning continue
//            ESP_LOGE(TAG_ZIP, "Unzip error %d", _lastErrorCode);
            break;
        }
    }

    if (_pInputBuffer) {
        free(_pInputBuffer);
        _pInputBuffer = NULL;
    }

    return totalWritten;
}

/**************************************************************************//**
 * @brief Unzips a chunk of data. 
 * 
 * @param pInData - Pointer to incoming data of chunk
 * @param size - Size of the chunk
 * @return size_t 
 **************************************************************************  */
size_t ZipStream::write(const uint8_t *pInData, size_t size) 
{
    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
        return _lastErrorCode;
    }

    if (_streamState <= ZIP_STREAM_INACTIVE) {
        _lastErrorCode = ZIP_STREAM_MODE_WRONG;
        _lastErrorString = String(zip_initialisation_error);
        return 0;
    }

    size_t getBytes = 0;
    size_t inPosition = 0;

    while (size > inPosition) {
        switch (_streamState) {
            case ZIP_STREAM_EXPECT_SIGNATURE : {
                getBytes = sizeof(_signature) - _position;
                if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                memcpy(_pBuffer + _position, (uint8_t *)pInData + inPosition, getBytes);
                _position += getBytes;
                inPosition += getBytes;
                if(_position < sizeof(_signature)) { break; }

                _signature = *(uint32_t *)_pBuffer;
                _streamState = ZIP_STREAM_EXPECT_HEADER;
                _position = 0;
                break; 
            }

            case ZIP_STREAM_EXPECT_HEADER : {
                switch(_signature) {
                    case ZIP_LFH_SIGNATURE: {  // Local file header
                        getBytes = sizeof(zip_local_file_header_t) - _position;
                        if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                        memcpy(_pBuffer + _position, (uint8_t *)pInData + inPosition, getBytes);
                        _position += getBytes;
                        inPosition += getBytes;
                        if (_position < sizeof(zip_local_file_header_t)) { break; }

                        _zipLocalHeader = *(zip_local_file_header_t *)_pBuffer;
                        _streamState = ZIP_STREAM_GET_FILENAME;

                        if (_zipLocalHeader.compression_method != ZIP_NO_COMPRESSION) {
//                            ESP_LOGW(TAG_ZIP, "Compression is not supported %d", _zipLocalHeader.compression_method);
                            _lastErrorCode = ZIP_STREAM_COMPRESSION_ERROR;
                            _lastErrorString = String(zip_compreession_unsupport);
                            _streamState = ZIP_STREAM_SKIP_DATA;
                        }

                        if (_zipLocalHeader.filename_length >= 255) {
//                            ESP_LOGW(TAG_ZIP, "Filename too long %d", _zipLocalHeader.filename_length);
                            _lastErrorCode = ZIP_STREAM_FILENAME_LENGTH;
                            _lastErrorString = String(zip_filename_length_error);
                            _streamState = ZIP_STREAM_SKIP_DATA;
                        }

                        if (_streamState == ZIP_STREAM_SKIP_DATA) {
                            _datalen = _zipLocalHeader.filename_length + _zipLocalHeader.extra_length + _zipLocalHeader.compressed_size;
                        }

                        _position = 0;
                        break;
                    }

                    case ZIP_CDFH_SIGNATURE: {  // Central directory file header
                        getBytes = sizeof(zip_central_dir_file_header_t) - _position;
                        if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                        memcpy(_pBuffer + _position, (uint8_t *)pInData + inPosition, getBytes);
                        _position += getBytes;
                        inPosition += getBytes;

                        if(_position < sizeof(zip_central_dir_file_header_t)) { break; }

                        zip_central_dir_file_header_t zipCentralDirHeader = *(zip_central_dir_file_header_t *)_pBuffer;

                        _streamState = ZIP_STREAM_SKIP_DATA;
                        _datalen = zipCentralDirHeader.filename_length + zipCentralDirHeader.extra_length + zipCentralDirHeader.comment_length;
                        _position = 0;
                        break;
                    }

                    case ZIP_EOCD_SIGNATURE: {  // End of central directory record
                        getBytes = sizeof(zip_end_central_dir_record_t) - _position;
                        if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                        memcpy(_pBuffer + _position, (uint8_t *)pInData + inPosition, getBytes);
                        _position += getBytes;
                        inPosition += getBytes;

                        if(_position < sizeof(zip_end_central_dir_record_t)) { break; }

                        zip_end_central_dir_record_t zipEndCentralDirHeader = *(zip_end_central_dir_record_t *)_pBuffer;

                        _streamState = ZIP_STREAM_INACTIVE;
                        _datalen = zipEndCentralDirHeader.comment_length;
                        _position = 0;

                        return inPosition;
                    }

                    case ZIP_DD_SIGNATURE: {  // Data descriptor
                        getBytes = sizeof(zip_data_discriptor_t) - _position;
                        if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                        memcpy(_pBuffer + _position, (uint8_t *)pInData + inPosition, getBytes);
                        _position += getBytes;
                        inPosition += getBytes;

                        if(_position < sizeof(zip_data_discriptor_t)) { break; }

                        _streamState = ZIP_STREAM_EXPECT_SIGNATURE;
                        _position = 0;
                        break;
                    }

                    default: {
//                        ESP_LOGE(TAG_ZIP, F("Invalid Data"));
                        _lastErrorCode = ZIP_STREAM_DATA_INVALID;
                        _lastErrorString = String(zip_file_invalid_data_error);
                        return inPosition;
                    }
                }
                break;
            }

            case ZIP_STREAM_GET_FILENAME : {
                getBytes = _zipLocalHeader.filename_length - _position;
                if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                memcpy(_pBuffer + _position, (uint8_t *)pInData + inPosition, getBytes);
                _position += getBytes;
                inPosition += getBytes;

                if(_position != _zipLocalHeader.filename_length) { break; }

                *(_pBuffer + _position) = 0;     // add string-terminator

                String filename((char *)_pBuffer);
                if (!filename.startsWith("/")) {
                    filename = "/" + filename;
                }

                if (HASP_FS.exists(filename)) {
//                    ESP_LOGI(TAG_ZIP, "Overwrite existing file %s", filename.c_str());
                    HASP_FS.remove(filename);
                }

                _File = HASP_FS.open(filename, "w");
                if (!_File) {
//                    ESP_LOGE(TAG_ZIP, "File cannot created %s", filename.c_str());
                    filename = String("");
                    _streamState = ZIP_STREAM_SKIP_DATA;
                    _datalen = _zipLocalHeader.extra_length + _zipLocalHeader.compressed_size;
                    _position = 0;
                    _lastErrorCode = ZIP_STREAM_FILE_OPEN_FAILED;
                    _lastErrorString = String(zip_file_open_failed_error);
                    break;
                }

                if (_zipLocalHeader.extra_length > 0) {
                    _streamState = ZIP_STREAM_GET_EXTRA_FIELD;
                } else {
                    _streamState = ZIP_STREAM_STORE_DATA;
                }

//                ESP_LOGI(TAG_ZIP, "Writing file %s", filename.c_str());
                filename = String();
                _position = 0;
                _fileCRC32 = 0;
                break;
            }

            case ZIP_STREAM_GET_EXTRA_FIELD : {
                getBytes = _zipLocalHeader.extra_length - _position;
                if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                _position += getBytes;
                inPosition += getBytes;

                if (_position != _zipLocalHeader.extra_length) { break; }

                _streamState = ZIP_STREAM_STORE_DATA;
                _position = 0;
                break;
            }

            case ZIP_STREAM_STORE_DATA : {
                if (!_File) {
                    _lastErrorCode = ZIP_STREAM_UNKNOWN_ERROR;
                    _lastErrorString = String(zip_file_open_failed_error);
                    break;
                }

                getBytes = _zipLocalHeader.compressed_size - _position;
                if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                _fileCRC32 = crc32_le(_fileCRC32, pInData + inPosition, getBytes);
                _File.write(pInData + inPosition, getBytes);
                _position += getBytes;
                inPosition += getBytes;

                // Check if the file is finished
                if (_position < _zipLocalHeader.compressed_size) { break; }

                _File.close();

                if(_fileCRC32 != _zipLocalHeader.crc) {
//                    ESP_LOGI(TAG_ZIP, "File CRC does not match %x <=> %x", _fileCRC32, _zipLocalHeader.crc);
//                    _lastErrorCode = ZIP_STREAM_CRC_ERROR;
//                    _lastErrorString = String(zip_file_crc_error);
                }

                _streamState = ZIP_STREAM_EXPECT_SIGNATURE;
                _position = 0;
            }

            case ZIP_STREAM_SKIP_DATA : {
                getBytes = _datalen - _position;
                if (getBytes > (size - inPosition)) { getBytes = (size - inPosition); }
                _position += getBytes;
                inPosition += getBytes;

                if (_position != _datalen) { break; }

                _streamState = ZIP_STREAM_EXPECT_SIGNATURE;
                _position = 0;
                break;
            }

            default : {
//                ESP_LOGE(TAG_ZIP, F("Invalid Data"));
                _lastErrorCode = ZIP_STREAM_DATA_INVALID;
                _lastErrorString = String(zip_file_invalid_data_error);
                return inPosition;
            }
        }
    }

    return inPosition; 
}

/**************************************************************************//**
 * @brief Fill buffer with local file header
 * 
 * @return zip_stream_error_t 
 **************************************************************************  */
void ZipStream::buildBufferLocalFileHeader() {
    _pFileDescriptorBuffer[_fileIdx].is_valid = false;
    _position = 0;

/* 
    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
//        ESP_LOGE(TAG_ZIP, "%s", _lastErrorString.c_str());
        return;
    }
*/

    *(uint32_t*)_pBuffer = ZIP_LFH_SIGNATURE;
    _content_length = sizeof(uint32_t);

    zip_local_file_header_t *pZipLocalHeader = (zip_local_file_header_t *)(_pBuffer + _content_length);
    pZipLocalHeader->min_version = 0x000A; // Version 2.0
    pZipLocalHeader->flags = 0x0000; // No flags
    pZipLocalHeader->compression_method = ZIP_NO_COMPRESSION;
    pZipLocalHeader->time_modified = 0x0000; // No time
    pZipLocalHeader->date_modified = 0x0000; // No date
    pZipLocalHeader->crc = 0;
    pZipLocalHeader->extra_length = 0;
    _content_length += sizeof(zip_local_file_header_t);

    String filename = _jFileArray[_fileIdx].as<JsonObject>()["name"].as<String>();
    if (filename.length() > 255) {
        _lastErrorCode = ZIP_STREAM_FILENAME_LENGTH;
        _lastErrorString = String(zip_filename_length_error);
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return;
    }

    memcpy(_pBuffer + _content_length, filename.c_str(), filename.length());
    _content_length += filename.length();
    pZipLocalHeader->filename_length = filename.length();

    if (!filename.startsWith("/")) { filename = "/" + filename; }

    if (!HASP_FS.exists(filename)) {
        _lastErrorCode = ZIP_STREAM_FILE_NOT_FOUND;
        _lastErrorString = String(zip_file_not_found_error);
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return;
    } 

    if (_File) { _File.close(); }

    _File = HASP_FS.open(filename, "r");
    if (!_File) {
        _lastErrorCode = ZIP_STREAM_FILE_OPEN_FAILED;
        _lastErrorString = String(zip_file_open_failed_error) + " " + filename;
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return;
    }

    pZipLocalHeader->compressed_size = _File.size();
    pZipLocalHeader->uncompressed_size = _File.size();

    uint32_t crc32 = 0;
    size_t bytesRead = 0;
    uint8_t *pCrcBuffer = (uint8_t *)malloc(CRC_BUFFER_SIZE);

    if (!pCrcBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return;
    }

    while (_File.available()) {
        bytesRead = _File.readBytes((char *)pCrcBuffer, sizeof(pCrcBuffer));
        crc32 = crc32_le(crc32, (uint8_t const *)pCrcBuffer, bytesRead);
    }

    pZipLocalHeader->crc = crc32;

    _pFileDescriptorBuffer[_fileIdx].is_valid = true;
    _pFileDescriptorBuffer[_fileIdx].crc = pZipLocalHeader->crc;
    _pFileDescriptorBuffer[_fileIdx].compressed_size = pZipLocalHeader->compressed_size;
    _pFileDescriptorBuffer[_fileIdx].relative_offset = _pEndCentralDirRecord.offset_central_dir_start;

    _pEndCentralDirRecord.offset_central_dir_start += _content_length + pZipLocalHeader->compressed_size;

    return;
}

/**************************************************************************//**
 * @brief Fill buffer with central directory file header
 * 
 * @return zip_stream_error_t 
 **************************************************************************  */
void ZipStream::buildBufferCentralDirFileHeader() {
    if (!_pFileDescriptorBuffer[_fileIdx].is_valid) { 
        _lastErrorCode = ZIP_STREAM_FILE_INVALID;
        _lastErrorString = String(zip_file_invalid_data_error);
        return; 
    }

    _position = 0;

/* 
    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
//        ESP_LOGE(TAG_ZIP, "%s", _lastErrorString.c_str());
        return;
    }
*/

    *(uint32_t*)_pBuffer = ZIP_CDFH_SIGNATURE;
    _content_length = sizeof(uint32_t);

    zip_central_dir_file_header_t *pHeader = (zip_central_dir_file_header_t *)(_pBuffer + _content_length);
    pHeader->version_made_by = 0x030A; // Unix, Zip Version 2.0
    pHeader->version_min = 0x000A; // Version 2.0
    pHeader->flags = 0x0000; // No flags
    pHeader->compression_method = ZIP_NO_COMPRESSION;
    pHeader->time_modified = 0x0000; // No time
    pHeader->date_modified = 0x0000; // No date
    pHeader->crc = _pFileDescriptorBuffer[_fileIdx].crc;
    pHeader->compressed_size = _pFileDescriptorBuffer[_fileIdx].compressed_size;
    pHeader->uncompressed_size = _pFileDescriptorBuffer[_fileIdx].compressed_size;
    pHeader->filename_length = 0;
    pHeader->extra_length = 0;
    pHeader->comment_length = 0;
    pHeader->disk_no = 0;
    pHeader->internal_file_attr = 0;
    pHeader->external_file_attr = 0;
    pHeader->relative_offset = _pFileDescriptorBuffer[_fileIdx].relative_offset;

    _content_length += sizeof(zip_central_dir_file_header_t);

    String filename = _jFileArray[_fileIdx].as<JsonObject>()["name"].as<String>();
    if (filename.length() > 255) {
        _lastErrorCode = ZIP_STREAM_FILENAME_LENGTH;
        _lastErrorString = String(zip_filename_length_error) + " " + filename;
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return;
    }

    memcpy(_pBuffer + _content_length, filename.c_str(), filename.length());
    _content_length += filename.length();
    pHeader->filename_length = filename.length();

    _pEndCentralDirRecord.central_dir_records++;
    _pEndCentralDirRecord.total_dir_records++;
    _pEndCentralDirRecord.central_dir_size += _content_length;

    return;
}

/**************************************************************************//**
 * @brief Fill buffer with end of central directory record
 * 
 * @return zip_stream_error_t 
 **************************************************************************  */
void ZipStream::buildBufferEndCentralFileHeader() {
    _position = 0;

/*
    if (!_pBuffer) {
        _lastErrorCode = ZIP_STREAM_ERROR_NO_MEMORY;
        _lastErrorString = String(zip_memory_error);
//        ESP_LOGE(TAG_ZIP, F(_lastErrorString.c_str()));
        return;
    }
 */

    *(uint32_t*)_pBuffer = ZIP_EOCD_SIGNATURE;
    _content_length = sizeof(uint32_t);

    memcpy(_pBuffer + _content_length, &_pEndCentralDirRecord, sizeof(zip_end_central_dir_record_t));

    _content_length += sizeof(zip_end_central_dir_record_t);

    return;
}

/**************************************************************************//**
 * @brief read data from files in the file list, unpack them and put them 
 *      into the buffer.
 * 
 * @param buffer 
 * @param length 
 * @return size_t 
 **************************************************************************  */
size_t ZipStream::readBytes(uint8_t *buffer, size_t length) {
    if (!_pBuffer || _streamState >= ZIP_STREAM_INACTIVE) {
        _lastErrorCode = ZIP_STREAM_MODE_WRONG;
        _lastErrorString = String(zip_initialisation_error);
        return 0;
    }

    switch (_streamState) {
        case ZIP_STREAM_LOCAL_HEADER: {
            if (length > _content_length - _position) { length = _content_length - _position; }

            if (length) { memcpy(buffer, _pBuffer + _position, length); }
            _position += length;

            if (_content_length - _position == 0) {
                // Local file header is complete, continue with file data
                _streamState = ZIP_STREAM_FILE_DATA;
                _content_length = _File.size();
                _position = 0;
                if (!_File) {
                    _lastErrorCode = ZIP_STREAM_FILE_OPEN_FAILED;
                    _lastErrorString = String(zip_file_open_failed_error);
//                    ESP_LOGE(TAG_ZIP, F(_lastErrorString));
                    return 0;
                }

                _File.seek(0, SeekSet);
            }
            break;
        }

        case ZIP_STREAM_FILE_DATA: {
            length = _File.readBytes((char *)buffer, length);

            if(!_File.available()) {
                _File.close();

                if (++_fileIdx < _jFileArray.size()) {
                    // continue with local file header of next file
                    _streamState = ZIP_STREAM_LOCAL_HEADER;
                    buildBufferLocalFileHeader();
                } else {
                    // All files processed, continue with central directory
                    _fileIdx = 0;
                    _streamState = ZIP_STREAM_CENTRAL_DIR_HEADER;
                    buildBufferCentralDirFileHeader();
                }
            }
            break;
        }

        case ZIP_STREAM_CENTRAL_DIR_HEADER: {
            if (length > _content_length - _position) { length = _content_length - _position; }

            if (length) { memcpy(buffer, _pBuffer + _position, length); }
            _position += length;

            if (_content_length - _position == 0) {
                if (++_fileIdx < _jFileArray.size()) {
                    // continue with central dir header of next file
                    _streamState = ZIP_STREAM_CENTRAL_DIR_HEADER;
                    buildBufferCentralDirFileHeader();
                } else {
                    _streamState = ZIP_STREAM_END_OF_CENTRAL_DIR;
                    buildBufferEndCentralFileHeader();
                }
            }
            break;
        }

        case ZIP_STREAM_END_OF_CENTRAL_DIR: {
            if (length > _content_length - _position) { length = _content_length - _position; }

            if (length) { memcpy(buffer, _pBuffer + _position, length); }
            _position += length;

            if (_content_length - _position == 0) {
                // All files processed, end of central directory
                flush();
            }
            break;
        }

        default:
            _lastErrorCode = ZIP_STREAM_UNKNOWN_ERROR;
            _lastErrorString = String(zip_unknown_error);
            return 0;
    }
    return length;
}

/**************************************************************************//**
 * @brief 
 * 
 **************************************************************************  */
void ZipStream::flush() {
    _streamState = ZIP_STREAM_INACTIVE;
    _content_length = 0;
    _position = 0;
    _fileIdx = 0;

    if (_File) {
        _File.close();
//        _File = fs::File();
    }

    if (_pBuffer) {
        free(_pBuffer);
        _pBuffer = NULL;
    }

    if (_pFileDescriptorBuffer) {
        free(_pFileDescriptorBuffer);
        _pFileDescriptorBuffer = NULL;
    }
}

#endif // HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0