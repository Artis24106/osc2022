#ifndef __SCHED_H__
#define __SCHED_H__

#include "exec.h"
#include "list.h"
#include "malloc.h"
#include "string.h"

#define THREAD_STACK_SIZE 0x2000  // TODO: why

// exit code
#define EX_OK 0
#define EX_KILLED 1
#define EX_SIG_BASE 128

typedef struct thread_info {
    uint64_t x19;  // callee saved register
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;  // = x29
    uint64_t lr;  // = x30
    uint64_t sp;
} thread_info_t;

typedef struct trap_frame {
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;
    uint64_t x30;

    // TODO: why them
    uint64_t elr_el1;   // When taking an exception to EL1, holds the address to return to.
    uint64_t spsr_el1;  // Holds the saved process state when an exception is taken to EL1
    uint64_t sp_el0;    // Holds the stack pointer associated with EL0
} trap_frame_t;

typedef struct task_struct {
    // thread_info should be the first element
    thread_info_t info;

    struct list_head node;
    int32_t pid;

    int16_t status;
#define EMPTY (0)
#define STOPPED (1 << 0)
#define RUNNING (1 << 1)
#define WAITING (1 << 2)
#define DEAD (1 << 3)
    int16_t exit_code;
    uint32_t prio;
    uint32_t time;
    uint64_t user_stack;
    uint64_t kernel_stack;
    // signal
    // signal_ctx
    // signal_queue
} task_struct_t;

// run queue, wait queue, dead queue
extern struct list_head rq, wq, dq;

extern void switch_to(task_struct_t* prev, task_struct_t* next);
extern task_struct_t* get_current();
#define current get_current()

void idle();
void kill_zombies();
void schedule();
void call_schedule();
void main_thread_init();
void thread_create(void* start);
void thread_release(task_struct_t* target, int16_t ec);
task_struct_t* next_task(task_struct_t* curr);

uint32_t create_kern_task(void (*func)(), void* arg);
uint32_t create_user_task(void (*func)(), void* arg);
uint32_t _fork(trap_frame_t* tf);
task_struct_t* new_task();

void _kern_thread_trampoline();
void _user_thread_trampoline();
void _fork_child_trampoline();
void thread_trampoline(void (*func)(), void* argv);

void show_q();

#endif