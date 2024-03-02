/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "lv_fs_if.h"

#include "hasp_conf.h" // include first
#include "hasp_debug.h"

void filesystem_list_path(const char* path)
{
    lv_fs_dir_t dir;
    lv_fs_res_t res;
    res = lv_fs_dir_open(&dir, path);
    if(res != LV_FS_RES_OK) {
        LOG_ERROR(TAG_LVFS, "Error opening directory %s", path);
    } else {
        char fn[256];
        while(1) {
            res = lv_fs_dir_read(&dir, fn);
            if(res != LV_FS_RES_OK) {
                LOG_ERROR(TAG_LVFS, "Directory %s can not be read", path);
                break;
            }

            /*fn is empty, if not more files to read*/
            if(strlen(fn) == 0) {
                LOG_WARNING(TAG_LVFS, "Directory %s listing complete", path);
                break;
            }

            LOG_VERBOSE(TAG_LVFS, D_BULLET "%s", fn);
        }
    }

    lv_fs_dir_close(&dir);
}
