#include "exec.h"

void exec(char* file_data, uint32_t data_size) {
    // char* ustack = kmalloc(USTACK_SIZE);
    char* ustack = frame_alloc(USTACK_SIZE / 0x1000);

    char* file_ptr = frame_alloc(data_size / 0x1000);  // memcpy, so the address will align 0x1000
    memcpy(file_ptr, file_data, data_size);

    // TODO: change to write_sysreg
    // const uint64_t zero = 0;
    // write_sysreg(spsr_el1, zero);                // M[4:0] = 0 -> User, 0 -> to enable interrupt in EL0
    // write_sysreg(elr_el1, file_data);            // return to start of file
    // write_sysreg(sp_el0, ustack + USTACK_SIZE);  // stack for EL0
    asm volatile(
        "msr spsr_el1, %0\n\t"  // M[4:0] = 0 -> User, 0 -> to enable interrupt in EL0
        "msr elr_el1, %1\n\t"   // return to start of file
        "msr sp_el0, %2\n\t"    // stack for EL0
        "eret\n\t" ::"r"(0),
        "r"(file_ptr),
        "r"(ustack + USTACK_SIZE));

    kfree(ustack);
}