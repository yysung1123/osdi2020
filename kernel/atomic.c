#include <include/atomic.h>
#include <include/types.h>
#include <include/spinlock.h>
#include <include/compiler.h>

void atomic_add(int64_t i, atomic_t *v) {
    uint64_t flags = spin_lock_irqsave(&v->lock);
    int64_t tmp = READ_ONCE(v->counter) + i;
    WRITE_ONCE(v->counter, tmp);
    spin_unlock_irqrestore(&v->lock, flags);
}

void atomic_set(int64_t i, atomic_t *v) {
    WRITE_ONCE(v->counter, i);
}

int64_t atomic_read(atomic_t *v) {
    return READ_ONCE(v->counter);
}
