
/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <stdlib.h>
#include "hasplib.h"
#include "hasp_mem.h"

#ifdef ESP32
bool hasp_use_psram()
{
    return psramFound() && ESP.getPsramSize() > 0;
}
#endif

void* hasp_calloc(size_t num, size_t size)
{
#ifdef ESP32
    return hasp_use_psram() ? ps_calloc(num, size) : calloc(num, size);
#else
    return calloc(num, size);
#endif
}

void* hasp_malloc(size_t size)
{
#ifdef ESP32
    return hasp_use_psram() ? ps_malloc(size) : malloc(size);
#else
    return malloc(size);
#endif
}

/* NOTE: when realloc returns NULL, it leaves the original memory untouched */
void* hasp_realloc(void* ptr, size_t new_size)
{
#ifdef ESP32
    return hasp_use_psram() ? ps_realloc(ptr, new_size) : realloc(ptr, new_size);
#else
    return realloc(ptr, new_size);
#endif
}

void hasp_free(void* ptr)
{
    free(ptr);
}

#ifdef LODEPNG_NO_COMPILE_ALLOCATORS
void* lodepng_malloc(size_t size)
{
#ifdef LODEPNG_MAX_ALLOC
    if(size > LODEPNG_MAX_ALLOC) return 0;
#endif

    // void* ptr = hasp_malloc(size);
    // if(ptr) return ptr;

    // /* PSram was full retry after clearing cache*/
    // lv_img_cache_invalidate_src(NULL);
    return hasp_malloc(size);
}

/* NOTE: when realloc returns NULL, it leaves the original memory untouched */
void* lodepng_realloc(void* ptr, size_t new_size)
{
#ifdef LODEPNG_MAX_ALLOC
    if(new_size > LODEPNG_MAX_ALLOC) return 0;
#endif

    return hasp_realloc(ptr, new_size);
}

void lodepng_free(void* ptr)
{
    hasp_free(ptr);
}
#endif // LODEPNG_NO_COMPILE_ALLOCATORS