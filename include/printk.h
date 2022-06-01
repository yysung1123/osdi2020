#pragma once

#include <include/types.h>

int32_t pl011_uart_printk(const char *, ...);
int32_t pl011_uart_printk_polling(const char *, ...);
