#include <include/syscall_test.h>
#include <include/types.h>
#include <include/atomic.h>
#include <include/task.h>
#include <include/printk.h>
#include <include/mutex.h>

void test_atomic_add();
void test_mutex();

int64_t do_test(TEST_OP test_op) {
    switch (test_op) {
    case ATOMIC_ADD:
        test_atomic_add();
        break;
    case MUTEX:
        test_mutex();
        break;
    default:
        break;
    }

    return 0;
}

static atomic_t cnt_atomic_add;
void  __attribute__((optimize("O0"))) test_atomic_add() {
    for (int i = 0; i < 100000; ++i) {
        atomic_add(1, &cnt_atomic_add);
    }
    pl011_uart_printk("task %d: %d\n", get_current()->id, atomic_read(&cnt_atomic_add));
}

static uint64_t cnt_mutex;
static DEFINE_MUTEX(mtx);
void  __attribute__((optimize("O0"))) test_mutex() {
    for (uint64_t i = 0; i < 10000; ++i) {
        do_mutex(&mtx, MUTEX_LOCK);
        uint64_t tmp = cnt_mutex;
        for (int i = 0; i < 1000; ++i) {}
        cnt_mutex = tmp + 1;
        do_mutex(&mtx, MUTEX_UNLOCK);
    }
    pl011_uart_printk("task %d: %d\n", get_current()->id, cnt_mutex);
}
