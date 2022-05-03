#ifndef __MEM_H__
#define __MEM_H__

#include <stdint.h>

#include "exception.h"
#include "page_alloc.h"

#define TCACHE_MAX_BINS 64
#define align(number, base) \
    ((number + (base - 1)) & (~(base - 1)))

/*
    | -------------------------------- |
    | prev_size/prev_data | chunk_size |
    |       8 bytes       |   8 bytes  |
    | -------------------------------- |
    |               data               |
    | -------------------------------- |
*/
typedef struct malloc_chunk {
    uint64_t prev_size;
    uint64_t chunk_size;  // chunk size
} malloc_chunk_t;

// https://code.woboq.org/userspace/glibc/malloc/malloc.c.html#tcache_entry
typedef struct tcache_entry {
    struct tcache_entry* next;

    /* This field is exists to detect double frees */
    struct tcache_perthread_struct* key;
} tcache_entry_t;

// TCACHE_MAX_BINS = 64
// which stores 0x20 ~ 0x410 chunk size
typedef struct tcache_perthread_struct {
    uint16_t counts[TCACHE_MAX_BINS];
    tcache_entry_t* entries[TCACHE_MAX_BINS];
} tcache_perthread_struct_t;

uint32_t get_tcache_idx(uint64_t chunk_size);
void renew_kheap_space(uint64_t size);
void init_tcache();

void* kmalloc(uint64_t size);
void kfree(void* ptr);

void show_tcache();

void ddd();

#endif