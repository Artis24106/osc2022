#ifndef __VFS_H__
#define __VFS_H__

#include "fs/vfs_type.h"
#include "list.h"
#include "malloc.h"
#include "printf.h"

vnode_t* vfs_get_dir_node_by_name(vnode_t* dir_node, char** path_name);

filesystem_t* vfs_get_filesystem_by_name(const char* fs_name);

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
        gen_ops_param(fs_name, get_name),            \
        gen_ops_param(fs_name, get_size),            \
        gen_ops_param(fs_name, is_dir),              \
        gen_ops_param(fs_name, is_file),             \
        gen_ops_param(fs_name, show_vnode),          \
    }

#define declare_ops(fs_name) \
    gen_f_vops(fs_name);     \
    gen_v_vops(fs_name);

/* filesystem init */
#define _vfs_init(fs_name) \
    filesystem_t* fs_name##_init()

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
    int fs_name##_get_name(vnode_t* dir_node, char** buf)
#define _vfs_is_dir(fs_name) \
    bool fs_name##_is_dir(vnode_t* node)
#define _vfs_is_file(fs_name) \
    bool fs_name##_is_file(vnode_t* node)
#define _vfs_show_vnode(fs_name) \
    void fs_name##_show_vnode(vnode_t* node, int layer)

#define pad(layer)                                                  \
    for (int lcount = 0; lcount < layer * 2; lcount++) printf(" "); \
    printf("\x1b[38;5;3m*\x1b[0m ")

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