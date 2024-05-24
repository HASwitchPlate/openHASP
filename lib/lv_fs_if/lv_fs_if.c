/**
 * @file lv_fs_if.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "lv_fs_if.h"

#if LV_USE_FS_IF

/*********************
 *      DEFINES
 *********************/
#ifndef LV_FS_PC_PATH
#ifndef WIN32
#define LV_FS_PC_PATH "./" /*Project root*/
#else
#define LV_FS_PC_PATH ".\\" /*Project root*/
#endif
#endif /*LV_FS_PATH*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if LV_FS_IF_FATFS != '\0'
void lv_fs_if_fatfs_init(void);
#endif

#if LV_FS_IF_PC != '\0' || LV_FS_IF_SD != '\0'
void lv_fs_if_pc_init(char letter, const char* path);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register driver(s) for the File system interface
 */
void lv_fs_if_init(void)
{
#if LV_FS_IF_FATFS != '\0'
    lv_fs_if_fatfs_init();
#endif

#if LV_FS_IF_PC != '\0'
    lv_fs_if_pc_init(LV_FS_IF_PC, LV_FS_PC_PATH);
#endif

#if LV_FS_IF_SD != '\0'
    lv_fs_if_pc_init(LV_FS_IF_SD, LV_FS_SD_PATH);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
