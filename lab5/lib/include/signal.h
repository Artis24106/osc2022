#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include "list.h"
#include "types.h"

// https://elixir.bootlin.com/linux/latest/source/arch/x86/include/uapi/asm/signal.h#L23
#define NSIG 32

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL 29
#define SIGPWR 30
#define SIGSYS 31
#define SIGUNUSED 31
#define SIGRTMAX NSIG

// signal for kernel
#define SIGDFL 0
#define SIGIGN 1

typedef void (*sighandler_t)(int);
typedef uint32_t sigset_t;

typedef struct signal {
    list_head_t node;
    uint32_t signum;
} signal_t;

typedef struct sigaction {
    sighandler_t sa_handler;  // signal handler
    sigset_t sa_flags;
    sigset_t sa_mask;
    bool is_kernel_hand;  // is kernel handler or not
} sigaction_t;

typedef struct sighand {
    sigaction_t sigactions[SIGRTMAX];
} sighand_t;

signal_t* new_signal();
sighand_t* new_sighand();
void reset_signal(signal_t* signal);
void reset_sighand(sighand_t* sighand);
void sigreturn();

// siganl action function
void sig_ignore(int ign);
void sig_terminate(int ign);

char* signal_to_str(uint32_t signo);

#endif