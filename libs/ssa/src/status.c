/*************************************************************************
*
*	status.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*
*  This module provides functions which manipulate the STATUS and
*    SCI_DBL data structures.
*
**************************************************************************
*
*  version 1.2.7, 2/1/09, SBS
*    - rename nId --> nPlatformID
*      ; sName --> sPlatformName
*      ; nType --> nPlatformType
*      ; sTypeName --> sPlatformTypeName
*      ; nMode --> nPlatformMode
*      ; sModeName --> sPlatformModeName
*
*  version 1.2.6, 1/8/09, SBS
*    - LogPrint --> LogLevelPrint
*
*  version 1.2.5, 1/2/09, SBS
*    - remove time and unit comparisons from compareSciDbl()
*
*  version 1.2.4, 10/22/08, SBS
*    - add isnan() check to SetTimeString()
*
*  version 1.2.3, 10/19/08, SBS
*    - add iType, sType;
*
*  version 1.2.2, 10/4/08, SBS
*    - documentation
*
*  version 1.2.1, 8/27/08, SBS
*    - add SetTimeString()
*
*  version 1.2, 8/18/08, SBS
*    - add complete waypoint list to STATUS struct
*    - cleanup
*
*  version 1.1.12, 8/13/08, SBS
*    - add VerifyStatus()
*
*  version 1.1.11, 5/28/08, SBS
*    - rename surfacevel -> currentspeed
*    - add currentangle
*
*  version 1.1.10, 5/5/08, SBS
*    - rename groundspeed -> waterspeed
*
*  version 1.1.9, 4/30/08, SBS
*    - add groundspeed
*
*  version 1.1.8, 3/21/08, SBS
*    - minor cleanup
*
*  version 1.1.7, 3/12/08, SBS
*    - minor changes to printscidbl and printstatus
*
*  version 1.1.6, 2/21/08, SBS
*    - add nCurrWaypt, CurrWpt to status struct
*    - (reconciles with forked version for simulator)
*
*  version 1.1.5, 1/28/08, SBS
*    - minor cleanup
*
*  version 1.1.4, 1/9/08, SBS
*    - add FooFromBar functions
*
*  version 1.1.3, 12/17/07, SBS
*    - changes to logfile interface
*
*  version 1.1.2, 11/30/07, SBS
*    - logfile output
*
*  version 1.1.1, 10/3/07, SBS
*    - minor cleanup
*
*  version 1.1, 6/7/07, SBS
*    - changes to structures due to incorporate science data
*    - split waypoint functions out to waypoints.c
*
*  version 1.0, 5/21/07, SBS
*    - separated from irc_client.c and mocu_client.c
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
*  This module provides functions which manipulate the STATUS and
*    SCI_DBL data structures.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include <libgnome/gnome-config.h>

#include "waypoints.h"
#include "readconfigfile.h"
#include "logfile.h"

#include "platform_types.h"
#include "platform_modes.h"
#include "status.h"

#define CMDSTRLENGTH 512


/*******************************
* initialization values
*******************************/

#define INITSTRING "-255"
#define INITVAL -255

#define TIMEFORMAT "%a %b %d %T %Z %Y"


/************************************************
*
*  status_WaypointFromStatus()
*/
/**
*
* @brief
*  Extract waypoint from STATUS struct
*
*  Parameters:
*   - s - status struct
*
*  Returns:
*   - waypoint struct
*
************************************************/
WAYPT status_WaypointFromStatus( STATUS s )
{
	WAYPT w;

	w.dLatDeg = s.dLatDeg;
	w.dLonDeg = s.dLonDeg;
	w.dSpeedMps = s.dSpeed_mps;

	return( w );
}


/************************************************
*
*  status_PositionFromStatus()
*/
/**
*
* @brief
*  Extract position from STATUS struct
*
*  Parameters:
*   - s - status struct
*
*  Returns:
*   - position struct
*
************************************************/
POSITION status_PositionFromStatus( STATUS s )
{
	POSITION p;

	p.dLatDeg = s.dLatDeg;
	p.dLonDeg = s.dLonDeg;

	return( p );
}


/************************************************
*
*  status_PrintSciDbl()
*/
/**
*
* @brief
*  Prints a SCI_DBL to the screen.
*
*  Parameters:
*    - d - the SCI_DBL
*
*  Returns:
*    - none
*
************************************************/
void status_PrintSciDbl( SCI_DBL d )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, LOGSTRLEN
	          , "     %6.3f (%s)"
	          , d.dValue
	          , d.sUnits );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN
	          , "     (%s)"
	          , d.sTime );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN
	          , "     (%8.5f, %8.5f)"
	          , d.position.dLatDeg
	          , d.position.dLonDeg );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

#endif

	return;
}


/************************************************
*
* status_PrintStatus()
*/
/**
*
* @brief
* Prints the contents of a STATUS structure to
*   the log file.
*
* Parameters:
*   - s - the structure to print
*
* Returns:
*   - none
*
************************************************/
void status_PrintStatus( STATUS s )
{

#ifdef DEBUG
	char temp[LOGSTRLEN];
	register int i;

	snprintf( temp, LOGSTRLEN, "Platform : %s", s.sPlatformName );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " sTime           : %s", s.sTime );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " nTime           : %ld", s.nTime );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " nPlatformMode           : %d", s.nPlatformMode );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " sPlatformModeName       : %s", s.sPlatformModeName );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " nPlatformType           : %d", s.nPlatformType );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " sPlatformTypeName       : %s", s.sPlatformTypeName );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " nCommsLatency   : %d", s.nCommsLatency );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " nWaypointCount  : %d", s.nWaypointCount );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " nCurrWaypt      : %d", s.nCurrWaypt );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	waypoints_WaypointPrintWithString( " Current Waypoint : ", s.CurrWpt );

	for ( i = 0;i < s.nWaypointCount;i++ ) {
		snprintf( temp, LOGSTRLEN, "waypoint #%d : ", i );

		waypoints_WaypointPrintWithString( temp, s.Waypoints[i] );
	}

	snprintf( temp, LOGSTRLEN, " dLatDeg         : %f", s.dLatDeg );

	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );
	snprintf( temp, LOGSTRLEN, " dLonDeg         : %f", s.dLonDeg );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " dAltMeters      : %f", s.dAltMeters );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );
	snprintf( temp, LOGSTRLEN, " dSpeed_mps      : %f", s.dSpeed_mps );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );
	snprintf( temp, LOGSTRLEN, " dWaterspeed_mps : %f"
	          , s.dWaterspeed_mps );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " dHdgDeg         : %f", s.dHdgDeg );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );
	snprintf( temp, LOGSTRLEN, " dPitchDeg       : %f", s.dPitchDeg );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );
	snprintf( temp, LOGSTRLEN, " dRollDeg        : %f", s.dRollDeg );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, " dBattVolts      : %f", s.dBattVolts );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Air Pressure : " );
	status_PrintSciDbl( s.dAirPress_mbar );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Air Temperature : " );
	status_PrintSciDbl( s.dAirTempDegC );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Bathymetry : " );
	status_PrintSciDbl( s.dBathymetry );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Fluorescence : " );
	status_PrintSciDbl( s.dFluorescence_V );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Humidity : " );
	status_PrintSciDbl( s.dHumidPct );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Salinity : " );
	status_PrintSciDbl( s.dSalinity_psu );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Water Temperature : " );
	status_PrintSciDbl( s.dWaterTempDegC );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Windspeed : " );
	status_PrintSciDbl( s.dWindSpeed_mps );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Wind angle : " );
	status_PrintSciDbl( s.dWindAngleDeg );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Current speed : " );
	status_PrintSciDbl( s.dCurrentSpeedMPS );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "Current angle : " );
	status_PrintSciDbl( s.dCurrentAngleDeg );
#endif

	return;
}


/************************************************
*
* status_compareEngStatus()
*/
/**
*
* @brief
* Compares the engineering portion of two status structures
*
* Parameters:
*   - s1 - old status structure
*   - s2 - new status structure
*
* Return value:
*   - 0 = the same
*   - 1 = not the same
*
************************************************/
int status_compareEngStatus( STATUS s1, STATUS s2 )
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	snprintf( temp, LOGSTRLEN
	          , "status_compareEngStatus(): Lat: old / new : %f / %f"
	          , s1.dLatDeg, s2.dLatDeg );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, temp );
#endif

	if ( s1.dLatDeg != s2.dLatDeg ) {
		return( 1 );
	}

	if ( s1.dLonDeg != s2.dLonDeg ) {
		return( 1 );
	}

	if ( s1.dAltMeters != s2.dAltMeters ) {
		return( 1 );
	}

	if ( s1.dSpeed_mps != s2.dSpeed_mps ) {
		return( 1 );
	}

	if ( s1.dWaterspeed_mps != s2.dWaterspeed_mps ) {
		return( 1 );
	}

	if ( s1.dHdgDeg != s2.dHdgDeg ) {
		return( 1 );
	}

	if ( s1.dPitchDeg != s2.dPitchDeg ) {
		return( 1 );
	}

	if ( s1.dRollDeg != s2.dRollDeg ) {
		return( 1 );
	}

	if ( s1.dBattVolts != s2.dBattVolts ) {
		return( 1 );
	}

	return( 0 );
}


/************************************************
*
* status_compareSciDbl()
*/
/**
*
* @brief
* Compares two SCI_DBL structures
*
* Parameters:
*   - s1 - old SCI_DBL structure
*   - s2 - new SCI_DBL structure
*
* Return value:
*   - 0 = the same
*   - 1 = not the same
*
************************************************/
int status_compareSciDbl( SCI_DBL s1, SCI_DBL s2 )
{

	if ( s1.dValue != s2.dValue ) {
		return( 1 );
	}

	if ( s1.position.dLatDeg != s2.position.dLatDeg ) {
		return( 1 );
	}

	if ( s1.position.dLonDeg != s2.position.dLonDeg ) {
		return( 1 );
	}

	/*
		if(strcmp(s1.sTime,s2.sTime)!=0){
			return(1);
		}
		if(strcmp(s1.sUnits,s2.sUnits)!=0){
			return(1);
		}
	*/
	return( 0 );
}


/************************************************
*
* Status_compareStatus()
*/
/**
*
* @brief
* Compares two status structures
*
* Parameters:
*   - s1 - old status structure
*   - s2 - new status structure
*
* Return value:
*   - 0 = the same
*   - 1 = not the same
*
************************************************/
int status_compareStatus( STATUS s1, STATUS s2 )
{

	if ( status_compareEngStatus( s1, s2 ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dHumidPct, s2.dHumidPct ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dAirTempDegC, s2.dAirTempDegC ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dWaterTempDegC, s2.dWaterTempDegC ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dWindSpeed_mps, s2.dWindSpeed_mps ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dWindAngleDeg, s2.dWindAngleDeg ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dAirPress_mbar, s2.dAirPress_mbar ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dWindAngleDeg, s2.dWindAngleDeg ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dSalinity_psu, s2.dSalinity_psu ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dFluorescence_V, s2.dFluorescence_V ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dBathymetry, s2.dBathymetry ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dCurrentSpeedMPS, s2.dCurrentSpeedMPS ) != 0 ) {
		return( 1 );
	}

	if ( status_compareSciDbl( s1.dCurrentAngleDeg, s2.dCurrentAngleDeg ) != 0 ) {
		return( 1 );
	}

	return( 0 );
}


/************************************************
*
* status_initSciDbl()
*/
/**
*
* @brief
* Returns an initialized SCI_DBL struct.
*
* Parameters:
*   - none
*
* Returns:
*   - the sci_dbl struct
*
************************************************/
SCI_DBL status_InitSciDbl()
{
	SCI_DBL d;

	d.dValue = INITVAL;

	strcpy( d.sUnits, INITSTRING );
	strcpy( d.sTime, INITSTRING );

	d.position = waypoints_PositionInit();

	return( d );
}


/************************************************
*
* status_InitStatus()
*/
/**
*
* @brief
* Returns an initialized STATUS struct.
*
* Parameters:
*   - none
*
* Returns:
*   - the status struct
*
************************************************/
STATUS status_InitStatus()
{
	STATUS s;
	register int i;

	s.nPlatformID = INITVAL;
	strcpy( s.sPlatformName, INITSTRING );
	s.nPlatformMode = INITVAL;
	strcpy( s.sPlatformModeName, INITSTRING );
	s.nPlatformType = INITVAL;
	strcpy( s.sPlatformTypeName, INITSTRING );

	s.nWaypointCount = INITVAL;

	s.nCurrWaypt = INITVAL;
	s.CurrWpt = waypoints_WaypointInit();

	for ( i = 0;i < s.nWaypointCount;i++ ) {
		s.Waypoints[i] = waypoints_WaypointInit();
	}

	strcpy( s.sTime, INITSTRING );

	s.nTime = INITVAL;

	s.nCommsLatency = COMMS_LATENCY_GOOD;

	s.dLonDeg = INITVAL;
	s.dLatDeg = INITVAL;
	s.dAltMeters = INITVAL;

	s.dSpeed_mps = INITVAL;
	s.dWaterspeed_mps = INITVAL;

	s.dHdgDeg = INITVAL;
	s.dPitchDeg = INITVAL;
	s.dRollDeg = INITVAL;
	s.dBattVolts = INITVAL;

	s.dHumidPct = status_InitSciDbl();
	s.dAirTempDegC = status_InitSciDbl();
	s.dWaterTempDegC = status_InitSciDbl();
	s.dWindSpeed_mps = status_InitSciDbl();
	s.dWindAngleDeg = status_InitSciDbl();
	s.dAirPress_mbar = status_InitSciDbl();
	s.dSalinity_psu = status_InitSciDbl();
	s.dFluorescence_V = status_InitSciDbl();
	s.dBathymetry = status_InitSciDbl();
	s.dCurrentSpeedMPS = status_InitSciDbl();
	s.dCurrentAngleDeg = status_InitSciDbl();

	return( s );
}


/************************************************
*
* status_VerifyStatus()
*/
/**
*
* @brief
* Evaluates a status struct to see if key entries
*    are invalid
*
* Returns:
*   - 0 if valid
*   - 1 otherwise
*
************************************************/
int status_VerifyStatus(
    STATUS s )
{
	if ( s.nPlatformID == INITVAL ) {
		return( 1 );
	}

	if ( s.nPlatformMode == INITVAL ) {
		return( 1 );
	}

	if ( s.dLonDeg == INITVAL ) {
		return( 1 );
	}

	if ( s.dLatDeg == INITVAL ) {
		return( 1 );
	}

	return( 0 );
}


/************************************************
*
* status_SetTimeString()
*/
/**
*
*
* Returns:
*   - none
*
************************************************/
void status_SetTimeString(
    char *s
    , int size
    , struct tm *time )
{
//	if(isnan(time)){
//		snprintf(s,size,"0");
//	}else{
	strftime( s, size, TIMEFORMAT, time );
//	}

	return;
}
