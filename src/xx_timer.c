#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include "../include/xx_timer.h"
#include "../include/xx_list.h"
#include "../include/xx_async_request.h"

#define TIMER_NAMELEN   23
struct Timer_ {
    AsyncRequest req;
    ListNode link;
    TimerCallback cb;
    void *arg;
    uint64_t interval;
    uint64_t schedule;

    TimerControl *control;
    bool oneshot;
    char name[TIMER_NAMELEN];
};

#define WHEEL_LEVEL     4
#define WHEEL_SIZE      64
#define WHEEL_BITWIDTH  6
#define WHEEL_MASK      ((1u << WHEEL_BITWIDTH) - 1)
#define WHEEL_CAPACITY  ((uint64_t)1 << (WHEEL_BITWIDTH * WHEEL_LEVEL))
typedef struct {
    uint8_t bits;
    uint64_t epoch[WHEEL_LEVEL];
    uint64_t bitmap[WHEEL_LEVEL];
    ListNode slot[WHEEL_LEVEL][WHEEL_SIZE];
} TimerControlImpl;

struct TimerControl_ {
    AsyncRequestList requests;
    TimerControlImpl impl;
};

static void timer_schedule_internal(Timer *timer)
{
    TimerControlImpl *wheel = &timer->control.impl;
    uint64_t epoch = timer->schedule >> wheel->bits;

    if (unlikely(epoch <= wheel->epoch[0])) {
        LIST_INSERT(&timer->link, &wheel->residue);
        return;
    }

    for (size_t n = 0; n < WHEEL_LEVEL; n++) {
        if (epoch < wheel->epoch[n] + WHEEL_SIZE) {
            LIST_INSERT(&timer->link, &wheel->slots[n][epoch & WHEEL_MASK]);
            wheel->bitmap[n] |= (uint64_t)1 << (epoch & WHEEL_MASK);
            return;
        }
        epoch >>= WHEEL_BITWIDTH;
    }

    abort();
}

static void timer_cancel_internal(Timer *timer, bool destroy)
{
    ListNode *head = LIST_PREV(&timer->link);
    TimerControl *control = timer->control;

    LIST_REMOVE(&timer->link);
    if (destroy) {
        free(timer);
    }

    /* If so, the node we just deleted must be the only one left. */
    if (unlikely(LIST_EMPTY(head))) {
        size_t n = head - &control->impl.slot[0][0];

        if (n < WHEEL_LEVEL * WHEEL_SIZE) {
            control->impl.bitmap[n / WHEEL_SIZE] &= ~(1 << (n % WHEEL_SIZE));
        } else {
            debug_assert(head == &control->impl.residue);
        }
    }
}

static void timer_request_handler(AsyncRequest *req, uint32_t payload)
{
    Timer *timer = container_of(req, Timer, req);

    switch (payload) {
    case ASYNC_SCHEDULE:
        timer_schedule_internal(timer);
        break;
    case ASYNC_CANCEL:
    case ASYNC_DESTROYED:
        timer_cancel_internal(timer, payload == ASYNC_DESTROYED);
        break;
    default:
        abort();
    }
}

static void timer_handle_requests(TimerControl *control)
{
    handle_requests(&control->requests, timer_request_handler);
}

static uint64_t timer_next_schedule(const TimerControlImpl *impl)
{
    return UINT64_MAX;
}

static void timer_collect(TimerControlImpl *impl)
{
}

static uint64_t timer_check(TimerControl *control, uint64_t cycle)
{
    ListNode head = LIST_HEAD_INIT(head);
    Timer *iter;

    timer_handle_requests(control);

    LIST_CONCAT(&head, &control->impl.residue);

    timer_collect(&control->impl, cycle);

    LIST_FOREACH(iter, &head, Timer, link) {
        timer->callback(timer->arg);
        if (!timer->oneshot) {
            timer->schedule += timer->interval;
            timer_schedule_internal(timer);
        } else {
            /* XXX */
        }
    }

    return LIST_EMPTY(&control->residue) ? cycle
                                         : timer_next_schedule(control);
}
