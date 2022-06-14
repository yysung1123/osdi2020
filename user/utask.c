#include <include/utask.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/syscall.h>

void utask3() {
    int32_t pid = fork();
    printf("after fork: %d\n", pid);
    if (pid == 0) {
        printf("child process\n");
    } else {
        printf("parent process\n");
    }
    exec(&utask_exec);
}

void __attribute__((optimize("O0"))) utask_exec() {
    while (1) {
        printf("exec taskid: %d\n", get_taskid());

        // delay
        for (uint32_t i = 0; i < 100000000; ++i) {}
    }
}
