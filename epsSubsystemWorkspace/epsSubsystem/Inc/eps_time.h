/*
 * eps_time.h
 *
 *  Created on: Jul 17, 2016
 *      Author: ariolis
 */

#ifndef INC_EPS_TIME_H_
#define INC_EPS_TIME_H_

#include "stm32l1xx_hal.h"

#define TIMED_EVENT_PERIOD ((uint32_t)50000)

#define SUBSYSTEM_TIMEOUT_PERIOD 2400 /*this value times TIMED_EVENT_PERIOD is the period that eps waits a timeout ping from every subsystem or it resets the subsystem that has not responded in time.*/

extern volatile uint32_t EPS_time_counter; /*a counter that increments at every interrupt every TIMED_EVENT_PERIOD microseconds */


uint32_t EPS_time_counter_get(void);

void EPS_time_counter_increment(void);


typedef enum {
	TIMED_EVENT_SERVICED =1,
	TIMED_EVENT_NOT_SERVICED,
	TIMED_EVENT_LAST_VALUE
}EPS_timed_event_status;

extern volatile EPS_timed_event_status EPS_event_period_status;/* initialize global timed event flag to true.*/



#endif /* INC_EPS_TIME_H_ */
