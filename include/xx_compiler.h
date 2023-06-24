#ifndef XX_COMPILER_H
#define XX_COMPILER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define likely(x)       __builtin_expect(!!(x), true)
#define unlikely(x)     __builtin_expect(!!(x), false)

#define container_of(ptr, type, field) ({                               \
    const typeof(((type){}).field) *ptr_ = ptr;    /* type-check */     \
    (type *)((void *)ptr_ - offsetof(type, field));                     \
})

#ifndef unreachable
#define unreachable() (__builtin_unreachable())
#endif

#define debug_assert(cond) do {                                         \
    if (!(cond)) {                                                      \
        assert(false);                                                  \
        unreachable();                                                  \
    }                                                                   \
} while (false)

#endif
