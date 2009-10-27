/*************************************************************************
*
*	waypoints.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*
*  This module contains functions that deal with the WAYPT and
*    POSITION structures.
*
**************************************************************************
*
*  version 1.2.4, 1/9/09, SBS
*    - add dCloseEnoughM to WAYPT struct and handling functions
*    - logfile_LogPrint --> logfile_LogLevelPrint
*    - cleanup
*
*  version 1.2.3, 10/21/08, SBS
*    - add #ifdef NEED_READFILE
*
*  version 1.2.2, 10/4/08, SBS
*    - documentation
*
*  version 1.2.1, 7/12/08, SBS
*    - use conversions.h
*
*  version 1.2, 4/13/08, SBS
*    - modify file writing to eliminate gnome push/pop prefix
*
*  version 1.1.3, 3/21/08, SBS
*    - minor cleanup
*
*  version 1.1.2, 3/18/08, SBS
*    - minor cleanup
*
*  version 1.1.1, 2/25/08, SBS
*    - fix bug in waypointscompare
*
*  version 1.1, 1/31/08, SBS
*    - move calcpositiondist, positionadd, positionsubtract,
*        positionrotate into here
*    - rename functions to begin with position/waypoint
*
*  version 1.0.2, 12/9/07, SBS
*    - add logfile output
*    - add initWaypoint
*    - add compareWaypoints
*
*  version 1.0.1, 10/3/07, SBS
*    - minor cleanup
*
*  version 1.0, 6/7/07, SBS
*    - separated from status.c and irc_client.c
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
*  This module contains functions that deal with the WAYPT and
*    POSITION structures.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libgnome/gnome-config.h>

#ifdef NEED_READFILE
#include "readconfigfile.h"
#endif //NEED_READFILE

#include "conversions.h"

#ifdef DEBUG
#include "logfile.h"
#endif

#include "waypoints.h"

#define CMDSTRLENGTH 512

// default initialization value
#define INITVAL -255

// max number of waypoints to consider
#define MAXWPT 16


/********************************************************
*
* HeadingFromWaypoint()
*/
/**
*
* @brief
* Given current location and target location,
*    calculate required heading.
*
* Parameters:
*    - s - current location
*    - w - target location
*
* Returns:
*    - heading from s to w
*
********************************************************/
double waypoints_HeadingFromWaypoint(
    WAYPT s
    , WAYPT w )
{
	double hdg;
	double oldLat, oldLon;
	double newLat, newLon;
	double deltaLon;

	oldLat = RADIANS( s.dLatDeg );
	oldLon = RADIANS( s.dLonDeg );
	newLat = RADIANS( w.dLatDeg );
	newLon = RADIANS( w.dLonDeg );

	deltaLon = newLon - oldLon;

	hdg = atan2( sin( deltaLon ) * cos( newLat )
	             , cos( oldLat ) * sin( newLat ) - sin( oldLat ) * cos( newLat ) * cos( deltaLon ) );

	hdg = DEGREES( hdg );

	hdg = MOD( hdg + 360., 360. );

	return( hdg );
}


/********************************************************
*
* DistanceFromWaypoint()
*/
/**
*
* @brief
* Calculates distance in meters from current position
*   to waypoint.
*
* Parameters:
*   - s - current location
*   - w - target location
*
* Returns:
*   - distance from s to w
*
********************************************************/
double waypoints_DistanceFromWaypoint(
    WAYPT s
    , WAYPT w )
{
	double dist;
	double deltaLon, deltaLat;
	double deltaLonM, deltaLatM;

	deltaLon = ( w.dLonDeg - s.dLonDeg );
	deltaLat = ( w.dLatDeg - s.dLatDeg );

	deltaLonM = LONDEGTOMETERS( deltaLon );
	deltaLatM = LATDEGTOMETERS( deltaLat );

	dist = sqrt( ( deltaLonM * deltaLonM ) + ( deltaLatM * deltaLatM ) );

#ifdef DEBUG9
	printf( "Position = %f,%f\tWaypoint = %f,%f\n"
	        , s.dLatDeg
	        , s.dLonDeg
	        , w.dLatDeg
	        , w.dLonDeg );

	printf( "deltaLon = %f\tdeltaLat = %f\n"
	        , deltaLon
	        , deltaLat );

	printf( "deltaLonM = %f\tdeltaLatM = %f\n"
	        , deltaLonM
	        , deltaLatM );

	printf( "dist = %f\n", dist );
#endif

	return( dist );
}


/************************************************
*
* waypoints_PositionCalcDist()
*/
/**
*
* @brief
* Calculates the distance in meters between
*    the points in two POSITION structs
*
* Parameters:
*   - wp1, wp2 - points
*
* Returns:
*   - distance from wp1 to wp2
*
************************************************/
double waypoints_PositionCalcDist(
    POSITION wp1
    , POSITION wp2 )
{
	double a, c, d;
	double dLatRad, dLonRad;
	double a1, a2, a3;

	dLatRad = RADIANS( fabs( wp1.dLatDeg - wp2.dLatDeg ) );
	dLonRad = RADIANS( fabs( wp1.dLonDeg - wp2.dLonDeg ) );

	a1 = SQ( sin( dLatRad / 2. ) );
	a2 = cos( RADIANS( wp1.dLatDeg ) ) * cos( RADIANS( wp1.dLatDeg ) );
	a3 = SQ( sin( dLonRad / 2. ) );

	a = a1 + a2 * a3;

	c = 2 * atan2( sqrt( a ), sqrt( 1 - a ) );

	d = EARTHRADIUS * c;

	return( d );
}


/************************************************
*
* waypoints_PositionAdd()
*/
/**
*
* @brief
* Add the points in two POSITION structs together
*
* Parameters:
*   - p1, p2 - points
*
* Returns:
*   - component-wise sum of p1 and p2
*
************************************************/
POSITION waypoints_PositionAdd(
    POSITION p1
    , POSITION p2 )
{
	POSITION sum;

	sum.dLatDeg = p1.dLatDeg + p2.dLatDeg;
	sum.dLonDeg = p1.dLonDeg + p2.dLonDeg;

	return( sum );
}


/************************************************
*
* waypoints_PositionSubtract()
*/
/**
*
* @brief
* Calculates the difference between the points
*    in two POSITION structs
*
* Parameters:
*   - p1 - current location
*   - p2 - target location
*
* Returns:
*   - component-wise difference between p1 and p2
*
************************************************/
POSITION waypoints_PositionSubtract(
    POSITION p1
    , POSITION p2 )
{
	POSITION sum;

	sum.dLatDeg = p1.dLatDeg - p2.dLatDeg;
	sum.dLonDeg = p1.dLonDeg - p2.dLonDeg;

	return( sum );
}


/************************************************
*
* waypoints_PositionRotate()
*/
/**
*
* @brief
* Rotates the point in a POSITION struct by
*    a given angle around a center point
*
* Parameters:
*   - p1     - point to rotate
*   - center - center of rotation
*   - angle  - rotation angle
*
* Returns:
*   - Rotated point
*
************************************************/
POSITION waypoints_PositionRotate(
    POSITION p1
    , POSITION center
    , float angle )
{
	POSITION p2, p3, p4;


	p2 = waypoints_PositionSubtract( p1, center );

	p3.dLatDeg = ( p2.dLatDeg ) * cos( angle * M_PI / 180. ) + ( p2.dLonDeg ) * sin( angle * M_PI / 180. );

	p3.dLonDeg = -( p2.dLatDeg ) * sin( angle * M_PI / 180. ) + ( p2.dLonDeg ) * cos( angle * M_PI / 180. );

	p4 = waypoints_PositionAdd( p3, center );

#ifdef DEBUG9
	printf( "initial point (%f,%f)\n", p1.dLatDeg, p1.dLonDeg );
	printf( "center point (%f,%f)\n", center.dLatDeg, center.dLonDeg );
	printf( "after subtract (%f,%f)\n", p2.dLatDeg, p2.dLonDeg );
	printf( "after rotate (%f,%f)\n", p3.dLatDeg, p3.dLonDeg );
	printf( "after add (%f,%f)\n", p4.dLatDeg, p4.dLonDeg );
#endif

	return( p4 );
}


/************************************************
*
* waypoints_WaypointsCompare()
*/
/**
*
* @brief
* Compare two waypoint structures
*
* Parameters:
*   - w1, w2 - waypoints
*
* Returns:
*   - 0 if same
*   - 1 if different
*
************************************************/
int waypoints_WaypointsCompare(
    WAYPT w1
    , WAYPT w2 )
{

	if ( w1.dLatDeg != w2.dLatDeg ) {
		return( 1 );
	}

	if ( w1.dLonDeg != w2.dLonDeg ) {
		return( 1 );
	}

	if ( w1.dSpeedMps != w2.dSpeedMps ) {
		return( 1 );
	}

	if ( w1.dCloseEnoughM != w2.dCloseEnoughM ) {
		return( 1 );
	}

	return( 0 );
}


/************************************************
*
* waypoints_WaypointInit()
*/
/**
*
* @brief
* Returns an initialized waypoint struct
*
* Parameters:
*   - none
*
* Returns:
*   - initialized waypoint struct
*
*
************************************************/
WAYPT waypoints_WaypointInit()
{
	WAYPT w;

	w.dLatDeg = INITVAL;
	w.dLonDeg = INITVAL;
	w.dSpeedMps = INITVAL;
	w.dCloseEnoughM = INITVAL;

	return( w );
}


/************************************************
*
* waypoints_PositionInit()
*/
/**
*
* @brief
* Returns an initialized position struct
*
* Parameters:
*   - none
*
* Returns:
*   - initialized position struct
*
************************************************/
POSITION waypoints_PositionInit()
{
	POSITION p;

	p.dLatDeg = INITVAL;
	p.dLonDeg = INITVAL;

	return( p );
}


/************************************************
*
* waypoints_WptPrint()
*/
/**
*
* @brief
* Prints a waypoint to the log file
*
* Parameters:
*   - wpt - list of waypoints in the current path
*   - num - index to the waypoint being printed
*
* Returns:
*   - none
*
************************************************/
void waypoints_WptPrint(
    WAYPT wpt
    , int num )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, sizeof( temp ), "PrintWpt() - wpts[%d].dLatDeg   = %f\n"
	          , num, wpt.dLatDeg );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );

	snprintf( temp, sizeof( temp ), "PrintWpt() - wpts[%d].dLonDeg   = %f\n"
	          , num, wpt.dLonDeg );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );

	snprintf( temp, sizeof( temp ), "PrintWpt() - wpts[%d].dSpeedMps = %f\n"
	          , num, wpt.dSpeedMps );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );

	snprintf( temp, sizeof( temp ), "PrintWpt() - wpts[%d].dCloseEnoughM = %f\n"
	          , num, wpt.dCloseEnoughM );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );

#endif
	return;
}


/************************************************
*
* waypoints_PositionPrint()
*/
/**
*
* @brief
* Prints a POSITION struct to the log file
*
* Parameters:
*   - p - position struct
*
* Returns:
*   - none
*
************************************************/
void waypoints_PositionPrint(
    POSITION p )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, sizeof( temp ), "(%f,%f)", p.dLatDeg, p.dLonDeg );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );
#endif

	return;
}


/************************************************
*
* waypoints_PositionPrintWithString()
*/
/**
*
* @brief
* Output a POSITION struct to the log file
*    with a leading string passed by the user
*
* Parameters:
*   - s - string
*   - p - position struct
*
* Returns:
*   - none
*
************************************************/
void waypoints_PositionPrintWithString(
    char *s
    , POSITION p )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, sizeof( temp ), "%s(%f,%f)", s, p.dLatDeg, p.dLonDeg );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );
#endif

	return;
}


/************************************************
*
* waypoints_WaypointPrint()
*/
/**
*
* @brief
* Prints a WAYPOINT struct to the log file
*
* Parameters:
*   - p - waypoint struct
*
* Returns:
*   - none
*
************************************************/
void waypoints_WaypointPrint(
    WAYPT p )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, LOGSTRLEN, "(%f,%f) @ %f m/s"
	          , p.dLatDeg, p.dLonDeg, p.dSpeedMps );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );
#endif

	return;
}


/************************************************
*
* waypoints_WaypointPrintWithString()
*/
/**
*
* @brief
* Prints a WAYPOINT struct to the log file
*    with a leading string passed by the user
*
* Parameters:
*   - s - string
*   - p - waypoint struct
*
* Returns:
*   - none
*
************************************************/
void waypoints_WaypointPrintWithString(
    char *s
    , WAYPT p )
{

#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, LOGSTRLEN, "%s(%f,%f) @ %f m/s"
	          , s, p.dLatDeg, p.dLonDeg, p.dSpeedMps );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );

#endif

	return;
}


/************************************************
*
* waypoints_WaypointsPrint()
*/
/**
*
* @brief
* Prints a list of waypoints to the screen
*
* Parameters:
*   - wp    - array of waypoints
*   - count - number of waypoints
*
* Returns:
*   - none
*
************************************************/
void waypoints_WaypointsPrint(
    WAYPT *wp
    , int count )
{
	register int i, j;

	if ( count > MAXWPT ) {
		j = MAXWPT;
	}
	else {
		j = count;
	}

	for ( i = 0; i < j; i++ ) {
		printf( "===SBS=== Waypoint #%d = (%f,%f)\n"
		        , i
		        , wp[i].dLatDeg
		        , wp[i].dLonDeg );

	}

	fflush( stdout );

	return;
}


/************************************************
*
* waypoints_ReadFile()
*/
/**
*
* @brief
* Reads waypoints from a file
*
* Parameters:
*  - filespec - the filename
*  - wpcount  - number of waypoints
*  - wp       - array of waypoints to fill
*
* Returns:
*  - none
*
************************************************/
#ifdef NEED_READFILE
void waypoints_ReadFile(
    char *filespec
    , int wpcount
    , WAYPT *wp )
{

	char cmd[CMDSTRLENGTH];
	register int i, j;

	if ( wpcount > MAXWPT ) {
		j = MAXWPT;
	}
	else {
		j = wpcount;
	}

	for ( i = 0; i < j; i++ ) {

		snprintf( cmd, CMDSTRLENGTH, "waypoint%d_lat", i );
		wp[i].dLatDeg = readconfig_float( filespec, "path", cmd, 0 );
		snprintf( cmd, CMDSTRLENGTH, "waypoint%d_lon", i );
		wp[i].dLonDeg = readconfig_float( filespec, "path", cmd, 0 );

	}

	return;
}

#endif //NEED_READFILE


/************************************************
*
* waypoints_WriteFile()
*/
/**
*
* @brief
* Writes waypoints into a command file
*
* Parameters:
*  - filespec - the filename
*  - count    - number of waypoints
*  - wp       - array of waypoints to fill
*
* Returns:
*  - none
*
************************************************/
void waypoints_WriteFile(
    char *filespec
    , int count
    , WAYPT *wp )
{
	register int i, j;
	char cmd[CMDSTRLENGTH];

//	char prefix[CMDSTRLENGTH];

//	snprintf(prefix,CMDSTRLENGTH,"=%s=/",filespec);

//	gnome_config_push_prefix(prefix);

	if ( count > MAXWPT ) {	//only use first MAXWPT waypoints
		j = MAXWPT;
	}
	else {
		j = count;
	}

	for ( i = 0; i < j; i++ ) {
		snprintf( cmd, CMDSTRLENGTH, "=%s=/path/waypoint%d_lat", filespec, i );
		gnome_config_set_float( cmd, wp[i].dLatDeg );

		snprintf( cmd, CMDSTRLENGTH, "=%s=/path/waypoint%d_lon", filespec, i );
		gnome_config_set_float( cmd, wp[i].dLonDeg );
	}

//	gnome_config_pop_prefix();

	gnome_config_sync();

	return;
}

