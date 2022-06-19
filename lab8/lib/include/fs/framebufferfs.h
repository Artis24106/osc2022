#ifndef __FRAMEBUFFERFS_H__
#define __FRAMEBUFFERFS_H__

#include "fs/vfs.h"
#include "mbox.h"

#define MBOX_REQUEST 0
#define MBOX_CH_PROP 8
#define MBOX_TAG_LAST 0

typedef struct fbfs_internal {
    char* name;
    vnode_t* old_node;

    // uint8_t lfb;           // raw frame buffer address
    uint32_t lfb;          // raw frame buffer address
    uint32_t lfb_size;     // mbox[29] FrameBufferInfo.size
    bool is_opened;        // check if "/dev/framebuffer" is opened
    bool is_ioctl_called;  // check if ioctl is called
} fbfs_internal_t;

typedef struct framebuffer_info {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int isrgb;
} fb_info_t;

/* filesystem init */
_vfs_init(fbfs);

/* filesystem operations */
_vfs_setup_mount(fbfs);
_vfs_alloc_root(fbfs);

/* file operations */
_vfs_write(fbfs);
_vfs_read(fbfs);
_vfs_open(fbfs);
_vfs_close(fbfs);
_vfs_lseek64(fbfs);
_vfs_ioctl(fbfs);

/* vnode operations */
_vfs_lookup(fbfs);
_vfs_create(fbfs);
_vfs_mkdir(fbfs);
_vfs_get_size(fbfs);
_vfs_get_name(fbfs);
_vfs_is_dir(fbfs);
_vfs_is_file(fbfs);
_vfs_show_vnode(fbfs);

#endif