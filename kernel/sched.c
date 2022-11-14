#include <include/sched.h>
#include <include/task.h>

extern runqueue_t rq[3];

void schedule() {
    task_t *cur = get_current();
    if (cur->state == TASK_RUNNING) {
        cur->state = TASK_RUNNABLE;
        runqueue_push(&rq[cur->priority], cur);
    }

    Priority pri = 0;
    for (; pri < 3; ++pri) {
        if (!runqueue_empty(&rq[pri])) break;
    }
    if (pri == 3) return;

    task_t *next = runqueue_pop(&rq[pri]);
    while (next && next->state != TASK_RUNNABLE) {
        next = runqueue_pop(&rq[pri]);
    }
    if (next == NULL) return;
    next->state = TASK_RUNNING;

    context_switch(next);
}
