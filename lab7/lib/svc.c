#include "svc.h"

void (*sys_tbl[SYS_NUM])(void*) = {
    sys_getpid,
    sys_uartread,
    sys_uartwrite,
    sys_uartread,
    sys_exec,
    sys_fork,
    sys_exit,
    sys_mbox_call,
    sys_kill
    //
};

int32_t sys_getpid() {
    // show_q();
    // printf("[DEBUG] sys_getpid()" ENDL);
    return current->pid;
}

int64_t sys_uartread(char buf[], int64_t size) {
    // printf("[DEBUG] sys_uartread()" ENDL);
    if (size == 0) return -1;

    // async_uart_read(buf, size);
    uart_read(buf, size);
    return size;
}

int64_t sys_uartwrite(const char buf[], int64_t size) {
    if (size == 0) return -1;
    // async_uart_putc(buf, size);
    uart_putc(buf, size);
    return size;
}

int32_t sys_exec(trap_frame_t* tf, const char* name, char* const argv[]) {
    // printf("[DEBUG] sys_exec(%s)" ENDL, name);
    void* file_ptr = cpio_get_file(name);
    if (file_ptr == NULL) {
        printf("[-] %s not found??" ENDL, name);
        while (1)
            ;
    }
    current->pid = current_pid++;
    tf->x30 = file_ptr;  // x30 is stored for fork

    tf->sp_el0 = current->user_stack;
    tf->sp_el0 += THREAD_STACK_SIZE;

    // reset sighand
    reset_signal(current->signal);
    reset_sighand(current->sighand);

    return 0;
}

int32_t sys_fork(trap_frame_t* tf) {
    // printf("[DEBUG] sys_fork()" ENDL);

    // fork
    return _fork(tf);
}

void sys_exit(int32_t status) {
    // printf("[DEBUG] sys_exit()" ENDL);
    thread_release(current, status);
}

int32_t sys_mbox_call(uint8_t ch, mail_t* mbox) {
    return mbox_call(mbox, ch);
}

void sys_kill(int32_t pid) {
    // printf("[DEBUG] sys_kill(%d)" ENDL, pid);
    task_struct_t* target = get_task_by_pid(pid);
    if (!target) return;

    thread_release(target, EX_KILLED);
}

void sys_signal(int32_t signal, void (*handler)()) {
    printf("signal -> %s(%d), handler = 0x%X" ENDL, signal_to_str(signal), signal, handler);

    if (signal >= SIGRTMAX) {
        printf("[-] Invalid Signal" ENDL);
        return;
    }

    printf("curr pid = %d" ENDL, current->pid);
    sigaction_t* sigaction = &current->sighand->sigactions[signal];

    sigaction->is_kernel_hand = true;
    switch ((uint64_t)handler) {
        case SIGDFL:
            sigaction->sa_handler = sig_terminate;
            break;
        case SIGIGN:
            sigaction->sa_handler = sig_ignore;
            break;
        default:  // user signal handler
            sigaction->is_kernel_hand = false;
            sigaction->sa_handler = handler;
            break;
    }
}

void sys_sigkill(int32_t pid, int signal) {
    // printf("sigkill(%d, %d)" ENDL, pid, signal);

    // find the task
    task_struct_t* target = get_task_by_pid(pid);
    if (!target) return;  // TODO: should "not running" task be killed?

    // assign `signal` to the task
    signal_t* new_sig = kmalloc(sizeof(signal_t));
    new_sig->signum = signal;
    list_add_tail(new_sig, &target->signal->node);
}

void sys_sigreturn(trap_frame_t* tf) {
    memcpy(tf, tf->sp_el0, sizeof(trap_frame_t));
}

int sys_open(trap_frame_t* tf, const char* pathname, int flags) {
    printf("sys_open(\"%s\", 0x%X)" ENDL, pathname, flags);

    // rootfs->root->v_ops->show_vnode(rootfs->root, 0);
    task_struct_t* curr = current;
    for (int i = 0; i < curr->max_fd; i++) {
        if (curr->fds[i].vnode != NULL) continue;
        // find one fd to assign
        int ret = vfs_open(pathname, flags, &curr->fds[i]);

        // if open failed
        if (ret < 0) {
            curr->fds[i].vnode = NULL;
            printf("OPEN_FAILED!!!!!!!!!!!!!!!!!" ENDL);
            return ret;
        }
        // tf->x0 = i;
        return i;
    }

    // if all fds are full, no fd to open
    return -1;
}

int sys_close(trap_frame_t* tf, int fd) {
    printf("sys_close(%d)" ENDL, fd);

    task_struct_t* curr = current;
    // check if fd is invalid
    if (fd < 0) return -1;
    // if (current->fds[fd].vnode == NULL) return -1;
    if (curr->fds[fd].vnode == NULL) return -1;

    file_t* f = &curr->fds[fd];
    // int ret = vfs_close(&current->fds[fd]);
    int ret = vfs_close(f);
    if (ret < 0) return ret;

    return 0;

}  // 12

int sys_write(trap_frame_t* tf, int fd, const void* buf, unsigned long count) {
    printf("sys_write(%d, \"%s\", 0x%X)" ENDL, fd, buf, count);
    if (fd < 0) return -1;

    task_struct_t* curr = current;
    if (curr->fds[fd].vnode == NULL) return -1;

    int ret = vfs_write(&curr->fds[fd], buf, count);

    return ret;
}

int sys_read(trap_frame_t* tf, int fd, void* buf, unsigned long count) {
    printf("sys_read(%d, 0x%X, 0x%X)" ENDL, fd, buf, count);
    if (fd < 0) return -1;

    task_struct_t* curr = current;
    if (curr->fds[fd].vnode == NULL) return -1;

    int ret = vfs_read(&curr->fds[fd], buf, count);

    return ret;
}

int sys_mkdir(trap_frame_t* tf, const char* pathname, unsigned mode) {
    printf("sys_mkdir(\"%s\", 0x%X)" ENDL, pathname, mode);

    int ret = vfs_mkdir(pathname);
    return ret;
}

int sys_mount(trap_frame_t* tf, const char* src, const char* target, const char* filesystem, unsigned long flags, const void* data) {
    rootfs->root->v_ops->show_vnode(rootfs->root, 0);
    printf("sys_mount(%s,\"%s\", \"%s\", 0x%X, \"%s\")" ENDL, src, target, filesystem, flags, data);
    rootfs->root->v_ops->show_vnode(rootfs->root, 0);

    int ret = vfs_mount(target, filesystem);

    return ret;
}  // 16

int sys_chdir(trap_frame_t* tf, const char* path) {
    printf("sys_chdir(\"%s\")" ENDL, path);

    // find the node to path
    vnode_t* target;
    int ret = vfs_lookup(path, &target);
    if (ret < 0) return ret;

    // the target should be DIR
    if (!target->v_ops->is_dir(target)) return -1;

    // set current working directory
    current->work_dir = target;

    return 0;
}

long sys_lseek64(trap_frame_t* tf, int fd, long offset, int whence) {
    printf("sys_lseek64(%d, %d, %d)" ENDL, fd, offset, whence);
}

int sys_ioctl(trap_frame_t* tf, int fd, unsigned long request, ...) {
    printf("sys_ioctl(%d, %d)" ENDL, fd, request);
}

void svc_handler(trap_frame_t* tf) {  // handle svc0
    // printf("svc_handler()" ENDL);

    uint32_t esr_el1 = read_sysreg(esr_el1);

    // printf("esr_el1 = 0x%X\n", esr_el1);

    // EC (Exception Class) should be SVC instruction
    if ((esr_el1 >> 26) != 0b010101) {
        printf("[-] Not svc!!" ENDL);
        invalid_handler(8);
    }

    /*  lab5 svc calling convention
        @args: x0, x1, ...
        @ret: x0
        @sys_num: x8
    */
    uint64_t sys_num = tf->x8;
    // printf("sys_num = %d" ENDL, sys_num);
    if (sys_num >= SYS_NUM) {
        printf("[-] Invalid syscall number %d" ENDL, sys_num);
        invalid_handler(8);
    }

    enable_intr();
    switch (sys_num) {
        case 0:
            sys_getpid();
            break;
        case 1:
            // printf("0x%X (0x%X), 0x%X" ENDL, tf->x0, *(char*)tf->x0, tf->x1);
            sys_uartread(tf->x0, tf->x1);
            // printf(" \b");
            // printf("read 0x%X (0x%X), 0x%X" ENDL, tf->x0, *(char*)tf->x0, tf->x1);
            // for (uint32_t i = 0; i < 0x100000; i++) asm("nop");
            break;
        case 2:
            sys_uartwrite(tf->x0, tf->x1);
            // printf(" \b");
            // printf("write 0x%X (0x%X), 0x%X" ENDL, tf->x0, *(char*)tf->x0, tf->x1);
            break;
        case 3:
            sys_exec(tf, tf->x0, tf->x1);
            break;
        case 4:
            sys_fork(tf);
            break;
        case 5:
            sys_exit(tf->x0);
            break;
        case 6:
            sys_mbox_call(tf->x0, tf->x1);
            // show_q();
            break;
        case 7:
            sys_kill(tf->x0);
            break;
        case 8:  // signal(int SIGNAL, void (*handler)())
            sys_signal(tf->x0, tf->x1);
            break;
        case 9:  // kill(int pid, int SIGNAL)
            sys_sigkill(tf->x0, tf->x1);
            break;
        case 10:
            sys_sigreturn(tf);
            break;
        case 11:
            sys_open(tf, tf->x0, tf->x1);
            break;
        case 12:
            sys_close(tf, tf->x0);
            break;
        case 13:
            sys_write(tf, tf->x0, tf->x1, tf->x2);
            break;
        case 14:
            sys_read(tf, tf->x0, tf->x1, tf->x2);
            break;
        case 15:
            sys_mkdir(tf, tf->x0, tf->x1);
            break;
        case 16:
            sys_mount(tf, tf->x0, tf->x1, tf->x2, tf->x3, tf->x4);
            break;
        case 17:
            sys_chdir(tf, tf->x0);
            break;
        case 18:
            sys_lseek64(tf, tf->x0, tf->x1, tf->x2);
            break;
        case 19:
            sys_ioctl(tf, tf->x0, tf->x1, tf->x2);
            break;
        default:
            break;
    }
    disable_intr();
    // sys_tbl[sys_num](tf->x0);  // syscall
    // printf("\n");
}
