

#ifndef NET_LOOP_H
#define NET_LOOP_H

struct loop;
typedef struct loop loop_t;

#include "timer.h"
#include "channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 新建一个事件循环
 */
loop_t* loop_new(unsigned hint);
void loop_destroy(loop_t *loop);
int loop_update_channel(loop_t *loop, channel_t* channel);
void loop_async(loop_t* loop, void(*callback)(void *userdata), void* userdata);
void loop_run_inloop(loop_t* loop, void(*callback)(void *userdata), void* userdata);
int loop_inloopthread(loop_t* loop);
void loop_loop(loop_t* loop);
void loop_quit(loop_t* loop);
loop_timer_t* loop_runafter(loop_t* loop, unsigned interval, onexpire_f expirecb, void *userdata);
loop_timer_t* loop_runevery(loop_t* loop, unsigned interval, onexpire_f expirecb, void *userdata);

void loop_cancel(loop_t* loop, loop_timer_t *timer);
void loop_refresh(loop_t* loop, loop_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif /* NET_LOOP_H */

