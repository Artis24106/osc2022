#ifndef __VFS_TYPE_H__
#define __VFS_TYPE_H__

#include "list.h"

/* whence values for lseek */
#define SEEK_SET 0x0  // the offset is set to `offset` bytes
#define SEEK_CUR 0x1  // the offset is set to its current location plus `offset` bytes
#define SEEK_END 0x2  // the offset is set to the size of the file plus offset bytes

/* open */
#define O_CREAT 00000100

// https://sites.uclouvain.be/SystInfo/usr/include/asm-generic/fcntl.h.html

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
    list_head_t node;
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
    int (*get_name)(vnode_t* dir_node, char** buf);
    bool (*is_dir)(vnode_t* node);
    bool (*is_file)(vnode_t* node);
    void (*show_vnode)(vnode_t* node, int layer);
} vnode_operations_t;

#endif