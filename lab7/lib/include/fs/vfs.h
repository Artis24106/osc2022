#ifndef __VFS_H__
#define __VFS_H__

#include "list.h"
#include "malloc.h"
#include "printf.h"

/* whence values for lseek */
#define SEEK_SET 0x0  // the offset is set to `offset` bytes
#define SEEK_CUR 0x1  // the offset is set to its current location plus `offset` bytes
#define SEEK_END 0x2  // the offset is set to the size of the file plus offset bytes

typedef struct vnode {  // represent a disk file
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    void* internal;
    struct vnode* parent;  // point to the partne vnode
} vnode_t;

// file handle
// https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s04.html
typedef struct file {  // represent an open file
    struct vnode* vnode;
    size_t f_pos;  // RW position of this file handle (current R/W position)
    struct file_operations* f_ops;
    int f_flags;
} file_t;

// http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch12lev1sec6.html
typedef struct mount {
    struct vnode* root;
    struct filesystem* fs;
} mount_t;

typedef struct filesystem {
    list_head_t* node;
    const char* name;
    int (*setup_mount)(struct filesystem* fs, struct mount* mount);
    int (*alloc_root)(struct filesystem* fs, struct vnode** root);
} filesystem_t;

typedef struct file_operations {
    int (*write)(struct file* file, const void* buf, size_t len);
    int (*read)(struct file* file, void* buf, size_t len);
    int (*open)(struct vnode* file_node, struct file* target);
    int (*close)(struct file* file);
    long (*lseek64)(struct file* file, long offset, int whence);
} file_operations_t;

typedef struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target,
                  const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target,
                  const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
    int (*get_size)(vnode_t* dir_node);
    int (*get_name)(vnode_t* dir_node, char* buf);
} vnode_operations_t;

#define gen_ops(ops_type, ops_name) \
    static ops_type ops_name

#define gen_ops_param(fs_name, param_name) \
    .param_name = fs_name##_##param_name

#define gen_f_vops(fs_name)                         \
    gen_ops(file_operations_t, fs_name##_f_ops) = { \
        gen_ops_param(fs_name, write),              \
        gen_ops_param(fs_name, read),               \
        gen_ops_param(fs_name, open),               \
        gen_ops_param(fs_name, close),              \
        gen_ops_param(fs_name, lseek64),            \
    }

#define gen_v_vops(fs_name)                          \
    gen_ops(vnode_operations_t, fs_name##_v_ops) = { \
        gen_ops_param(fs_name, lookup),              \
        gen_ops_param(fs_name, create),              \
        gen_ops_param(fs_name, mkdir),               \
        gen_ops_param(fs_name, get_size),            \
    }

#define declare_ops(fs_name) \
    gen_f_vops(fs_name);     \
    gen_v_vops(fs_name);

/* filesystem operations */
#define _vfs_setup_mount(fs_name) \
    int fs_name##_setup_mount(filesystem_t* fs, mount_t* mount)
#define _vfs_alloc_root(fs_name) \
    int fs_name##_alloc_root(filesystem_t* fs, vnode_t** root)

/* file operations */
#define _vfs_write(fs_name) \
    int fs_name##_write(file_t* file, const void* buf, size_t len)
#define _vfs_read(fs_name) \
    int fs_name##_read(file_t* file, void* buf, size_t len)
#define _vfs_open(fs_name) \
    int fs_name##_open(vnode_t* file_node, file_t* target)
#define _vfs_close(fs_name) \
    int fs_name##_close(file_t* file)
#define _vfs_lseek64(fs_name) \
    long fs_name##_lseek64(file_t* file, long offset, int whence)

/* vnode operations */
#define _vfs_lookup(fs_name) \
    int fs_name##_lookup(vnode_t* dir_node, vnode_t** target, const char* component_name)
#define _vfs_create(fs_name) \
    int fs_name##_create(vnode_t* dir_node, vnode_t** target, const char* component_name)
#define _vfs_mkdir(fs_name) \
    int fs_name##_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name)
/* extend vnode operations */
#define _vfs_get_size(fs_name) \
    int fs_name##_get_size(vnode_t* dir_node)
#define _vfs_get_name(fs_name) \
    int fs_name##_get_name(vnode_t* dir_node, char* buf)

void vfs_init();
void rootfs_init(filesystem_t* fs);
int register_filesystem(filesystem_t* fs);
int vfs_open(const char* pathname, int flags, file_t* target);
int vfs_close(file_t* file);
int vfs_write(file_t* file, const void* buf, size_t len);
int vfs_read(file_t* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, vnode_t** target);

#endif