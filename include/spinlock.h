#pragma once

#include <include/spinlock_types.h>
#include <include/preempt.h>
#include <include/irq.h>

#define __acquire(x) (void)0
#define __release(x) (void)0
#define __LOCK(lock) \
    do { preempt_disable(); __acquire(lock); (void)(lock); } while(0)
#define __LOCK_IRQ(lock) \
    do { irq_disable(); __LOCK(lock); } while(0)
#define __LOCK_IRQ_SAVE(lock, flags) \
    do { flags = irq_save(); __LOCK(lock); } while(0)
#define __UNLOCK(lock) \
    do { preempt_enable(); __release(lock); (void)(lock); } while(0)
#define __UNLOCK_IRQ(lock) \
    do { irq_enable(); __UNLOCK(lock); } while(0)
#define __UNLOCK_IRQ_RESTORE(lock, flags) \
    do { irq_restore(flags); __UNLOCK(lock); } while(0)

static inline void spin_lock(spinlock_t *lock) {
    __LOCK(&lock->lock);
}

static inline void spin_unlock(spinlock_t *lock) {
    __UNLOCK(&lock->lock);
}

static inline void spin_lock_irq(spinlock_t *lock) {
    __LOCK_IRQ(&lock->lock);
}

static inline void spin_unlock_irq(spinlock_t *lock) {
    __UNLOCK_IRQ(&lock->lock);
}

static inline uint64_t spin_lock_irqsave(spinlock_t *lock) {
    uint64_t flags;

    __LOCK_IRQ_SAVE(&lock->lock, flags);

    return flags;
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, uint64_t flags) {
   __UNLOCK_IRQ_RESTORE(&lock->lock, flags);
}

static inline void raw_spin_lock_irq(spinlock_t *lock) {
    irq_disable();
    __acquire(x);
    (void)(lock);
}

static inline void raw_spin_unlock_irq(spinlock_t *lock) {
    irq_enable();
    __release(x);
    (void)(lock);
}
