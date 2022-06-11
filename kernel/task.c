#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>

static task_t task_pool[NR_TASKS];
static uint8_t kstack_pool[NR_TASKS][4096];

// 0: free, 1: used
// 0-th task is main() thread
static uint64_t task_state = 1;

void privilege_task_create(void(*func)()) {
    /* find a free task structure */
    uint32_t pid = __builtin_ffs(~task_state) - 1;
    task_state |= (1 << pid);
    task_t *ts = &task_pool[pid];

    ts->id = pid;
    ts->cpu_context.pc = (uint64_t)func;
    ts->cpu_context.sp = (uint64_t)((uint8_t *)&kstack_pool + (pid + 1) * 4096);
}

void context_switch(task_t *next){
    task_t *prev = get_current();
    switch_to(prev, next);
}

task_t* get_task(uint32_t pid) {
    return &task_pool[pid];
}

void __attribute__((optimize("O0"))) task1() {
    while (1) {
        pl011_uart_printk_polling("1...\n");

        // delay
        for (uint32_t i = 0; i < 100000000; ++i) {}

        context_switch(&task_pool[2]);
    }
}

void __attribute__((optimize("O0"))) task2() {
    while (1) {
        pl011_uart_printk_polling("2...\n");

        // delay
        for (uint32_t i = 0; i < 100000000; ++i) {}

        context_switch(&task_pool[1]);
    }
}
