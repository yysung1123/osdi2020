#pragma once

#include <include/exc.h>
#include <include/types.h>
#include <include/list.h>
#include <include/spinlock_types.h>
#include <include/mm.h>
#include <include/mm_types.h>
#include <include/error.h>

#define NR_TASKS 64
#define STACK_SIZE 4096
#define USTACKTOP 0x0000ffffffffe000
#define USTACK (USTACKTOP - STACK_SIZE)
#define EXECUTABLE_START 0x400000

typedef int32_t pid_t;
typedef int32_t ppid_t;

typedef enum {
    TASK_FREE = 0,
    TASK_RUNNABLE,
    TASK_RUNNING,
    TASK_ZOMBIE,
    TASK_INTERRUPTIBLE
} TaskState;

typedef enum {
    HIGH = 0,
    MEDIUM,
    LOW,
    NUM_PRIORITY
} Priority;

struct cpu_context {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;
    uint64_t sp;
    uint64_t pc;
};

struct task_struct {
    struct cpu_context cpu_context;
    pid_t id;
    ppid_t ppid;
    TaskState state;
    bool resched;
    bool sigpending;
    sigset_t signal;
    spinlock_t sig_lock;
    Priority priority;
    struct list_head list;
    uint64_t preempt_count;
    spinlock_t lock;
    mm_struct mm;
};

typedef struct task_struct task_t;

void task_init();
int32_t privilege_task_create(void(*func)());
int32_t privilege_task_create_priority(void(*func)(), Priority);
void context_switch(task_t *);
task_t* get_task(pid_t);
void runqueue_push(struct list_head *, task_t *);
task_t* runqueue_pop(struct list_head *);
bool runqueue_empty(struct list_head *);
uint32_t runqueue_size(struct list_head *);
extern void switch_to(task_t *, task_t *);
void do_exec(kernaddr_t, size_t);
pid_t do_get_taskid();
uint8_t* get_kstack_by_id(pid_t);
uint8_t* get_kstacktop_by_id(pid_t);
int32_t do_fork(struct TrapFrame *);
void do_exit();
pid_t do_wait();
uint32_t num_runnable_tasks();
void setpriority(pid_t, Priority);
Priority getpriority(pid_t);
void idle();
void user_shell();

static inline task_t* get_current() {
    task_t *res;
    __asm__ inline("mrs %0, tpidr_el1"
                   : "=r"(res));
    return res;
}

static inline void set_current_state(TaskState state) {
    task_t *cur = get_current();

    WRITE_ONCE(cur->state, state);

    // TODO: SMP memory barrier
    barrier();
}

static inline int mm_alloc_pgd(mm_struct *mm) {
    mm->pgd = pgd_alloc(mm);
    if (!mm->pgd) return -E_NO_MEM;

    return 0;
}
