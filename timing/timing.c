/******************************************************************************
 *
 *  Title:        timing.c
 *
 *****************************************************************************/


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "timing.h"


/*------------------------------------------------------------------------------
 * int timing_check_elapsed()
 * Check if a period has elapsed for a timer.
 *----------------------------------------------------------------------------*/

int timing_check_elapsed(TIMING *timer, float period)
{
	/// Declare variables.
	struct timeval t = {0, 0};
	int t1 = 0;
	int t2 = 0;

	/// Get the current system time.
	gettimeofday(&t, NULL);

	/// Convert times to microseconds.
	t1 = (timer->s * 1000000) + timer->us;
	t2 = (t.tv_sec * 1000000) + t.tv_usec;

	/// Check to see if period has elapsed.
	if(t2-t1>(period * 1000000)) {
		return TIMING_SUCCESS;
	}

	return TIMING_ERROR;
} /* end timing_check_elapsed() */


/*------------------------------------------------------------------------------
 * int timing_set_timer()
 * Sets a timer to the current system time.
 *----------------------------------------------------------------------------*/

int timing_set_timer(TIMING *timer)
{
	/// Declare variables.
	struct timeval t = {0, 0};

	/// Get the current system time.
	gettimeofday(&t, NULL);

	/// Set timer to the current system time.
	timer->s  = t.tv_sec;
	timer->us = t.tv_usec;

	return TIMING_SUCCESS;
} /* end timing_set_timer() */
