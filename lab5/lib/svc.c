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
    show_q();
    // printf("[DEBUG] sys_getpid()" ENDL);
    return current->pid;
}

int64_t sys_uartread(char buf[], int64_t size) {
    // printf("[DEBUG] sys_uartread()" ENDL);
    if (size == 0) return -1;

    async_uart_read(buf, size);
    return size;
}

int64_t sys_uartwrite(const char buf[], int64_t size) {
    if (size == 0) return -1;
    async_uart_putc(buf, size);
    return size;
}

int32_t sys_exec(trap_frame_t* tf, const char* name, char* const argv[]) {
    printf("[DEBUG] sys_exec(%s)" ENDL, name);
    show_q();
    // cpio_newc_parser_tf(cpio_exec_callback_tf, name, tf);
    cpio_newc_parser(cpio_exec_callback, name);
}

int32_t sys_fork(trap_frame_t* tf) {
    printf("[DEBUG] sys_fork()" ENDL);

    // fork
    return _fork(tf);
}

void sys_exit(int32_t status) {
    printf("[DEBUG] sys_exit()" ENDL);
    thread_release(current, status);
}

int32_t sys_mbox_call(uint8_t ch, mail_t* mbox) {
    printf("[DEBUG] sys_mbox_call(%d, 0x%X)" ENDL, ch, *mbox);
    return mbox_call(mbox, ch);
}

void sys_kill(int32_t pid) {
    printf("[DEBUG] sys_kill()" ENDL);
}

void svc_handler(trap_frame_t* tf) {  // handle svc0
    // printf("svc_handler()" ENDL);

    uint32_t esr_el1 = read_sysreg(esr_el1);

    // printf("esr_el1 = 0x%X\n", esr_el1);

    // EC (Exception Class) should be SVC instruction
    // if ((esr_el1 >> 26) != 0b010101) {
    //     invalid_handler(8);
    // }

    /*  lab5 svc calling convention
        @args: x0, x1, ...
        @ret: x0
        @sys_num: x8
    */
    // ddd();
    uint64_t sys_num = tf->x8;
    // printf("sys_num = %d" ENDL, sys_num);
    if (sys_num >= SYS_NUM) {
        invalid_handler(8);
    }

    enable_intr();
    switch (sys_num) {
        case 0:
            sys_getpid();
            break;
        case 1:
            sys_uartread(tf->x0, tf->x1);
            break;
        case 2:
            sys_uartwrite(tf->x0, tf->x1);
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
            // ddd();
            sys_mbox_call(tf->x0, tf->x1);
            break;
        case 7:
            sys_kill(tf->x0);
            break;
        default:
            break;
    }
    disable_intr();
    // sys_tbl[sys_num](tf->x0);  // syscall
    // printf("\n");
}