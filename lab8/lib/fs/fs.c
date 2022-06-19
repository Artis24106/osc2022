#include "fs/fs.h"

void fs_init() {
    vfs_init();

    filesystem_t* tmpfs = tmpfs_init();
    filesystem_t* cpiofs = cpiofs_init();
    filesystem_t* uartfs = uartfs_init();
    filesystem_t* fbfs = fbfs_init();
    register_filesystem(tmpfs);
    register_filesystem(cpiofs);
    register_filesystem(uartfs);
    register_filesystem(fbfs);

    rootfs_init(tmpfs);

    // mount "/initramfs"
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "cpiofs");

    // mount "/dev/uart"
    vfs_mkdir("/dev");
    vfs_mkdir("/dev/uart");
    vfs_mount("/dev/uart", "uartfs");

    // mount "/dev/framebuffer"
    vfs_mkdir("/dev/framebuffer");
    vfs_mount("/dev/framebuffer", "fbfs");
#ifdef DEBUG_VFS
    rootfs->root->v_ops->show_vnode(rootfs->root, 0);
#endif
}