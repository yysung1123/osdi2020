#include <include/mutex.h>
#include <include/task.h>
#include <include/types.h>
#include <include/wait.h>
#include <include/atomic.h>
#include <include/compiler.h>

int64_t mutex_lock(struct mutex *mtx) {
    task_t *cur = get_current();
    int64_t error = wait_event_interruptible(mtx->wq, atomic_compare_and_swap(&mtx->owner, 0, (uint64_t)cur));

    barrier();
    return error;
}

int64_t mutex_unlock(struct mutex *mtx) {
    barrier();

    task_t *cur = get_current();
    if (atomic_read(&mtx->owner) == (uint64_t)cur) {
        atomic_set(0, &mtx->owner);
        wake_up_interruptible(&mtx->wq);
    }

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
