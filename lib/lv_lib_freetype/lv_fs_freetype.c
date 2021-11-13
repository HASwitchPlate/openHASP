/**
 * @file lv_fs_freetype.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include "lv_fs_freetype.h"

/*********************
 *      DEFINES
 *********************/
#if LV_USE_FS_IF
#include "lv_fs_if.h"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

// FILE * fopen ( const char * filename, const char * mode );
lv_ft_stream_t* lv_ft_fopen(const char* filename, const char* mode)
{
    lv_fs_file_t* file_p = malloc(sizeof(lv_fs_file_t)); // reserve memory

    if(file_p) {
        lv_fs_mode_t rw = mode[0] == 'r' ? LV_FS_MODE_RD : LV_FS_MODE_WR;
        lv_fs_res_t res = lv_fs_open(file_p, filename, rw);

        if(res == LV_FS_RES_OK) { // success
            return (lv_ft_stream_t*)file_p;
        } else {          // error
            free(file_p); // release memory
        }
    }

    return NULL;
}

// int fclose ( FILE * stream );
int lv_ft_fclose(lv_ft_stream_t* stream)
{
    lv_fs_file_t* f_ptr = (lv_fs_file_t*)stream;

    lv_fs_close(f_ptr);
    free(f_ptr); // release memory

    return 0;
}

// size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
size_t lv_ft_fread(void* ptr, size_t size, size_t count, lv_ft_stream_t* stream)
{
    lv_fs_file_t* f_ptr = (lv_fs_file_t*)stream;
    uint32_t bytes_read;

    lv_fs_res_t res = lv_fs_read(f_ptr, ptr, size * count, &bytes_read);
    if(res != LV_FS_RES_OK) { // error
        bytes_read = 0;
    }

    return bytes_read;
}

// long int ftell ( FILE * stream );
int lv_ft_ftell(lv_ft_stream_t* stream)
{
    lv_fs_file_t* f_ptr = (lv_fs_file_t*)stream;
    uint32_t pos;

    lv_fs_res_t res = lv_fs_tell(f_ptr, &pos);
    if(res == LV_FS_RES_OK) return pos;

    return -1;
}

// int fseek ( FILE * stream, long int offset, int origin );
int lv_ft_fseek(lv_ft_stream_t* stream, long int offset, int origin)
{
    lv_fs_file_t* f_ptr = (lv_fs_file_t*)stream;
    uint32_t start      = 0;

    switch(origin) {
        case SEEK_SET:
            start = 0;
            break;

        case SEEK_CUR: {
            int pos = lv_ft_ftell(f_ptr);
            if(pos < 0) { // error
                return -1;
            }
            start = pos;
        } break;

        case SEEK_END: {
            lv_fs_res_t res = lv_fs_size(f_ptr, &start);
            if(res != LV_FS_RES_OK) { // error
                return -1;
            }
            break;
        }

        default:
            return -1; // Unknown origin
    }

    if(start + offset < 0) { // underflow, go to beginning of the file
        start  = 0;
        offset = 0;
    }

    lv_fs_res_t res = lv_fs_seek(f_ptr, start + offset);
    return res == LV_FS_RES_OK ? 0 : -1;
}

#endif // LV_USE_FS_IF