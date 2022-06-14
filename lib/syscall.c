#include <include/types.h>
#include <include/syscall.h>
#include <include/utils.h>

#define SYSCALL_NOARG(name, ret_t) \
ret_t name(void) { return syscall((SYS_##name), 0, 0, 0, 0, 0); }

#define SYSCALL_1ARG(name, ret_t, typ_arg0) \
ret_t name(typ_arg0 a1) { return syscall((SYS_##name), (uint64_t)a1, 0, 0, 0, 0); }

#define SYSCALL_2ARG(name, ret_t, typ_arg0, typ_arg1) \
ret_t name(typ_arg0 a0, typ_arg1 a1) { return syscall((SYS_##name), (uint64_t)a0, (uint64_t)a1, 0, 0, 0); }

#define SYSCALL_3ARG(name, ret_t, typ_arg0, typ_arg1, typ_arg2) \
ret_t name(typ_arg0 a0, typ_arg1 a1, typ_arg2 a2) { return syscall((SYS_##name), (uint64_t)a0, (uint64_t)a1, (uint64_t)a2, 0, 0); }

#define SYSCALL_4ARG(name, ret_t, typ_arg0, typ_arg1, typ_arg2, typ_arg3) \
ret_t name(typ_arg0 a0, typ_arg1 a1, typ_arg2 a2, typ_arg3 a3) { return syscall((SYS_##name), (uint64_t)a0, (uint64_t)a1, (uint64_t)a2, (uint64_t)a3, 0); }

static inline int64_t syscall(int64_t num, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    register uint64_t x0 __asm__ ("x0") = a0;
    register uint64_t x1 __asm__ ("x1") = a1;
    register uint64_t x2 __asm__ ("x2") = a2;
    register uint64_t x3 __asm__ ("x3") = a3;
    register uint64_t x4 __asm__ ("x4") = a4;
    register uint64_t x8 __asm__ ("x8") = num;

    __asm__ volatile(
        "svc 0\n\t"
        : "+r" (x0)
        : "r" (x1),
          "r" (x2),
          "r" (x3),
          "r" (x4),
          "r" (x8)
    );

    return x0;
}

SYSCALL_2ARG(uart_read, ssize_t, char *, size_t);
SYSCALL_2ARG(uart_write, ssize_t, const char *, size_t);
SYSCALL_1ARG(get_timestamp, int64_t, struct Timestamp *);
SYSCALL_NOARG(init_timers, int64_t);
SYSCALL_NOARG(get_taskid, uint32_t);
int64_t exec(void(*a1)()) { return syscall((SYS_exec), (uint64_t)a1, 0, 0, 0, 0); }
SYSCALL_NOARG(fork, int32_t);
SYSCALL_1ARG(exit, int64_t, int64_t);
