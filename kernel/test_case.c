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

static volatile uint64_t count;
DEFINE_MUTEX(mtx);

void  __attribute__((optimize("O0"))) mutex_test() {
    if (fork() == 0) {
        while (1) {
            for (int i = 0; i < 8; ++i) {
                kill(i + 2, 8);
            }
            delay(1000000);
        }
        exit(0);
    }
    pid_t pid1, pid2, pid3;
    pid1 = fork();
    pid2 = fork();
    pid3 = fork();

    for (uint64_t i = 0; i < 1000; ++i) {
        mutex(&mtx, MUTEX_LOCK);
        uint64_t tmp = count;
        delay(1000);
        count = tmp + 1;
        mutex(&mtx, MUTEX_UNLOCK);
    }

    if (!pid3) goto exit;
    wait();

    if (!pid2) goto exit;
    wait();

    if (!pid1) goto exit;
    wait();

    if (count != 8000) {
        printf("test failed: count != 8000\ncount = %d\n", count);
    } else {
        printf("pass test\ncount = %d\n", count);
    }

exit:
    exit(0);
}

void uart_read_test() {
    const size_t CMD_SIZE = 1024;
    char cmd[CMD_SIZE];

    int pid = fork();
    if (pid == 0) {
        while (1) {
            printf("# ");
            gets_s(cmd, CMD_SIZE);
            printf("%s\n", cmd);
        }
        exit(0);
    } else {
        while (1) {
            kill(pid, 8);
            delay(1000000);
        }
        exit(0);
    }
}

void user_test() {
    do_exec(uart_read_test);
}

void idle() {
    while (1) {
#ifndef CONFIG_PREEMPTION
        schedule();
#endif
    }
}
