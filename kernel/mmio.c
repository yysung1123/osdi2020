#include <include/mmio.h>

void mmio_write(uint64_t reg, uint32_t val) {
    *(volatile uint32_t *)reg = val;
}

uint32_t mmio_read(uint64_t reg) {
    return *(volatile uint32_t *)reg;
}
