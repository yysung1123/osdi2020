#include <include/uart.h>
#include <include/mmio.h>
#include <include/types.h>

int32_t mini_uart_read() {
    while (!(mmio_read(AUX_MU_LSR_REG) & 1)) {}
    return mmio_read(AUX_MU_IO_REG);
}

void mini_uart_write(char c) {
    while (!(mmio_read(AUX_MU_LSR_REG) & (1 << 5))) {}
    mmio_write(AUX_MU_IO_REG, c);
}

void mini_uart_init() {
    mmio_write(AUX_ENABLES, 1);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3);
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_BAUD_REG, 270);
    mmio_write(AUX_MU_IIR_REG, 6);
    mmio_write(AUX_MU_CNTL_REG, 3);
}

char mini_uart_getchar() {
    char c = mini_uart_read();
    if (c == '\r') c = '\n';
    mini_uart_write(c);

    return c;
}

void mini_uart_gets_s(char *buf, size_t len) {
    size_t idx = 0;
    char c;

    do {
        c = mini_uart_getchar();
        buf[idx++] = c;
    } while (idx < len && c != '\n');
    buf[idx - 1] = 0;
}

void mini_uart_putchar(char c) {
    mini_uart_write(c);
}

void mini_uart_puts(const char *buf) {
    ssize_t i = 0;
    while (buf[i]) {
        mini_uart_putchar(buf[i++]);
    }

    mini_uart_putchar('\n');
}
