#include "signal.h"

signal_t* new_signal() {
    signal_t* signal = kmalloc(sizeof(signal_t));
    INIT_LIST_HEAD(&signal->node);
    return signal;
}

sighand_t* new_sighand() {
    sighand_t* sighand = kmalloc(sizeof(sighand_t));

    // set default sig handlers
    reset_sighand(sighand);

    return sighand;
}

void reset_signal(signal_t* signal) {
    signal_t *curr, *temp;
    list_for_each_entry_safe(curr, temp, &signal->node, node) {
        list_del(&curr->node);
        kfree(curr);
    }
}

void reset_sighand(sighand_t* sighand) {
    // set default sig handlers
    for (uint32_t i = 0; i < SIGRTMAX; i++) {
        sighand->sigactions[i].sa_handler = sig_ignore;
        sighand->sigactions[i].is_kernel_hand = true;
    }
    sighand->sigactions[SIGKILL].sa_handler = sig_terminate;
}

void sigreturn() {
    // call syscall 10 to get back user mode
    asm volatile(
        "mov x8, 10\n"
        "svc 0\n");
}

void sig_ignore(int ign) {
    // just do nothing
}

void sig_terminate(int ign) {
    thread_release(get_current(), 1);  // 1 -> EX_KILLED
}

char* signal_to_str(uint32_t signo) {
    switch (signo) {
        case SIGHUP:
            return "SIGHUP";
        case SIGINT:
            return "SIGINT";
        case SIGQUIT:
            return "SIGQUIT";
        case SIGILL:
            return "SIGILL";
        case SIGTRAP:
            return "SIGTRAP";
        case SIGABRT:
            return "SIGABRT";
        case SIGBUS:
            return "SIGBUS";
        case SIGFPE:
            return "SIGFPE";
        case SIGKILL:
            return "SIGKILL";
        case SIGUSR1:
            return "SIGUSR1";
        case SIGSEGV:
            return "SIGSEGV";
        case SIGUSR2:
            return "SIGUSR2";
        case SIGPIPE:
            return "SIGPIPE";
        case SIGALRM:
            return "SIGALRM";
        case SIGTERM:
            return "SIGTERM";
        case SIGSTKFLT:
            return "SIGSTKFLT";
        case SIGCHLD:
            return "SIGCHLD";
        case SIGCONT:
            return "SIGCONT";
        case SIGSTOP:
            return "SIGSTOP";
        case SIGTSTP:
            return "SIGTSTP";
        case SIGTTIN:
            return "SIGTTIN";
        case SIGTTOU:
            return "SIGTTOU";
        case SIGURG:
            return "SIGURG";
        case SIGXCPU:
            return "SIGXCPU";
        case SIGXFSZ:
            return "SIGXFSZ";
        case SIGVTALRM:
            return "SIGVTALRM";
        case SIGPROF:
            return "SIGPROF";
        case SIGWINCH:
            return "SIGWINCH";
        case SIGIO:
            return "SIGIO";
        case SIGPWR:
            return "SIGPWR";
        case SIGSYS:
            return "SIGSYS";
        default:
            return "INVALID_SIGNAL";
    }
}
