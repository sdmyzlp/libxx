#ifndef XX_TIMER_H
#define XX_TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*TimerCallback)(void *arg);

typedef struct Timer_ Timer;
typedef struct TimerControl_ TimerControl;

typedef struct {
    uint32_t ms;
    bool oneshot;
    void *ptr;
} TimerConfig;

Timer *timer_create(const char *name, TimerCallback cb, void *arg,
                    TimerControl *control, const TimerConfig *cfg);
int32_t timer_schedule(Timer *timer);
int32_t timer_cancel(Timer *timer);
void timer_destroy(Timer *timer);

#endif
