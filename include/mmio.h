#pragma once

#include <include/types.h>

void mmio_write(uint64_t, uint32_t);
uint32_t mmio_read(uint64_t);

enum {
    PERIPHERAL_BASE = 0x3f000000
};

enum {
    AUX_BASE = PERIPHERAL_BASE + 0x215000,
    AUX_ENABLES = AUX_BASE + 0x4,
    AUX_MU_IO_REG = AUX_BASE + 0x40,
    AUX_MU_IER_REG = AUX_BASE + 0x44,
    AUX_MU_IIR_REG = AUX_BASE + 0x48,
    AUX_MU_LCR_REG = AUX_BASE + 0x4C,
    AUX_MU_MCR_REG = AUX_BASE + 0x50,
    AUX_MU_LSR_REG = AUX_BASE + 0x54,
    AUX_MU_CNTL_REG = AUX_BASE + 0x60,
    AUX_MU_BAUD_REG = AUX_BASE + 0x68
};
