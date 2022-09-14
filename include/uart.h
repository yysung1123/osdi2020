#pragma once

#include <include/types.h>

#define PL011_UART_SET_CLOCK_RATE_TAG 0x00038002
#define PL011_UART_CLOCK_ID 0x000000002

int32_t mini_uart_read();
void mini_uart_write(char);
void mini_uart_init();
char mini_uart_getchar();
void mini_uart_gets_s(char *, size_t);
void mini_uart_putchar(char);
void mini_uart_puts(const char *);
void pl011_uart_init();
int32_t pl011_uart_read();
void pl011_uart_write(char);
char pl011_uart_getchar();
void pl011_uart_gets_s(char *, size_t);
void pl011_uart_putchar(char);
void pl011_uart_puts(const char *);
