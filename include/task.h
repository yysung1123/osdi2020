#pragma once

#include <include/exc.h>
#include <include/types.h>

#define NR_TASKS 64

typedef enum {
    TASK_FREE = 0,
    TASK_RUNNABLE,
    TASK_RUNNING,
    TASK_ZOMBIE
} TaskState;

typedef struct cpu_context {
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
} cpu_context;

typedef struct task_struct {
    cpu_context cpu_context;
    uint32_t id;
    TaskState state;
    bool resched;
} task_struct;

typedef task_struct task_t;

typedef struct {
    task_t *tasks[NR_TASKS + 1]; // circular queue actual size = array size - 1
    uint32_t head, tail;
} runqueue_t;

void task_init();
int32_t privilege_task_create(void(*func)());
void context_switch(task_t *);
task_t* get_task(uint32_t);
void task1();
void task2();
void task3();
void runqueue_push(runqueue_t *, task_t *);
task_t* runqueue_pop(runqueue_t *);
bool runqueue_empty(runqueue_t *);
bool runqueue_full(runqueue_t *);
uint32_t runqueue_size(runqueue_t *);
extern void switch_to(task_t *, task_t *);
extern task_t* get_current();
void do_exec(void(*func)());
void check_resched();
uint32_t do_get_taskid();
uint8_t* get_kstack_by_id(uint32_t);
uint8_t* get_ustack_by_id(uint32_t);
uint8_t* get_kstacktop_by_id(uint32_t);
uint8_t* get_ustacktop_by_id(uint32_t);
int32_t do_fork(struct TrapFrame *);
void do_exit();
void zombie_reaper();
uint32_t num_runnable_tasks();
