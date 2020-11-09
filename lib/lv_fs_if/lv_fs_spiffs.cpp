/**
 * @file lv_fs_spiffs.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <Arduino.h>
#include "lv_fs_if.h"
#include "lv_fs_spiffs.h"
#include "ArduinoLog.h"

#if LV_USE_FS_IF
#if LV_FS_IF_SPIFFS != '\0'

#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#endif
#include "FS.h" // Include the SPIFFS library

#define TAG_LVFS 91

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/* Create a type to store the required data about your file.*/
typedef File lv_spiffs_file_t;

/*Similarly to `file_t` create a type for directory reading too */
#if defined(ARDUINO_ARCH_ESP32)
typedef File lv_spiffs_dir_t;
#elif defined(ARDUINO_ARCH_ESP8266)
typedef Dir lv_spiffs_dir_t;
#define FILE_READ "r"
#define FILE_WRITE "r+"
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static lv_fs_res_t fs_open(lv_fs_drv_t * drv, void * file_p, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos);
static lv_fs_res_t fs_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
static lv_fs_res_t fs_remove(lv_fs_drv_t * drv, const char * path);
static lv_fs_res_t fs_trunc(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_rename(lv_fs_drv_t * drv, const char * oldname, const char * newname);
static lv_fs_res_t fs_free(lv_fs_drv_t * drv, uint32_t * total_p, uint32_t * free_p);
static lv_fs_res_t fs_dir_open(lv_fs_drv_t * drv, void * dir_p, const char * path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * dir_p, char * fn);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_fs_if_spiffs_init(void)
{
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface  in LittlevGL
     *--------------------------------------------------*/

    /* Add a simple drive to open images */
    lv_fs_drv_t fs_drv; /*A driver descriptor*/
    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.file_size     = sizeof(lv_spiffs_file_t);
    fs_drv.letter        = LV_FS_IF_SPIFFS;
    fs_drv.open_cb       = fs_open;
    fs_drv.close_cb      = fs_close;
    fs_drv.read_cb       = fs_read;
    fs_drv.write_cb      = fs_write;
    fs_drv.seek_cb       = fs_seek;
    fs_drv.tell_cb       = fs_tell;
    fs_drv.free_space_cb = fs_free;
    fs_drv.size_cb       = fs_size;
    fs_drv.remove_cb     = fs_remove;
    fs_drv.rename_cb     = fs_rename;
    fs_drv.trunc_cb      = fs_trunc;

    fs_drv.rddir_size   = sizeof(lv_spiffs_dir_t);
    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb  = fs_dir_open;
    fs_drv.dir_read_cb  = fs_dir_read;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your Storage device and File system. */
static void fs_init(void)
{
    SPIFFS.begin();
}

/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_open(lv_fs_drv_t * drv, void * file_p, const char * path, lv_fs_mode_t mode)
{
    (void)drv; /*Unused*/

    char filename[32];
    snprintf_P(filename, sizeof(filename), PSTR("/%s"), path);

    Log.verbose(TAG_LVFS, F("Opening %s"), filename);
    File file = SPIFFS.open(filename, mode == LV_FS_MODE_WR ? FILE_WRITE : FILE_READ);

    Log.verbose(TAG_LVFS, F("%d"), __LINE__);
    if(!file) {
        return LV_FS_RES_NOT_EX;

    } else if(file.isDirectory()) {
        return LV_FS_RES_UNKNOWN;

    } else {
        // f.seek(0, SeekSet);
        // Log.verbose(TAG_LVFS,F("Assigning %s"), f.name());
        Log.verbose(TAG_LVFS, F("%d"), __LINE__);
        lv_spiffs_file_t * fp = (lv_spiffs_file_t *)file_p; /*Just avoid the confusing casings*/
        // Log.verbose(TAG_LVFS,F("Copying %s"), f.name());
        Log.verbose(TAG_LVFS, F("%d - %x - %d"), __LINE__, fp, sizeof(lv_spiffs_file_t));
        if(fp != NULL) (*fp) = file;
        // memcpy(fp,&file,sizeof(lv_spiffs_file_t));
        Log.verbose(TAG_LVFS, F("%d"), __LINE__);
        return LV_FS_RES_OK;
    }
}

/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p)
{
    (void)drv; /*Unused*/
    lv_spiffs_file_t file = *(lv_spiffs_file_t *)file_p;
    // File file           = fp;
    // file = SPIFFS.open("/background.bin");

    if(!file) {
        // Log.verbose(TAG_LVFS,F("Invalid file"));
        return LV_FS_RES_NOT_EX;

    } else if(file.isDirectory()) {
        return LV_FS_RES_UNKNOWN;

    } else {
        file.close();
        return LV_FS_RES_OK;
    }
}

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    (void)drv; /*Unused*/
    lv_spiffs_file_t * fp = (lv_spiffs_file_t *)file_p;
    lv_spiffs_file_t file = *fp;

    if(!file) {
        // Log.verbose(TAG_LVFS,F("Invalid file"));
        return LV_FS_RES_NOT_EX;

    } else {
        // Log.verbose(TAG_LVFS,F("Reading %u bytes from %s at position %u"), btr, file.name(), file.position());
        *br = (uint32_t)file.readBytes((char *)buf, btr);
        Serial.print("!");
        return LV_FS_RES_OK;
    }
}

/**
 * Write into a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param buf pointer to a buffer with the bytes to write
 * @param btr Bytes To Write
 * @param br the number of real written bytes (Bytes Written). NULL if unused.
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    (void)drv; /*Unused*/
    lv_spiffs_file_t file = *(lv_spiffs_file_t *)file_p;
    // File file           = fp;

    if(!file) {
        // Log.verbose(TAG_LVFS,F("Invalid file"));
        return LV_FS_RES_NOT_EX;

    } else {
        *bw = (uint32_t)file.write((byte *)buf, btw);
        return LV_FS_RES_OK;
    }
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos)
{
    (void)drv; /*Unused*/
    lv_spiffs_file_t file = *(lv_spiffs_file_t *)file_p;
    // File file           = fp;

    if(!file) {
        // Log.verbose(TAG_LVFS,F("Invalid file"));
        return LV_FS_RES_NOT_EX;

    } else {
        file.seek(pos, SeekSet);
        return LV_FS_RES_OK;
    }
}

/**
 * Give the size of a file bytes
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param size pointer to a variable to store the size
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p)
{
    (void)drv; /*Unused*/
    lv_spiffs_file_t file = *(lv_spiffs_file_t *)file_p;
    // File file           = fp;

    if(!file) {
        // Log.verbose(TAG_LVFS,F("Invalid file"));
        return LV_FS_RES_NOT_EX;

    } else {
        *size_p = (uint32_t)file.size();
        return LV_FS_RES_OK;
    }
}

/**
 * Give the position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param pos_p pointer to to store the result
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    (void)drv; /*Unused*/
    lv_spiffs_file_t file = *(lv_spiffs_file_t *)file_p;
    // File file           = fp;

    if(!file) {
        // Log.verbose(TAG_LVFS,F("Invalid file"));
        return LV_FS_RES_NOT_EX;

    } else {
        *pos_p = (uint32_t)file.position();
        return LV_FS_RES_OK;
    }
}

/**
 * Delete a file
 * @param drv pointer to a driver where this function belongs
 * @param path path of the file to delete
 * @return  LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_remove(lv_fs_drv_t * drv, const char * path)
{
    (void)drv; /*Unused*/

    char filename[32];
    snprintf_P(filename, sizeof(filename), PSTR("/%s"), path);

    if(!SPIFFS.exists(filename)) {
        return LV_FS_RES_NOT_EX;

    } else if(SPIFFS.remove(filename)) {
        return LV_FS_RES_OK;

    } else {
        return LV_FS_RES_UNKNOWN;
    }
}

/**
 * Truncate the file size to the current position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to an 'ufs_file_t' variable. (opened with lv_fs_open )
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_trunc(lv_fs_drv_t * drv, void * file_p)
{
    return LV_FS_RES_NOT_IMP;
}

/**
 * Rename a file
 * @param drv pointer to a driver where this function belongs
 * @param oldname path to the file
 * @param newname path with the new name
 * @return LV_FS_RES_OK or any error from 'fs_res_t'
 */
static lv_fs_res_t fs_rename(lv_fs_drv_t * drv, const char * oldname, const char * newname)
{
    (void)drv; /*Unused*/
    char fromname[32];
    char toname[32];

    snprintf_P(fromname, sizeof(fromname), PSTR("/%s"), oldname);
    snprintf_P(toname, sizeof(toname), PSTR("/%s"), newname);

    if(SPIFFS.rename(fromname, toname)) {
        return LV_FS_RES_OK;
    } else {
        return LV_FS_RES_UNKNOWN;
    }
}

/**
 * Get the free and total size of a driver in kB
 * @param drv pointer to a driver where this function belongs
 * @param letter the driver letter
 * @param total_p pointer to store the total size [kB]
 * @param free_p pointer to store the free size [kB]
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_free(lv_fs_drv_t * drv, uint32_t * total_p, uint32_t * free_p)
{
    (void)drv; /*Unused*/

#if defined(ARDUINO_ARCH_ESP8266)
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    *total_p = (uint32_t)fs_info.totalBytes;
    *free_p  = (uint32_t)fs_info.totalBytes - fs_info.usedBytes;
    return LV_FS_RES_OK;

#elif defined(ARDUINO_ARCH_ESP32)
    *total_p = (uint32_t)SPIFFS.totalBytes();
    *free_p  = (uint32_t)SPIFFS.totalBytes() - SPIFFS.usedBytes();
    return LV_FS_RES_OK;

#endif

    return LV_FS_RES_NOT_IMP;
}

/**
 * Initialize a 'fs_read_dir_t' variable for directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to a 'fs_read_dir_t' variable
 * @param path path to a directory
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_open(lv_fs_drv_t * drv, void * dir_p, const char * path)
{
    lv_spiffs_dir_t dir;

#if defined(ARDUINO_ARCH_ESP32)
    dir = SPIFFS.open(path);
    if(!dir) {
        return LV_FS_RES_UNKNOWN;
    }
#endif

#if defined(ARDUINO_ARCH_ESP8266)
    dir = SPIFFS.openDir(path);
#endif

    lv_spiffs_dir_t * dp = (lv_spiffs_dir_t *)dir_p; /*Just avoid the confusing casings*/
    *dp                  = dir;
    return LV_FS_RES_OK;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @param fn pointer to a buffer to store the filename
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * dir_p, char * fn)
{
    lv_spiffs_dir_t dir = *(lv_spiffs_dir_t *)dir_p; /*Convert type*/

#if defined(ARDUINO_ARCH_ESP32)
    File file = dir.openNextFile();
    if(file) {
        strcpy(fn, file.name());
        return LV_FS_RES_OK;
    } else {
        return LV_FS_RES_UNKNOWN;
    }
#endif

#if defined(ARDUINO_ARCH_ESP8266)
    if(dir.next()) {
        strcpy(fn, dir.fileName().c_str());
        return LV_FS_RES_OK;
    } else {
        return LV_FS_RES_UNKNOWN;
    }
#endif

    return LV_FS_RES_NOT_IMP;
}

/**
 * Close the directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p)
{
    return LV_FS_RES_OK;
}

#endif /*LV_USE_FS_IF*/
#endif /*LV_FS_IF_SPIFFS*/
