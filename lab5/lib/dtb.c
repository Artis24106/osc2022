#include "dtb.h"

// #include "uart.h"
void *DTB_START, *DTB_END;
void *INITRD_START, *INITRD_END;

void dtb_init(uint64_t x0) {
    DTB_START = (void*)x0;
    fdt_header_t* header = (fdt_header_t*)DTB_START;
    DTB_END = DTB_START + get_be_uint32(&header->totalsize);
    printf("dtb range 0x%X ~ 0x%X" ENDL, DTB_START, DTB_END);
    dtb_parser(dtb_get_initrd_callback);
}

void dtb_parser(dtb_callback_t callback) {
    fdt_header_t* header = (fdt_header_t*)DTB_START;
    // Check magic
    if (get_be_uint32(&header->magic) != 0xd00dfeed) {
        printf("[+] BAD" ENDL);
        return;
    }

    char *dt_sturct = (char*)header + get_be_uint32(&header->off_dt_struct),
         *dt_strings = (char*)header + get_be_uint32(&header->off_dt_strings);
    char* ptr = dt_sturct;
    uint32_t token_type;

    while (1) {
        token_type = get_be_uint32(ptr);
        ptr += 4;

        switch (token_type) {
            case FDT_BEGIN_NODE:
                callback(token_type, ptr, 0);
                ptr += strlen(ptr) + 1;
                if ((uint64_t)ptr % 4) ptr += 4 - (uint64_t)ptr % 4;  // 4 bytes alignment
                break;
            case FDT_END_NODE:
                callback(token_type, 0, 0);
                break;
            case FDT_PROP:
                uint32_t len = get_be_uint32(ptr);
                ptr += 4;
                char* name = (char*)dt_strings + get_be_uint32(ptr);  // dt_strings + nameoff
                ptr += 4;
                callback(token_type, name, ptr);
                ptr += len;
                if ((uint64_t)ptr % 4) ptr += 4 - (uint64_t)ptr % 4;  // 4 bytes alignment
                break;
            case FDT_NOP:
                callback(token_type, 0, 0);
                break;
            case FDT_END:
                callback(token_type, 0, 0);
                break;
            default:
                printf("%08lX", *ptr);
                return;
                break;
        }
        if (token_type == FDT_END) break;
    }
}

void dtb_get_initrd_callback(uint32_t token_type, char* name, char* data) {
    if (token_type == FDT_PROP && !strcmp(name, "linux,initrd-start")) {
        INITRD_START = get_be_uint32(data);
        printf("[+] Initrd start address: 0x%08lX" ENDL, INITRD_START);
    } else if (token_type == FDT_PROP && !strcmp(name, "linux,initrd-end")) {
        INITRD_END = get_be_uint32(data);
        printf("[+] Initrd end address: 0x%08lX" ENDL, INITRD_END);
    }
}

void dtb_show_callback(uint32_t token_type, char* name, char* data) {
    static unsigned int level = 0;
    switch (token_type) {
        case FDT_BEGIN_NODE:
            for (uint32_t i = 0; i < level; i++) printf("    ");
            printf("%s {" ENDL, name);
            level++;
            break;

        case FDT_END_NODE:
            level--;
            for (uint32_t i = 0; i < level; i++) printf("    ");
            printf("}" ENDL);
            break;

        case FDT_PROP:
            for (uint32_t i = 0; i < level; i++) printf("    ");
            printf("%s" ENDL, name);
            break;
    }
}
