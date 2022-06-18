#include "fs/cpiofs.h"

static filesystem_t cpiofs = {
    .name = "cpiofs",
    .alloc_root = cpiofs_alloc_root,
    .setup_mount = cpiofs_setup_mount,
};

static vnode_t cpiofs_root_node;
static vnode_t cpiofs_mount_node_bk;
static bool cpiofs_mounted;

/* declare cpiofs_f_ops, cpiofs_v_ops */
declare_ops(cpiofs);

// XXX: FIX!!!!!!!!!!
vnode_t* get_dir_node_by_name(vnode_t* dir_node, char** path_name) {
    struct vnode* result;
    const char* start;
    const char* end;
    char buf[0x100];

    start = end = *path_name;

    if (*start == '/') {
        result = &cpiofs_root_node;
    } else {
        result = dir_node;
    }

    while (1) {
        if (!strncmp("./", start, 2)) {
            start += 2;
            end = start;
            continue;
        } else if (!strncmp("../", start, 3)) {
            if (result->parent) {
                result = result->parent;
            }

            start += 3;
            end = start;
            continue;
        }

        // Find next component
        while (*end != '\0' && *end != '/') {
            end++;
        }

        if (*end != '/') break;
        int ret;

        if (start == end) {
            // Case like "//"
            end++;
            start = end;
            continue;
        }

        // TODO: Check if the length is less than 0x100
        memcpy(buf, start, end - start);
        buf[end - start] = 0;

        ret = result->v_ops->lookup(result, &result, buf);

        if (ret < 0) {
            return NULL;
        }

        end++;
        start = end;
    }

    *path_name = *start ? start : NULL;

    return result;
}

/* filesystem init */
_vfs_init(cpiofs) {
    // init cpiofs_root_node
    cpiofs_alloc_root(NULL, NULL);

    cpiofs_root_node.v_ops->show_vnode(&cpiofs_root_node, 0);

    // parse cpio
    char *cpio_ptr = INITRD_START,
         *curr_file_name, *curr_file_data;
    uint32_t name_size, data_size;
    uint32_t header_size = sizeof(cpio_newc_header_t);
    cpio_newc_header_t* header;

    CPIOFS_TYPE_T file_type;
    void* file_buf = NULL;

    const char* magic = "070701";
    while (1) {
        // parse header
        header = cpio_ptr;
        if (strncmp(header, magic, 6) != 0) {
            printf("[-] Only support cpio newc format!" ENDL);
            while (1)
                ;
        }
        cpio_ptr += header_size;

        // get name_size and data_size
        name_size = hex_ascii_to_uint32(header->c_namesize, sizeof(header->c_namesize));
        data_size = hex_ascii_to_uint32(header->c_filesize, sizeof(header->c_filesize));
        file_type = hex_ascii_to_uint32(header->c_mode, sizeof(header->c_mode)) & CPIOFS_TYPE_DIR;

        // parse file name
        curr_file_name = cpio_ptr;
        while ((name_size + header_size) % 4 != 0) name_size += 1;  // balance the offset
        cpio_ptr += name_size;

        // parse file data
        curr_file_data = cpio_ptr;
        while (((uint64_t)data_size % 4) != 0) data_size += 1;  // balance the offset
        cpio_ptr += data_size;

        //
        if (strcmp(curr_file_name, "TRAILER!!!") == 0) break;

        if (file_type == CPIOFS_TYPE_DIR) {  //
            cpiofs_mkdir(curr_file_name, NULL, NULL);
        } else if (file_type == CPIOFS_TYPE_FILE) {
            cpiofs_create(curr_file_name, curr_file_data, &data_size);
        } else {
            printf("[-] Unkown cpio file type!" ENDL);
            while (1)
                ;
        }
    }
    cpiofs_root_node.v_ops->show_vnode(&cpiofs_root_node, 0);

    return &cpiofs;
}

/* filesystem operations */
_vfs_setup_mount(cpiofs) {
    vnode_t* curr_node = mount->root;

    char* name;
    curr_node->v_ops->get_name(curr_node, &name);

    cpiofs_dir_t* tmp_d = kmalloc(sizeof(cpiofs_dir_t));

    cpiofs_internal_t* tmp_int = cpiofs_root_node.internal;
    tmp_int->name = name;

    // backup curr_node for umount
    cpiofs_mount_node_bk.mount = curr_node->mount;
    cpiofs_mount_node_bk.v_ops = curr_node->v_ops;
    cpiofs_mount_node_bk.f_ops = curr_node->f_ops;
    cpiofs_mount_node_bk.parent = curr_node->parent;
    cpiofs_mount_node_bk.internal = curr_node->internal;

    // change mounted vnode info
    curr_node->mount = mount;
    curr_node->v_ops = cpiofs_root_node.v_ops;
    curr_node->f_ops = cpiofs_root_node.f_ops;
    curr_node->internal = tmp_int;

    // set mounted
    cpiofs_mounted = true;

    return 0;
}
_vfs_alloc_root(cpiofs) {
    // setup the internal for root
    cpiofs_internal_t* root_int = kmalloc(sizeof(cpiofs_internal_t));
    root_int->name = NULL;
    root_int->type = CPIOFS_TYPE_DIR;
    INIT_LIST_HEAD(&root_int->data.dir.node);
    root_int->vnode = &cpiofs_root_node;
    INIT_LIST_HEAD(&root_int->node);

    cpiofs_root_node.mount = NULL;
    cpiofs_root_node.f_ops = &cpiofs_f_ops;
    cpiofs_root_node.v_ops = &cpiofs_v_ops;
    cpiofs_root_node.parent = NULL;
    cpiofs_root_node.internal = root_int;
}

/* file operations */
_vfs_write(cpiofs) {
    return -1;  // read-only
}

_vfs_read(cpiofs) {
    cpiofs_internal_t* tmp_int = file->vnode->internal;

    // can only read from CPIOFS_TYPE_FILE
    if (tmp_int->type != CPIOFS_TYPE_FILE) return -1;

    cpiofs_file_t* tmp_f = &tmp_int->data.file;

    // should not read out of range
    if (file->f_pos + len > tmp_f->size) {
        len = tmp_f->size - file->f_pos;
    }

    memcpy(buf, tmp_f->data + file->f_pos, len);

    file->f_pos += len;

    return len;
}

_vfs_open(cpiofs) {
    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}

_vfs_close(cpiofs) {
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

_vfs_lseek64(cpiofs) {
    int file_size = file->vnode->v_ops->get_size(file->vnode);
    if (file_size < 0) return -1;

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
            new_offset = file_size + offset;
            break;
        default:
            return -1;
    }

    // check if new_offset is valid
    if (new_offset > file_size) return -1;

    // update f_pos
    file->f_pos = new_offset;

    return 0;
}

/* vnode operations */
_vfs_lookup(cpiofs) {
    // // can only be used in initialization
    // if (cpiofs_mounted) return -1;

    cpiofs_internal_t* tmp_int = dir_node->internal;

    // should find data in CPIOFS_TYPE_DIR
    if (tmp_int->type != CPIOFS_TYPE_DIR) return -1;

    // iterate the whole tmp_int
    cpiofs_internal_t* curr;
    list_for_each_entry(curr, &tmp_int->data.dir.node, node) {
        if (strcmp(curr->name, component_name) == 0) {
            // found!
            *target = curr->vnode;
            return 0;
        }
    }

    return -1;
}

_vfs_create(cpiofs) {
    // can only be used in initialization
    if (cpiofs_mounted) return -1;

    // XXX: garbage code QQ
    char* file_name = (char*)dir_node;
    char* file_data = (char*)target;
    uint32_t data_size = *(uint32_t*)component_name;
    if (data_size < 0x50)
        printf("[CPIOFS] file_name = '%s', data = '%s', data_size = 0x%X" ENDL, file_name, file_data, data_size);

    if (!file_name) return -1;

    // get the directory node to create new file
    vnode_t* curr_dir_node = get_dir_node_by_name(&cpiofs_root_node, &file_name);
    if (!curr_dir_node) return -1;

    // should be directory
    cpiofs_internal_t* dir_int = curr_dir_node->internal;
    if (dir_int->type != CPIOFS_TYPE_DIR) return -1;

    cpiofs_internal_t* tmp_int = kmalloc(sizeof(cpiofs_internal_t));
    vnode_t* tmp_node = kmalloc(sizeof(vnode_t));

    tmp_int->name = file_name;
    tmp_int->type = CPIOFS_TYPE_FILE;
    tmp_int->data.file.data = file_data;  // read-only, so simply assign pointer
    tmp_int->data.file.size = data_size;
    tmp_int->vnode = tmp_node;

    tmp_node->mount = NULL;
    tmp_node->v_ops = &cpiofs_v_ops;
    tmp_node->f_ops = &cpiofs_f_ops;
    tmp_node->parent = curr_dir_node;
    tmp_node->internal = tmp_int;

    list_add_tail(&tmp_int->node, &dir_int->data.dir.node);
}

_vfs_mkdir(cpiofs) {
    // can only be used in initialization
    if (cpiofs_mounted) return -1;

    // XXX: garbage code QQ
    char* dir_name = (char*)dir_node;
    printf("[CPIOFS] dir_name = '%s'" ENDL, dir_name);

    if (!dir_name) return -1;

    // get the directory node to create new dir
    vnode_t* curr_dir_node = get_dir_node_by_name(&cpiofs_root_node, &dir_name);
    if (!curr_dir_node) return -1;

    // should be directory
    cpiofs_internal_t* dir_int = curr_dir_node->internal;
    if (dir_int->type != CPIOFS_TYPE_DIR) return -1;

    cpiofs_internal_t* tmp_int = kmalloc(sizeof(cpiofs_internal_t));
    vnode_t* tmp_node = kmalloc(sizeof(vnode_t));

    tmp_int->name = dir_name;
    tmp_int->type = CPIOFS_TYPE_DIR;
    INIT_LIST_HEAD(&tmp_int->data.dir.node);
    tmp_int->vnode = tmp_node;

    tmp_node->mount = NULL;
    tmp_node->v_ops = &cpiofs_v_ops;
    tmp_node->f_ops = &cpiofs_f_ops;
    tmp_node->parent = curr_dir_node;
    tmp_node->internal = tmp_int;

    // add to the directory
    list_add_tail(&tmp_int->node, &dir_int->data.dir.node);
    cpiofs_root_node.v_ops->show_vnode(&cpiofs_root_node, 0);
}

_vfs_get_size(cpiofs) {
    cpiofs_internal_t* tmp_int = dir_node->internal;

    // can only get size from CPIOFS_TYPE_FILE type
    if (tmp_int->type != CPIOFS_TYPE_FILE) return -1;

    return tmp_int->data.file.size;
}

_vfs_get_name(cpiofs) {
    cpiofs_internal_t* tmp_int = dir_node->internal;

    *buf = tmp_int->name;
    return strlen(*buf);
}

_vfs_is_dir(cpiofs) {
    cpiofs_internal_t* tmp_int = node->internal;
    return tmp_int->type == CPIOFS_TYPE_DIR;
}

_vfs_is_file(cpiofs) {
    cpiofs_internal_t* tmp_int = node->internal;
    return tmp_int->type == CPIOFS_TYPE_FILE;
}

_vfs_show_vnode(cpiofs) {
    cpiofs_internal_t* tmp_int = node->internal;
    if (layer == 0) printf("\x1b[38;5;4m===== show_vnode() =====\x1b[0m" ENDL);
    pad(layer);
    if (tmp_int->type == CPIOFS_TYPE_DIR) {
        printf("\x1b[38;5;1m[CPIO_DIR]\x1b[0m -> \x1b[38;5;2m\"%s\"\x1b[0m" ENDL, tmp_int->name);
        cpiofs_internal_t* curr;
        list_for_each_entry(curr, &tmp_int->data.dir.node, node) {
            curr->vnode->v_ops->show_vnode(curr->vnode, layer + 1);  // off = 0x28
        }
    } else if (tmp_int->type == CPIOFS_TYPE_FILE) {
        printf("\x1b[38;5;4m[CPIO_FILE]\x1b[0m \x1b[38;5;2m\"%s\"\x1b[0m -> \x1b[38;5;2m\"%s\"\x1b[0m (%d) - 0x%X" ENDL, tmp_int->name, tmp_int->data.file.data, tmp_int->data.file.size, node->mount);
    } else {
        printf("[CPIO_UNKNOWN]" ENDL);
    }
}