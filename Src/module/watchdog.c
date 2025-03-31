#include "watchdog.h"
#include <malloc.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

struct watchdog_t
{
	timer_t timer;
	unsigned int  delay;
	watchdog_callback callback;
};

static void watchdog_function(union sigval val)
{
	watchdog_t *watchdog = (watchdog_t *)val.sival_ptr;
    
    if (watchdog->callback != NULL) {
        watchdog->callback(watchdog);
    }
}

watchdog_t * watchdog_create(unsigned int delay, watchdog_callback callback)
{
	watchdog_t * watchdog = (watchdog_t *)malloc(sizeof(watchdog_t));
	if (watchdog == NULL) {
		return NULL;
	}

	watchdog->delay = delay;
	watchdog->callback = callback;
	return watchdog;
}

void watchdog_start(watchdog_t * watchdog)
{
	struct sigevent sev;
	struct itimerspec its;

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_value.sival_ptr = watchdog;
	sev.sigev_notify_function = watchdog_function;
	sev.sigev_notify_attributes = NULL;

	if (timer_create(CLOCK_REALTIME, &sev, &watchdog->timer) != 0) {
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

	its.it_value.tv_sec = watchdog->delay;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(watchdog->timer, 0, &its, NULL) != 0) {
		perror("timer_settime");
		exit(EXIT_FAILURE);
	}
}

void watchdog_cancel(watchdog_t * watchdog)
{
	struct itimerspec its;
    
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    
    timer_settime(watchdog->timer, 0, &its, NULL);
}

void watchdog_destroy(watchdog_t * watchdog)
{
	if (watchdog != NULL) {
		timer_delete(watchdog->timer);
		free(watchdog);
	}
}

