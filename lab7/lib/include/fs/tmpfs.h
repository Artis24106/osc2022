#ifndef __TMPFS_H__
#define __TMPFS_H__

#include "fs/vfs.h"
#include "malloc.h"
#include "printf.h"
// #include "string.h"

#define COMP_MAXLEN 0x10      // Component name won't exceed 15 characters
#define DIR_MAX_ENTRIES 0x10  // max entries in dir
#define FILE_MAX_CAPACITY 0x1000

typedef enum TMPFS_TYPE {
    TMPFS_TYPE_UNDEFINED,
    TMPFS_TYPE_DIR,
    TMPFS_TYPE_FILE
} TMPFS_TYPE_T;

typedef struct tmpfs_dir {
    int size;  // size of the entry used
    vnode_t* entries[DIR_MAX_ENTRIES];
} tmpfs_dir_t;

typedef struct tmpfs_file {
    char* data;
    int size;
    int capacity;
} tmpfs_file_t;

typedef struct tmpfs_internal {
    char name[COMP_MAXLEN];
    TMPFS_TYPE_T type;
    union tmpfs_data {
        tmpfs_dir_t* dir;
        tmpfs_file_t* file;
    } data;
    vnode_t* old_node;  // TOOD: ??
} tmpfs_internal_t;

/* filesystem init */
_vfs_init(tmpfs);

/* filesystem operations */
_vfs_setup_mount(tmpfs);
_vfs_alloc_root(tmpfs);

/* file operations */
_vfs_write(tmpfs);
_vfs_read(tmpfs);
_vfs_open(tmpfs);
_vfs_close(tmpfs);
_vfs_lseek64(tmpfs);

/* vnode operations */
_vfs_lookup(tmpfs);
_vfs_create(tmpfs);
_vfs_mkdir(tmpfs);
_vfs_get_size(tmpfs);
_vfs_get_name(tmpfs);
_vfs_is_dir(tmpfs);
_vfs_is_file(tmpfs);

#endif