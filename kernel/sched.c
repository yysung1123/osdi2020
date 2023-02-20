#include <include/sched.h>
#include <include/task.h>
#include <include/irq.h>
#include <include/preempt.h>

extern struct list_head rq[NUM_PRIORITY];

void __schedule() {
    task_t *cur = get_current();
    clear_tsk_need_resched(cur);

    if (cur->state == TASK_RUNNING) {
        cur->state = TASK_RUNNABLE;
        runqueue_push(&rq[cur->priority], cur);
    }

    Priority pri = 0;
    for (; pri < NUM_PRIORITY; ++pri) {
        if (!runqueue_empty(&rq[pri])) break;
    }
    if (pri == NUM_PRIORITY) return;

    task_t *next = runqueue_pop(&rq[pri]);
    while (next && next->state != TASK_RUNNABLE) {
        next = runqueue_pop(&rq[pri]);
    }
    if (next == NULL) return;
    next->state = TASK_RUNNING;

    context_switch(next);
}

void schedule() {
    do {
        preempt_disable();
        __schedule();
        preempt_enable_no_resched();
    } while (need_resched());
}

// irq -> kernel_exit 0
void check_resched() {
    if (!need_resched()) return;

    schedule();
}

// kernel mode (preempt_enable)
void preempt_schedule() {
    if (!preemptible() || !need_resched()) return;

    do {
        // preempt disabled and irq enabled
        // reentrancy is not possible in the following interrupt context
        // since the preemption is disabled.
        preempt_disable();
        __schedule();
        preempt_enable_no_resched();
    } while (need_resched());
}

// irq -> kernel_exit 1
void preempt_schedule_irq() {
    // preempt disabled or resched == false
    if (!should_resched(0)) return;

    do {
        // preempt disabled and irq enabled
        // reentrancy is not possible in the following interrupt context
        // since the preemption is disabled.
        preempt_disable();
        irq_enable();
        __schedule();
        irq_disable();
        preempt_enable_no_resched();
    } while (need_resched());
}
