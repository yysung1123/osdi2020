#include <include/syscall.h>
#include <include/types.h>
#include <include/exc.h>
#include <include/uart.h>
#include <include/utils.h>
#include <include/timer.h>
#include <include/task.h>
#include <include/signal.h>
#include <include/mutex.h>

void syscall_handler(struct TrapFrame *tf) {
    uint64_t nr = tf->x[8];
    int64_t ret = -1;

    tf->orig_x0 = tf->x[0];
    tf->syscallno = nr;

    switch (nr) {
        case SYS_uart_read:
            ret = sys_uart_read((char *)tf->x[0], (size_t)tf->x[1]);
            break;
        case SYS_uart_write:
            ret = sys_uart_write((const char *)tf->x[0], (size_t)tf->x[1]);
            break;
        case SYS_get_timestamp:
            ret = sys_get_timestamp((struct Timestamp *)tf->x[0]);
            break;
        case SYS_init_timers:
            ret = sys_init_timers();
            break;
        case SYS_get_taskid:
            ret = sys_get_taskid();
            break;
        case SYS_exec:
            ret = sys_exec(tf);
            break;
        case SYS_fork:
            ret = sys_fork(tf);
            break;
        case SYS_exit:
            ret = sys_exit((int64_t)tf->x[0]);
            break;
        case SYS_kill:
            ret = sys_kill((int32_t)tf->x[0], (uint8_t)tf->x[1]);
            break;
        case SYS_wait:
            ret = sys_wait();
            break;
        case SYS_mutex:
            ret = sys_mutex((struct mutex *)tf->x[0], (MUTEX_OP)tf->x[1]);
            break;
        case SYS_test:
            ret = sys_test((TEST_OP)tf->x[0]);
            break;
        default:
    }

    tf->x[0] = (uint64_t)ret;
}

int64_t sys_uart_read(char *buf, size_t len) {
    return pl011_uart_read(buf, len);
}

int64_t sys_uart_write(const char *buf, size_t len) {
    return pl011_uart_write(buf, len);
}

int64_t sys_get_timestamp(struct Timestamp *ts) {
    return do_get_timestamp(ts);
}

int64_t sys_init_timers() {
    return do_init_timers();
}

int64_t sys_get_taskid() {
    return (int64_t)do_get_taskid();
}

int64_t sys_exec(struct TrapFrame *tf) {
    tf->elr_el1 = tf->x[0];
    tf->sp_el0 = USTACKTOP;
    return 0;
}

int64_t sys_fork(struct TrapFrame *tf) {
    return (int64_t)do_fork(tf);
}

int64_t sys_exit(int64_t status) {
    do_exit();
    return 0;
}

int64_t sys_kill(int32_t pid, uint8_t sig) {
    return do_kill(pid, sig);
}

int64_t sys_wait() {
    return (int64_t)do_wait();
}

int64_t sys_mutex(struct mutex *mtx, MUTEX_OP mutex_op) {
    return do_mutex(mtx, mutex_op);
}

int64_t sys_test(TEST_OP test_op) {
    return do_test(test_op);
}
