#pragma once

#include <include/types.h>
#include <include/spinlock_types.h>
#include <include/task.h>
#include <include/spinlock.h>
#include <include/sched.h>
#include <include/signal.h>

struct wait_queue_head {
    spinlock_t lock;
    struct list_head head;
};

struct wait_queue_entry {
    void *private;
    struct list_head entry;
};

#define __WAIT_QUEUE_HEAD_INITIALIZER(name) { \
    .head = LIST_HEAD_INIT(name.head) }

#define DECLARE_WAIT_QUEUE_HEAD(name) \
    struct wait_queue_head name = __WAIT_QUEUE_HEAD_INITIALIZER(name)

#define wait_event_interruptible(wq_head, condition)            \
({                                                              \
    int __ret = 0;                                              \
    if (!(condition)) {                                         \
        __ret = __wait_event_interruptible(wq_head, condition); \
    }                                                           \
    __ret;                                                      \
})

#define ___wait_is_interruptible(state) \
    (state == TASK_INTERRUPTIBLE)

#define __wait_event(wq_head, condition, state)                                 \
({                                                                              \
    struct wait_queue_entry __wq_entry;                                         \
    int64_t __ret = 0;                                                          \
                                                                                \
    init_wait_entry(&__wq_entry);                                               \
    for (;;) {                                                                  \
        int64_t __int = prepare_to_wait_event(&wq_head, &__wq_entry, state);    \
                                                                                \
        if (condition)                                                          \
            break;                                                              \
                                                                                \
        if (___wait_is_interruptible(state)	&& __int) {                         \
            __ret = __int;                                                      \
            goto __out;                                                         \
        }                                                                       \
                                                                                \
        schedule();                                                             \
    }                                                                           \
    finish_wait(&wq_head, &__wq_entry);                                         \
__out: __ret;                                                                   \
})

#define __wait_event_interruptible(wq_head, condition) \
    __wait_event(wq_head, condition, TASK_INTERRUPTIBLE)

#define wake_up_interruptible(x)	__wake_up(x, TASK_INTERRUPTIBLE, 1)
#define wake_up_interruptible_all(x)	__wake_up(x, TASK_INTERRUPTIBLE, 0)

void init_wait_entry(struct wait_queue_entry *);
int64_t prepare_to_wait_event(struct wait_queue_head *, struct wait_queue_entry *, TaskState);
void finish_wait(struct wait_queue_head *, struct wait_queue_entry *);
void __wake_up(struct wait_queue_head *, TaskState, int64_t);
void __wake_up_common(struct wait_queue_head *, TaskState, int64_t);
