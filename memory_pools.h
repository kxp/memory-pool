#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POINTER_OFFSET 8

#pragma pack(push, POINTER_OFFSET)
struct entry_s {
    uint32_t size;          // 4 bytes
    int8_t is_use;          // 1 byte    
    int8_t reserved[3];     // 3 bytes
    uintptr_t ptr;            // 8 bytes
};
#pragma pack(pop)

typedef  struct memory_pool_s {
    struct entry_s* storage_begin;
    struct entry_s* storage_end;
    struct entry_s* next_free_location;
    uint64_t alloc_calls;
    uint64_t free_calls;
    size_t cache_size;
    size_t padded_size;
    uint32_t entries;
    size_t block_size;
    size_t total_size;
} memory_pool;


memory_pool* init_pool(int32_t amount_entries, size_t ptr_size);

/// <summary>
/// Frees the  memory pool and all its contents.
/// </summary>
/// <param name="pool">The pool.</param>
/// <version author="André Cachopas" date="24-Sep-20"  machine="TV-MRROBOT"></version> 
void free_pool(memory_pool* pool);

// external agnostic functions 
#ifndef DISABLE_MEMORY_POOLING
uintptr_t _malloc_ref(void* pool, size_t size);
void _free_ref(void* pool, uintptr_t ptr);
#else
#include <stdlib.h>
#define _malloc_ref(p, d) malloc(d)
#define _free_ref(p, d) free(d)
#endif

//void* malloc_ref(void* pool, size_t size);
//void free_ref(void* pool, void* ptr);

#ifdef __cplusplus
}
#endif
