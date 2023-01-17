#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/sched.h>
#include <include/irq.h>
#include <include/asm/tlbflush.h>
#include <include/mm.h>
#include <include/mango_tree.h>
#include <include/limits.h>
#include <include/pgtable-hwdef.h>
#include <include/memory.h>
#include <include/assert.h>

static task_t task_pool[NR_TASKS];
static uint8_t kstack_pool[NR_TASKS][STACK_SIZE];
runqueue_t rq[3];

void mtree_free(struct mango_tree *mt) {
    struct mango_node *node;
    mt_for_each(mt, node) {
        if (node->entry) {
            vma_free(node->entry);
        }
    }

    mtree_destroy(mt);
}

void copy_mm(mm_struct *dst, mm_struct *src) {
    copy_pgd(dst, src);
    mtree_free(&dst->mt);
    dst->mt = mtree_init(0, 0, UULONG_MAX);

    struct mango_node *node;
    mt_for_each(&src->mt, node) {
        struct vm_area_struct *new_vma = NULL;
        if (node->entry) {
            struct vm_area_struct *vma = node->entry;
            new_vma = vma_alloc();
            if (new_vma == NULL) {
                panic("vma_alloc error");
            }

            new_vma->vm_start = vma->vm_start;
            new_vma->vm_end = vma->vm_end;
            new_vma->vm_mm = dst;
            new_vma->vm_page_prot = vma->vm_page_prot;
        }

        mtree_insert_range(&dst->mt, node->index, node->last, new_vma);
    }
}

void mm_init(mm_struct *mm) {
    mm_alloc_pgd(mm);
    mm->mt = mtree_init(0, 0, UULONG_MAX);
}

void mm_destroy(mm_struct *mm) {
    free_pgtables(mm);
    mtree_free(&mm->mt);
}

void task_init() {
    for (pid_t i = 0; i < NR_TASKS; ++i) {
        memset(&(task_pool[i]), 0, sizeof(task_t));
        task_pool[i].state = TASK_FREE;
    }

    // 0-th task is main() thread
    task_pool[0].state = TASK_RUNNING;
    task_pool[0].priority = LOW;
    __asm__ volatile("msr tpidr_el1, %0"
                     :: "r"(&task_pool[0]));
    mm_init(&task_pool[0].mm);

    // print init task done information
    pl011_uart_printk_time_polling("Init task done\n");
}

int32_t privilege_task_create(void(*func)()) {
    return privilege_task_create_priority(func, HIGH);
}

int32_t privilege_task_create_priority(void(*func)(), Priority priority) {
    /* find a free task structure */
    pid_t pid;
    task_t *ts = NULL;
    for (pid = 0; pid < NR_TASKS; ++pid) {
        if (task_pool[pid].state == TASK_FREE) {
            ts = &task_pool[pid];
            break;
        }
    }
    // the maximum number of PIDs was reached
    if (ts == NULL) return -1;

    ts->id = pid;
    ts->state = TASK_RUNNABLE;
    ts->priority = priority;
    ts->cpu_context.pc = (uint64_t)func;
    ts->cpu_context.sp = (uint64_t)(get_kstacktop_by_id(pid));

    mm_init(&ts->mm);

    runqueue_push(&rq[ts->priority], ts);

    return pid;
}

void context_switch(task_t *next){
    task_t *prev = get_current();

    __asm__ volatile("msr TTBR0_EL1, %0"
                     :: "r"(PADDR((kernaddr_t)next->mm.pgd)));
    flush_tlb_all();

    switch_to(prev, next);
}

task_t* get_task(pid_t pid) {
    return &task_pool[pid];
}

void runqueue_push(runqueue_t *rq, task_t *ts) {
    if (runqueue_full(rq)) return;

    rq->tasks[rq->tail] = ts;
    rq->tail = (rq->tail + 1) % (NR_TASKS + 1);
}

task_t* runqueue_pop(runqueue_t *rq) {
    if (runqueue_empty(rq)) return NULL;

    task_t *ts = rq->tasks[rq->head];
    rq->head = (rq->head + 1) % (NR_TASKS + 1);

    return ts;
}

bool runqueue_empty(runqueue_t *rq) {
    return rq->tail == rq->head;
}

bool runqueue_full(runqueue_t *rq) {
    return (rq->tail + 1) % (NR_TASKS + 1) == rq->head;
}

uint32_t runqueue_size(runqueue_t *rq) {
    if (rq->tail >= rq->head) {
        return rq->tail - rq->head;
    } else {
        return rq->tail + (NR_TASKS + 1) - rq->head;
    }
}

void do_exec(kernaddr_t start, size_t size) {
    task_t *cur = get_current();
    mm_struct *mm = &cur->mm;
    struct vm_area_struct *vma;

    mm_destroy(mm);

    mm_init(mm);

    // user stack
    vma = vma_alloc();
    vma->vm_start = USTACK;
    vma->vm_end = USTACKTOP - 1;
    vma->vm_mm = mm;
    vma->vm_page_prot = __pgprot(PTE_ATTR_NORMAL | PD_RW | PD_NX);

    for (virtaddr_t addr = USTACK; addr < USTACKTOP; addr += STACK_SIZE) {
        page_t *pp = page_alloc(0);
        insert_page(mm, pp, addr, vma->vm_page_prot);
    }

    mtree_insert_range(&mm->mt, vma->vm_start, vma->vm_end, vma);

    // user program
    vma = vma_alloc();
    vma->vm_start = EXECUTABLE_START;
    vma->vm_end = EXECUTABLE_START + size - 1;
    vma->vm_mm = mm;
    vma->vm_page_prot = __pgprot(PTE_ATTR_NORMAL | PD_RO);

    for (virtaddr_t addr = 0; addr < size; addr += PAGE_SIZE) {
        page_t *pp = page_alloc(0);
        insert_page(mm, pp, addr + EXECUTABLE_START, vma->vm_page_prot);
        memcpy((void *)page2kva(pp), (void *)(start + addr), MIN(PAGE_SIZE, size - addr));
    }

    mtree_insert_range(&mm->mt, PAGE_ALIGN(EXECUTABLE_START), PAGE_ALIGN(EXECUTABLE_START + size) - 1, vma);

    tlbi_vmalle1is();

    uint64_t elr_el1 = EXECUTABLE_START;
    uint64_t ustack = USTACKTOP;

    __asm__ volatile("msr SPSR_EL1, xzr\n\t" // EL0t
                     "msr ELR_EL1, %0\n\t"
                     "msr SP_EL0, %1\n\t"
                     "eret\n\t"
                     :: "r"(elr_el1), "r"(ustack));
}

void check_resched() {
    task_t *cur = get_current();
    if (!cur->resched) return;

    cur->resched = false;
    schedule();
}

pid_t do_get_taskid() {
    task_t *cur = get_current();
    return cur->id;
}

uint8_t* get_kstack_by_id(pid_t id) {
    return ((uint8_t *)&kstack_pool + id * STACK_SIZE);
}

uint8_t* get_kstacktop_by_id(pid_t id) {
    return ((uint8_t *)&kstack_pool + (id + 1) * STACK_SIZE);
}

int32_t do_fork(struct TrapFrame *tf) {
    task_t *cur = get_current();
    int32_t pid_new = privilege_task_create(NULL);
    if (pid_new < 0) return -1;
    task_t *ts_new = get_task(pid_new);

    uint8_t *kstacktop_new = get_kstacktop_by_id(ts_new->id);

    // set cpu_context and trapframe
    extern void ret_to_user();
    struct TrapFrame *tf_new = (struct TrapFrame *)(kstacktop_new - 16 * 18);
    ts_new->cpu_context.pc = (uint64_t)ret_to_user;
    ts_new->cpu_context.sp = (uint64_t)tf_new;

    // copy user context
    copy_mm(&ts_new->mm, &cur->mm);
    *tf_new = *tf;
    // child's return value is 0
    tf_new->x[0] = 0;

    return pid_new;
}

void do_exit() {
    task_t *cur = get_current();
    cur->state = TASK_ZOMBIE;

    mm_destroy(&cur->mm);

    schedule();
}

void zombie_reaper() {
    while (1) {
        for (pid_t pid = 0; pid < NR_TASKS; ++pid) {
            if (task_pool[pid].state == TASK_ZOMBIE) {
                memset(&(task_pool[pid]), 0, sizeof(task_t));
                pl011_uart_printk_time_polling("zombie reaper: task %d\n", pid);
            }
        }
        schedule();
    }
}

uint32_t num_runnable_tasks() {
    uint32_t num_tasks = 0;
    for (Priority pri = 0; pri < 3; ++pri) {
        num_tasks += runqueue_size(&rq[pri]);
    }

    return num_tasks;
}

void setpriority(pid_t pid, Priority priority) {
    task_pool[pid].priority = priority;
}

Priority getpriority(pid_t pid) {
    return task_pool[pid].priority;
}

void idle() {
    while (1) {
        schedule();
    }
}

void user_shell() {
    extern char _binary_user_shell_start[], _binary_user_shell_size[];

    do_exec((kernaddr_t)&_binary_user_shell_start, (size_t)&_binary_user_shell_size);
}
