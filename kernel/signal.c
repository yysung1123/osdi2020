#include <include/signal.h>
#include <include/task.h>
#include <include/types.h>

void check_signal() {
    task_t *cur = get_current();
    if (!cur->sigpending) return;

    if (cur->signal & (1 << SIGKILL)) {
        do_exit();
    }

    if (cur->signal == 0) {
        cur->sigpending = false;
    }
}

int64_t do_kill(int32_t pid, uint8_t sig) {
    task_t *ts = get_task(pid);
    ts->sigpending = true;
    ts->signal |= ((uint64_t)1 << sig);

    return 0;
}
