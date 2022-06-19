#include "fs/fs.h"

void fs_init() {
    vfs_init();

    filesystem_t* tmpfs = tmpfs_init();
    filesystem_t* cpiofs = cpiofs_init();
    filesystem_t* uartfs = uartfs_init();
    register_filesystem(tmpfs);
    register_filesystem(cpiofs);
    register_filesystem(uartfs);

    rootfs_init(tmpfs);

    // mount "/initramfs"
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "cpiofs");
#ifdef DEBUG_VFS
    rootfs->root->v_ops->show_vnode(rootfs->root, 0);
#endif

    // mount "/dev/uart"
    vfs_mkdir("/dev");
    vfs_mkdir("/dev/uart");
    vfs_mount("/dev/uart", "uartfs");
#ifdef DEBUG_VFS
    rootfs->root->v_ops->show_vnode(rootfs->root, 0);
#endif
}