#include <include/sched.h>
#include <include/task.h>
#include <include/irq.h>
#include <include/preempt.h>
#include <include/spinlock_types.h>
#include <include/spinlock.h>
#include <include/compiler.h>

extern struct list_head rq[NUM_PRIORITY];
extern spinlock_t rq_lock;

void __schedule() {
    raw_spin_lock_irq(&rq_lock);

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
    if (pri == NUM_PRIORITY) goto unlock;

    task_t *next = runqueue_pop(&rq[pri]);
    while (next && next->state != TASK_RUNNABLE) {
        next = runqueue_pop(&rq[pri]);
    }
    if (next == NULL) goto unlock;
    next->state = TASK_RUNNING;
    context_switch(next);

unlock:
    raw_spin_unlock_irq(&rq_lock);
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

void try_to_wake_up(task_t *ts, TaskState state) {
    uint64_t flags = spin_lock_irqsave(&ts->lock);

    bool no_wake_up = false;
    if (READ_ONCE(ts->state) != state) {
        no_wake_up = true;
    } else {
        WRITE_ONCE(ts->state, TASK_RUNNABLE);
    }
    spin_unlock_irqrestore(&ts->lock, flags);

    if (no_wake_up) return;

    flags = spin_lock_irqsave(&rq_lock);
    runqueue_push(&rq[ts->priority], ts);
    spin_unlock_irqrestore(&rq_lock, flags);
}
