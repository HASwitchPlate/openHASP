/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_PNG_H
#define HASP_PNG_H

#include <stdlib.h>

#ifdef LODEPNG_NO_COMPILE_ALLOCATORS

#ifdef __cplusplus
extern "C" {
#endif

void* lodepng_calloc(size_t num,size_t size);
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // LODEPNG_NO_COMPILE_ALLOCATORS

#endif // HASP_PNG_H