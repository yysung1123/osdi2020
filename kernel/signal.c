#include <include/signal.h>
#include <include/task.h>
#include <include/types.h>
#include <include/spinlock.h>

void check_signal() {
    task_t *cur = get_current();

    uint64_t flags = spin_lock_irqsave(&cur->sig_lock);

    if (!cur->sigpending) goto unlock;

    if (cur->signal & (1 << SIGKILL)) {
        do_exit();
    }

    if (cur->signal == 0) {
        cur->sigpending = false;
    }

unlock:
    spin_unlock_irqrestore(&cur->sig_lock, flags);
}

int64_t do_kill(int32_t pid, uint8_t sig) {
    task_t *ts = get_task(pid);

    spin_lock(&ts->sig_lock);

    ts->sigpending = true;
    ts->signal |= ((uint64_t)1 << sig);

    spin_unlock(&ts->sig_lock);

    return 0;
}
