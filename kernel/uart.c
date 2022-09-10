#include <include/uart.h>
#include <include/mmio.h>
#include <include/types.h>

int32_t uart_read() {
    while (!(mmio_read(AUX_MU_LSR_REG) & 1)) {}
    return mmio_read(AUX_MU_IO_REG);
}

void uart_write(char c) {
    while (!(mmio_read(AUX_MU_LSR_REG) & (1 << 5))) {}
    mmio_write(AUX_MU_IO_REG, c);
}

void uart_init() {
    mmio_write(AUX_ENABLES, 1);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3);
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_BAUD_REG, 541);
    mmio_write(AUX_MU_IIR_REG, 6);
    mmio_write(AUX_MU_CNTL_REG, 3);

    mmio_write(GPIO_PUP_PDN_CNTRL_REG0, (mmio_read(GPIO_PUP_PDN_CNTRL_REG0) & 0x0fffffff) | 0x00000000);
    mmio_write(GPFSEL1, (mmio_read(GPFSEL1) & 0xfffc0fff) | 0x00012000);
}
