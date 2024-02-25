/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_MEM_H
#define HASP_MEM_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LODEPNG_NO_COMPILE_ALLOCATORS
void* lodepng_calloc(size_t num, size_t size);
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);
#endif // LODEPNG_NO_COMPILE_ALLOCATORS

bool hasp_use_psram();
void* hasp_calloc(size_t num, size_t size);
void* hasp_malloc(size_t size);
void* hasp_realloc(void* ptr, size_t new_size);
void hasp_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // HASP_MEM_H