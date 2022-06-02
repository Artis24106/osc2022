#include "page_alloc.h"

static frame_t* frame_list[FRAME_SIZE];
static free_area_t* free_area[MAX_ORDER + 1];

uint32_t __find_buddy_pfn(uint32_t page_fpn, uint32_t order) {
    return page_fpn ^ (1 << order);
}

void show_free_area() {
    frame_t* curr;
    printf(ENDL "----- show_free_area()" ENDL);
    for (uint32_t order = 0; order < MAX_ORDER + 1; order++) {
        if (list_empty(&free_area[order]->free_list)) continue;
        printf("%d : ", order);
        uint32_t cnt = 0;
        list_for_each_entry(curr, &free_area[order]->free_list, node) {
            printf("(%d, %d) ", curr->page_fpn, curr->val);
            if (!list_is_last(curr, &free_area[order]->free_list)) printf("-> ");
            cnt++;
            if (cnt > 4) break;
        }
        printf(ENDL);
    }
    printf("-----" ENDL ENDL);
}

uint32_t fp2ord(uint32_t fp) {
    uint32_t order = 0;
    for (uint32_t t = 1; t < fp; t *= 2) {
        order += 1;
    }
    return order;
}

// TODO: add bound check
void* fpn2addr(uint32_t fpn) {
    return MEM_REGION_START + 0x1000 * fpn;
}

// TODO: add bound check
uint32_t addr2fpn(void* addr) {
    return ((uint32_t)addr - MEM_REGION_START) / 0x1000;
}

bool is_allocated(frame_t* f) {
    return f->is_used;
}

void frame_init() {
#ifdef DEBUG_PAGE_ALLOC
    printf("[DEBUG] frame_init()" ENDL);
#endif

    // Initialize fram_list
    printf("[+] Init frame_list * 0x%X" ENDL, FRAME_SIZE);
    for (uint32_t fpn = 0; fpn < FRAME_SIZE; fpn++) {
        frame_list[fpn] = startup_alloc(sizeof(frame_t));
        INIT_LIST_HEAD(&frame_list[fpn]->node);
        frame_list[fpn]->is_used = false;
        // to implement `memory_reserve()`, set all val to 0
        frame_list[fpn]->val = 0;
        frame_list[fpn]->page_fpn = fpn;
    }
    printf("QQQ: 0x%X" ENDL, frame_list[FRAME_SIZE - 1]);
    // frame_list[0].val = MAX_ORDER;

    // handle reserved memory
    // 1. reserve frames
    memory_reserve(0x0, 0x1000);                           // spin table for multicore boot
    memory_reserve(0x60000, 0x80000);                      // TODO: bootloader image ???
    memory_reserve(0x80000, 0xd0000);                      // kernel image
    memory_reserve(INITRD_START, INITRD_END);              // initramfs
    memory_reserve(DTB_START, DTB_END);                    // devicetree
    memory_reserve(STARTUP_HEAP_START, STARTUP_HEAP_END);  // startup allocator
    memory_reserve(0x180000, 0x260000);
    // memory_reserve(0x0, 0x2C000000);

    // 2. merge the unused frames
    bool modified = false;
    uint32_t first_fpn = 0,
             inc = 1,
             buddy;
    for (uint32_t ord = 0; ord < MAX_ORDER; ord++) {
        for (uint32_t fpn = 0; fpn < FRAME_SIZE; fpn += inc) {
            if (frame_list[fpn]->val != ord) continue;

            buddy = __find_buddy_pfn(fpn, ord);

            if (frame_list[buddy]->val != ord) continue;

            // if the buddy is reserved, can't be mergered
            if (frame_list[fpn]->is_used || frame_list[buddy]->is_used) continue;
            frame_list[buddy]->val = FRAME_IS_BUDDY;
            frame_list[fpn]->val += 1;

            // record the first modified fpn
            if (!modified) first_fpn = fpn;
            modified = true;
        }
        if (!modified) break;
        modified = false;
        inc <<= 1;
        // printf(ENDL);
    }

    // Initialize free_area
    for (uint32_t order = 0; order < MAX_ORDER + 1; order++) {  // TODO: more check
        free_area[order] = startup_alloc(sizeof(free_area_t));
        INIT_LIST_HEAD(&free_area[order]->free_list);
    }
    for (uint32_t fpn = 0; fpn < FRAME_SIZE; fpn++) {
        if (frame_list[fpn]->is_used || frame_list[fpn]->val == FRAME_IS_BUDDY) continue;
        list_add_tail(frame_list[fpn], free_area[frame_list[fpn]->val]);
    }
    // show_free_area();

#ifdef DEBUG_PAGE_ALLOC
    show_free_area();
#endif
}

void* frame_alloc(uint32_t fp) {
#ifdef DEBUG_PAGE_ALLOC
    printf("[DEBUG] frame_alloc(%d)" ENDL, fp);
#endif
    void* retaddr = NULL;
    uint32_t order = fp2ord(fp);

    // find from free_area
    frame_t* frame = get_frame_from_freelist(order);
    if (frame) {  // found a frame in free_area
        while (true) {
            if (frame->val <= order) break;

            // if we found a larger frame, try finding its buddy and split it
            uint32_t buddy = __find_buddy_pfn(frame->page_fpn, --(frame->val));
            if (!is_allocated(frame_list[buddy])) {                              // check if the buddy is not allocated
                frame_list[buddy]->val = frame->val;                             // split the buddy
                list_add(frame_list[buddy], &free_area[frame->val]->free_list);  // add to new frame_list
            } else {
                printf("[ERROR] frame_alloc()" ENDL);
                break;  // TODO: impossible!!
            }
        }
        // show_free_area();
        // now the frame is allocated
        frame->is_used = true;
        retaddr = fpn2addr(frame->page_fpn);
    } else {  // no space to allocate @@
        printf("[+] No frame to allocate!!" ENDL);
    }

#ifdef DEBUG_PAGE_ALLOC
    show_free_area();
#endif

    return retaddr;
}

void frame_free(void* addr) {
    uint32_t fpn = addr2fpn(addr);
    frame_t* frame = &frame_list[fpn];

#ifdef DEBUG_PAGE_ALLOC
    printf("--------------[DEBUG] frame_free(%d)" ENDL, fpn);
#endif

    // check if the frame can be freed
    if (!is_allocated(frame)) {
        printf("[+] frame_list[%d] is not allocated." ENDL, fpn);
        return;
    }

    // try merging the frame
    uint32_t curr_val = frame->val,
             min_fpn = frame->page_fpn;

    // the frame is freed, so set `is_used` to false
    frame->is_used = false;

    while (curr_val < MAX_ORDER) {
        uint32_t buddy = __find_buddy_pfn(min_fpn, curr_val);
        printf("[+] Get buddy -> %d" ENDL, buddy);

        // if the buddy is allocated -> can not be merged
        if (is_allocated(&frame_list[buddy])) {
            printf("[+] is allocated" ENDL);
            break;
        }

        if (buddy < min_fpn) min_fpn = buddy;

        // remove the buddy from freelist
        frame_t *curr, *temp;
        list_for_each_entry_safe(curr, temp, &free_area[curr_val]->free_list, node) {
            if (curr->page_fpn == buddy) {
                curr->val = 0;  // TODO:
                list_del(&curr->node);
                kfree(&curr->node);
                break;
            }
        }

        // merge successfully, so increase the frame order
        curr_val++;
    }

    frame_list[min_fpn]->val = curr_val;
    // insert the new frame to free_list
    list_add(&frame_list[min_fpn], &free_area[curr_val]);

#ifdef DEBUG_PAGE_ALLOC
    show_free_area();
#endif
}

frame_t* get_frame_from_freelist(uint32_t order) {
    frame_t* ret = NULL;
    for (uint32_t i = order; i < MAX_ORDER + 1; i++) {
        if (list_empty(&free_area[i]->free_list)) continue;

        // get the first element in the list
        ret = free_area[i]->free_list.next;  // TODO: check @@
        list_del_init(free_area[i]->free_list.next);

        break;
    }
    return ret;
}

void memory_reserve(void* start, void* end) {
#ifdef DEBUG_PAGE_ALLOC
    printf("[+] memory_reserve(0x%X, 0x%X)" ENDL, start, end);
#endif

    uint32_t start_fpn = addr2fpn(start),
             end_fpn = addr2fpn(end - 1);
    printf("[+] memory_reserve_t(0x%X, 0x%X)" ENDL, start_fpn * 0x1000, (end_fpn + 1) * 0x1000);

    for (uint32_t fpn = start_fpn; fpn <= end_fpn; fpn++) {
        if (frame_list[fpn]->is_used) {  // TODO: simple check
            printf("[+] The reserved memory is used (0x%X ~ 0x%X)" ENDL, start, end);
            while (1)
                ;
        }
        frame_list[fpn]->is_used = true;
    }
}