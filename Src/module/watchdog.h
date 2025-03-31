#ifndef _WATCHDOG_H
#define _WATCHDOG_H

typedef struct watchdog_t watchdog_t;

//Def du type ''watchdog_callback qui est un pointeur de fonction qui prend en argument en pointeur vers une structure 'watchdog_t'
typedef void (* watchdog_callback)(watchdog_t * watchdog);

/**
 * Watchdog's constructor.
 *
 * @param delay expressed in microseconds
 * @param callback function to be called at expiration
 */
watchdog_t * watchdog_create(unsigned int delay, watchdog_callback callback);

/**
 * Arms the watchdog.
 *
 * @param this watchdog's instance
 */
void watchdog_start(watchdog_t * watchdog);

/**
 * Disarms the watchdog.
 *
 * @param this watchdog's instance
 */
void watchdog_cancel(watchdog_t * watchdog);

/**
 * watchdog's destructor
 *
 * @param this watchdog's instance
 */
void watchdog_destroy(watchdog_t * watchdog);

#endif /* _WATCHDOG_H */

