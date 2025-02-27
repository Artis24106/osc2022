.global switch_to
switch_to:
    msr DAIFSet, 0xf // disable interrupt

    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldr x9, [x1, 16 * 6]
    mov sp,  x9

    msr tpidr_el1, x1

    msr DAIFClr, 0xf // enable interrupt after switch_to

    ret

.global get_current
get_current:
    mrs x0, tpidr_el1
    ret

.global _kern_thread_trampoline
_kern_thread_trampoline:
    mov x0, x19
    mov x1, x20
    b thread_trampoline

.global _user_thread_trampoline
_user_thread_trampoline:
    msr elr_el1, x19    // return to start of file
    msr sp_el0, x20     // stack for EL0

    # mov x19, 0
    # msr spsr_el1, x19   // M[4:0] = 0 -> User, 0 -> to enable interrupt in EL0
    mov x0, 0x0
    msr spsr_el1, x0

    eret

.global _fork_child_trampoline
_fork_child_trampoline: // handle child
    // restore data stored in _fork()
    msr elr_el1, x19
    msr sp_el0, x20

    // restore registers
    ldr     x1, [sp, 16 * 0 + 8]
    ldp     x2, x3, [sp ,16 * 1]
    ldp     x4, x5, [sp ,16 * 2]
    ldp     x6, x7, [sp ,16 * 3]
    ldp     x8, x9, [sp ,16 * 4]
    ldp     x10, x11, [sp ,16 * 5]
    ldp     x12, x13, [sp ,16 * 6]
    ldp     x14, x15, [sp ,16 * 7]
    ldp     x16, x17, [sp ,16 * 8]
    ldp     x18, x19, [sp ,16 * 9]
    ldp     x20, x21, [sp ,16 * 10]
    ldp     x22, x23, [sp ,16 * 11]
    ldp     x24, x25, [sp ,16 * 12]
    ldp     x26, x27, [sp ,16 * 13]
    ldp     x28, x29, [sp ,16 * 14]

    add     sp, sp, 16 * 17

    // return 0
    mov x0, 0 
    msr spsr_el1, x0
    eret
