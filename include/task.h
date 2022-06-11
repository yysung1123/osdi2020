#pragma once

#include <include/exc.h>
#include <include/types.h>

#define NR_TASKS 64

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
} task_struct;

typedef task_struct task_t;

void privilege_task_create(void(*func)());
void context_switch(task_t *);
task_t* get_task(uint32_t);
void task1();
void task2();
extern void switch_to(task_t *, task_t *);
extern task_t* get_current();
