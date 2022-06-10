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
        printf("[-] Invalid syscall number" ENDL);
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
        default:
            break;
    }
    disable_intr();
    // sys_tbl[sys_num](tf->x0);  // syscall
    // printf("\n");
}
