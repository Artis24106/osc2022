#include "mmio.h"

#include <stdint.h>

void mmio_write(uint32_t reg, uint32_t data) {
    *(volatile uint32_t*)(reg) = data;
}

uint32_t mmio_read(uint32_t reg) {
    return *(volatile uint32_t*)(reg);
}
