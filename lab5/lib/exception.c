#include "exception.h"

void irq_handler() {
    uint32_t daif = get_intr();
    disable_intr();

    uint64_t c0_int_src = mmio_read(CORE0_IRQ_SRC),
             irq_pend_1 = mmio_read(IRQ_PEND_1),
             aux_mu_iir = mmio_read(AUX_MU_IIR_REG);
    // async_printf("IRQ_PEND_1: 0x%lX" ENDL, mmio_read(IRQ_PEND_1));
    // async_printf("CORE0_IRQ_SRC: 0x%lX" ENDL, c0_int_src);

    // [ Lab3 - AD2 ] 2. move data from the device’s buffer through DMA, or manually copy,
    // TODO: necessary???

    // [ Lab3 - AD2 ] 3. enqueues the processing task to the event queue,
    // classify the interrupt type, and call add_task()
    if (c0_int_src & SRC_CNTPNSIRQ_INT) {
        disable_timer();
        current->time--;
        // printf("irq_handler(timer)" ENDL);
        add_task(try_schedule, PRIORITY_TIMER);
    } else if (aux_mu_iir & 0b110) {  // TODO: better condition
        if (aux_mu_iir & 1) goto irq_handler_end;
        // printf("irq_handler(uart)" ENDL);
        uart_int_handler();
    } else {
        printf("[-] unkown exception" ENDL);
        while (1)
            ;
    }
    // disable_intr();

    // [ Lab3 - AD2 ] 4. do the tasks with interrupts enabled,
    run_task();

irq_handler_end:
    set_intr(daif);
    // printf("[END] irq_handler()" ENDL);
}

void invalid_handler(uint32_t x0) {
    printf("[Invalid Exception] %d" ENDL, x0);
    while (1)
        ;
}

void enable_intr() {
    // printf("[---------] enable_intr()" ENDL);
    asm volatile("msr DAIFClr, 0xf");
}

void disable_intr() {
    // printf("[---------] disable_intr()" ENDL);
    // asm volatile("msr DAIFSet, 0xf");
}

uint32_t get_intr() {
    return read_sysreg(DAIF);
}

void set_intr(uint32_t daif) {
    write_sysreg(DAIF, daif);
}