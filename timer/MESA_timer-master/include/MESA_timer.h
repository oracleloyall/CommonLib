/************************************************
*				MESA timer API
* author:tangqi@iie.ac.cn,zhengchao@iie.ac.cn
* version: v1.0
* last modify:2015.08.18
************************************************/

#ifndef	_MESA_TIMER_INCLUDE_
#define	_MESA_TIMER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "queue.h"

/* Timer's handler */
typedef struct{
}MESA_timer_t;

typedef struct{
}MESA_timer_index_t;

typedef void timer_cb_t(void * event);

#define	TM_TYPE_QUEUE 0
#define	TM_TYPE_WHEEL 1

#define MAX_WHEEL_SIZE 10000

/**
 * Description:
 *     Create a timer with type of TS_TYPE. Until now we support queue
 *     and time wheel.
 * Params:
 *     wheel_size: When timer's type is TM_TYPE_QUEUE, wheel_size is unused. It is
 *                the size to initlize time wheel
 *     TS_TYPE: It is a micro defination for timer's type.
 *              TM_TYPE_QUEUE represents double linedlist,
 *              TM_TYPE_WHEEL represents time wheel.
 *  Return:
 *     On success, return a timer, else return NULL
 *
 **/
MESA_timer_t * MESA_timer_create(long wheel_size, int TM_TYPE);


/**
 * Description:
 *     Add a timeout work to a given timer
 * Params:
 *     timer: The timer returned by MESA_timer_create function.
 *     current_time: The current time when add the timer element. It MUST >= 0
 *     timeout: The work's timeout time. It MUST >= 0
 *     callback: It is callback function of a work when timeout.
 *     event: It is the event for user to define.
 *     index: Address(Index) of the timer_node pointer related to the event is
 *            stored in index.
 * Return:
 *      On success 0 is returned, else -1 is returned
 **/
int MESA_timer_add(MESA_timer_t * timer, long current_time, long timeout, \
    timer_cb_t callback, void* event, MESA_timer_index_t ** index);


/**
 * Description:
 *     Delete a MESA_timer_index_t from timer, and then execute callback function.
 * Params:
 *     timer: The timer created by MESA_timer_create.
 *     index: MESA_timer_index_t structure returned by MESA_timer_add function.
 *            Now we want to delete it.
 *     callback: Callback function we want to invoke
 *     para: The parameters used by callback.
 * Return:
 *     On success, return the event's expire. Otherwise -1 is returned.
 **/
long MESA_timer_del(MESA_timer_t * timer, MESA_timer_index_t * index, \
    timer_cb_t callback, void * para);


/**
 * Description:
 *     This function is called by user one or severial times every time_tick.
 *     It will check event's in timer and find the events which are timeout.
 *     Then invoke the callback function registered by related event.
 * Params:
 *     timer: The same as upper funtion.
 *     current_tick: Current time when call MESA_timer_check function. NOTE
 *                   that it is also absolute time, and have the same accuracy
 *                   with the timeout in function MESA_timer_add.
 *     max_cb_times: Max times to call callback function.
 * Return:
 *     Return execute times of callback if success, 0 means no timeout event.
 *     Return -1 when error occurs.
 **/
long MESA_timer_check(MESA_timer_t * timer, long current_time, long max_cb_times);


/**
 * Description:
 *     Destroy the given timer, free the memory and execute callback function.
 * Params:
 *     timer: The timer we wants to destroy.
 *     callback: Callback function.
 *     para: Parameters of callback.
 * Return:
 *     void
 **/

void MESA_timer_destroy(MESA_timer_t * timer, timer_cb_t * callback, void * para);


/**
 * Description:
 *    Get the count of events in timer
 * Params:
 *    timer: Timer returned by MESA_timer_create function.
 * Return:
 *    Return the count of events in timer.
 **/
long MESA_timer_count(MESA_timer_t * timer);


/**
 * Description:
 *     Get the memory of timer
 * Params:
 *     timer: Timer returned by MESA_timer_create function.
 * Return:
 *     Return the memory occupancy of timer.
 **/

long MESA_timer_memsize(MESA_timer_t * timer);


/**
 * Description:
 *     Reset an existing timer element to new current_time and timeout.
 * Params:
 *     timer: The timer returned by MESA_timer_create function.
 *     index: pointer to a pointer of timer element't index.
 *     current_time: current time.
 *     timeout: relative timeout of timer element.
 * Return:
 *     On success, 0 is returned, else -1 is returned.
 **/
int MESA_timer_reset(MESA_timer_t * timer, MESA_timer_index_t * index, long current_time, long timeout);

#ifdef	__cplusplus
}
#endif

#endif	//_MESA_TIMER_INCLUDE_
