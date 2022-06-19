#ifndef __CPIOFS_H__
#define __CPIOFS_H__

#include "cpio.h"
#include "fs/vfs.h"
#include "list.h"

typedef enum CPIOFS_TYPE {
    CPIOFS_TYPE_UNDEFINED,
    CPIOFS_TYPE_DIR = 0x4000,
    CPIOFS_TYPE_FILE = 0x0000,
} CPIOFS_TYPE_T;

typedef struct cpiofs_file {
    const char* data;  // file data must be read-only
    int size;
} cpiofs_file_t;

typedef struct cpiofs_dir {
    list_head_t node;  // link to cpiofs_internal in the directory
} cpiofs_dir_t;

typedef struct cpiofs_internal {
    const char* name;  // file name must be read-only
    CPIOFS_TYPE_T type;
    union cpiofs_data {
        cpiofs_file_t file;
        cpiofs_dir_t dir;
    } data;
    vnode_t* vnode;
    list_head_t node;
} cpiofs_internal_t;

vnode_t* get_dir_node_by_name(vnode_t* dir_node, char** path_name);

/* filesystem init */
_vfs_init(cpiofs);

/* filesystem operations */
_vfs_setup_mount(cpiofs);
_vfs_alloc_root(cpiofs);

/* file operations */
_vfs_write(cpiofs);
_vfs_read(cpiofs);
_vfs_open(cpiofs);
_vfs_close(cpiofs);
_vfs_lseek64(cpiofs);
_vfs_show_vnode(cpiofs);

/* vnode operations */
_vfs_lookup(cpiofs);
_vfs_create(cpiofs);
_vfs_mkdir(cpiofs);
_vfs_get_size(cpiofs);
_vfs_get_name(cpiofs);
_vfs_is_dir(cpiofs);
_vfs_is_file(cpiofs);

#endif