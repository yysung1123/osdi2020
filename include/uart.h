#pragma once

#include <include/types.h>

#define PL011_UART_SET_CLOCK_RATE_TAG 0x00038002
#define PL011_UART_CLOCK_ID 0x00000002
#define PL011_UART_IMSC_TXIM 0x00000020
#define PL011_UART_IMSC_RXIM 0x00000010
#define PL011_UART_MIS_TXMIS 0x00000020
#define PL011_UART_MIS_RXMIS 0x00000010

#define UART_IOBUFSIZE 512 // should > 1

struct UARTIORingBuffer {
  uint8_t buf[UART_IOBUFSIZE];
  uint32_t rpos;
  uint32_t wpos;
};

int32_t mini_uart_read();
void mini_uart_write(char);
void mini_uart_init();
char mini_uart_getchar();
void mini_uart_gets_s(char *, size_t);
void mini_uart_putchar(char);
void mini_uart_puts(const char *);
void pl011_uart_init();
ssize_t pl011_uart_read(void *, size_t);
ssize_t pl011_uart_write(const void *, size_t);
void pl011_uart_write_polling(char);
char pl011_uart_getchar();
void pl011_uart_gets_s(char *, size_t);
void pl011_uart_putchar(char);
void pl011_uart_puts(const char *);
void pl011_uart_enable_tx_interrupt();
void pl011_uart_disable_tx_interrupt();
void pl011_uart_enable_rx_interrupt();
void pl011_uart_disable_rx_interrupt();
void pl011_uart_intr();
bool ringbuf_empty(struct UARTIORingBuffer *);
bool ringbuf_full(struct UARTIORingBuffer *);
void ringbuf_push(struct UARTIORingBuffer *, uint8_t);
uint8_t ringbuf_pop(struct UARTIORingBuffer *);
