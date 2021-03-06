/******************************************************************************
 *
 *  Title:        timing.c
 *
 *****************************************************************************/

#include "timing.h"

/*------------------------------------------------------------------------------
 * float timing_check_period()
 * Check if a period (in seconds) has elapsed for a timer. Return the elapsed
 * time if period has elapsed.
 *----------------------------------------------------------------------------*/

float timing_check_period(TIMING *timer, float period)
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
	if (t2 - t1 > (period * 1000000)) {
		return (float)(t2 - t1) / (1000000.);
	}

	return (float)TIMING_ERROR;
} /* end timing_check_period() */


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


/*------------------------------------------------------------------------------
 * int timing_get_dt()
 * Get the time elapsed for a given timer. Return the elapsed time in another
 * TIMING element.
 *----------------------------------------------------------------------------*/

int timing_get_dt(TIMING *timer, TIMING *elapsed)
{
	/// Declare variables.
	struct timeval t = {0, 0};

	/// Get the current system time.
	gettimeofday(&t, NULL);

	/// Check to see which fraction of a second is larger.
	if (t.tv_usec > timer->us) {
		elapsed->s  = timer->s - t.tv_sec;
		elapsed->us = 1000000 + timer->us - t.tv_usec;
		/// This should never happen. If it does then there is a problem with the system time.
		if(elapsed->s < 0)
			return TIMING_ERROR;
	}
	else {
		elapsed->s  = timer->s - t.tv_sec;
		elapsed->us = timer->us - t.tv_usec;
	}

	return TIMING_SUCCESS;
} /* end timing_get_dt() */


/*------------------------------------------------------------------------------
 * float timing_get_dts()
 * Get the time elapsed for a given timer in seconds.
 *----------------------------------------------------------------------------*/

float timing_get_dts(TIMING *timer)
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

	/// Return time in seconds.
	return (float)(t2 - t1) / (1000000.);
} /* end timing_get_dts() */


/*------------------------------------------------------------------------------
 * int timing_s2us()
 * Convert a TIMING struct to the time in microseconds.
 *----------------------------------------------------------------------------*/

int timing_s2us(TIMING *timer)
{
	/// Convert from seconds.microseconds to microseconds.
	return (timer->s * 1000000) + timer->us;
} /* end timing_s2us() */
