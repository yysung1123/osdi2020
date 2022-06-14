#include <include/syscall.h>
#include <include/types.h>
#include <include/exc.h>
#include <include/uart.h>
#include <include/utils.h>
#include <include/timer.h>
#include <include/task.h>

void syscall_handler(struct TrapFrame *tf) {
    uint64_t nr = tf->x[8];
    int64_t ret = -1;

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
