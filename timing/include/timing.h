/**
 *  \file timing.h
 *  \brief Functions for timers and checking to see if an amount of
 * 		   time has passed.
 */

#include <time.h>
#include <sys/time.h>


#ifndef _TIMING_H_
#define _TIMING_H_

/******************************
 *
 * #defines
 *
 *****************************/

#ifndef TIMING_RETURN_VALS
#define TIMING_SUCCESS			1
#define TIMING_ERROR			0
#endif /* PTU_RETURN_VALS */


/******************************
 *
 * Data types
 *
 *****************************/

#ifndef _TIMING_
#define _TIMING_
/*! Timer struct. */
typedef struct _HEADER {
	int us;	//!< Time in microseconds
    int s;	//!< Time in seconds
} TIMING;
#endif /* _TIMING_ */


/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Checks to see if time has elapsed.
//! \param timer A timer value to check.
//! \param period The amount of time to check against, in seconds.
//! \return 1 on success, 0 on failure.
int timing_check_elapsed(TIMING *timer, float period);

//! Sets a timer to the current system time.
//! \param timer A timer to set.
//! \return 1 on success, 0 on failure.
int timing_set_timer(TIMING *timer);


#endif /* _TIMING_H_ */
