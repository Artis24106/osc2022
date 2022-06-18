#include "fs/fs.h"

void fs_init() {
    vfs_init();

    filesystem_t* tmpfs = tmpfs_init();
    filesystem_t* cpiofs = cpiofs_init();
    register_filesystem(tmpfs);
    register_filesystem(cpiofs);

    rootfs_init(tmpfs);

    rootfs->root->v_ops->show_vnode(rootfs->root, 0);
    vfs_mkdir("/initramfs");
    rootfs->root->v_ops->show_vnode(rootfs->root, 0);
    vfs_mount("/initramfs", "cpiofs");
    // rootfs->root->v_ops->show_vnode(rootfs->root, 0);
}