#include <include/sched.h>
#include <include/task.h>

extern struct list_head rq[NUM_PRIORITY];

void schedule() {
    task_t *cur = get_current();
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
