#pragma once

#include <include/exc.h>

enum {
    SYS_uart_read = 0,
    SYS_uart_write
};

void syscall_handler(struct TrapFrame *);

ssize_t uart_read(char *, size_t);
ssize_t uart_write(const char *, size_t);

int64_t sys_uart_read(char *, size_t);
int64_t sys_uart_write(const char *, size_t);
