#include <stdint.h>

#include "cpio.h"
#include "dtb.h"
#include "fs/fs.h"
#include "fs/vfs.h"
#include "page_alloc.h"
#include "printf.h"
#include "sched.h"
#include "svc.h"
#include "uart.h"

void foo() {
    for (int i = 0; i < 10; i++) {
        printf("Thread id: 0x%X 0x%X" ENDL, get_current(), i);
        // enable_intr();
        // update_timer();
        delay(1000000);
        schedule();
    }
}

void test() {
    void* a = frame_alloc(64);
    void* b = frame_alloc(64);
    void* c = frame_alloc(64);
    void* d = frame_alloc(64);
    printf("0x%X, 0x%X, 0x%X, 0x%X" ENDL, a, b, c, d);
    // frame_free(a);
    // void* b = frame_alloc(2);
    // void* c = frame_alloc(1);
    // void* d = frame_alloc(1);
    // frame_free(b);
    // frame_free(c);
    // frame_free(d);

    // uint32_t c = 0x20;
}

void kernel_main(char* x0) {
    // initialize dtb
    dtb_init(x0);

    // initialize the page frame allocator
    frame_init();
    // test();
    // initialize slab cache
    init_slab_cache();

    // initialize the timer and task list
    timer_list_init();
    task_list_init();

    fs_init();
    while (1)
        ;
    // // enable interrupts (AUX, uart RX/TX)
    // uart_enable_int(RX | TX);
    // uart_enable_aux_int();

    // TODO: what's that??
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1"
                 : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0"
                 :
                 : "r"(tmp));
    main_thread_init();

    printf("[+] start" ENDL);
    // for (int i = 0; i < 3; i++) {
    //     create_kern_task(foo, NULL);
    // }

    void* file_ptr = cpio_get_file("syscall.img");
    if (file_ptr == NULL) {
        printf("[-] syscall.img not found??" ENDL);
        while (1)
            ;
    }
    printf("[+] syscall.img base: 0x%X" ENDL, file_ptr);
    printf("create_user_task" ENDL);
    create_user_task(file_ptr, 0);

    // enable interrupts (AUX, uart RX/TX)
    uart_enable_int(RX | TX);
    uart_enable_aux_int();

    update_timer();
    enable_intr();

    idle();

    // No shell anymore QQ
    // start the shell!!
    // shell();
}