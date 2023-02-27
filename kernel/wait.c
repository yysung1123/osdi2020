#include <include/wait.h>
#include <include/task.h>
#include <include/list.h>
#include <include/error.h>

void init_wait_entry(struct wait_queue_entry *wq_entry) {
    wq_entry->private = (void *)get_current();
    INIT_LIST_HEAD(&wq_entry->entry);
}

int64_t prepare_to_wait_event(struct wait_queue_head * wq_head, struct wait_queue_entry *wq_entry, TaskState state) {
    task_t *cur = get_current();
    int64_t ret = 0;

    uint64_t flags = spin_lock_irqsave(&wq_head->lock);
    if (signal_pending_state(state, cur)) {
        list_del_init(&wq_entry->entry);
        ret = -E_RESTARTSYS;
    } else {
        if (list_empty(&wq_entry->entry)) {
            list_add_tail(&wq_entry->entry, &wq_head->head);
        }
        set_current_state(state);
    }
    spin_unlock_irqrestore(&wq_head->lock, flags);

    return ret;
}

void finish_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry) {
    set_current_state(TASK_RUNNING);

    // TODO: SMP
    if (!list_empty(&wq_entry->entry)) {
        uint64_t flags = spin_lock_irqsave(&wq_head->lock);
        list_del_init(&wq_entry->entry);
        spin_unlock_irqrestore(&wq_head->lock, flags);
    }
}

void __wake_up(struct wait_queue_head *wq_head, TaskState state, int64_t nr_exclusive) {
    uint64_t flags = spin_lock_irqsave(&wq_head->lock);
    __wake_up_common(wq_head, state, nr_exclusive);
    spin_unlock_irqrestore(&wq_head->lock, flags);
}

void __wake_up_common(struct wait_queue_head *wq_head, TaskState state, int64_t nr_exclusive) {
    struct wait_queue_entry *cur, *next;
    cur = list_first_entry(&wq_head->head, struct wait_queue_entry, entry);
    if (&cur->entry == &wq_head->head) {
        return;
    }

    list_for_each_entry_safe_from(cur, next, &wq_head->head, entry) {
        try_to_wake_up(cur->private, state);
        list_del_init_careful(&cur->entry);
        if (--nr_exclusive == 0) break;
    }
}


