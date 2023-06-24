#ifndef XX_LIST_H
#define XX_LIST_H

#include <stdbool.h>
#include "xx_compiler.h"

typedef struct ListNode_ {
    struct ListNode_ *prev, *next;
} ListNode;

#define LIST_HEAD_INITIALIZER(head) { .prev = &(head), .next = &(head) }

#define LIST_INIT(head) do {                                            \
    *(head) = (ListNode){ .prev = (head), .next = (head) };             \
} while (false)

#define LIST_PREV(node)         ((node)->prev)
#define LIST_NEXT(node)         ((node)->next)
#define LIST_EMPTY(head)        (LIST_PREV(head) == LIST_NEXT(head))

#define LIST_INSERT(node, head) do {                                    \
    *(node) = (ListNode){ .prev = (head)->prev, .next = (head) };       \
    (head)->prev->next = (node);                                        \
    (head)->prev = (node);                                              \
} while (false)

#define LIST_REMOVE(node) do {                                          \
    LIST_PREV(node)->next = LIST_NEXT(node);                            \
    LIST_NEXT(node)->prev = LIST_PREV(node);                            \
} while (false)

#define LIST_FOREACH(iter, head, type, field)                           \
    for (ListNode *_ = (head)->next; _ != (head); _ = _->next)          \
        if (iter = container_of(_, type, field), false) { continue; } else

#define LIST_FIRST(head)        (LIST_EMPTY(head) ? nullptr : LIST_NEXT(head))
#define LIST_LAST(head)         (LIST_EMPTY(head) ? nullptr : LIST_PREV(head))

#define LIST_CONCAT(head, head2) do {                                   \
    if (!LIST_EMPTY(head2)) {                                           \
        LIST_FIRST(head2)->prev = (head)->prev;                         \
        (head)->prev->next = LIST_FIRST(head2);                         \
        LIST_LAST(head2)->next = (head);                                \
        (head)->prev = LIST_LAST(head2);                                \
        LIST_INIT(head2);                                               \
    }                                                                   \
} while (false)

#endif
