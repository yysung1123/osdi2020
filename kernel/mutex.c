#include <include/mutex.h>
#include <include/irq.h>
#include <include/task.h>
#include <include/types.h>
#include <include/wait.h>

int64_t mutex_lock(struct mutex *mtx) {
    // Preemptive kernel & UP: do not require any lock
    // should be an atomic test-and-set-lock to protect critical section in SMP
    int64_t error = 0;
    preempt_disable();

    task_t *cur = get_current();

    error = wait_event_interruptible(mtx->wq, (mtx->owner == 0));
    if (error) goto finish;

    WRITE_ONCE(mtx->owner, (uint64_t)cur);

finish:
    preempt_enable();
    return error;
}

int64_t mutex_unlock(struct mutex *mtx) {
    preempt_disable();

    task_t *cur = get_current();
    if (mtx->owner == (uint64_t)cur) {
        mtx->owner = 0;
        wake_up_interruptible(&mtx->wq);
    }

    preempt_enable();

    return 0;
}

int64_t do_mutex(struct mutex *mtx, MUTEX_OP mutex_op) {
    if (mutex_op == MUTEX_LOCK) {
        return mutex_lock(mtx);
    } else if (mutex_op == MUTEX_UNLOCK) {
        return mutex_unlock(mtx);
    }

    return 0;
}
