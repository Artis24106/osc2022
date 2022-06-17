#include "fs/tmpfs.h"

static filesystem_t tmpfs = {
    .name = "tmpfs",
    .setup_mount = tmpfs_setup_mount,
    .alloc_root = tmpfs_alloc_root,
};

/* declare tmpfs_f_ops, tmpfs_v_ops */
declare_ops(tmpfs);

/* filesystem init */
_vfs_init(tmpfs) {
    return &tmpfs;
}

/* filesystem operations */
_vfs_setup_mount(tmpfs) {
    vnode_t* curr_node = mount->root;
    char* name;

    curr_node->v_ops->get_name(curr_node, name);

    tmpfs_dir_t* tmp_d = kmalloc(sizeof(tmpfs_dir_t));
    tmp_d->size = 0;

    // backup curr_node
    vnode_t* node_bk = kmalloc(sizeof(vnode_t));
    node_bk->mount = curr_node->mount;
    node_bk->v_ops = curr_node->v_ops;
    node_bk->f_ops = curr_node->f_ops;
    node_bk->internal = curr_node->internal;

    tmpfs_internal_t* tmp_int = kmalloc(sizeof(tmpfs_internal_t));
    strcpy(tmp_int->name, name);
    tmp_int->type = TMPFS_TYPE_DIR;
    tmp_int->data.dir = tmp_d;
    tmp_int->old_node = node_bk;

    // generate new mount infomation
    curr_node->mount = mount;
    curr_node->v_ops = &tmpfs_v_ops;
    curr_node->f_ops = &tmpfs_f_ops;
    curr_node->internal = tmp_int;

    return 0;
}

_vfs_alloc_root(tmpfs) {
    tmpfs_dir_t* tmp_d = kmalloc(sizeof(tmpfs_dir_t));
    tmp_d->size = 0;

    tmpfs_internal_t* tmp_int = kmalloc(sizeof(tmpfs_internal_t));
    *tmp_int->name = '\0';
    tmp_int->type = TMPFS_TYPE_DIR;
    tmp_int->data.dir = tmp_d;
    tmp_int->old_node = NULL;

    vnode_t* node = kmalloc(sizeof(vnode_t));
    node->f_ops = &tmpfs_f_ops;
    node->v_ops = &tmpfs_v_ops;
    node->mount = NULL;
    node->internal = tmp_int;

    *root = node;
    return 0;
}

/* file operations */
_vfs_write(tmpfs) {
    tmpfs_internal_t* tmp_int = file->vnode->internal;

    // can only write TMPFS_TYPE_FILE
    if (tmp_int->type != TMPFS_TYPE_FILE) return -1;

    tmpfs_file_t* tmp_f = tmp_int->data.file;

    // should not write out of range (capacity)
    if (file->f_pos + len > tmp_f->capacity) {
        len = tmp_f->capacity - file->f_pos;
    }

    memcpy(tmp_f->data + file->f_pos, buf, len);

    file->f_pos += len;

    // update size if needed
    if (file->f_pos > tmp_f->size) tmp_f->size = file->f_pos;

    return len;
}

_vfs_read(tmpfs) {
    tmpfs_internal_t* tmp_int = file->vnode->internal;

    // can only read TMPFS_TYPE_FILE
    if (tmp_int->type != TMPFS_TYPE_FILE) return -1;

    tmpfs_file_t* tmp_f = tmp_int->data.file;

    // should not read out of range
    if (file->f_pos + len > tmp_f->size) {
        len = tmp_f->size - file->f_pos;
    }

    memcpy(buf, tmp_f->data + file->f_pos, len);

    file->f_pos += len;

    return len;
}

_vfs_open(tmpfs) {
    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;
    target->f_flags = 0;

    return 0;
}

_vfs_close(tmpfs) {
    // clear the file handle
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

_vfs_lseek64(tmpfs) {
    // check file size
    int size = file->vnode->v_ops->get_size(file->vnode);
    if (size < 0) return -1;

    // calculate new_offset
    long new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = 0 + offset;
            break;
        case SEEK_CUR:
            new_offset = file->f_pos + offset;
            break;
        case SEEK_END:
            new_offset = size + offset;
            break;
        default:
            return -1;
    }

    // check if new_offset is valid
    if (new_offset > size) return -1;

    // update f_pos
    file->f_pos = new_offset;

    return 0;
}

/* vnode operations */
_vfs_lookup(tmpfs) {
    tmpfs_internal_t* tmp_int = dir_node->internal;

    // can only get size from TMPFS_TYPE_FILE
    if (tmp_int->type != TMPFS_TYPE_DIR) return -1;

    tmpfs_dir_t* tmp_d = tmp_int->data.dir;

    // traverse all entries
    vnode_t* node = NULL;
    char* curr_name;
    for (int i = 0; i < tmp_d->size; i++) {
        node = tmp_d->entries[i];

        if (node->v_ops->get_name(node, &curr_name) < 0) continue;

        if (!strcmp(curr_name, component_name)) break;
        node = NULL;
    }
    if (!node) return -1;

    *target = node;
    return 0;
}

_vfs_create(tmpfs) {
    // component name won't execeed 15 characters
    if (strlen(component_name) >= COMP_MAXLEN) return -1;

    // should create component at TMPFS_TYPE_DIR
    tmpfs_internal_t* tmp_int = dir_node->internal;
    if (tmp_int->type != TMPFS_TYPE_DIR) return -1;

    // can't have more than 0x10 entries
    tmpfs_dir_t* tmp_d = tmp_int->data.dir;
    if (tmp_d->size >= DIR_MAX_ENTRIES) return -1;

    // if the component exist, fail
    vnode_t* node;
    if (!tmpfs_lookup(dir_node, &node, component_name)) return -1;

    // start creating the file
    tmpfs_file_t* new_f = kmalloc(sizeof(tmpfs_file_t));
    tmp_int = kmalloc(sizeof(tmpfs_internal_t));
    node = kmalloc(sizeof(vnode_t));

    // generate new tmpfs file
    new_f->data = kmalloc(FILE_MAX_CAPACITY);
    new_f->size = 0;
    new_f->capacity = FILE_MAX_CAPACITY;

    // generate new tmpfs file internal
    strcpy(tmp_int->name, component_name);
    tmp_int->type = TMPFS_TYPE_FILE;
    tmp_int->data.file = new_f;
    tmp_int->old_node = NULL;

    // generate new vnode
    node->mount = dir_node->mount;
    node->f_ops = &tmpfs_f_ops;
    node->v_ops = &tmpfs_v_ops;
    node->parent = dir_node;
    node->internal = tmp_int;

    // add new node to entry
    tmp_d->entries[tmp_d->size] = node;
    tmp_d->size += 1;

    // return the new vnode
    *target = node;

    return 0;
}

_vfs_mkdir(tmpfs) {
    // component name won't execeed 15 characters
    if (strlen(component_name) >= COMP_MAXLEN) return -1;

    // should create component at TMPFS_TYPE_DIR
    tmpfs_internal_t* tmp_int = dir_node->internal;
    if (tmp_int->type != TMPFS_TYPE_DIR) return -1;

    // can't have more than 0x10 entries
    tmpfs_dir_t* tmp_d = tmp_int->data.dir;
    if (tmp_d->size >= DIR_MAX_ENTRIES) return -1;

    // if the component exist, fail
    vnode_t* node;
    if (!tmpfs_lookup(dir_node, &node, component_name)) return -1;

    // start creating the dir
    tmpfs_dir_t* new_d = kmalloc(sizeof(tmpfs_dir_t));
    tmp_int = kmalloc(sizeof(tmpfs_internal_t));
    node = kmalloc(sizeof(vnode_t));

    // generate new tmpfs file
    new_d->size = 0;

    // generate new tmpfs file internal
    strcpy(tmp_int->name, component_name);
    tmp_int->type = TMPFS_TYPE_DIR;
    tmp_int->data.file = new_d;
    tmp_int->old_node = NULL;

    // generate new vnode
    node->mount = dir_node->mount;
    node->f_ops = &tmpfs_f_ops;
    node->v_ops = &tmpfs_v_ops;
    node->parent = dir_node;
    node->internal = tmp_int;

    // add new node to entry
    tmp_d->entries[tmp_d->size] = node;
    tmp_d->size += 1;

    // return the new vnode
    *target = node;

    return 0;
}

_vfs_get_size(tmpfs) {
    tmpfs_internal_t* tmp_int = dir_node->internal;

    // can only get size from TMPFS_TYPE_FILE
    if (tmp_int->type != TMPFS_TYPE_FILE) return -1;

    tmpfs_file_t* tmp_f = tmp_int->data.file;

    return tmp_f->size;
}

_vfs_get_name(tmpfs) {
    tmpfs_internal_t* tmp_int = dir_node->internal;
    // strcpy(buf, tmp_int->name);
    *buf = tmp_int->name;
    return strlen(*buf);
}

_vfs_is_dir(tmpfs) {
    tmpfs_internal_t* tmp_int = node->internal;
    return tmp_int->type == TMPFS_TYPE_DIR;
}

_vfs_is_file(tmpfs) {
    tmpfs_internal_t* tmp_int = node->internal;
    return tmp_int->type == TMPFS_TYPE_FILE;
}

_vfs_show_vnode(tmpfs) {
    tmpfs_internal_t* tmp_int = node->internal;
    pad(layer);
    printf("name = \"%s\", mount = 0x%X, ", tmp_int->name, node->mount);
    if (tmp_int->type == TMPFS_TYPE_DIR) {
        printf("type = DIR (%d)" ENDL, tmp_int->data.dir->size);
        for (int i = 0; i < tmp_int->data.dir->size; i++) tmpfs_show_vnode(tmp_int->data.dir->entries[i], layer + 1);
    } else if (tmp_int->type == TMPFS_TYPE_FILE) {
        printf("type = FILE \"%s\" (%d)" ENDL, tmp_int->data.file->data, tmp_int->data.file->size);
    } else {
        printf("type = UNKNOWN" ENDL);
    }

    // printf(" - parent = 0x%X" ENDL, node->parent);
}