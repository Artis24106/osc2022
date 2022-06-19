
#ifndef __UARTFS_H__
#define __UARTFS_H__

#include "fs/vfs.h"
#include "uart.h"

typedef struct uartfs_internal {
    char* name;
    vnode_t* old_node;
} uartfs_internal_t;

/* filesystem init */
_vfs_init(uartfs);

/* filesystem operations */
_vfs_setup_mount(uartfs);
_vfs_alloc_root(uartfs);

/* file operations */
_vfs_write(uartfs);
_vfs_read(uartfs);
_vfs_open(uartfs);
_vfs_close(uartfs);
_vfs_lseek64(uartfs);
_vfs_show_vnode(uartfs);

/* vnode operations */
_vfs_lookup(uartfs);
_vfs_create(uartfs);
_vfs_mkdir(uartfs);
_vfs_get_size(uartfs);
_vfs_get_name(uartfs);
_vfs_is_dir(uartfs);
_vfs_is_file(uartfs);

#endif