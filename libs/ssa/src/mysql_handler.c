/*************************************************************************
*
*	mysql_handler.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module provides functions which interact with the
*    MySQL database.
*
**************************************************************************
*
*  version 1.1.1, 1/2/09, SBS
*    - modify writeMySQL_SciDbl() to not write if value = -255
*
*  version 1.1, 1/1/09, SBS
*    - add writeMySQL_status2()
*
*  version 1.0.7, 11/11/08, SBS
*    - changes to accomodate new status.c
*    - sprintf ->snprintf
*
*  version 1.0.6, 10/4/08, SBS
*    - documentation
*
*  version 1.0.5, 5/29/08, SBS
*    - add current speed/angle
*
*  version 1.0.5, 5/5/08, SBS
*    - rename groundspeed --> waterspeed
*
*  version 1.0.4, 4/30/08, SBS
*    - add groundspeed
*
*  version 1.0.3, 4/1/08, SBS
*    - minor cleanup
*
*  version 1.0.3, 1/28/08, SBS
*    - minor cleanup
*
*  version 1.0.2, 12/17/07, SBS
*    - changes to logfile interface
*
*  version 1.0.1, 11/30/07, SBS
*    - logfile output
*
*  version 1.0, 10/2/07, SBS
*		- split from mysql_client.c
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
*  along with this program; if not, write to the Free Software Foundation, query
*  Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*
**************************************************************************/
/**
*  @file
*  @ingroup common
*  @brief
*  This module provides functions which interact with the
*    MySQL database.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#include <mysql.h>

#include "waypoints.h"
#include "config.h"

#include "platform_types.h"
#include "platform_modes.h"
#include "status.h"
#include "logfile.h"

#include "mysql_handler.h"


#define QUERYSTRLENGTH 512


/************************************************
*
* writeMySQL_EngStatus()
*
* Write engineering portion of STATUS structure to MySQL database
*
* Parameters:
*  db   - the database struct
*  st   - the status struct
*  time - current time (seconds since 1/1/70)
*
* Returns:
*  none
*
************************************************/
static void writeMySQL_EngStatus( pMYSQL db, STATUS st, int time )
{
	char query[QUERYSTRLENGTH];
	int rval;

#ifdef DEBUG
	char temp[LOGSTRLEN];
	logfile_LogPrint( "Writing to engineering table\t" );
#endif

	snprintf( query, QUERYSTRLENGTH
	          , "INSERT INTO engineering (platformId, platformName, "
	          "mode, modeName, lat_deg,lon_deg, altitude_m,"
	          "speed_mps,waterspeed_mps,heading_deg,pitch_deg,roll_deg,"
	          "battery_volts,numWaypoints,time,timestamp) "
	          "VALUES ('%i','%s','%d','%s','%f','%f','%f','%f','%f', "
	          "'%f','%f','%f','%f','%d','%s','%d')"
	          , st.nId
	          , st.sName
	          , st.nMode
	          , st.sModeName
	          , st.dLatDeg
	          , st.dLonDeg
	          , st.dAltMeters
	          , st.dSpeed_mps
	          , st.dWaterspeed_mps
	          , st.dHdgDeg
	          , st.dPitchDeg
	          , st.dRollDeg
	          , st.dBattVolts
	          , st.nWaypointCount
	          , st.sTime
	          , time );

	rval = mysql_query( db, query );

#ifdef DEBUG
	logfile_LogPrint( query );

	snprintf( temp, LOGSTRLEN, "rval = %d\t", rval );
	logfile_LogPrint( temp );

	snprintf( temp, LOGSTRLEN, "error = %s\n", mysql_error( db ) );
	logfile_LogPrint( temp );
#endif

	return;
}


/************************************************
*
* writeMySQL_SciDbl()
*
* Write SCIDBL structure to MySQL database
*
* Parameters:
*  db     - the database struct
*  table  - name of the table to write to
*  scidbl - the scidbl to write
*  time   - current time (seconds since 1/1/70)
*  st     - the current status struct
*
* Returns:
*  none
*
************************************************/
static void writeMySQL_SciDbl( pMYSQL db, char *table, SCI_DBL scidbl, int time, STATUS st )
{
	char query[QUERYSTRLENGTH];
	int rval;

	if ( scidbl.dValue > -250 ) {

#ifdef DEBUG
		char temp[LOGSTRLEN];
		snprintf( temp, LOGSTRLEN, "Writing to %s table\t", table );
		logfile_LogPrint( temp );
#endif

		snprintf( query, QUERYSTRLENGTH
		          , "INSERT INTO %s (value, units, lat_deg,lon_deg,time,"
		          "timestamp,platformID,platformName) "
		          "VALUES ('%f','%s','%f','%f','%s','%d','%d','%s')"
		          , table
		          , scidbl.dValue
		          , scidbl.sUnits
		          , scidbl.position.dLatDeg
		          , scidbl.position.dLonDeg
		          , scidbl.sTime
		          , time
		          , st.nId
		          , st.sName
		        );

		rval = mysql_query( db, query );

#ifdef DEBUG
		logfile_LogPrint( query );

		snprintf( temp, LOGSTRLEN, "rval = %d\t", rval );
		logfile_LogPrint( temp );

		snprintf( temp, LOGSTRLEN, "error = %s\n", mysql_error( db ) );
		logfile_LogPrint( temp );
#endif
	}

	return;
}


/************************************************
*
* writeMySQL_status()
*
* Write STATUS structure to MySQL database
*
* Parameters:
*  db  - the database struct
*  st  - the current status struct
*  old - the previous status struct
*
* Returns:
*  none
*
************************************************/
void writeMySQL_status( pMYSQL db, STATUS st, STATUS old )
{

	struct timeb tim;
	int time;

#if DEBUG
	char temp[LOGSTRLEN];
	snprintf( temp, LOGSTRLEN, "Writing data to database" );
#endif

	ftime( &tim );  //get current time
	time = ( int )( tim.time ); // in second since 1/1/70

	if ( status_compareEngStatus( st, old ) == 1 ) {
		writeMySQL_EngStatus( db, st, time );
	}

	if ( status_compareSciDbl( st.dHumidPct, old.dHumidPct ) == 1 ) {
		writeMySQL_SciDbl( db, "humidity", st.dHumidPct, time, st );
	}

	if ( status_compareSciDbl( st.dAirTempDegC, old.dAirTempDegC ) == 1 ) {
		writeMySQL_SciDbl( db, "airtemp", st.dAirTempDegC, time, st );
	}

	if ( status_compareSciDbl( st.dWaterTempDegC, old.dWaterTempDegC ) == 1 ) {
		writeMySQL_SciDbl( db, "watertemp", st.dWaterTempDegC, time, st );
	}

	if ( status_compareSciDbl( st.dWindSpeed_mps, old.dWindSpeed_mps ) == 1 ) {
		writeMySQL_SciDbl( db, "windspeed", st.dWindSpeed_mps, time, st );
	}

	if ( status_compareSciDbl( st.dWindAngleDeg, old.dWindAngleDeg ) == 1 ) {
		writeMySQL_SciDbl( db, "windangle", st.dWindAngleDeg, time, st );
	}

	if ( status_compareSciDbl( st.dAirPress_mbar, old.dAirPress_mbar ) == 1 ) {
		writeMySQL_SciDbl( db, "airpressure", st.dAirPress_mbar, time, st );
	}

	if ( status_compareSciDbl( st.dSalinity_psu, old.dSalinity_psu ) == 1 ) {
		writeMySQL_SciDbl( db, "salinity", st.dSalinity_psu, time, st );
	}

	if ( status_compareSciDbl( st.dFluorescence_V, old.dFluorescence_V ) == 1 ) {
		writeMySQL_SciDbl( db, "fluorescence", st.dFluorescence_V, time, st );
	}

	if ( status_compareSciDbl( st.dBathymetry, old.dBathymetry ) == 1 ) {
		writeMySQL_SciDbl( db, "bathymetry", st.dBathymetry, time, st );
	}

	if ( status_compareSciDbl( st.dCurrentSpeedMPS, old.dCurrentSpeedMPS ) == 1 ) {
		writeMySQL_SciDbl( db, "currentspeed", st.dCurrentSpeedMPS, time, st );
	}

	if ( status_compareSciDbl( st.dCurrentAngleDeg, old.dCurrentAngleDeg ) == 1 ) {
		writeMySQL_SciDbl( db, "currentangle", st.dCurrentAngleDeg, time, st );
	}


	return;
}


/************************************************
*
* writeMySQL_status()
*
* Write STATUS structure to MySQL database
*
* Parameters:
*  db  - the database struct
*  st  - the current status struct
*  old - the previous status struct
*
* Returns:
*  none
*
************************************************/
void writeMySQL_status2( pMYSQL db, STATUS st )
{

	struct timeb tim;
	int time;

#if DEBUG
	char temp[LOGSTRLEN];
	snprintf( temp, LOGSTRLEN, "Writing data to database" );
#endif

	ftime( &tim );  //get current time
	time = ( int )( tim.time ); // in second since 1/1/70

	writeMySQL_EngStatus( db, st, time );

	writeMySQL_SciDbl( db, "humidity", st.dHumidPct, time, st );
	writeMySQL_SciDbl( db, "airtemp", st.dAirTempDegC, time, st );
	writeMySQL_SciDbl( db, "watertemp", st.dWaterTempDegC, time, st );
	writeMySQL_SciDbl( db, "windspeed", st.dWindSpeed_mps, time, st );
	writeMySQL_SciDbl( db, "windangle", st.dWindAngleDeg, time, st );
	writeMySQL_SciDbl( db, "airpressure", st.dAirPress_mbar, time, st );
	writeMySQL_SciDbl( db, "salinity", st.dSalinity_psu, time, st );
	writeMySQL_SciDbl( db, "fluorescence", st.dFluorescence_V, time, st );
	writeMySQL_SciDbl( db, "bathymetry", st.dBathymetry, time, st );
	writeMySQL_SciDbl( db, "currentspeed", st.dCurrentSpeedMPS, time, st );
	writeMySQL_SciDbl( db, "currentangle", st.dCurrentAngleDeg, time, st );

	return;
}


/************************************************
*
* init_MySQL()
*
* Opens connection to MySQL database
*
* Parameters:
*   db - pointer to MYSQL structure which will be created
*   dbhost - name of the host
*   dbname - name of the database
*   dbuser - mysql user name
*   dbpswd - password for dbuser
*
* Returns
*   none
*
************************************************/
int init_MySQL( pMYSQL db, char *dbhost, char *dbname, char *dbuser, char *dbpswd )
{
	mysql_init( db );

#ifdef DEBUG
	char temp[LOGSTRLEN];
#endif

#ifdef DEBUG
	snprintf( temp, LOGSTRLEN, "Connecting to database '%s:%s' ... ", dbhost, dbname );
	logfile_LogPrint( temp );
#endif

	if ( !mysql_real_connect( db, dbhost, dbuser, dbpswd, dbname, 0, NULL, 0 ) ) {

#ifdef DEBUG
		snprintf( temp, LOGSTRLEN, "failed (%s)", mysql_error( db ) );
		logfile_LogPrint( temp );
#endif

		printf( "failed (%s)\n", mysql_error( db ) );

		return( -1 );

	}
	else {

#ifdef DEBUG
		logfile_LogPrint( "succeeded." );
#endif
	}

	return( 0 );
}

