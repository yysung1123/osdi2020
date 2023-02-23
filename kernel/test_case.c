#include <include/test_case.h>
#include <include/types.h>
#include <include/task.h>
#include <include/printk.h>
#include <include/sched.h>
#include <include/stdio.h>
#include <include/syscall.h>

void __attribute__((optimize("O0"))) delay(uint32_t d) {
    for (uint32_t i = 0; i < d; ++i) {}
}

void foo() {
    int tmp = 5;
    printf("Task %d after exec, tmp address 0x%x, tmp value %d\n", get_taskid(), &tmp, tmp);
    exit(0);
}

void test() {
    int cnt = 1;
    if (fork() == 0) {
        fork();
        delay(2000000);
        fork();
        while (cnt < 10) {
            printf("Task id: %d, cnt: %d\n", get_taskid(), cnt);
            delay(2000000);
            ++cnt;
        }
        exit(0);
        printf("Should not be printed\n");
    } else {
        printf("Task %d before exec, cnt address 0x%x, cnt value %d\n", get_taskid(), &cnt, cnt);
        exec(foo);
    }
}

void signal_test() {
    int pid = fork();
    if (pid == 0) {
        while (1) {
            printf("Task id: %d\n", get_taskid());
            delay(20000000);
        }
    } else {
        delay(300000000);
        kill(pid, 9);
        printf("kill task %d\n", pid);
        pid = wait();
        exit(0);
    }
}

void user_test() {
    do_exec(signal_test);
}

void idle() {
    while (1) {
        if (num_runnable_tasks() == 0) {
            break;
        }
#ifndef CONFIG_PREEMPTION
        schedule();
#endif
    }
    pl011_uart_printk("Test finished\n");
    while (1);
}
