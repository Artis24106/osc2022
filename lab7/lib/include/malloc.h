#ifndef __MEM_H__
#define __MEM_H__

#include <stdint.h>

#include "exception.h"
#include "list.h"
#include "page_alloc.h"

#define TCACHE_MAX_BINS 64
#define align(number, base) \
    ((number + (base - 1)) & (~(base - 1)))

#define SLAB_POOL_COUNT 15
static const uint32_t slab_size[SLAB_POOL_COUNT] = {
    0x10, 0x20, 0x30, 0x60, 0x80, 0x100, 0x200, 0x400, 0x800,
    0xc00, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000};

#define SLAB_CACHE_ALLOC_PAGE_SIZE ((4 * slab_size[SLAB_POOL_COUNT - 1]) / 0x1000)  // 64

typedef struct slab_cache {
    uint32_t cache_size;  // the cache size

    // save the slab cache range to determine the cache size of freed addr
    void* start;
    void* end;

    list_head_t node;        // link to next slab cache (same size)
    list_head_t free_cache;  // store avaliable cache
} slab_cache_t;

slab_cache_t* get_slab_cache_by_addr(void* addr);
void init_slab_cache();
void* slab_alloc(uint64_t size);
bool slab_free(void* ptr);

void* kmalloc(uint64_t size);
void kfree(void* ptr);

void show_slab_cache();

void ddd();

#endif