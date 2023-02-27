#pragma once

#include <include/types.h>
#include <include/wait.h>

struct mutex {
    volatile uint64_t owner;
    struct wait_queue_head wq;
};

#define __MUTEX_INITIALIZER(lockname) \
    { .owner = 0, \
      .wq = __WAIT_QUEUE_HEAD_INITIALIZER(lockname.wq) }

#define DEFINE_MUTEX(mutexname) \
    struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

typedef enum {
    MUTEX_LOCK = 0,
    MUTEX_UNLOCK
} MUTEX_OP;

int64_t mutex_lock(struct mutex *);
int64_t mutex_unlock(struct mutex *);
int64_t do_mutex(struct mutex *, MUTEX_OP);
