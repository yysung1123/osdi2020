#pragma once

#include <include/types.h>
#include <include/compiler.h>

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list) {
    WRITE_ONCE(list->next, list);
    WRITE_ONCE(list->prev, list);
}

static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    WRITE_ONCE(prev->next, new);
}

static inline void list_add(struct list_head *new, struct list_head *head) {
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head) {
    __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next) {
    next->prev = prev;
    WRITE_ONCE(prev->next, next);
}

static inline void __list_del_entry(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry) {
    __list_del_entry(entry);
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del_entry(entry);
	INIT_LIST_HEAD(entry);
}

static inline bool list_empty(const struct list_head *head) {
    return READ_ONCE(head->next) == head;
}

static inline int list_is_head(const struct list_head *list, const struct list_head *head) {
	return list == head;
}

#define list_for_each(pos, head) \
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, __typeof__(*(pos)), member)

#define list_entry_is_head(pos, head, member) \
   (&pos->member == (head))

#define list_for_each_entry_safe_from(pos, n, head, member) \
    for (n = list_next_entry(pos, member); \
        !list_entry_is_head(pos, head, member); \
        pos = n, n = list_next_entry(n, member))

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member);

static inline void list_del_init_careful(struct list_head *entry)
{
    __list_del_entry(entry);
    WRITE_ONCE(entry->prev, entry);
    // TODO: SMP
    entry->next = entry;
}
