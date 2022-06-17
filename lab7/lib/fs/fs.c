#include "fs/fs.h"

void fs_init() {
    vfs_init();

    filesystem_t* tmpfs = tmpfs_init();
    // filesystem_t* cpiofs = cpiofs_init();
    register_filesystem(tmpfs);
    // register_filesystem(cpiofs);

    rootfs_init(tmpfs);

    // vfs_mkdir("/initramfs");
    // vfs_mount("/initranfs", "cpiofs");

    // printf("-------- start testing ----------" ENDL);
    // char buf[0x100];
    // file_t* target;
    // vfs_open("/initramfs/./b.txt", NULL, target);
    // printf("AAA" ENDL);
    // ddd();
    // vfs_read(target, buf, 0x20);
    // printf("BBB" ENDL);
    // ddd();
    // printf("buf = %s" ENDL);

    // printf("-------- end of testing ----------" ENDL);
}