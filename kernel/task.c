#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/sched.h>
#include <include/irq.h>
#include <include/list.h>
#include <include/preempt.h>
#include <include/spinlock_types.h>
#include <include/spinlock.h>
#include <include/asm/tlbflush.h>
#include <include/mm.h>
#include <include/mango_tree.h>
#include <include/limits.h>
#include <include/pgtable-hwdef.h>
#include <include/memory.h>
#include <include/assert.h>
#include <include/utils.h>
#include <include/elf.h>
#include <include/mmap.h>

static task_t task_pool[NR_TASKS];
static uint8_t kstack_pool[NR_TASKS][STACK_SIZE];
struct list_head rq[NUM_PRIORITY];
spinlock_t rq_lock;
spinlock_t task_pool_lock;

void mtree_free(struct mango_tree *mt) {
    struct mango_node *node;
    mt_for_each(mt, node) {
        if (node->entry) {
            vma_free(node->entry);
        }
    }

    mtree_destroy(mt);
}

struct mango_tree mtree_init_ts(uint64_t min_val, uint64_t max_val) {
    int32_t ts = (int32_t)timestamp_read_counts();
    return mtree_init(ts, min_val, max_val);
}

void copy_mm(mm_struct *dst, mm_struct *src) {
    copy_pgd(dst, src);

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
    mm->mt = mtree_init_ts(0, UULONG_MAX);
}

void mm_destroy(mm_struct *mm) {
    free_pgtables(mm);
    mtree_free(&mm->mt);
}

void task_init() {
    for (pid_t i = 0; i < NR_TASKS; ++i) {
        memset(&(task_pool[i]), 0, sizeof(task_t));
        task_pool[i].state = TASK_FREE;
        INIT_LIST_HEAD(&task_pool[i].list);
    }

    for (Priority pri = 0; pri < NUM_PRIORITY; ++pri) {
        INIT_LIST_HEAD(&rq[pri]);
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

    spin_lock(&task_pool_lock);
    for (pid = 0; pid < NR_TASKS; ++pid) {
        if (task_pool[pid].state == TASK_FREE) {
            ts = &task_pool[pid];
            break;
        }
    }

    // the maximum number of PIDs was reached
    if (ts == NULL) {
        pid = -1;
        goto unlock_task_pool;
    }

    ts->id = pid;
    ts->ppid = 0;
    ts->state = TASK_RUNNABLE;
    ts->priority = priority;
    ts->cpu_context.pc = (uint64_t)func;
    ts->cpu_context.sp = (uint64_t)(get_kstacktop_by_id(pid));
    ts->sigpending = false;
    ts->signal = 0;

    mm_init(&ts->mm);

unlock_task_pool:
    spin_unlock(&task_pool_lock);
    if (pid < 0) return pid;

    uint64_t flags = spin_lock_irqsave(&rq_lock);

    runqueue_push(&rq[ts->priority], ts);
    spin_unlock_irqrestore(&rq_lock, flags);

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

void runqueue_push(struct list_head *rq, task_t *ts) {
    list_add_tail(&ts->list, rq);
}

task_t* runqueue_pop(struct list_head *rq) {
    if (list_empty(rq)) return NULL;

    struct list_head *head = rq->next;
    list_del_init(head);

    return container_of(head, task_t, list);
}

bool runqueue_empty(struct list_head *rq) {
    return list_empty(rq);
}

uint32_t runqueue_size(struct list_head *rq) {
    struct list_head *pos;
    int count = 0;

    list_for_each(pos, rq) {
        ++count;
    }

    return count;
}

void do_exec(kernaddr_t start) {
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

    // ELF64 parser
    Elf64_Ehdr *elf = (Elf64_Ehdr *)start;
    assert(*(uint32_t *)elf->e_ident == ELF_MAGIC);

    Elf64_Phdr *ph, *eph;
    ph = (Elf64_Phdr *)((kernaddr_t)elf + elf->e_phoff);
    eph = ph + elf->e_phnum;
    for (; ph < eph; ++ph) {
        if (ph->p_type == PT_LOAD) {
            kernaddr_t addr = (kernaddr_t)ph->p_vaddr - (ph->p_vaddr % PAGE_SIZE);
            off_t file_offset = (off_t)ph->p_offset - (ph->p_offset % PAGE_SIZE);
            size_t len = (size_t)ph->p_memsz + (ph->p_offset % PAGE_SIZE);

            mmap_flags_t flags;
            if (ph->p_filesz == ph->p_memsz) {
                flags = MAP_FIXED | MAP_POPULATE;
            } else {
                flags = MAP_FIXED | MAP_ANONYMOUS;
            }

            mmap_prot_t prot = 0;
            if (ph->p_flags & PF_R) {
                prot |= PROT_READ;
            }
            if (ph->p_flags & PF_W) {
                prot |= PROT_WRITE;
            }
            if (ph->p_flags & PF_X) {
                prot |= PROT_EXEC;
            }

            do_mmap((void *)addr, len, prot, flags, (void *)start, file_offset);

            if (ph->p_memsz > ph->p_filesz) {
                memcpy((void *)ph->p_vaddr, (void *)((kernaddr_t)start + ph->p_offset), ph->p_filesz); // .data initialization
                memset((void *)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz); // .bss initialization
            }
        }
    }

    uint64_t elr_el1 = elf->e_entry;
    uint64_t ustack = USTACKTOP;

    __asm__ volatile("msr SPSR_EL1, xzr\n\t" // EL0t
                     "msr ELR_EL1, %0\n\t"
                     "msr SP_EL0, %1\n\t"
                     "eret\n\t"
                     :: "r"(elr_el1), "r"(ustack));
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

pid_t do_fork(struct TrapFrame *tf) {
    preempt_disable();

    task_t *cur = get_current();
    pid_t pid_new = privilege_task_create(NULL);
    if (pid_new < 0) goto finish;
    task_t *ts_new = get_task(pid_new);

    uint8_t *kstacktop_new = get_kstacktop_by_id(ts_new->id);

    // set ppid
    ts_new->ppid = cur->id;

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

finish:
    preempt_enable();

    return pid_new;
}

void do_exit() {
    task_t *cur = get_current();
    cur->state = TASK_ZOMBIE;
    preempt_disable();

    mm_destroy(&cur->mm);

    schedule();
}

pid_t do_wait() {
    task_t *cur = get_current();
    pid_t ret = 0; // 0-th task should not waited

    while (1) {
        spin_lock(&task_pool_lock);
        for (pid_t pid = 0; pid < NR_TASKS; ++pid) {
            if (task_pool[pid].state == TASK_ZOMBIE && task_pool[pid].ppid == cur->id) {
                memset(&(task_pool[pid]), 0, sizeof(task_t));
                pl011_uart_printk_time_polling("wait: task %d\n", pid);
                ret = pid;
                break;
            }
        }
        spin_unlock(&task_pool_lock);

        if (ret) return ret;

        schedule();
    }
}

uint32_t num_runnable_tasks() {
    uint32_t num_tasks = 0;
    for (Priority pri = 0; pri < NUM_PRIORITY; ++pri) {
        uint64_t flags = spin_lock_irqsave(&rq_lock);
        num_tasks += runqueue_size(&rq[pri]);
        spin_unlock_irqrestore(&rq_lock, flags);
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
#ifndef CONFIG_PREEMPTION
        schedule();
#endif
    }
}

void user_shell() {
    extern char _binary_user_shell_start[];

    do_exec((kernaddr_t)&_binary_user_shell_start);
}
