#ifndef STM32SPIFFS_H
#define STM32SPIFFS_H

#define FLASH_CHIP_SELECT_PIN PA15

// these are declared in params_test.h
// typedef signed int32_t s32_t;
// typedef unsigned int u32_t;
// typedef signed short s16_t;
// typedef unsigned short u16_t;
// typedef signed char s8_t;
// typedef unsigned char u8_t;

#include <memory>
#include <Arduino.h>
#include "SPIMemory.h"
#include "spiffs.h"

//#define FILE_READ "r"
//#define FILE_WRITE "w"
//#define FILE_APPEND "a"

#define FILE_READ (SPIFFS_CREAT | SPIFFS_RDONLY )
#define FILE_WRITE (SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_TRUNC)
#define FILE_APPEND (SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_APPEND)

namespace SpiffsLib
{

class File : public Stream
{
private:
    spiffs_file _file; // file handle
    spiffs *_fs;       // file system

public:
    File(spiffs_file f, spiffs *fs); // wraps an underlying spiffs_file
    File(void);                      // 'empty' constructor
    ~File();
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    int availableForWrite();
    int available();
    int read();
    int peek();
    void flush();
    int read(void *buf, uint16_t nbyte);
    bool seek(uint32_t pos);
    uint32_t position();
    uint32_t size();
    void close();
    operator bool();
    char *name();

    // bool isDirectory(void);
    // File openNextFile(uint8_t mode = FILE_READ);
    // void rewindDirectory(void);

    using Print::write;
};

class SpiffsClass
{

private:
    // my quick&dirty iterator, should be replaced
    spiffs_file getParentDir(const char *filepath, int *indx);

public:
    // These are required for initialisation and use of spiffs
    static SPIFlash *flash;
    static spiffs my_fs;
    spiffs_file root;

    // This needs to be called to set up the connection to the SD card
    // before other methods are used.
    bool begin(uint8_t csPin = FLASH_CHIP_SELECT_PIN);
    bool begin(uint32_t clock, uint8_t csPin);

    //call this when a card is removed. It will allow you to insert and initialise a new card.
    void end();

    // Open the specified file/directory with the supplied mode (e.g. read or
    // write, etc). Returns a File object for interacting with the file.
    // Note that currently only one file can be open at a time.
    File open(const char *filename, uint8_t mode = FILE_READ);
    File open(const String &filename, uint8_t mode = FILE_READ)
    {
        return open(filename.c_str(), mode);
    }

    // Methods to determine if the requested file path exists.
    bool exists(const char *filepath);
    bool exists(const String &filepath)
    {
        return exists(filepath.c_str());
    }

    // Create the requested directory heirarchy--if intermediate directories
    // do not exist they will be created.
    bool mkdir(const char *filepath);
    bool mkdir(const String &filepath)
    {
        return mkdir(filepath.c_str());
    }

    // Delete the file.
    bool remove(const char *filepath);
    bool remove(const String &filepath)
    {
        return remove(filepath.c_str());
    }

    bool rmdir(const char *filepath);
    bool rmdir(const String &filepath)
    {
        return rmdir(filepath.c_str());
    }

    bool rename(const char *oldPath, const char *newPath);

private:
    // This is used to determine the mode used to open a file
    // it's here because it's the easiest place to pass the
    // information through the directory walking function. But
    // it's probably not the best place for it.
    // It shouldn't be set directly--it is set via the parameters to `open`.
    int fileOpenMode;

    friend class File;
    friend bool callback_openPath(spiffs_file &, const char *, bool, void *);

    static u8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
    static u8_t spiffs_fds[32 * 4];
    static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

    int mount();
    static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst);
    static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src);
    static s32_t my_spiffs_erase(u32_t addr, u32_t size);
};

/*
class SPIFFS
{
public:
    static SPIFlash *flash;
    static spiffs my_fs;

    SPIFFS();
    int mount();
    bool begin(uint8_t cs);
    spiffs_file open(const char *path, const char *mode);

private:
    static u8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
    static u8_t spiffs_fds[32 * 4];
    static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

    static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst);
    static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src);
    static s32_t my_spiffs_erase(u32_t addr, u32_t size);
};*/

enum SeekMode
{
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
};

extern SpiffsClass SPIFFS;

}; // namespace SpiffsLib

typedef SpiffsLib::File File;
typedef SpiffsLib::SpiffsClass SpiffsFS;
static SpiffsFS SPIFFS;

#endif //FS_H