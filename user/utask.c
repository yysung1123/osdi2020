#include <include/utask.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/syscall.h>

void __attribute__((optimize("O0"))) utask3() {
    while (1) {
        printf("taskid: %d\n", get_taskid());

        // delay
        for (uint32_t i = 0; i < 100000000; ++i) {}
    }
}
