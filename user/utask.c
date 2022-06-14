#include <include/utask.h>
#include <include/stdio.h>
#include <include/types.h>

void __attribute__((optimize("O0"))) utask3() {
    while (1) {
        printf("3...\n");

        // delay
        for (uint32_t i = 0; i < 100000000; ++i) {}
    }
}
