#pragma once

#include <include/exc.h>
#include <include/utils.h>
#include <include/types.h>
#include <include/mutex.h>
#include <include/syscall_test.h>

enum {
    SYS_uart_read = 0,
    SYS_uart_write,
    SYS_get_timestamp,
    SYS_init_timers,
    SYS_get_taskid,
    SYS_exec,
    SYS_fork,
    SYS_exit,
    SYS_kill,
    SYS_wait,
    SYS_mutex,
    SYS_test,
    SYS_get_remain_page_num
};

void syscall_handler(struct TrapFrame *);

ssize_t uart_read(char *, size_t);
ssize_t uart_write(const char *, size_t);
int64_t get_timestamp(struct Timestamp *ts);
int64_t init_timers();
int32_t get_taskid();
int64_t exec(void(*func)());
int32_t fork();
int64_t exit(int64_t);
int64_t kill(int32_t, uint8_t);
int32_t wait();
int64_t mutex(struct mutex *, int32_t);
int64_t test(int32_t);
size_t get_remain_page_num();

int64_t sys_uart_read(char *, size_t);
int64_t sys_uart_write(const char *, size_t);
int64_t sys_get_timestamp(struct Timestamp *ts);
int64_t sys_init_timers();
int64_t sys_get_taskid();
int64_t sys_exec(struct TrapFrame *);
int64_t sys_fork(struct TrapFrame *);
int64_t sys_exit(int64_t);
int64_t sys_kill(int32_t, uint8_t);
int64_t sys_wait();
int64_t sys_mutex(struct mutex *, MUTEX_OP);
int64_t sys_test(TEST_OP);
size_t sys_get_remain_page_num();
