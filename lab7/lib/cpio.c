#include "cpio.h"

cpio_newc_header_t* header;

void cpio_init() {
    header = kmalloc(sizeof(cpio_newc_parse_header) / sizeof(char));
}

void cpio_newc_parser(void* callback, char* param) {
    char *cpio_ptr = INITRD_START,
         *file_name, *file_data;
    uint32_t name_size, data_size;
    cpio_newc_header_t* header = kmalloc(CPIO_NEWC_HEADER_SIZE);

    while (1) {
        cpio_newc_parse_header(&cpio_ptr, &header);
        // cpio_newc_show_header(header);

        name_size = hex_ascii_to_uint32(header->c_namesize, sizeof(header->c_namesize) / sizeof(char));
        data_size = hex_ascii_to_uint32(header->c_filesize, sizeof(header->c_filesize) / sizeof(char));

        cpio_newc_parse_data(&cpio_ptr, &file_name, name_size, CPIO_NEWC_HEADER_SIZE);

        cpio_newc_parse_data(&cpio_ptr, &file_data, data_size, 0);

        if (strcmp(file_name, "TRAILER!!!") == 0) {
            break;
        }
        ((void (*)(char* param, cpio_newc_header_t* header, char* file_name, uint32_t name_sizez, char* file_data, uint32_t data_size))callback)(
            param, header, file_name, name_size, file_data, data_size);
    }
}

void cpio_newc_parse_header(char** cpio_ptr, cpio_newc_header_t** header) {
    *header = *cpio_ptr;
    *cpio_ptr += sizeof(cpio_newc_header_t) / sizeof(char);
}

void cpio_newc_show_header(cpio_newc_header_t* header) {
    char buf[0x10] = {};

    memcpy(buf, (header)->c_magic, 6);
    printf("c_magic: %s" ENDL, buf);

    memcpy(buf, (header)->c_ino, 8);
    printf("c_ino: %s" ENDL, buf);

    memcpy(buf, (header)->c_mode, 8);
    printf("c_mode: %s" ENDL, buf);

    memcpy(buf, (header)->c_uid, 8);
    printf("c_uid: %s" ENDL, buf);

    memcpy(buf, (header)->c_gid, 8);
    printf("c_gid: %s" ENDL, buf);

    memcpy(buf, (header)->c_nlink, 8);
    printf("c_nlink: %s" ENDL, buf);

    memcpy(buf, (header)->c_mtime, 8);
    printf("c_mtime: %s" ENDL, buf);

    memcpy(buf, (header)->c_filesize, 8);
    printf("c_filesize: %s" ENDL, buf);

    memcpy(buf, (header)->c_devmajor, 8);
    printf("c_devmajor: %s" ENDL, buf);

    memcpy(buf, (header)->c_devminor, 8);
    printf("c_devminor: %s" ENDL, buf);

    memcpy(buf, (header)->c_rdevmajor, 8);
    printf("c_rdevmajor: %s" ENDL, buf);

    memcpy(buf, (header)->c_rdevminor, 8);
    printf("c_rdevminor: %s" ENDL, buf);

    memcpy(buf, (header)->c_namesize, 8);
    printf("c_namesize: %s" ENDL, buf);

    memcpy(buf, (header)->c_check, 8);
    printf("c_check: %s" ENDL, buf);
}

void cpio_newc_parse_data(char** cpio_ptr, char** buf, uint32_t size, uint32_t offset) {
    *buf = *cpio_ptr;
    while ((size + offset) % 4 != 0) {
        size += 1;
    }
    *cpio_ptr += size;
}

void cpio_ls_callback(char* param, cpio_newc_header_t* header, char* file_name, uint32_t name_size, char* file_data, uint32_t data_size) {
    // TODO: impelment parameter
    char buf[0x100] = {};
    memcpy(buf, file_name, name_size);
    printf("%s" ENDL, buf);
}

void cpio_cat_callback(char* param, cpio_newc_header_t* header, char* file_name, uint32_t name_size, char* file_data, uint32_t data_size) {
    // TODO: implement multi-parameter
    if (strcmp(param, file_name)) return;
    char buf[0x100] = {};

    memcpy(buf, file_name, name_size);
    printf("Filename: %s" ENDL, buf);

    data_size %= 0x100;
    memcpy(buf, file_data, data_size);
    printf("%s" ENDL, buf);
}

void cpio_exec_callback(char* param, cpio_newc_header_t* header, char* file_name, uint32_t name_size, char* file_data, uint32_t data_size) {
    // TODO: implement multi-parameter
    if (strcmp(param, file_name)) return;
    exec(file_data, data_size);
}

// void cpio_exec_callback_tf(char* param, cpio_newc_header_t* header, char* file_name, char* file_data, uint32_t data_size, trap_frame_t* tf) {
//     // TODO: implement multi-parameter
//     if (strcmp(param, file_name)) return;
//     _exec(file_data, data_size, tf);
// }

void cpio_exec_sched_callback(char* param, cpio_newc_header_t* header, char* file_name, uint32_t name_size, char* file_data, uint32_t data_size) {
    if (strcmp(param, file_name)) return;

    data_size = 246920;  // TODO: temp size

    // char* file_ptr = frame_alloc(data_size / 0x1000);  // memcpy, so the address will align 0x1000
    char* file_ptr = kmalloc(data_size + 0x1000);
    printf("[+] %s base: 0x%X" ENDL, file_name, file_ptr);
    file_ptr += 0x1000;
    file_ptr = (((uint64_t)file_ptr >> 16) << 16);
    // file_ptr = 0x7e00000;
    printf("memcpy(0x%X, 0x%X, 0x%X)" ENDL, file_ptr, file_data, data_size);
    for (uint32_t i = 0; i < 20; i++) {
        printf("0x%X ", file_data[i]);
    }
    printf(ENDL);
    memcpy(file_ptr, file_data, data_size);
    printf("create_user_task" ENDL);
    // create_user_task(file_ptr, 0);
}

void* cpio_get_file(char* file_name) {
    char *cpio_ptr = INITRD_START,
         *curr_file_name;
    uint32_t name_size, data_size;
    uint32_t header_size = sizeof(cpio_newc_header_t);
    cpio_newc_header_t* header = kmalloc(header_size);
    void* file_buf = NULL;

    while (1) {
        // parse header
        memcpy(header, cpio_ptr, header_size);
        cpio_ptr += header_size;

        // get name_size and data_size
        name_size = hex_ascii_to_uint32(header->c_namesize, sizeof(header->c_namesize) / sizeof(char));
        data_size = hex_ascii_to_uint32(header->c_filesize, sizeof(header->c_filesize) / sizeof(char));

        // parse data
        curr_file_name = cpio_ptr;
        while ((name_size + header_size) % 4 != 0) name_size += 1;  // add `header_size` is to balance the offset
        cpio_ptr += name_size;

        if (strcmp(file_name, curr_file_name) == 0) {
            file_buf = kmalloc(data_size);
            memcpy(file_buf, cpio_ptr, data_size);
            break;
        } else if (strcmp(curr_file_name, "TRAILER!!!") == 0) {
            break;
        }

        while (((uint64_t)data_size % 4) != 0) data_size += 1;
        cpio_ptr += data_size;
    }
    kfree(header);
    return file_buf;
}