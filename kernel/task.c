#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/sched.h>

static task_t task_pool[NR_TASKS];
static uint8_t kstack_pool[NR_TASKS][4096];
runqueue_t rq;

void task_init() {
    for (uint32_t i = 0; i < NR_TASKS; ++i) {
        memset(&(task_pool[i]), 0, sizeof(task_t));
        task_pool[i].state = TASK_FREE;
    }

    // 0-th task is main() thread
    task_pool[0].state = TASK_RUNNING;
    __asm__ volatile("msr tpidr_el1, %0"
                     :: "r"(&task_pool[0]));

    // print init task done information
    pl011_uart_printk_time_polling("Init task done\n");
}

void privilege_task_create(void(*func)()) {
    /* find a free task structure */
    uint32_t pid;
    task_t *ts = NULL;
    for (pid = 0; pid < NR_TASKS; ++pid) {
        if (task_pool[pid].state == TASK_FREE) {
            ts = &task_pool[pid];
            break;
        }
    }
    // the maximum number of PIDs was reached
    if (ts == NULL) return;

    ts->id = pid;
    ts->state = TASK_RUNNABLE;
    ts->cpu_context.pc = (uint64_t)func;
    ts->cpu_context.sp = (uint64_t)((uint8_t *)&kstack_pool + (pid + 1) * 4096);

    runqueue_push(&rq, ts);
}

void context_switch(task_t *next){
    task_t *prev = get_current();
    switch_to(prev, next);
}

task_t* get_task(uint32_t pid) {
    return &task_pool[pid];
}

void task1() {
    while (1) {
        task_t *cur = get_current();
        while (!cur->resched) {
            __sync_synchronize();
        }
        pl011_uart_printk("1...\n");
        cur->resched = false;
        schedule();
    }
}

void task2() {
    while (1) {
        task_t *cur = get_current();
        while (!cur->resched) {
            __sync_synchronize();
        }
        pl011_uart_printk("2...\n");
        cur->resched = false;
        schedule();
    }
}

void task3() {
    while (1) {
        task_t *cur = get_current();
        while (!cur->resched) {
            __sync_synchronize();
        }
        pl011_uart_printk("3...\n");
        cur->resched = false;
        schedule();
    }
}

void runqueue_push(runqueue_t *rq, task_t *ts) {
    if (runqueue_full(rq)) return;

    rq->tasks[rq->tail] = ts;
    rq->tail = (rq->tail + 1) % (NR_TASKS + 1);
}

task_t* runqueue_pop(runqueue_t *rq) {
    if (runqueue_empty(rq)) return NULL;

    task_t *ts = rq->tasks[rq->head];
    rq->head = (rq->head + 1) % (NR_TASKS + 1);

    return ts;
}

bool runqueue_empty(runqueue_t *rq) {
    return rq->tail == rq->head;
}

bool runqueue_full(runqueue_t *rq) {
    return (rq->tail + 1) % (NR_TASKS + 1) == rq->head;
}
