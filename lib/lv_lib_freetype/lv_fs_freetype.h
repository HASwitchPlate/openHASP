/**
 * @file lv_fs_freetype.h
 *
 */
#ifndef LV_FS_FREETYPE_H
#define LV_FS_FREETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_fs_if.h"

/*********************
 *      DEFINES
 *********************/
#if LV_USE_FILESYSTEM

/**********************
 *      TYPEDEFS
 **********************/
typedef void lv_ft_stream_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * fopen glue for freetype library to access lv_fs api
 * @param filename The full path to the file including lv_fs drive letter
 * @param mode "r" for read, "w" fro write.
 * @return A file stream pointer on success, NULL on error.
 */
lv_ft_stream_t* lv_ft_fopen(const char* filename, const char* mode);

/**
 * fclose glue for freetype library to access lv_fs api
 * @param stream The file stream to close.
 * @return Always 0.
 */
int lv_ft_fclose(lv_ft_stream_t* stream);

/**
 * fread glue for freetype library to access lv_fs api
 * @param ptr The buffer read to.
 * @param size Size of the read buffer.
 * @param count The number of bytes to read.
 * @param stream The file stream to read.
 * @return The number of bytes read on success, otherwise 0.
 */
size_t lv_ft_fread(void* ptr, size_t size, size_t count, lv_ft_stream_t* stream);

/**
 * fseek glue for freetype library to access lv_fs api
 * @param stream The file stream to seek.
 * @param offset The offset from the origin position.
 * @param origin The start position within the file, either SEEK_SET, SEEK_CUR or SEEK_END.
 * @return 1 on success, otherwise 0.
 */
int lv_ft_fseek(lv_ft_stream_t* stream, long int offset, int origin);

/**
 * ftell glue for freetype library to access lv_fs api
 * @param stream The file stream to tell.
 * @return The current file cursor position on success, otherwise -1.
 */
int lv_ft_ftell(lv_ft_stream_t* stream);

#endif // LV_USE_FILESYSTEM

#ifdef __cplusplus
}
#endif

#endif // LV_FS_FREETYPE_H
