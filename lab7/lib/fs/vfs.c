#include "fs/vfs.h"

mount_t* rootfs;

// https://www.cnblogs.com/fengkang1008/p/4691231.html
static list_head_t file_systems;  // store all registered filesystems

void vfs_init() {
    INIT_LIST_HEAD(&file_systems);
}

void rootfs_init(filesystem_t* fs) {
    // allocate a root vnode for rootfs
    vnode_t* root;
    if (fs->alloc_root(fs, &root) < 0) {
        printf("[-] rootfs_init() failed!\r\n");
        while (1)
            ;
    }

    rootfs = kmalloc(sizeof(mount_t));
    rootfs->fs = fs;
    rootfs->root = root;

    root->mount = rootfs;
    root->parent = NULL;
}

int register_filesystem(filesystem_t* fs) {
    // register the file system to the kernel.
    list_add_tail(&fs->node, &file_systems);

    // you can also initialize memory pool of the file system here.
    return 0;
}

int vfs_open(const char* pathname, int flags, file_t* target) {
    /* 1. Lookup pathname */
    /* 2. Create a new file handle for this vnode if found. */
    /* 3. Create a new file if O_CREAT is specified in flags and vnode not found
          lookup error code shows if file exist or not or other error occurs */
    /* 4. Return error code if fails */
}

int vfs_close(file_t* file) {
    // 1. release the file handle
    // 2. Return error code if fails
}

int vfs_write(file_t* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
}

int vfs_read(file_t* file, void* buf, size_t len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
}

int vfs_mkdir(const char* pathname) {
}

int vfs_mount(const char* target, const char* filesystem) {
}

int vfs_lookup(const char* pathname, vnode_t** target) {
}