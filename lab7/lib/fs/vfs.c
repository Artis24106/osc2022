#include "fs/vfs.h"

mount_t* rootfs;

// https://www.cnblogs.com/fengkang1008/p/4691231.html
static list_head_t file_systems;  // store all registered filesystems

vnode_t* vfs_get_dir_node_by_name(vnode_t* dir_node, char** path_name) {
    char buf[0x100];
    char *start, *end;
    vnode_t* ret_node;

    start = end = *path_name;

    if (*start == '/') {  // start searching from root
        ret_node = rootfs->root;
    } else {
        ret_node = dir_node;
    }

    while (1) {
        // case: "./" and "../"
        if (!strncmp("./", start, 2)) {
            start += 2;
            end = start;
            continue;
        } else if (!strncmp("../", start, 3)) {
            if (ret_node->parent) ret_node = ret_node->parent;
            start += 3;
            end = start;
            continue;
        }

        // get next component
        while (*end != '/' && *end != '\0') end += 1;

        if (*end == '\0') break;

        // case: "//"
        if (start == end) {
            start = ++end;
            continue;
        }

        memcpy(buf, start, end - start);
        buf[end - start] = '\0';
        start = ++end;

        // try looking up `buf` in `ret_node`
        // if found, update `ret_node`
        if (ret_node->v_ops->lookup(ret_node, &ret_node, buf) < 0) return NULL;
    }

    *path_name = *start ? start : NULL;

    return ret_node;
}

filesystem_t* vfs_get_filesystem_by_name(const char* fs_name) {
    filesystem_t* curr;
    list_for_each_entry(curr, &file_systems, node) {
        if (strcmp(curr->name, fs_name) == 0) {
            return curr;
        }
    }
    return NULL;
}

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
    // rootfs = kmalloc(0x1000);
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
    const char* curr_name = pathname;
    task_struct_t* curr = current;
    vnode_t* dir_node = vfs_get_dir_node_by_name(curr->work_dir, &curr_name);
    // vnode_t* dir_node = vfs_get_dir_node_by_name(rootfs->root, &curr_name);
    if (!dir_node || !curr_name) return -1;

    /* 2. Create a new file handle for this vnode if found. */
    vnode_t* new_file;
    int ret = dir_node->v_ops->lookup(dir_node, &new_file, curr_name);

    /* 3. Create a new file if O_CREAT is specified in flags and vnode not found
          lookup error code shows if file exist or not or other error occurs */
    /* 4. Return error code if fails */
    if (flags & O_CREAT && ret == -1) {  // if vnode not found and O_CREAT is set -> create new vnode
        ret = dir_node->v_ops->create(dir_node, &new_file, curr_name);
    }
    if (ret < 0) return ret;
    if (!new_file) return -1;

    // now open the file
    ret = new_file->f_ops->open(new_file, target);
    if (ret < 0) return ret;

    target->f_flags = 0;

    return 0;
}

int vfs_close(file_t* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    return file->f_ops->close(file);
}

int vfs_write(file_t* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return file->f_ops->write(file, buf, len);
}

int vfs_read(file_t* file, void* buf, size_t len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char* pathname) {
    vnode_t* dir_node;
    task_struct_t* curr = current;
    if (curr) {
        dir_node = vfs_get_dir_node_by_name(curr->work_dir, &pathname);
    } else {
        dir_node = vfs_get_dir_node_by_name(rootfs->root, &pathname);
    }

    if (!dir_node || !pathname) return -1;

    vnode_t* tmp_node;
    return dir_node->v_ops->mkdir(dir_node, &tmp_node, pathname);
}

int vfs_mount(const char* target, const char* filesystem) {
    vnode_t* dir_node;
    char* folder_name = target;
    int ret;
    if (current) {
        dir_node = vfs_get_dir_node_by_name(current->work_dir, &folder_name);
    } else {
        dir_node = vfs_get_dir_node_by_name(rootfs->root, &folder_name);
    }

    if (!dir_node) return -1;

    if (folder_name) {
        // check if `folder_name` exist
        ret = dir_node->v_ops->lookup(dir_node, &dir_node, folder_name);
        if (ret < 0) return ret;
    }

    // check if `folder_name` is directory
    if (!dir_node->v_ops->is_dir(dir_node)) return -1;

    // get the filesystem
    filesystem_t* fs = vfs_get_filesystem_by_name(filesystem);
    if (!fs) return -1;

    mount_t* tmp_mount = kmalloc(sizeof(mount_t));
    tmp_mount->root = dir_node;
    tmp_mount->fs = fs;

    ret = fs->setup_mount(fs, tmp_mount);
    if (ret < 0) {
        kfree(tmp_mount);
        return ret;
    }

    return 0;
}

int vfs_lookup(const char* pathname, vnode_t** target) {
    char* curr_name = pathname;
    vnode_t* dir_node = vfs_get_dir_node_by_name(current->work_dir, &curr_name);

    if (!dir_node) return -1;
    if (!curr_name) {  // TODO: not sure about this
        *target = dir_node;
        return 0;
    }

    vnode_t* file_node;
    int ret = dir_node->v_ops->lookup(dir_node, &file_node, curr_name);
    if (ret < 0) return ret;

    *target = file_node;

    return 0;
}

long vfs_lseek64(file_t* file, long offset, int whence) {
    return file->f_ops->lseek64(file, offset, whence);
}

int vfs_ioctl(file_t* file, uint64_t request, va_list args) {
    return file->f_ops->ioctl(file, request, args);
}