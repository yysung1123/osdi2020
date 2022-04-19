#pragma once

#include <include/types.h>

int32_t mini_uart_read();
void mini_uart_write(char);
void mini_uart_init();
char mini_uart_getchar();
void mini_uart_gets_s(char *, size_t);
void mini_uart_putchar(char);
void mini_uart_puts(const char *);
