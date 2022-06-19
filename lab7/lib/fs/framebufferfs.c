#include "fs/framebufferfs.h"

static filesystem_t fbfs = {
    .name = "fbfs",
    .setup_mount = fbfs_setup_mount,
    .alloc_root = fbfs_alloc_root,
};

/* declare uartfs_f_ops, uartfs_v_ops */
declare_ops(fbfs);

/* filesystem init */
_vfs_init(fbfs) { return &fbfs; }

/* filesystem operations */
_vfs_setup_mount(fbfs) {
    vnode_t* curr_node = mount->root;
    char* name;
    curr_node->v_ops->get_name(curr_node, &name);

    // backup curur_node
    vnode_t* node_bk = kmalloc(sizeof(vnode_t));
    node_bk->mount = curr_node->mount;
    node_bk->v_ops = curr_node->v_ops;
    node_bk->f_ops = curr_node->f_ops;
    node_bk->internal = curr_node->internal;

    fbfs_internal_t* tmp_int = kmalloc(sizeof(fbfs_internal_t));
    tmp_int->name = kmalloc(sizeof(name));
    strcpy(tmp_int->name, name);
    tmp_int->old_node = node_bk;
    tmp_int->lfb = NULL;
    tmp_int->lfb_size = 0;
    tmp_int->is_opened = false;
    tmp_int->is_ioctl_called = false;

    // new mount info
    curr_node->mount = mount;
    curr_node->v_ops = &fbfs_v_ops;
    curr_node->f_ops = &fbfs_f_ops;
    curr_node->internal = tmp_int;

    return 0;
}

_vfs_alloc_root(fbfs) { return -1; }

/* file operations */
_vfs_write(fbfs) {
    fbfs_internal_t* curr_int = file->vnode->internal;
    if (!curr_int->is_ioctl_called) return -1;

    if (file->f_pos + len > curr_int->lfb_size) return -1;

    memcpy((void*)(curr_int->lfb) + file->f_pos, buf, len);

    file->f_pos += len;

    return len;
}
_vfs_read(fbfs) { return -1; }

_vfs_open(fbfs) {
    fbfs_internal_t* curr_int = file_node->internal;
    if (curr_int->is_opened) return -1;

    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;
    target->f_flags = 0;

    curr_int->is_opened = true;

    return 0;
}

_vfs_close(fbfs) {
    fbfs_internal_t* curr_int = file->vnode->internal;

    // clear the file handle
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    curr_int->is_opened = false;

    return 0;
}

_vfs_lseek64(fbfs) {
    fbfs_internal_t* tmp_int = file->vnode->internal;
    uint32_t size = file->vnode->v_ops->get_size(file->vnode);
    if (size < 0) return -1;

    // calculate new offset
    long new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = 0 + offset;
            break;
        case SEEK_CUR:
            new_offset = file->f_pos + offset;
            break;
        case SEEK_END:
            new_offset = tmp_int->lfb_size + offset;
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

_vfs_ioctl(fbfs) {
    unsigned int __attribute__((aligned(16))) mbox[36];
    unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
    unsigned char* lfb;                       /* raw frame buffer address */

    fbfs_internal_t* curr_int = file->vnode->internal;

    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003;  // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024;  // FrameBufferInfo.width
    mbox[6] = 768;   // FrameBufferInfo.height

    mbox[7] = 0x48004;  // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024;  // FrameBufferInfo.virtual_width
    mbox[11] = 768;   // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009;  // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;  // FrameBufferInfo.x_offset
    mbox[16] = 0;  // FrameBufferInfo.y.offset

    mbox[17] = 0x48005;  // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;  // FrameBufferInfo.depth

    mbox[21] = 0x48006;  // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;  // RGB, not BGR preferably

    mbox[25] = 0x40001;  // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;  // FrameBufferInfo.pointer
    mbox[29] = 0;     // FrameBufferInfo.size

    mbox[30] = 0x40008;  // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;  // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(mbox, MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF;  // convert GPU address to ARM address
        width = mbox[5];         // get actual physical width
        height = mbox[6];        // get actual physical height
        pitch = mbox[33];        // get number of bytes per line
        isrgb = mbox[24];        // get the actual channel order
        lfb = (void*)((unsigned long)mbox[28]);
        curr_int->lfb = mbox[28];
        curr_int->lfb_size = mbox[29];
    } else {
        printf("Unable to set screen resolution to 1024x768x32\n");
        return -1;
    }

    fb_info_t* fb_info = va_arg(args, void*);
    fb_info->width = width;
    fb_info->height = height;
    fb_info->pitch = pitch;
    fb_info->isrgb = isrgb;

    curr_int->is_ioctl_called = true;

    return 0;
}

/* vnode operations */
_vfs_lookup(fbfs) { return -1; }
_vfs_create(fbfs) { return -1; }
_vfs_mkdir(fbfs) { return -1; }
_vfs_get_size(fbfs) {
    fbfs_internal_t* tmp_int = dir_node->internal;
    return tmp_int->lfb_size;
}

_vfs_get_name(fbfs) {
    fbfs_internal_t* tmp_int = dir_node->internal;
    *buf = tmp_int->name;
    return strlen(*buf);
}

_vfs_is_dir(fbfs) { return -1; }
_vfs_is_file(fbfs) { return -1; }

_vfs_show_vnode(fbfs) {
    fbfs_internal_t* tmp_int = node->internal;
    if (layer == 0) printf("\x1b[38;5;4m===== show_vnode() =====\x1b[0m" ENDL);
    pad(layer);
    printf("\x1b[38;5;4m[FRAME_BUFFER]\x1b[0m \x1b[38;5;2m\"%s\"\x1b[0m" ENDL, tmp_int->name);
}