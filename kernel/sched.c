#include <include/sched.h>
#include <include/task.h>

extern runqueue_t rq;

void schedule() {
    task_t *cur = get_current();
    if (cur->state == TASK_RUNNING) {
        cur->state = TASK_RUNNABLE;
        runqueue_push(&rq, cur);
    }

    task_t *next = runqueue_pop(&rq);
    if (next == NULL) return;
    next->state = TASK_RUNNING;

    context_switch(next);
}
