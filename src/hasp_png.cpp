
/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef LODEPNG_NO_COMPILE_ALLOCATORS

#include <stdlib.h>
#include "hasplib.h"
#include "hasp_png.h"

void* lodepng_malloc(size_t size)
{
#ifdef LODEPNG_MAX_ALLOC
    if(size > LODEPNG_MAX_ALLOC) return 0;
#endif

#ifdef ESP32
    return psramFound() ? ps_malloc(size) : malloc(size);
#else
    return malloc(size);
#endif
}

/* NOTE: when realloc returns NULL, it leaves the original memory untouched */
void* lodepng_realloc(void* ptr, size_t new_size)
{
#ifdef LODEPNG_MAX_ALLOC
    if(new_size > LODEPNG_MAX_ALLOC) return 0;
#endif

#ifdef ESP32
    return psramFound() ? ps_realloc(ptr, new_size) : realloc(ptr, new_size);
#else
    return realloc(ptr, new_size);
#endif
}

void lodepng_free(void* ptr)
{
    free(ptr);
}

#endif // LODEPNG_NO_COMPILE_ALLOCATORS