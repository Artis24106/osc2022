#include <stdint.h>

#include "cpio.h"
#include "dtb.h"
#include "page_alloc.h"
#include "printf.h"
#include "sched.h"
#include "svc.h"
#include "uart.h"

void foo() {
    for (int i = 0; i < 10; i++) {
        printf("Thread id: %d %d\n", get_current(), i);
        delay(1000000);
        schedule();
    }
}

void test() {
    void* a = frame_alloc(1);
    frame_free(a);
    void* b = frame_alloc(2);
    void* c = frame_alloc(1);
    void* d = frame_alloc(1);
    frame_free(b);
    frame_free(c);
    frame_free(d);

    // uint32_t c = 0x20;
}

void kernel_main(char* x0) {
    // initialize dtb
    dtb_init(x0);

    // initialize the page frame allocator
    frame_init();

    // simply initialize tcache_perthread_struct here
    init_tcache();

    // initialize the timer and task list
    timer_list_init();
    task_list_init();

    // enable interrupts (AUX, uart RX/TX)
    uart_enable_int(RX | TX);
    uart_enable_aux_int();

    // page_init();
    // buddy_init();
    // slab_init();

    enable_intr();
    enable_timer();
    // TODO: what's that??
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1"
                 : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0"
                 :
                 : "r"(tmp));

    main_thread_init();

    // for (int i = 0; i < 3; i++) {
    //     create_kern_task(foo, NULL);
    // }

    cpio_newc_parser(cpio_exec_sched_callback, "syscall.img");
    // cpio_newc_parser(cpio_exec_callback, "syscall.img");
    idle();

    // No shell anymore QQ
    // start the shell!!
    // shell();
}