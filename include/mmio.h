#pragma once

#include <include/types.h>

enum {
    PERIPHERAL_BASE = 0xffff00003f000000,
    PERIPHERAL_END = 0xffff00003fffffff,
    ARM_LOCAL_PERIPERHAL_BASE = 0xffff000040000000
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

enum {
    MAILBOX_BASE = PERIPHERAL_BASE + 0xb880,
    MAILBOX_READ = MAILBOX_BASE,
    MAILBOX_STATUS = MAILBOX_BASE + 0x18,
    MAILBOX_WRITE = MAILBOX_BASE + 0x20,
};

enum {
    PL011_UART_BASE = PERIPHERAL_BASE + 0x201000,
    PL011_UART_DR = PL011_UART_BASE,
    PL011_UART_FR = PL011_UART_BASE + 0x18,
    PL011_UART_IBRD = PL011_UART_BASE + 0x24,
    PL011_UART_FBRD = PL011_UART_BASE + 0x28,
    PL011_UART_LCRH = PL011_UART_BASE + 0x2c,
    PL011_UART_CR = PL011_UART_BASE + 0x30,
    PL011_UART_IMSC = PL011_UART_BASE + 0x38,
    PL011_UART_MIS = PL011_UART_BASE + 0x40,
    PL011_UART_ICR = PL011_UART_BASE + 0x44
};

enum {
    IRQ_BASE = PERIPHERAL_BASE + 0xb200,
    IRQ_BASIC_PENDING = IRQ_BASE,
    IRQ_PENDING_1 = IRQ_BASE + 0x4,
    IRQ_PENDING_2 = IRQ_BASE + 0x8,
    IRQ_ENABLE_1 = IRQ_BASE + 0x10,
    IRQ_ENABLE_2 = IRQ_BASE + 0x14
};

static inline void mmio_write(uint64_t reg, uint32_t val) {
    *(volatile uint32_t *)reg = val;
}

static inline uint32_t mmio_read(uint64_t reg) {
    return *(volatile uint32_t *)reg;
}
