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
    while (1) {
        printf("taskid: %d\n", get_taskid());

        delay(100000000);
    }
}
