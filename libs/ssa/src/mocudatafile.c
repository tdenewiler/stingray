/*************************************************************************
*
*	mocudatafile.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This file contains functions which interact with the data files used
*    to transfer telemetry to MOCU
*
**************************************************************************
*
*  version 1.0.7, 1/1/09, SBS
*    - reconcile with newer common files
*
*  version 1.0.6, 10/4/08, SBS
*    - documentation
*
*  version 1.0.5, 7/12/08, SBS
*    - use conversions.h
*
*  version 1.0.4, 5/28/08, SBS
*    - remove spaces from asf_modename
*    - add current speed/angle
*
*  version 1.0.3, 5/17/08, SBS
*    - add waterspeed
*
*  version 1.0.2, 4/1/08, SBS
*    - minor cleanup
*
*  version 1.0.1, 12/17/07, SBS
*    - replace PI with M_PI
*
*  version 1.0, 7/13/07, SBS
*    - split from telemfile.c
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
*  This file contains functions which interact with the data files used
*    to transfer telemetry to MOCU
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "RobotDatFileFormat.h"

#include "config.h"

#include "waypoints.h"
#include "platform_types.h"
#include "platform_modes.h"
#include "status.h"

//#include "sendretr.h"

#include "conversions.h"

#define FILENAMELENGTH 128

#define CMDSTRLENGTH 512

//#define DEGTORAD(x)	(x/360.*2.*M_PI)

#define WPARRAYSIZE 16


/************************************************
*
*
************************************************/
static void remove_space( char *input )
{
	char *p;

	while ( 1 ) {

		p = strchr( ( const char * )input, ' ' );

		if ( p == NULL ) {
			break;
		}

		*p = '_';

	};

	return;
}


/************************************************
*
* datafile_writeDataFile()
*
* Writes the STATUS structure to a data file for
*  MOCU to read.
*
* Parameters:
*   cf     - configuration data
*   st     - robot status
*   robnum - which robot
*
* Returns:
*    none
*
************************************************/
void mocudatafile_writeDataFile( CONFIGDATA cf, STATUS st, int robnum )
{
	FILE *fp;
	char filespec[FILENAMELENGTH];
	char modename[FILENAMELENGTH];

	snprintf( filespec, FILENAMELENGTH
	          , "%s/ASF/robot%d.dat"
	          , cf.mocudir
	          , robnum );

	fp = fopen( filespec, "w" );

	if ( fp == NULL ) {
		printf( "Error opening '%s'\n", filespec );
		return;
	}

	fprintf( fp, FORMAT_LAT, st.dLatDeg );

	fprintf( fp, FORMAT_LON, st.dLonDeg );
	fprintf( fp, FORMAT_ALT, st.dAltMeters );

	fprintf( fp, FORMAT_HDG, RADIANS( st.dHdgDeg ) );
	fprintf( fp, FORMAT_PITCH, RADIANS( st.dPitchDeg ) );
	fprintf( fp, FORMAT_ROLL, RADIANS( st.dRollDeg ) );

	fprintf( fp, FORMAT_BATT, st.dBattVolts );
	fprintf( fp, FORMAT_SPEED, st.dSpeed_mps );

	fprintf( fp, FORMAT_HUMID, st.dHumidPct.dValue * 100. );
	fprintf( fp, FORMAT_AIRTEMP, st.dAirTempDegC.dValue - 273. );
	fprintf( fp, FORMAT_WATERTEMP, st.dWaterTempDegC.dValue - 273. );

	fprintf( fp, FORMAT_WINDSPEED, st.dWindSpeed_mps.dValue );
	fprintf( fp, FORMAT_WINDANGLE, st.dWindAngleDeg.dValue );

	fprintf( fp, FORMAT_AIRPRESS, st.dAirPress_mbar.dValue / 100. );

	fprintf( fp, FORMAT_SALINITY, st.dSalinity_psu.dValue );
	fprintf( fp, FORMAT_CHLORO, st.dFluorescence_V.dValue );

	fprintf( fp, FORMAT_NUMWPT, st.nWaypointCount );

	fprintf( fp, FORMAT_ASFMODE, st.nMode );

	strcpy( modename, st.sModeName );
	remove_space( modename );

	fprintf( fp, FORMAT_ASFMODENAME, modename );

	fprintf( fp, FORMAT_COMMSLATENCY, st.nCommsLatency );

	fprintf( fp, FORMAT_WATERSPEED, st.dWaterspeed_mps );

	fprintf( fp, FORMAT_CURRENTSPEED, st.dCurrentSpeedMPS.dValue );
	fprintf( fp, FORMAT_CURRENTANGLE, st.dCurrentAngleDeg.dValue );

	fclose( fp );

	return;
}


