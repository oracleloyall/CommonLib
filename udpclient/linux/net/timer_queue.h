
#ifndef NET_TIMER_QUEUE_H
#define NET_TIMER_QUEUE_H

struct timer_queue;
typedef struct timer_queue timer_queue_t;

#include "timer.h"
#include "loop.h"

#ifdef __cplusplus
extern "C" {
#endif

timer_queue_t* timer_queue_create(loop_t *loop);

void timer_queue_destroy(timer_queue_t* timer_queue);

/* add timer，if interval !=0 ，is timer */
loop_timer_t *timer_queue_add(timer_queue_t *timer_queue, unsigned long long timestamp, unsigned interval, onexpire_f expirecb, void *userdata);

void timer_queue_cancel(timer_queue_t *timer_queue, loop_timer_t *timer);

void timer_queue_refresh(timer_queue_t *timer_queue, loop_timer_t *timer);

/* 最近的一个timer的时差 */
long timer_queue_gettimeout(timer_queue_t *timer_queue);

void timer_queue_process_inloop(timer_queue_t *timer_queue);

#ifdef __cplusplus
}
#endif

#endif /* !NET_TIMER_QUEUE_H */
