#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/sched.h>
#include <include/utask.h>
#include <include/irq.h>

static task_t task_pool[NR_TASKS];
static uint8_t kstack_pool[NR_TASKS][STACK_SIZE];
static uint8_t ustack_pool[NR_TASKS][STACK_SIZE];
runqueue_t rq[3];

void task_init() {
    for (pid_t i = 0; i < NR_TASKS; ++i) {
        memset(&(task_pool[i]), 0, sizeof(task_t));
        task_pool[i].state = TASK_FREE;
    }

    // 0-th task is main() thread
    task_pool[0].state = TASK_RUNNING;
    task_pool[0].priority = LOW;
    __asm__ volatile("msr tpidr_el1, %0"
                     :: "r"(&task_pool[0]));

    // print init task done information
    pl011_uart_printk_time_polling("Init task done\n");
}

int32_t privilege_task_create(void(*func)()) {
    return privilege_task_create_priority(func, HIGH);
}

int32_t privilege_task_create_priority(void(*func)(), Priority priority) {
    /* find a free task structure */
    pid_t pid;
    task_t *ts = NULL;
    for (pid = 0; pid < NR_TASKS; ++pid) {
        if (task_pool[pid].state == TASK_FREE) {
            ts = &task_pool[pid];
            break;
        }
    }
    // the maximum number of PIDs was reached
    if (ts == NULL) return -1;

    ts->id = pid;
    ts->state = TASK_RUNNABLE;
    ts->priority = priority;
    ts->cpu_context.pc = (uint64_t)func;
    ts->cpu_context.sp = (uint64_t)(get_kstacktop_by_id(pid));

    runqueue_push(&rq[ts->priority], ts);

    return pid;
}

void context_switch(task_t *next){
    task_t *prev = get_current();
    switch_to(prev, next);
}

task_t* get_task(pid_t pid) {
    return &task_pool[pid];
}

void task1() {
    do_exec(&utask1);
}

void task2() {
    do_exec(&utask2);
}

void task3() {
    do_exec(&utask3);
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

uint32_t runqueue_size(runqueue_t *rq) {
    if (rq->tail >= rq->head) {
        return rq->tail - rq->head;
    } else {
        return rq->tail + (NR_TASKS + 1) - rq->head;
    }
}

void do_exec(void(*func)()) {
    task_t *cur = get_current();
    uint64_t elr_el1 = (uint64_t)func;
    uint64_t ustack = (uint64_t)(get_ustacktop_by_id(cur->id));

    __asm__ volatile("msr SPSR_EL1, xzr\n\t" // EL0t
                     "msr ELR_EL1, %0\n\t"
                     "msr SP_EL0, %1\n\t"
                     "eret\n\t"
                     :: "r"(elr_el1), "r"(ustack));
}

void check_resched() {
    task_t *cur = get_current();
    if (!cur->resched) return;

    cur->resched = false;
    schedule();
}

pid_t do_get_taskid() {
    task_t *cur = get_current();
    return cur->id;
}

uint8_t* get_kstack_by_id(pid_t id) {
    return ((uint8_t *)&kstack_pool + id * STACK_SIZE);
}

uint8_t* get_ustack_by_id(pid_t id) {
    return ((uint8_t *)&ustack_pool + id * STACK_SIZE);
}

uint8_t* get_kstacktop_by_id(pid_t id) {
    return ((uint8_t *)&kstack_pool + (id + 1) * STACK_SIZE);
}

uint8_t* get_ustacktop_by_id(pid_t id) {
    return ((uint8_t *)&ustack_pool + (id + 1) * STACK_SIZE);
}

int32_t do_fork(struct TrapFrame *tf) {
    task_t *cur = get_current();
    int32_t pid_new = privilege_task_create(NULL);
    if (pid_new < 0) return -1;
    task_t *ts_new = get_task(pid_new);

    uint8_t *kstacktop_new = get_kstacktop_by_id(ts_new->id);
    uint8_t *ustack_cur = get_ustack_by_id(cur->id);
    uint8_t *ustack_new = get_ustack_by_id(ts_new->id);
    uint8_t *ustacktop_cur = get_ustacktop_by_id(cur->id);
    uint8_t *ustacktop_new = get_ustacktop_by_id(ts_new->id);

    // set cpu_context and trapframe
    extern void ret_to_user();
    struct TrapFrame *tf_new = (struct TrapFrame *)(kstacktop_new - 16 * 18);
    ts_new->cpu_context.pc = (uint64_t)ret_to_user;
    ts_new->cpu_context.sp = (uint64_t)tf_new;

    // copy user context
    memcpy(ustack_new, ustack_cur, STACK_SIZE);
    *tf_new = *tf;
    // child's return value is 0
    tf_new->x[0] = 0;
    // change child's stack
    tf_new->sp_el0 = (uint64_t)(ustacktop_new + ((uint8_t *)tf->sp_el0 - ustacktop_cur));

    return pid_new;
}

void do_exit() {
    task_t *cur = get_current();
    cur->state = TASK_ZOMBIE;
    schedule();
}

void zombie_reaper() {
    while (1) {
        for (pid_t pid = 0; pid < NR_TASKS; ++pid) {
            if (task_pool[pid].state == TASK_ZOMBIE) {
                memset(&(task_pool[pid]), 0, sizeof(task_t));
                pl011_uart_printk_time_polling("zombie reaper: task %d\n", pid);
            }
        }
        schedule();
    }
}

uint32_t num_runnable_tasks() {
    uint32_t num_tasks = 0;
    for (Priority pri = 0; pri < 3; ++pri) {
        num_tasks += runqueue_size(&rq[pri]);
    }

    return num_tasks;
}

void setpriority(pid_t pid, Priority priority) {
    task_pool[pid].priority = priority;
}

Priority getpriority(pid_t pid) {
    return task_pool[pid].priority;
}
