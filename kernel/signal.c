#include <include/signal.h>
#include <include/task.h>
#include <include/types.h>
#include <include/spinlock.h>
#include <include/exc.h>
#include <include/error.h>
#include <include/asm/entry.h>
#include <include/list.h>

static inline bool in_syscall(struct TrapFrame *tf) {
    return tf->syscallno != NO_SYSCALL;
}

static inline void forgot_syscall(struct TrapFrame *tf) {
    tf->syscallno = NO_SYSCALL;
}

void check_signal(struct TrapFrame *tf) {
    task_t *cur = get_current();

    // prepare restart syscall
    bool syscall = in_syscall(tf);
    uint64_t restart_addr = 0;

    if (syscall) {
        restart_addr = tf->elr_el1 - 4; // rewind one instruction
        uint64_t retval = tf->x[0];

        forgot_syscall(tf);

        if (retval == -E_RESTARTSYS) {
            tf->x[0] = tf->orig_x0;
            tf->elr_el1 = restart_addr;
        }
    }

    uint64_t flags = spin_lock_irqsave(&cur->sig_lock);

    if (!cur->sigpending) goto unlock;

    if (cur->signal & (1 << SIGKILL)) {
        spin_unlock_irqrestore(&cur->sig_lock, flags);
        do_exit();
    } else {
        cur->signal = 0;
    }

    if (cur->signal == 0) {
        cur->sigpending = false;
    }

unlock:
    spin_unlock_irqrestore(&cur->sig_lock, flags);
}

int64_t do_kill(int32_t pid, uint8_t sig) {
    task_t *ts = get_task(pid);

    uint64_t flags = spin_lock_irqsave(&ts->sig_lock);

    ts->sigpending = true;
    ts->signal |= ((uint64_t)1 << sig);

    spin_unlock_irqrestore(&ts->sig_lock, flags);

    try_to_wake_up(ts, TASK_INTERRUPTIBLE);

    return 0;
}
