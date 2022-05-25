#include "malloc.h"

// at first, a 0x1000 space is reserved for timer list and task list
static void* s_heap_ptr = STARTUP_HEAP_START;
list_head_t* slab_cache_pool[SLAB_POOL_COUNT];

void* startup_alloc(uint32_t size) {
    // printf("startup_alloc(0x%X)" ENDL, size);
    if (size == 0) return NULL;
    size = align(size, 0x10);

    void* ret = s_heap_ptr;
    s_heap_ptr += size;
    if (s_heap_ptr >= STARTUP_HEAP_END) {
        printf("[-] startup_alloc() out of range!!" ENDL);
        return NULL;
    }
    return ret;
}

slab_cache_t* get_slab_cache_by_addr(void* addr) {
    slab_cache_t* target;
    for (uint32_t i = 0; i < SLAB_POOL_COUNT; i++) {
        list_for_each_entry(target, slab_cache_pool[i], node) {
            if (addr < target->start || addr >= target->end) continue;
            return target;
        }
    }
    return NULL;
}

void init_slab_cache() {
    for (uint32_t i = 0; i < SLAB_POOL_COUNT; i++) {
        uint32_t curr_cache_size = slab_size[i];

        // allocate 64 pages
        void* ptr = frame_alloc(SLAB_CACHE_ALLOC_PAGE_SIZE);

        slab_cache_pool[i] = startup_alloc(sizeof(list_head_t));
        INIT_LIST_HEAD(slab_cache_pool[i]);
        slab_cache_t* curr = startup_alloc(sizeof(slab_cache_t));
        curr->cache_size = curr_cache_size;
        curr->start = ptr;
        INIT_LIST_HEAD(&curr->node);
        INIT_LIST_HEAD(&curr->free_cache);

        list_add_tail(&curr->node, slab_cache_pool[i]);

        // split the 64 pages into `curr_cache_size` caches
        uint32_t cache_cnt = (SLAB_CACHE_ALLOC_PAGE_SIZE * 0x1000) / curr_cache_size;
        while (cache_cnt--) {
            INIT_LIST_HEAD(ptr);
            list_add_tail(ptr, &curr->free_cache);
            ptr += curr_cache_size;
        }
        curr->end = ptr;
    }
}

void* slab_alloc(uint64_t size) {
    if (size == 0) return NULL;
    // printf("slab_alloc(%d)" ENDL, size);
    uint32_t alloc_size;
    list_head_t* ret = NULL;
    for (uint32_t i = 0; i < SLAB_POOL_COUNT; i++) {
        alloc_size = slab_size[i];
        if (alloc_size < size) continue;

        slab_cache_t *curr, *temp;
        // find the first non empty slab_cache_pool
        list_for_each_entry_safe(curr, temp, slab_cache_pool[i], node) {
            if (list_empty(&curr->free_cache)) continue;
            // get the first slab cache
            ret = curr->free_cache.next;
            // ddd();
            list_del(ret);
            // printf("GOOD ret: 0x%X" ENDL, ret);
            return ret;
        }
    }
    // printf("ret: 0x%X" ENDL, ret);
    return ret;
}

bool slab_free(void* ptr) {
    slab_cache_t* curr_slab_cache = get_slab_cache_by_addr(ptr);
    if (curr_slab_cache == NULL) return false;

    list_head_t* new_free_cache = ptr;
    INIT_LIST_HEAD(new_free_cache);

    list_add_tail(new_free_cache, &curr_slab_cache->free_cache);  // add the free ptr to free_cache
    return true;
}

void* kmalloc(uint64_t size) {
    uint32_t daif = get_intr();
    disable_intr();

    void* ret = NULL;
    if (size == 0) goto kmalloc_end;
    if (size <= slab_size[SLAB_POOL_COUNT - 1]) {
        ret = slab_alloc(size);
    } else {
        ret = frame_alloc(align(size, 0x1000) / 0x100);
    }

kmalloc_end:
    // enable_intr();
    set_intr(daif);
    // printf("kmalloc() -> 0x%X" ENDL, ret);
    return ret;
}

void kfree(void* ptr) {
    // return;
    uint32_t daif = get_intr();
    disable_intr();

    if (ptr == NULL) goto kfree_end;
    if (slab_free(ptr) == true) goto kfree_end;
    frame_free(ptr);

kfree_end:
    // enable_intr();
    set_intr(daif);
}

void show_slab_cache() {
    for (uint32_t i = 0; i < SLAB_POOL_COUNT; i++) {
        slab_cache_t* curr;
        list_for_each_entry(curr, slab_cache_pool[i], node) {
        }
    }
}

void ddd() {
}