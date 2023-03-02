#include <include/syscall_test.h>
#include <include/types.h>
#include <include/atomic.h>
#include <include/task.h>
#include <include/printk.h>

void test_atomic_add();

int64_t do_test(TEST_OP test_op) {
    switch (test_op) {
    case ATOMIC_ADD:
        test_atomic_add();
        break;
    default:
        break;
    }

    return 0;
}

atomic_t cnt;
void  __attribute__((optimize("O0"))) test_atomic_add() {
    for (int i = 0; i < 100000; ++i) {
        atomic_add(1, &cnt);
    }
    pl011_uart_printk("task %d: %d\n", get_current()->id, atomic_read(&cnt));
}
