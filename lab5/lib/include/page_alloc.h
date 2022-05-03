#ifndef __PAGE_ALLOC_H__
#define __PAGE_ALLOC_H__

#include "list.h"
#include "printf.h"

#define MEM_REGION_START 0x10000000
#define MEM_REGION_END 0x20000000
#define PAGE_SIZE 0x1000

// TODO: better define
//#define FRAME_SIZE ((MEM_REGION_END - MEM_REGION_START) / PAGE_SIZE)  // 0x10000 -> 65536
#define FRAME_SIZE 8
// #define MAX_ORDER 16 - 1                                              // log2(0x10000) = 16
#define MAX_ORDER 3
#define FRAME_IS_BUDDY (MAX_ORDER + 1)

//#define FRAME_VAL_X_MASK (1 << 8)  // The frame is allocated, if the bit is set

#define DEBUG_PAGE_ALLOC

enum migratetype {  // TODO: what is this??
    MIGRATE_UNMOVABLE,
    MIGRATE_MOVABLE,
    MIGRATE_RECLAIMABLE,

    MIGRATE_TYPES
};

// struct free_area {
//     struct list_head free_list[MIGRATE_TYPES];
//     uint64_t nr_free;  // TODO: what is this??
// };

struct free_area {
    struct list_head free_list;
};

typedef struct frame {
    struct list_head node;
    bool is_used;       // true, if the frame is allocated
    uint32_t val;       // order (only active when is_used == true)
    uint32_t page_fpn;  // frame page number (index)
} frame_t;

uint32_t __find_buddy_pfn(uint32_t page_fpn, uint32_t order);

void show_free_area();
uint32_t fp2ord(uint32_t fp);   // number of frame page -> minimal order
void* fpn2addr(uint32_t fpn);   // frame page index -> physical address
uint32_t addr2fpn(void* addr);  // physical address -> frame page index
bool is_allocated(frame_t* f);

void frame_init();
void* frame_alloc(uint32_t fp);
void frame_free(void* addr);
frame_t* get_frame_from_freelist(uint32_t order);
void memory_reserve(void* start, void* end);

#endif