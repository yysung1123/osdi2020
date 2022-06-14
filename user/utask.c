#include <include/utask.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/syscall.h>

void utask3() {
    exec(&utask3_exec);
}

void __attribute__((optimize("O0"))) utask3_exec() {
    while (1) {
        printf("exec taskid: %d\n", get_taskid());

        // delay
        for (uint32_t i = 0; i < 100000000; ++i) {}
    }
}
