/**********************************************************
*
*	command.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module defines functions which manipulate
*    the COMMAND structure which holds commands
*    from the OCU.
*
**************************************************************************
*
*  version 1.0.7, 2/1/09, SBS
*    - add _WriteString()
*    - rename nBoatId -> nPlatformID
*       sBoatName -> sPlatformName
*       nCmd --> nCmdID
*
*  version 1.0.6, 1/27/09, SBS
*    - add throttle, rudder, heading to COMMAND struct
*    - documentation
*
*  version 1.0.5, 1/2/09, SBS
*    - LogPrint --> LogLevelPrint
*
*  version 1.0.4, 10/4/08, SBS
*    - documentation
*
*  version 1.0.3, 7/12/08, SBS
*    - reduce log verbosity
*
*  version 1.0.2, 4/3/08, SBS
*    - correct problems when DEBUG is not defined
*
*  version 1.0.1, 4/1/08, SBS
*    - update documentation
*
*  version 1.0, 12/17/07, SBS
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
*  This module defines functions which manipulate
*    the COMMAND structure which holds commands
*    from the OCU.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "waypoints.h"
#include "logfile.h"

#include "command.h"

/*******************************/

#define INITSTRING "-255"
#define INITVAL -255


/***********************************************************
*
*	command_InitCommand()
*/
/**
*
* @brief
*  Returns an initialized COMMAND struct
*
* Parameters:
*   - none
*
* Returns:
*   - the struct
*
***********************************************************/
COMMAND command_InitCommand()
{
	COMMAND c;
	register int i;

	c.nPlatformID = INITVAL;
	strcpy( c.sPlatformName, INITSTRING );

	c.nCmdID = INITVAL;
	strcpy( c.sCmdName, INITSTRING );

	c.nWaypointCount = INITVAL;

	for ( i = 0;i < MAXWAYPOINTS;i++ ) {
		c.Waypoints[i] = waypoints_WaypointInit();
	}

	c.nRudder = 0;

	c.nThrottle = 0;
	c.nHeading = 0;

	return( c );
}


/***********************************************************
*
*	command_PrintCommand()
*/
/**
*
* @brief
*  Prints a COMMAND struct to the log file
*
*  Parameters:
*   - c - the struct
*
*  Returns:
*   - none
*
***********************************************************/
void command_PrintCommand( COMMAND c )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, LOGSTRLEN, "PrintCommand() - nPlatformID : %d", c.nPlatformID );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - sPlatformName : %s", c.sPlatformName );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - nCmdID : %d", c.nCmdID );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - sCmdName : %s", c.sCmdName );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - nRudder : %d", c.nRudder );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - nThrottle : %d", c.nThrottle );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - nHeading : %d", c.nHeading );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "PrintCommand() - nWaypointCount : %d", c.nWaypointCount );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

#ifdef DEBUG3
	{
		register int i;

		for ( i = 0; i < c.nWaypointCount; i++ ) {
			waypoints_WptPrint( c.Waypoints[i], i );
		}
	}
#endif

#endif

	return;
}


/******************************************************************************
*
* command_WriteString()
*/
/**
*
* @brief
* Writes COMMAND struct to a string
*
* Parameters:
*   - outstr - string to write into
*   - size   - size of output string
*   - cmd    - command structure
*
* Returns:
*   - none
*
******************************************************************************/
void command_WriteString(
    char *outstr
    , int size
    , COMMAND cmd )
{
	char str[128];

	strncpy( outstr, "", size );

	snprintf( str, 128, "[global]\n" );
	strncat( outstr, str, size - strlen( outstr ) );

	snprintf( str, 128, "platformID=%d\n", cmd.nPlatformID );
	strncat( outstr, str, size - strlen( outstr ) );
	snprintf( str, 128, "platform=%s\n",   cmd.sPlatformName );
	strncat( outstr, str, size - strlen( outstr ) );

	snprintf( str, 128, "commandID=%d\n",  cmd.nCmdID );
	strncat( outstr, str, size - strlen( outstr ) );
	snprintf( str, 128, "command=%s\n",    cmd.sCmdName );
	strncat( outstr, str, size - strlen( outstr ) );

	switch ( cmd.nCmdID ) {

		case CMD_WPT:

		case CMD_STATIONKEEP: {
			register int i;

			snprintf( str, 128, "numwaypoints=%d\n", cmd.nWaypointCount );
			strncat( outstr, str, size - strlen( outstr ) );

			for ( i = 0; i < cmd.nWaypointCount; i++ ) {
				snprintf( str, 128, "\n" );
				strncat( outstr, str, size - strlen( outstr ) );
				snprintf( str, 128, "[waypoint%d]\n", i );
				strncat( outstr, str, size - strlen( outstr ) );
				snprintf( str, 128, "lat_deg=%f\n"
				          , cmd.Waypoints[i].dLatDeg );
				strncat( outstr, str, size - strlen( outstr ) );
				snprintf( str, 128, "lon_deg=%f\n"
				          , cmd.Waypoints[i].dLonDeg );
				strncat( outstr, str, size - strlen( outstr ) );
				snprintf( str, 128, "speed_mps=%f\n"
				          , cmd.Waypoints[i].dSpeedMps );
				strncat( outstr, str, size - strlen( outstr ) );
				snprintf( str, 128, "closeenough_m=%f\n"
				          , cmd.Waypoints[i].dCloseEnoughM );
				strncat( outstr, str, size - strlen( outstr ) );
			}
		}

		break;

		case CMD_NULL:

		case CMD_HALT:

		case CMD_PAUSE:

		case CMD_RESUME:

		case CMD_INTERRUPT:

		case CMD_UNINTERRUPT:

		case CMD_FREEDRIFT:
			break;

		case CMD_MANUAL:
			snprintf( str, 128, "\n" );
			strncat( outstr, str, size - strlen( outstr ) );
			snprintf( str, 128, "[manual]\n" );
			strncat( outstr, str, size - strlen( outstr ) );
			snprintf( str, 128, "rudder=%d\n", cmd.nRudder );
			strncat( outstr, str, size - strlen( outstr ) );
			snprintf( str, 128, "throttle=%d\n", cmd.nThrottle );
			strncat( outstr, str, size - strlen( outstr ) );
			break;

		case CMD_CALIBRATE:
			break;

		case CMD_HEADINGHOLD:
			snprintf( str, 128, "\n" );
			strncat( outstr, str, size - strlen( outstr ) );
			snprintf( str, 128, "[headinghold]\n" );
			strncat( outstr, str, size - strlen( outstr ) );
			snprintf( str, 128, "heading=%d\n", cmd.nHeading );
			strncat( outstr, str, size - strlen( outstr ) );
			break;

	}

	return;
}


/***********************************************************
*
*	command_CompareCommand()
*/
/**
*
* @brief
*  Compares two COMMAND structs
*
*  Parameters:
*   - c1,c2 - the structs
*
*  Returns:
*   - 0 if identical
*   - 1 if different
*
***********************************************************/
int command_CompareCommand( COMMAND c1, COMMAND c2 )
{
	register int i;

	if ( c1.nPlatformID != c2.nPlatformID ) {
		return( 1 );
	}

	if ( strcmp( c1.sPlatformName, c2.sPlatformName ) != 0 ) {
		return( 1 );
	}

	if ( c1.nCmdID != c2.nCmdID ) {
		return( 1 );
	}

	if ( strcmp( c1.sCmdName, c2.sCmdName ) != 0 ) {
		return( 1 );
	}

	if ( c1.nRudder != c2.nRudder ) {
		return( 1 );
	}

	if ( c1.nThrottle != c2.nThrottle ) {
		return( 1 );
	}

	if ( c1.nHeading != c2.nHeading ) {
		return( 1 );
	}


	if ( c1.nWaypointCount != c2.nWaypointCount ) {
		return( 1 );
	}

	for ( i = 0; i < c1.nWaypointCount; i++ ) {
		if ( waypoints_WaypointsCompare( c1.Waypoints[i], c2.Waypoints[i] ) != 0 ) {
			return( 1 );
		}
	}

	return( 0 );
}


