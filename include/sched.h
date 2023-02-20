#pragma once

#include <include/types.h>
#include <include/task.h>

static inline void set_tsk_need_resched(task_t *ts) {
    ts->resched = true;
}

static inline void clear_tsk_need_resched(task_t *ts) {
    ts->resched = false;
}

static inline bool test_tsk_need_resched(task_t *ts) {
    return ts->resched;
}

static inline bool need_resched() {
    task_t *cur = get_current();
    return test_tsk_need_resched(cur);
}

void schedule();
void check_resched();
void preempt_schedule();
void preempt_schedule_irq();
