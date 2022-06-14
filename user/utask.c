#include <include/utask.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/syscall.h>

static void __attribute__((optimize("O0"))) delay(uint32_t d) {
    // delay
    for (uint32_t i = 0; i < d; ++i) {}
}

void utask1() {
    while (1) {
        printf("1...\n");

        delay(100000000);
    }
}

void utask2() {
    while (1) {
        printf("2...\n");

        delay(100000000);
    }
}

void utask3() {
    int32_t pid = fork();
    printf("after fork: %d\n", pid);
    if (pid == 0) {
        printf("child process\n");
        printf("exec taskid: %d\n", get_taskid());
        exec(&utask3);
    } else {
        printf("parent process\n");
        exit(0);
    }
}

void utask_exec() {
    while (1) {
        printf("exec taskid: %d\n", get_taskid());

        delay(100000000);
    }
}
