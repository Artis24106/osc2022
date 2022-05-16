#include "exec.h"

void exec(char* file_data, uint32_t data_size) {
    // char* ustack = kmalloc(USTACK_SIZE);
    // char* ustack = frame_alloc(USTACK_SIZE / 0x1000);
    char* ustack = kmalloc(USTACK_SIZE);

    // char* file_ptr = frame_alloc(data_size / 0x1000);  // memcpy, so the address will align 0x1000
    char* file_ptr = kmalloc(data_size);
    printf("file_ptr: 0x%X" ENDL, file_ptr);
    memcpy(file_ptr, file_data, data_size);

    el1_to_el0(file_ptr, ustack);

    // kfree(ustack);
}

void _exec(char* file_data, uint32_t data_size, trap_frame_t* tf) {
    // char* ustack = frame_alloc(USTACK_SIZE / 0x1000);
    char* ustack = kmalloc(USTACK_SIZE);

    // char* file_ptr = frame_alloc(data_size / 0x1000);  // memcpy, so the address will align 0x1000
    char* file_ptr = kmalloc(data_size);
    memcpy(file_ptr, file_data, data_size);
    tf->x30 = file_ptr;
    tf->sp_el0 = ustack + THREAD_STACK_SIZE - 8;
    el1_to_el0(file_ptr, ustack);
}

// this is for exec
void el1_to_el0(char* file_ptr, char* stack_ptr) {
    asm volatile(
        "msr spsr_el1, %0\n\t"  // M[4:0] = 0 -> User, 0 -> to enable interrupt in EL0
        "msr elr_el1, %1\n\t"   // return to start of file
        "msr sp_el0, %2\n\t"    // stack for EL0
        "eret\n\t" ::"r"(0),
        "r"(file_ptr),
        "r"(stack_ptr + USTACK_SIZE));
}