#include "memory_pools.h"
#include "cache_size.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1110 || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_ALIGNED(n,t,v)  t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_CONST(n,t,v)    const t __attribute__ ((aligned (n))) v
#elif defined(__DJGPP__)
#define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (FFMIN(n, 16)))) v
#define DECLARE_ASM_ALIGNED(n,t,v)  t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
#define DECLARE_ASM_CONST(n,t,v)    static const t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
#elif defined(__GNUC__) || defined(__clang__)
#define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_ALIGNED(n,t,v)  t av_used __attribute__ ((aligned (n))) v
#define DECLARE_ASM_CONST(n,t,v)    static const t av_used __attribute__ ((aligned (n))) v
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n,t,v)      __declspec(align(n)) t v
#define DECLARE_ASM_ALIGNED(n,t,v)  __declspec(align(n)) t v
#define DECLARE_ASM_CONST(n,t,v)    __declspec(align(n)) static const t v
#else
#define DECLARE_ALIGNED(n,t,v)      t v
#define DECLARE_ASM_ALIGNED(n,t,v)  t v
#define DECLARE_ASM_CONST(n,t,v)    static const t v
#endif


inline uintptr_t _malloc_ref_internal(memory_pool* pool, uint32_t size);
inline void _free_ref_internal(memory_pool* pool, uintptr_t ptr);


memory_pool* init_pool(uint32_t amount_entries, size_t ptr_size) {
    if (amount_entries == 0 || ptr_size == 0)
        return NULL;

    size_t cache_size = CacheLineSize();
    memory_pool* pool_s = malloc(sizeof(pool_s));

    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    unsigned int padding = ptr_size + sizeof(struct entry_s); // compute the next highest power of 2 of 32-bit v
    padding--;
    padding |= padding >> 1;
    padding |= padding >> 2;
    padding |= padding >> 4;
    padding |= padding >> 8;
    padding |= padding >> 16;
    padding++;
    pool_s->padded_size = padding;


    pool_s->block_size = ptr_size;
    pool_s->entries = amount_entries;
    pool_s->alloc_calls = 0;
    pool_s->free_calls = 0;
    pool_s->total_size = padding/*(ptr_size + sizeof(struct entry_s))*/ * amount_entries;
    pool_s->cache_size = cache_size;
    
    /// https://stackoverflow.com/questions/227897/how-to-allocate-aligned-memory-only-using-the-standard-library

    uintptr_t memory = _aligned_malloc(pool_s->total_size, padding);
    memset(memory, 0, pool_s->total_size);

    pool_s->next_free_location = pool_s->storage_begin = memory;

    //pool_s->storage_end = pool_s->storage_begin + (sizeof(struct entry_s)* amount_entries);
    pool_s->storage_end = pool_s->storage_begin + (pool_s->total_size );
    
    return pool_s;
}


void free_pool(memory_pool* pool) {
    if (pool == NULL)
        return;

    if (pool->storage_begin != NULL)
        free(pool->storage_begin);
    
    pool->storage_begin = NULL;
    free(pool);
    pool = NULL;
}

inline void switch_next_entry(memory_pool* pool, uint32_t amount_requested) {
    struct entry_s* temp = NULL;
    //if (pool->next_free_location <= pool->storage_end) {
    if (pool->next_free_location != NULL /*&& (pool->next_free_location + amount_requested + sizeof(struct entry_s)) < pool->storage_end 9*/) {
        //temp = (pool->next_free_location + pool->cache_size);
        temp = (pool->next_free_location + amount_requested + sizeof(struct entry_s));
        //using padded memory should be optional.
        //temp = pool->next_free_location + pool->padded_size;
        if (temp >= pool->storage_end)
            temp = NULL;
    }

    if (temp == NULL){
        //printf("deu a volta\n");
        temp = pool->storage_begin;
    }    

    pool->alloc_calls++;
    if (temp->is_use == 0) {
        pool->next_free_location = temp;
    } else {
        // no more memory avail
        pool->next_free_location = NULL;
    }
}


inline uintptr_t _malloc_ref_internal(memory_pool* pool, uint32_t size) {
    if (pool == NULL || pool->block_size != size)
        return NULL;

    if (pool->next_free_location == NULL) {
        printf("No more memory available.\n");
        return NULL;
    }
    struct entry_s* mem = pool->next_free_location;
    mem->is_use = 1;
    mem->size = size;
    mem->ptr = mem + POINTER_OFFSET;
    switch_next_entry(pool, size);
    return mem->ptr;
}


inline void _free_ref_internal(memory_pool* pool, int8_t* ptr) {
    if (pool == NULL  || ptr == NULL)
        return;
    // we access directly to the ptr and change its state    
    struct entry_s *entry = (struct entry_s*) (ptr - 2*pool->cache_size /*- POINTER_OFFSET*/);
    entry->is_use = 0;
    pool->free_calls++;
}

#ifndef DISABLE_MEMORY_POOLING

uintptr_t _malloc_ref(void* pool, size_t size) {
    return _malloc_ref_internal((memory_pool*)pool, size);
}

void _free_ref(void* pool, uintptr_t ptr) {
    _free_ref_internal((memory_pool*) pool, ptr);
}
#endif

void* malloc_ref(void* pool, size_t size) {
    return _malloc_ref_internal((memory_pool*)pool, size);
}

void free_ref(void* pool, void* ptr) {
    _free_ref_internal((memory_pool*)pool, (uintptr_t) ptr);
}


#ifdef __cplusplus
}
#endif
