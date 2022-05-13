#include "malloc.h"

// at first, a 0x1000 space is reserved for timer list and task list
extern char _heap_start;
static void* ptr = &_heap_start;
static uint64_t kheap_space = 0;
static tcache_perthread_struct_t tcache;
static large_chunk_perthread_struct_t large_chunk;

uint32_t get_tcache_idx(uint64_t chunk_size) {
    return (chunk_size - 0x20) / 0x10;
}

void renew_kheap_space(uint64_t size) {
    // printf("[+] renew_kheap_space(0x%X)" ENDL, size);
    kheap_space = align(size, 0x1000);  // renew kheap size
    size = kheap_space / 0x1000;        // this is page number

    ptr = frame_alloc(size);  // renew ptr
    // TODO: by now, I don't care about the old ptr
    // TODO: fail check

    //printf("[+] get new ptr -> 0x%X, space -> 0x%X" ENDL, ptr, kheap_space);
}

void init_tcache() {
    for (uint32_t i = 0; i < TCACHE_MAX_BINS; i++) {
        tcache.counts[i] = 0;
        tcache.entries[i] = NULL;
    }
}

void* kmalloc(uint64_t size) {
    disable_intr();
    // The smallest chunk size is 0x20 -> at least 0x18 can be used
    //if (size < 0x18) size = 0x18;
    size += 0x8;
    if (size < 0x20) size = 0x20;

    // 0x10 alignment
    //size = (size + 0x17) & 0xfffffff0;
    size = align(size, 0x10);

    // TODO: only support malloc size between 0x20 to 0x410
    if (size > 0x410) {
        goto large_chunk_handling;
    } else if (size < 0x20 || size > 0x410) {
        printf("[-] kmalloc -> chunk size 0x%X not supported." ENDL, size);
        while (1)
            ;
    }

    malloc_chunk_t* ret;  // ptr to chunk head

tcache_handling:
    // TODO: try getting space from tcache
    uint32_t idx = get_tcache_idx(size);
    if (tcache.counts[idx]) {
        //printf("[++++++++++++] FOUND in tcache!!!" ENDL);

        ret = tcache.entries[idx];

        tcache.entries[idx] = ((tcache_entry_t*)((void*)tcache.entries[idx] + 0x10))->next;
        // printf("TTTTT 0x%X\n", tcache.entries[idx] + 0x10);
        // tcache.entries[idx] = container_of((void*)tcache.entries[idx] + 0x10, tcache_entry_t, next);
        tcache.entries[idx] = (void*)tcache.entries[idx] - 0x10;
        tcache.counts[idx] -= 1;

        show_tcache();
        goto kmalloc_end;
    }
    goto malloc_new_space;

large_chunk_handling:
    // try getting space from large_chunk
    large_chunk_entry_t *tmp = large_chunk.entry, *prev = NULL;
    for (int i = 0; i < large_chunk.count; i++) {
        if (tmp->chunk_size >= size) {  // found a chunk!!
            ret = tmp;
            if (prev != NULL) {
                prev->next = tmp->next;
            } else {
                large_chunk.entry = tmp->next;
            }
            large_chunk.count -= 1;
            goto kmalloc_end;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    goto malloc_new_space;

malloc_new_space:
    // check if there is enough space to malloc
    if (size > kheap_space) {  // no space -> call page frame allocator to get more page!!
        renew_kheap_space(size);
    }

    // generate the chunk
    ret = ptr;
    ret->chunk_size = size;

    // update ptr and space
    ptr += size;
    kheap_space -= size;

kmalloc_end:
    //printf("[+] kmalloc: ptr -> 0x%X, space -> 0x%X" ENDL, ptr, kheap_space);
    enable_intr();
    // printf("[+] kmalloc(0x%X) @ 0x%X" ENDL, size, ret);
    return (void*)ret + sizeof(malloc_chunk_t);  // return the data ptr
}

void kfree(void* ptr) {
    disable_intr();
    malloc_chunk_t* chunk = ptr - sizeof(malloc_chunk_t);
    //printf("[+] chunk size to free -> 0x%X" ENDL, chunk->chunk_size);

    if (chunk->chunk_size > 0x410) {
        goto free_large_chunk;
    } else if (chunk->chunk_size < 0x20 || chunk->chunk_size > 0x410) {
        // ddd();
        printf("[-] kfree -> chunk size 0x%X not supported." ENDL, chunk->chunk_size);
        while (1)
            ;
        return;
    }
    uint32_t idx = get_tcache_idx(chunk->chunk_size);

free_tcache:
    // insert into tcache.entries[idx]
    if (tcache.entries[idx] != NULL) {
        ((tcache_entry_t*)ptr)->next = (void*)tcache.entries[idx] + sizeof(tcache_entry_t);
    } else {
        ((tcache_entry_t*)ptr)->next = NULL;
    }
    tcache.entries[idx] = chunk;
    tcache.counts[idx] += 1;
    goto kfree_end;

free_large_chunk:
    if (large_chunk.entry != NULL) {
        ((large_chunk_entry_t*)ptr)->next = (void*)large_chunk.entry + sizeof(large_chunk_entry_t);
    } else {
        ((large_chunk_entry_t*)ptr)->next = NULL;
    }
    large_chunk.entry = chunk;
    large_chunk.count += 1;

kfree_end:
    show_tcache();
    enable_intr();
}

void show_tcache() {
    return;
    printf("[+] ---- tcache" ENDL);

    for (uint32_t i = 0; i < TCACHE_MAX_BINS; i++) {
        if (tcache.counts[i] == 0) continue;
        printf("tcache[0x%X]", (i + 2) * 0x10);

        tcache_entry_t* ptr = (void*)tcache.entries[i] + 0x10;  // !! first ptr points to chunk head @@
        for (uint32_t j = 0; j < tcache.counts[i]; j++) {
            printf(" -> 0x%X", ptr);
            ptr = ptr->next;
        }
        printf(ENDL);
    }

    printf("[+] ----" ENDL);
}

void ddd() {
}