#include <include/test_case.h>
#include <include/types.h>
#include <include/task.h>
#include <include/printk.h>
#include <include/sched.h>

void __attribute__((optimize("O0"))) delay(uint32_t d) {
    for (uint32_t i = 0; i < d; ++i) {}
}

void foo() {
    while (1) {
        task_t *cur = get_current();

        pl011_uart_printk("Task id: %d\n", cur->id);
        delay(20000000);
        schedule();
    }
}

void idle() {
    while (1) {
        schedule();
        delay(20000000);
    }
}
