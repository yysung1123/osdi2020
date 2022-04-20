#include <include/uart.h>
#include <include/mmio.h>
#include <include/types.h>
#include <include/mbox.h>

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

void pl011_uart_init() {
    // disable UART
    mmio_write(PL011_UART_CR, 0);

    // clear interrupts
    mmio_write(PL011_UART_ICR, 0x7ff);

    // configure the UART clock frequency by mailbox
    uint32_t mailbox[9] __attribute__((aligned(16)));
    mailbox[0] = 9 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE; // request code
    // tags begin
    mailbox[2] = PL011_UART_SET_CLOCK_RATE_TAG; // tag identifier
    mailbox[3] = 3 * 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = PL011_UART_CLOCK_ID; // clock id
    mailbox[6] = 11520000; // rate
    mailbox[7] = 0; // skip setting turbo
    // tags end
    mailbox[8] = END_TAG;

    mailbox_call((uintptr_t)mailbox, 8);

    // set IBRD and FBRD to configure baud rate
    // BAUDDIV = 11520000 / (16 * 115200) = 6.25
    // BAUDDIV = IBRD + (FBRD / 64)
    mmio_write(PL011_UART_IBRD, 6);
    mmio_write(PL011_UART_FBRD, 16); // 0.25 = 16 / 64

    // set LCRH to configure line control
    // 8 bits, enable FIFOs, no parity
    mmio_write(PL011_UART_LCRH, 0x70);

    // set CR to enable UART
    // enable Receive, Transmit, UART
    mmio_write(PL011_UART_CR, 0x301);
}

int32_t pl011_uart_read() {
    while (mmio_read(PL011_UART_FR) & (1 << 4)) {}
    return mmio_read(PL011_UART_DR);
}

void pl011_uart_write(char c) {
    while (mmio_read(PL011_UART_FR) & (1 << 5)) {}
    mmio_write(PL011_UART_DR, c);
}

char pl011_uart_getchar() {
    char c = pl011_uart_read();
    if (c == '\r') c = '\n';
    pl011_uart_write(c);

    return c;
}

void pl011_uart_gets_s(char *buf, size_t len) {
    size_t idx = 0;
    char c;

    do {
        c = pl011_uart_getchar();
        buf[idx++] = c;
    } while (idx < len && c != '\n');
    buf[idx - 1] = 0;
}

void pl011_uart_putchar(char c) {
    pl011_uart_write(c);
}

void pl011_uart_puts(const char *buf) {
    ssize_t i = 0;
    while (buf[i]) {
        pl011_uart_putchar(buf[i++]);
    }

    pl011_uart_putchar('\n');
}
