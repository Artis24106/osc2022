#include "fs/fs.h"

void fs_init() {
    vfs_init();

    filesystem_t* tmpfs = tmpfs_init();
    register_filesystem(tmpfs);

    rootfs_init(tmpfs);
}