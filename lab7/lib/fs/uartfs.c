#include "fs/uartfs.h"

static filesystem_t uartfs = {
    .name = "uartfs",
    .setup_mount = uartfs_setup_mount,
    .alloc_root = uartfs_alloc_root,
};

/* declare uartfs_f_ops, uartfs_v_ops */
declare_ops(uartfs);

/* filesystem init */
_vfs_init(uartfs) { return &uartfs; }

/* filesystem operations */
_vfs_setup_mount(uartfs) {
    vnode_t* curr_node = mount->root;
    char* name;
    curr_node->v_ops->get_name(curr_node, &name);

    // backup curr_node
    vnode_t* node_bk = kmalloc(sizeof(vnode_t));
    node_bk->mount = curr_node->mount;
    node_bk->v_ops = curr_node->v_ops;
    node_bk->f_ops = curr_node->f_ops;
    node_bk->internal = curr_node->internal;

    uartfs_internal_t* tmp_int = kmalloc(sizeof(uartfs_internal_t));
    tmp_int->name = kmalloc(sizeof(name));
    strcpy(tmp_int->name, name);
    tmp_int->old_node = node_bk;

    // new mount info
    curr_node->mount = mount;
    curr_node->v_ops = &uartfs_v_ops;
    curr_node->f_ops = &uartfs_f_ops;
    curr_node->internal = tmp_int;

    return 0;
}
_vfs_alloc_root(uartfs) { return -1; }

/* file operations */
_vfs_write(uartfs) {
    uart_putc(buf, len);

    return len;
}

_vfs_read(uartfs) {
    uart_read(buf, len);

    return len;
}

_vfs_open(uartfs) {
    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;
    target->f_flags = 0;

    return 0;
}

_vfs_close(uartfs) {
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

_vfs_lseek64(uartfs) { return -1; }
_vfs_ioctl(uartfs) { return -1; }

/* vnode operations */
_vfs_lookup(uartfs) { return -1; }
_vfs_create(uartfs) { return -1; }
_vfs_mkdir(uartfs) { return -1; }
_vfs_get_size(uartfs) { return -1; }

_vfs_get_name(uartfs) {
    uartfs_internal_t* tmp_int = dir_node->internal;
    *buf = tmp_int->name;
    return strlen(*buf);
}

_vfs_is_dir(uartfs) { return -1; }
_vfs_is_file(uartfs) { return -1; }

_vfs_show_vnode(uartfs) {
    uartfs_internal_t* tmp_int = node->internal;
    if (layer == 0) printf("\x1b[38;5;4m===== show_vnode() =====\x1b[0m" ENDL);
    pad(layer);
    printf("\x1b[38;5;4m[UART]\x1b[0m \x1b[38;5;2m\"%s\"\x1b[0m" ENDL, tmp_int->name);
}