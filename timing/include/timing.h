/**
 *  \file timing.h
 *  \brief Functions for timers and checking to see if an amount of
 * 		   time has passed.
 */

#include <stdio.h>
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
#endif /* RETURN_VALS */


/******************************
 *
 * Data types
 *
 *****************************/

#ifndef _TIMING_
#define _TIMING_
/*! Timer struct. */
typedef struct _TIMING {
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
//! \return dt in seconds if TRUE, 0 if FALSE.
float timing_check_period(TIMING *timer, float period);

//! Sets a timer to the current system time.
//! \param timer A timer to set.
//! \return 1 on success, 0 on failure.
int timing_set_timer(TIMING *timer);

//! Computes elapsed time for a given timer.
//! \param timer A timer value to check.
//! \param elapsed The amount of time elapsed for timer.
//! \return 1 on success, 0 on failure.
int timing_get_dt(TIMING *timer, TIMING *elapsed);

//! Get the time elapsed for a given timer in seconds.
//! \param timer A timer value to check.
//! \return The time elapsed in seconds as a float.
float timing_get_dts(TIMING *timer);

//! Convert from a timing element to microseconds.
//! \param timer A timer value to convert.
//! \return The time in microseconds.
int timing_s2us(TIMING *timer);


#endif /* _TIMING_H_ */
