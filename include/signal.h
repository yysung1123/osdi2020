#pragma once

#include <include/types.h>
#include <include/task.h>

enum {
    SIGKILL = 9
};

static inline bool __fatal_signal_pending(task_t *p) {
    return p->signal & (1 << SIGKILL);
}

static inline bool signal_pending(task_t *p) {
    return (p->sigpending);
}

static inline bool signal_pending_state(TaskState state, task_t *p) {
    if (!(state == TASK_INTERRUPTIBLE)) return false;
    if (!signal_pending(p)) return false;

    return (state == TASK_INTERRUPTIBLE) || __fatal_signal_pending(p);
}

void check_signal();
int64_t do_kill(int32_t, uint8_t);
