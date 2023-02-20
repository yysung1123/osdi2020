#pragma once

#include <include/compiler.h>
#include <include/types.h>
#include <include/task.h>
#include <include/irq.h>
#include <include/sched.h>

#define preempt_count_inc() preempt_count_add(1)
#define preempt_count_dec() preempt_count_sub(1)
#define preempt_count_dec_and_test() ({ preempt_count_sub(1); should_resched(0); })

#ifdef CONFIG_PREEMPTION

#define preempt_disable() \
do { \
    preempt_count_inc(); \
    barrier(); \
} while (0)

#define preempt_enable() \
do { \
    barrier(); \
    if (preempt_count_dec_and_test()) \
        preempt_schedule(); \
} while (0)

#define preempt_enable_no_resched() \
do { \
    barrier(); \
    preempt_count_dec(); \
} while (0)

#else

#define preempt_disable() do {} while(0)
#define preempt_enable() do {} while(0)
#define preempt_enable_no_resched() do {} while(0)

#endif

#define preemptible() \
    (preempt_count() == 0 && !irqs_disabled())

static inline void preempt_count_add(uint64_t val) {
    uint64_t pc = READ_ONCE(get_current()->preempt_count);
    pc += val;
    WRITE_ONCE(get_current()->preempt_count, pc);
}

static inline void preempt_count_sub(uint64_t val) {
    uint64_t pc = READ_ONCE(get_current()->preempt_count);
    pc -= val;
    WRITE_ONCE(get_current()->preempt_count, pc);
}

static inline bool should_resched(int preempt_offset) {
    uint64_t pc = READ_ONCE(get_current()->preempt_count);
    return pc == preempt_offset && READ_ONCE(get_current()->resched);
}

static inline uint64_t preempt_count() {
    return READ_ONCE(get_current()->preempt_count);
}
