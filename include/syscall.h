#pragma once

#include <include/exc.h>
#include <include/utils.h>

enum {
    SYS_uart_read = 0,
    SYS_uart_write,
    SYS_get_timestamp,
    SYS_init_timers,
    SYS_get_taskid,
    SYS_exec,
    SYS_fork,
    SYS_exit,
    SYS_kill
};

void syscall_handler(struct TrapFrame *);

ssize_t uart_read(char *, size_t);
ssize_t uart_write(const char *, size_t);
int64_t get_timestamp(struct Timestamp *ts);
int64_t init_timers();
uint32_t get_taskid();
int64_t exec(void(*func)());
int32_t fork();
int64_t exit(int64_t);
int64_t kill(int32_t, uint8_t);

int64_t sys_uart_read(char *, size_t);
int64_t sys_uart_write(const char *, size_t);
int64_t sys_get_timestamp(struct Timestamp *ts);
int64_t sys_init_timers();
int64_t sys_get_taskid();
int64_t sys_exec(struct TrapFrame *);
int64_t sys_fork(struct TrapFrame *);
int64_t sys_exit(int64_t);
int64_t sys_kill(int32_t, uint8_t);
