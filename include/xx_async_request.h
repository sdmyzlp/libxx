#ifndef XX_ASYNC_REQUEST_H
#define XX_ASYNC_REQUEST_H

#include <stdatomic.h>
#include <stdint.h>
#include "xx_compiler.h"

#define ASYNC_IDLE              0u
#define ASYNC_DESTROYED         0x80000000u
#define ASYNC_SCHEDULE          1u
#define ASYNC_CANCEL            2u

typedef struct AsyncRequest_ {
    uint32_t payload;
    struct AsyncRequest_ *next;
} AsyncRequest;

typedef struct AsyncRequest {
    AsyncRequest *head;
} AsyncRequestList;

typedef void (*RequestHandler)(AsyncRequest *req, uint32_t payload);

static inline bool make_request(AsyncRequestList *list, AsyncRequest *req,
                                uint32_t payload)
{
    uint32_t old;

    if (payload != ASYNC_DESTROYED) {
        old = atomic_load(&req->payload);
        do {
            if (old == ASYNC_DESTROYED) {
                return false;
            }
        } while (!atomic_compare_exchange_weak(&req->payload, &old, payload));
    } else {
        old = atomic_exchange(&req->payload, payload);
    }

    if (old == ASYNC_IDLE) {
        req->next = atomic_load(&list->head);
        while (!atomic_compare_exchange_weak(&list->head, &req->next, req)) {}
    }
    return true;
}

static inline void handle_requests(AsyncRequestList *list, RequestHandler fn)
{
    AsyncRequest *iter, *next;

    /* Setup the whole damn thing only to handle seldomly-happen events. */
    if (likely(!atomic_load_explicit(&list->head, memory_order_relaxed))) {
        return;
    }

    for (iter = atomic_exchange(&list->head, nullptr);
         iter && (next = iter->next, true); iter = next) {
        iter->next = nullptr;
        fn(iter, atomic_fetch_and(&iter->payload, ASYNC_DESTROYED));
    }
}

#endif
