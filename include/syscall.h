#pragma once

#include <include/exc.h>
#include <include/utils.h>

enum {
    SYS_uart_read = 0,
    SYS_uart_write,
    SYS_get_timestamp,
    SYS_init_timers
};

void syscall_handler(struct TrapFrame *);

ssize_t uart_read(char *, size_t);
ssize_t uart_write(const char *, size_t);
int64_t get_timestamp(struct Timestamp *ts);
int64_t init_timers();

int64_t sys_uart_read(char *, size_t);
int64_t sys_uart_write(const char *, size_t);
int64_t sys_get_timestamp(struct Timestamp *ts);
int64_t sys_init_timers();
