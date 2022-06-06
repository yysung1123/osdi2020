#include <include/uart.h>
#include <include/mmio.h>
#include <include/types.h>
#include <include/mbox.h>
#include <include/irq.h>

static struct UARTIORingBuffer pl011_inbuf, pl011_outbuf;

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

    // enable rx interrupt
    pl011_uart_enable_rx_interrupt();

    // set CR to enable UART
    // enable Receive, Transmit, UART
    mmio_write(PL011_UART_CR, 0x301);

    // qemu tx interrupt bug
    while (mmio_read(PL011_UART_FR) & (1 << 5)) {}
    mmio_write(PL011_UART_DR, 0);

    // set UART interrupt
    mmio_write(IRQ_ENABLE_2, 1 << 25);
}

ssize_t pl011_uart_read(void *buf, size_t count) {
    ssize_t num_read;
    for (num_read = 0; num_read < count && !ringbuf_empty(&pl011_inbuf); ++num_read) {
        *((uint8_t *)buf + num_read) = ringbuf_pop(&pl011_inbuf);
    }

    pl011_uart_enable_rx_interrupt();
    return num_read;
}

ssize_t pl011_uart_write(const void *buf, size_t count) {
    ssize_t num_write;
    for (num_write = 0; num_write < count && !ringbuf_full(&pl011_outbuf); ++num_write) {
        ringbuf_push(&pl011_outbuf, *((uint8_t *)buf + num_write));
    }

    pl011_uart_enable_tx_interrupt();
    return num_write;
}

void pl011_uart_write_polling(char c) {
    while (mmio_read(PL011_UART_FR) & (1 << 5)) {}
    mmio_write(PL011_UART_DR, c);
}

char pl011_uart_getchar() {
    char buf[1];
    while (pl011_uart_read(&buf, 1) != 1) {};

    char c = buf[0];
    if (c == '\r') c = '\n';
    pl011_uart_putchar(c);

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
    char buf[1] = {c};
    while (pl011_uart_write(buf, 1) != 1) {}
}

void pl011_uart_puts(const char *buf) {
    ssize_t i = 0;
    while (buf[i]) {
        pl011_uart_putchar(buf[i++]);
    }

    pl011_uart_putchar('\n');
}

void pl011_uart_enable_tx_interrupt() {
    *(volatile uint32_t *)PL011_UART_IMSC |= PL011_UART_IMSC_TXIM;
}

void pl011_uart_disable_tx_interrupt() {
    *(volatile uint32_t *)PL011_UART_IMSC &= ~PL011_UART_IMSC_TXIM;
}

void pl011_uart_enable_rx_interrupt() {
    *(volatile uint32_t *)PL011_UART_IMSC |= PL011_UART_IMSC_RXIM;
}

void pl011_uart_disable_rx_interrupt() {
    *(volatile uint32_t *)PL011_UART_IMSC &= ~PL011_UART_IMSC_RXIM;
}

void pl011_uart_intr() {
    uint32_t status = mmio_read(PL011_UART_MIS);
    if (status & PL011_UART_MIS_RXMIS) {
        if (!ringbuf_full(&pl011_inbuf)) {
            uint8_t data = (uint8_t)mmio_read(PL011_UART_DR);
            ringbuf_push(&pl011_inbuf, data);

            if (ringbuf_full(&pl011_inbuf)) {
                pl011_uart_disable_rx_interrupt(); // disable interrupt until inbuf is not full
            }
        }
    }

    if (status & PL011_UART_MIS_TXMIS) {
        if (!ringbuf_empty(&pl011_outbuf)) {
            uint8_t data = ringbuf_pop(&pl011_outbuf);
            mmio_write(PL011_UART_DR, (uint32_t)data);

            if (ringbuf_empty(&pl011_outbuf)) {
                pl011_uart_disable_tx_interrupt(); // disable interrupt until outbuf is not empty
            }
        }
    }
}

bool ringbuf_empty(struct UARTIORingBuffer *ringbuf) {
    __sync_synchronize();
    return ringbuf->wpos == ringbuf->rpos;
}

bool ringbuf_full(struct UARTIORingBuffer *ringbuf) {
    __sync_synchronize();
    return (ringbuf->wpos + 1) % UART_IOBUFSIZE == ringbuf->rpos;
}

// assume ringbuf is not full
// should check ringbuf_full before calling ringbuf_push
void ringbuf_push(struct UARTIORingBuffer *ringbuf, uint8_t elem) {
    ringbuf->buf[ringbuf->wpos] = elem;
    ringbuf->wpos = (ringbuf->wpos + 1) % UART_IOBUFSIZE;
}

// assume ringbuf is not empty
// should check ringbuf_empty before calling ringbuf_pop
uint8_t ringbuf_pop(struct UARTIORingBuffer *ringbuf) {
    uint8_t elem = ringbuf->buf[ringbuf->rpos];
    ringbuf->rpos = (ringbuf->rpos + 1) % UART_IOBUFSIZE;

    return elem;
}
