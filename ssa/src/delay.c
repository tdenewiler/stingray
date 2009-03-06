/**********************************************************
*
*	delay.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*
*  This module provides wrapper functions for nanosleep
*   which allow the program to sleep for a designated
*   amount of time.
*
**************************************************************************
*
*  version 1.0.4, 1/9/09, SBS
*    - move deltatime() here from platform.c
*
*  version 1.0.3, 10/4/08, SBS
*    - documentation
*
*  version 1.0.2, 4/1/08, SBS
*    - minor cleanup / documentation
*
*  version 1.0.1, 1/14/08, SBS
*    - fix bug with period = 1
*
*	version 1.0, 4/5/07, SBS
*
*
**************************************************************************
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful, but
*  WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*  General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software Foundation,
*  Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*
**************************************************************************/
/**
*  @file
*  @ingroup common
*  @brief
*  This module provides wrapper functions for nanosleep
*   which allow the program to sleep for a designated
*   amount of time.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "delay.h"


/**********************************************************
*
*	setdelay()
*
*  Converts a float into a struct timespec.
*
*  Parameters:
*    period - the desired delay in decimal seconds
*
*  Returns:
*    struct timespec
*
**********************************************************/

struct timespec delay_setdelay( float period ) {

	struct timespec delay;

	if ( period >= 1 ) {
		delay.tv_sec = floor( period );
		delay.tv_nsec = ( period - floor( period ) ) * 1.0e9;
	} else {

		delay.tv_sec = 0;
		delay.tv_nsec = period * 1.0e9;
	}

	return( delay );
}


/**********************************************************
*
*	delay_sleep()
*
*  Wrapper for nanosleep
*
*  Parameters:
*    delay - TIMESPEC struct representing the delay time
*
*  Returns:
*    none
*
**********************************************************/
void delay_sleep( TIMESPEC *delay )
{
	nanosleep( delay, NULL );

	return;
}


/**
********************************************************
*
* delay_deltatime()
*/
/**
*
* @brief
*  Calculates seconds elapsed between two timeb structs
*
*  Parameters:
*    - start - the beginning time
*    - end - the ending time
*
*  Returns:
*    - number of seconds
*
********************************************************/
double delay_deltatime(
    struct timeb start
    , struct timeb end )
{
	double sec;
	double msec;

	sec  = ( double )( end.time - start.time );
	msec = ( double )( end.millitm - start.millitm );

	sec += msec / 1000.;

	return( sec );
}

