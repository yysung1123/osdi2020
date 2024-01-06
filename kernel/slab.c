#include <include/slab.h>
#include <include/mm.h>
#include <include/list.h>
#include <include/spinlock.h>
#include <include/mm_types.h>

void *slab_alloc(struct slab *slab) {
    uint64_t flags = spin_lock_irqsave(&slab->lock);
    struct list_head *obj;

    if (list_empty(&slab->free_list)) {
        page_t *pp = buddy_alloc(0);
        pp->slab_cache = slab;
        struct slab_page *slab_page = (struct slab_page *)page2kva(pp);
        list_add(&slab_page->list, &slab->page_list);

        obj = (struct list_head *)((kernaddr_t)slab_page + SLAB_PAGE_OFFSET);
        while ((kernaddr_t)obj + ROUNDUP(slab->object_size + sizeof(struct list_head), 8) < (kernaddr_t)slab_page + PAGE_SIZE) {
            INIT_LIST_HEAD(obj);
            list_add(obj, &slab->free_list);
            slab_page->nr_free++;
            obj += ROUNDUP(slab->object_size + sizeof(struct list_head), 8);
        }
    }

    assert(!list_empty(&slab->free_list));
    obj = slab->free_list.next;
    list_del_init(obj);
    ((struct slab_page *)(ROUNDDOWN((kernaddr_t)obj, PAGE_SIZE)))->nr_free--;

    spin_unlock_irqrestore(&slab->lock, flags);
    return (void *)obj + sizeof(struct list_head);
}

void slab_free(struct slab *slab, void *mem) {
    uint64_t flags = spin_lock_irqsave(&slab->lock);
    struct list_head *obj = (struct list_head *)((kernaddr_t)mem - sizeof(struct list_head));

    list_add(obj, &slab->free_list);

    struct slab_page *slab_page = (struct slab_page *)(ROUNDDOWN((kernaddr_t)obj, PAGE_SIZE));
    slab_page->nr_free++;
    if (slab_page->nr_free == (PAGE_SIZE - SLAB_PAGE_OFFSET) / ROUNDUP((slab->object_size + sizeof(struct list_head)), 8)) {
        obj = (struct list_head *)((kernaddr_t)slab_page + SLAB_PAGE_OFFSET);
        while ((kernaddr_t)obj + ROUNDUP(slab->object_size + sizeof(struct list_head), 8) < (kernaddr_t)slab_page + PAGE_SIZE) {
            list_del(obj);
            slab_page->nr_free++;
            obj += ROUNDUP(slab->object_size + sizeof(struct list_head), 8);
        }

        list_del(&slab_page->list);
        buddy_free(pa2page(PADDR((kernaddr_t)slab_page)));
    }

    spin_unlock_irqrestore(&slab->lock, flags);
}

void slab_init(struct slab *slab, size_t size) {
    INIT_LIST_HEAD(&slab->page_list);
    slab->object_size = size;
    INIT_LIST_HEAD(&slab->free_list);
}
