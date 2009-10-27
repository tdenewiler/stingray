/*************************************************************************
*
*	telemfile.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*
*  This module provides functions that deal with sending, retrieving,
*    writing, and parsing boat telemetry files.
*
**************************************************************************
*
*  version 1.3.5, 2/1/09, SBS
*    - reconcile with var renaming in STATUS struct
*
*  version 1.3.4, 1/2/09, SBS
*    - create telemfile_WriteFileDirect() / rename WriteFileDirect()
*
*  version 1.3.3, 12/31/08, SBS
*    - add FetchTelemServer
*    - cleanup
*
*  version 1.3.2, 12/2/08, SBS
*    - add logging output to WriteTelemServer
*    - change to nonblocking connect() with timeout in WriteTelemServer
*
*  version 1.3.1, 11/28/08, SBS
*    - change functions to take filespec instead of CONFIGDATA
*
*  version 1.3, 11/26/08, SBS
*    - add new functions to write telemetry to a socket
*
*  version 1.2.8, 11/12/08, SBS
*    - replace gnome_config_set_float with
*        gnome_config_set_string to reduce
*        extraneous digits / decrease file size
*    - cleanup
*
*  version 1.2.7, 11/11/08, SBS
*    - modify to use cf.rsynclocalpath
*
*  version 1.2.6, 10/4/08, SBS
*    - documentation
*
*  version 1.2.5, 7/12/08, SBS
*    - changes to use data/ directory
*    - minor cleanup
*
*  version 1.2.4, 6/4/08, SBS
*    - change to readconfig_str2
*
*  version 1.2.3, 5/28/08, SBS
*    - rename surfacevel -> currentspeed
*    - add currentangle
*
*  version 1.2.2, 5/5/08, SBS
*    - rename groundspeed --> waterspeed
*
*  version 1.2.1, 4/30/08, SBS
*    - add groundspeed
*    - minor cleanup
*
*  version 1.2, 4/13/08, SBS
*    - rewrite file output functions to
*         eliminate gnome_config_push/pop_prefix
*         (appears to have memory leaks)
*
*  version 1.1.7, 3/21/08, SBS
*    - minor cleanup
*
*  version 1.1.6, 3/18/08, SBS
*    - add InitStatus() call in ReadFile()
*
*  version 1.1.5, 3/9/08, SBS
*    - add FetchFileS
*
*  version 1.1.4, 1/28/08, SBS
*    - minor cleanup
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
*  version 1.1, 7/13/07, SBS
*    - move parse/create functions here from status.c
*
*  version 1.0, 6/7/07, SBS.
*    - split from ssadll.c
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
*  This module provides functions that deal with sending, retrieving,
*    writing, and parsing boat telemetry files.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include <libgnome/gnome-config.h>

#include "waypoints.h"
#include "platform_types.h"
#include "platform_modes.h"
#include "status.h"

#include "readconfigfile.h"

#ifdef DEBUG
#include "logfile.h"
#endif

#include "delay.h"
#include "socket.h"

#include "telemfile.h"


#define FILESPECLEN 128
#define CMDSTRLEN 512

/*******************************/
// default status values
/*******************************/
#define DEFAULT_LON_DEG   -75.727
#define DEFAULT_LAT_DEG   37.919
#define DEFAULT_ALT_M     0.2

#define DEFAULT_SPEED_MPS 1.1

#define DEFAULT_WATERSPEED_MPS 2.2
#define DEFAULT_WATERANGLE_DEG 105.0

#define DEFAULT_HDG_DEG   178.0
#define DEFAULT_PITCH_DEG 2.0
#define DEFAULT_ROLL_DEG  3.0

#define DEFAULT_VOLTS    11.3

#define DEFAULT_HUMID    72.5
#define DEFAULT_AIRTEMP  65.0
#define DEFAULT_WATERTEMP 60.0

#define DEFAULT_WINDSPEED 5.5
#define DEFAULT_WINDANGLE 87.0
#define DEFAULT_AIRPRESS  900.0

#define DEFAULT_SALINITY	20.0
#define DEFAULT_CHLORO		1.0

/*******************************
* initialization values
*******************************/

#define INITSTRING "-255"
#define INITVAL -255


/************************************************
*
*	telemfile_readSciDbl()
*/
/**
*
* @brief
*	Reads a science double from a telemetry file
*
*  Parameters:
*   - filespec - the file to read
*   - name - name of the scidbl
*
*	Returns:
*   - the filled SCI_DBL struct
*
************************************************/
static SCI_DBL telemfile_readSciDbl(
    char *filespec
    , char *name )
{
	SCI_DBL	d;

	d.dValue = readconfig_float(
	               filespec
	               , name
	               , "value"
	               , INITVAL );

	readconfig_str2(
	    d.sUnits
	    , sizeof( d.sUnits )
	    , filespec
	    , name
	    , "units"
	    , INITSTRING );

	readconfig_str2(
	    d.sTime
	    , sizeof( d.sTime )
	    , filespec
	    , name
	    , "time"
	    , INITSTRING );

	d.position.dLatDeg = readconfig_float(
	                         filespec
	                         , name
	                         , "lat_deg"
	                         , INITVAL );

	d.position.dLonDeg = readconfig_float(
	                         filespec
	                         , name
	                         , "lon_deg"
	                         , INITVAL );

	return( d );
}


/************************************************
*
* telemfile_ReadFile()
*/
/**
*
* @brief
* Reads telemetry from a file into a STATUS structure.
*
* Parameters:
*   - filespec - the filename
*
* Returns:
*   - the filled STATUS structure
*
************************************************/
STATUS telemfile_ReadFile(
    char *filespec )
{
	STATUS s;

	s = status_InitStatus();

	/***********************
	* general section
	***********************/
	readconfig_str2(
	    s.sPlatformName
	    , sizeof( s.sPlatformName )
	    , filespec
	    , "general"
	    , "platform"
	    , "NULL" );

	s.nPlatformID = readconfig_int(
	                    filespec
	                    , "general"
	                    , "platformID"
	                    , -255 );

	s.nPlatformMode = readconfig_int(
	                      filespec
	                      , "general"
	                      , "mode"
	                      , DEFAULT_LAT_DEG );

	readconfig_str2(
	    s.sPlatformModeName
	    , sizeof( s.sPlatformModeName )
	    , filespec
	    , "general"
	    , "modename"
	    , "NULL" );

	/***********************
	* engineering section
	***********************/
	readconfig_str2(
	    s.sTime
	    , sizeof( s.sTime )
	    , filespec
	    , "engineering"
	    , "time"
	    , INITSTRING );

	s.nTime = readconfig_int(
	              filespec
	              , "engineering"
	              , "time_secs"
	              , 0 );

	s.dLatDeg = readconfig_float(
	                filespec
	                , "engineering"
	                , "lat_deg"
	                , DEFAULT_LAT_DEG );

	s.dLonDeg = readconfig_float(
	                filespec
	                , "engineering"
	                , "lon_deg"
	                , DEFAULT_LON_DEG );

	s.dAltMeters = readconfig_float(
	                   filespec
	                   , "engineering"
	                   , "altitude_m"
	                   , DEFAULT_ALT_M );

	s.dSpeed_mps = readconfig_float(
	                   filespec
	                   , "engineering"
	                   , "speed_mps"
	                   , DEFAULT_SPEED_MPS );

	s.dWaterspeed_mps = readconfig_float(
	                        filespec
	                        , "engineering"
	                        , "waterspeed_mps"
	                        , DEFAULT_WATERSPEED_MPS );

	s.dHdgDeg = readconfig_float(
	                filespec
	                , "engineering"
	                , "heading_deg"
	                , DEFAULT_HDG_DEG );

	s.dPitchDeg = readconfig_float(
	                  filespec
	                  , "engineering"
	                  , "pitch_deg"
	                  , DEFAULT_PITCH_DEG );

	s.dRollDeg = readconfig_float(
	                 filespec
	                 , "engineering"
	                 , "roll_deg"
	                 , DEFAULT_ROLL_DEG );

	s.dBattVolts = readconfig_float(
	                   filespec
	                   , "engineering"
	                   , "battery_volts"
	                   , DEFAULT_VOLTS );


	/***********************
	* science data
	***********************/
	s.dHumidPct = telemfile_readSciDbl( filespec, "Humidity" );
	s.dAirTempDegC = telemfile_readSciDbl( filespec, "AirTemp" );
	s.dWaterTempDegC = telemfile_readSciDbl( filespec, "WaterTemp" );

	s.dWindSpeed_mps = telemfile_readSciDbl( filespec, "Windspeed" );
	s.dWindAngleDeg = telemfile_readSciDbl( filespec, "WindAngle" );
	s.dAirPress_mbar = telemfile_readSciDbl( filespec, "AirPressure" );

	s.dSalinity_psu = telemfile_readSciDbl( filespec, "Salinity" );
	s.dFluorescence_V = telemfile_readSciDbl( filespec, "Fluorescence" );

	s.dBathymetry = telemfile_readSciDbl( filespec, "Bathymetry" );

	s.dCurrentSpeedMPS = telemfile_readSciDbl( filespec, "CurrentSpeed" );
	s.dCurrentAngleDeg = telemfile_readSciDbl( filespec, "CurrentAngle" );

	/***********************
	* path waypoints
	***********************/
	s.nWaypointCount = readconfig_float( filespec, "path"
	                                     , "numwaypoints", 0 );

	return( s );
}


/************************************************
*
* telemfile_ParseTelemFile()
*/
/**
*
* @brief
* Reads a telemetry file into a STATUS structure.
*
* Parameters:
*   - cf     - structure holding program configuration data
*   - robnum - which robot to read telemetry for
*
* Returns:
*   - the filled STATUS structure
*
************************************************/
//STATUS telemfile_ParseTelemFile(CONFIGDATA cf, int robnum, WAYPT *wp)
STATUS telemfile_ParseTelemFile( char *filespec )
//STATUS telemfile_ParseTelemFile(
//			CONFIGDATA cf
//			, int robnum)
{
	STATUS s;

//	char filespec[FILESPECLEN];

//	s = status_InitStatus();

	/*	snprintf(filespec,FILESPECLEN
					,"%s/%s%d.%s"
					,cf.localdatapath
					,cf.telemfilebase
					,robnum+1
					,cf.telemfileext);
	*/
#ifdef DEBUG
	logfile_LogPrint( "telemfile_ParseTelemFile() - reading from file:" );
	logfile_LogPrint( filespec );
#endif

	s = telemfile_ReadFile( filespec );

//	waypoints_ReadFile(filespec,s.nWaypointCount,wp);

	return( s );
}


/************************************************
*
* WriteSciDbl()
*/
/**
*
* @brief
* Writes a SCI_DBL to a telemetry file
*
*
* Parameters:
*   - name    - the name of the science value
*   - SCI_DBL - data structure
*
* Returns:
*   - none
*
************************************************/
static void WriteSciDbl(
    char *filespec
    , char *name
    , int places
    , SCI_DBL s )
{
	char str[FILESPECLEN];
	char str2[FILESPECLEN];
	char str3[FILESPECLEN];

	snprintf( str, FILESPECLEN, "=%s=/%s/value", filespec, name );
	snprintf( str2, FILESPECLEN, "%%.%df", places );
	snprintf( str3, FILESPECLEN, str2, s.dValue );
	gnome_config_set_string( str, str3 );
//	gnome_config_set_float(str, s.dValue);

	snprintf( str, FILESPECLEN, "=%s=/%s/units", filespec, name );
	gnome_config_set_string( str, s.sUnits );

	snprintf( str, FILESPECLEN, "=%s=/%s/time", filespec, name );
	gnome_config_set_string( str, s.sTime );

	snprintf( str, FILESPECLEN, "=%s=/%s/lat_deg", filespec, name );
	snprintf( str2, FILESPECLEN, "%.6f", s.position.dLatDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.position.dLatDeg);

	snprintf( str, FILESPECLEN, "=%s=/%s/lon_deg", filespec, name );
	snprintf( str2, FILESPECLEN, "%.6f", s.position.dLonDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.position.dLonDeg);

	return;
}


/************************************************
*
* WriteEngStatus()
*/
/**
*
* @brief
* Writes engineering telemetry to a telemetry file
*
*
* Parameters:
*   - s - status struct
*
* Returns:
*   - none
*
************************************************/
static void WriteEngStatus(
    char *filespec
    , STATUS s )
{
	char str[CMDSTRLEN];
	char str2[CMDSTRLEN];

	snprintf( str, CMDSTRLEN, "=%s=/general/platform", filespec );
	gnome_config_set_string( str, s.sPlatformName );


	snprintf( str, CMDSTRLEN, "=%s=/general/platformID", filespec );
	gnome_config_set_int( str, s.nPlatformID );

	snprintf( str, CMDSTRLEN, "=%s=/general/mode", filespec );
	gnome_config_set_int( str, s.nPlatformMode );

	snprintf( str, CMDSTRLEN, "=%s=/general/modename", filespec );
	gnome_config_set_string( str, s.sPlatformModeName );

	snprintf( str, CMDSTRLEN, "=%s=/engineering/time", filespec );
	gnome_config_set_string( str, s.sTime );

	snprintf( str, CMDSTRLEN, "=%s=/engineering/time_secs", filespec );
	gnome_config_set_int( str, s.nTime );

	snprintf( str, CMDSTRLEN, "=%s=/engineering/lat_deg", filespec );
	snprintf( str2, CMDSTRLEN, "%.6f", s.dLatDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dLatDeg);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/lon_deg", filespec );
	snprintf( str2, CMDSTRLEN, "%.6f", s.dLonDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dLonDeg);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/altitude_m", filespec );
	snprintf( str2, CMDSTRLEN, "%.1f", s.dAltMeters );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dAltMeters);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/speed_mps", filespec );
	snprintf( str2, CMDSTRLEN, "%.1f", s.dSpeed_mps );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dSpeed_mps);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/waterspeed_mps", filespec );
	snprintf( str2, CMDSTRLEN, "%.1f", s.dWaterspeed_mps );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dWaterspeed_mps);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/heading_deg", filespec );
	snprintf( str2, CMDSTRLEN, "%.1f", s.dHdgDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dHdgDeg);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/pitch_deg", filespec );
	snprintf( str2, CMDSTRLEN, "%.0f", s.dPitchDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dPitchDeg);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/roll_deg", filespec );
	snprintf( str2, CMDSTRLEN, "%.0f", s.dRollDeg );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dRollDeg);

	snprintf( str, CMDSTRLEN, "=%s=/engineering/battery_volts", filespec );
	snprintf( str2, CMDSTRLEN, "%.1f", s.dBattVolts );
	gnome_config_set_string( str, str2 );
//	gnome_config_set_float(str, s.dBattVolts);

	snprintf( str, CMDSTRLEN, "=%s=/path/numwaypoints", filespec );
	gnome_config_set_int( str, s.nWaypointCount );

	return;
}


/************************************************
*
* PopGnomePrefix()
*/
/**
*
* @brief
* Clears config file prefix.
*
* Parameters:
*  - cf       - structure holding program configuration data
*  - robotID  - number of current robot
*
* Returns:
*  - none
*
************************************************/
static void PopGnomePrefix( char *filespec )
//static void PopGnomePrefix(
//			CONFIGDATA cf
//			, int robotID)
{
//	char filespec[FILESPECLEN];
	char cmd[CMDSTRLEN];

#ifdef DEBUG3
	char temp[LOGSTRLEN];
#endif

//	gnome_config_pop_prefix();

	gnome_config_sync();

	/*	snprintf(filespec,FILESPECLEN
				,"data/%s%d.%s"
				,cf.telemfilebase
				,robotID
				,cf.telemfileext);
	*/
	snprintf( cmd, CMDSTRLEN, "chmod 644 %s", filespec );

#ifdef DEBUG3
	snprintf( temp, LOGSTRLEN, "command is \"%s\"", cmd );
	logfile_LogPrint( temp );
#endif

	system( cmd );

	return;
}


/************************************************
*
* telemfile_WriteEngStatus()
*/
/**
*
* @brief
* Writes the engineering part of a status struct
*  to a telemetry file
*
* Parameters:
*   - cf - structure holding program configuration data
*   - s  - status structure
*
* Returns:
*   - none
*
************************************************/
void telemfile_WriteEngStatus(
//			CONFIGDATA c
    char *filespec
    , STATUS s )
{
//	char filespec[FILESPECLEN];

	/*	snprintf(filespec,FILESPECLEN
				,"data/%s%d.%s"
				,cf.telemfilebase
				,s.nPlatformID
				,cf.telemfileext);
	*/
	WriteEngStatus( filespec, s );

	PopGnomePrefix( filespec );

	return;
}


/************************************************
*
* telemfile_WriteSciData()
*/
/**
*
* @brief
* Writes a science data structure to a telemetry file
*
* Parameters:
*   - name    - the name of the science value
*   - robotID - number of current robot
*   - cf      - structure holding program configuration data
*   - s       - SCI_DBL structure to write
*
* Returns:
*   - none
*
************************************************/
void telemfile_WriteSciData(
    char *name
    , int robotID
//			, CONFIGDATA cf
    , char *filespec
    , SCI_DBL s
    , int places )
{
//	char filespec[FILESPECLEN];

	/*	snprintf(filespec,FILESPECLEN
				,"%s/%s%d.%s"
				,cf.localdatapath
				,cf.telemfilebase
				,robotID
				,cf.telemfileext);
	*/
	WriteSciDbl( filespec, name, places, s );

	PopGnomePrefix( filespec );

	return;
}


/************************************************
*
* telemfile_WriteFile()
*/
/**
*
* @brief
* Writes a status structure to a telemetry file
*
* Parameters:
*   - cf - structure holding program configuration data
*   - s  - status structure
*
* Returns:
*    none
*
************************************************/
void telemfile_WriteFile(
//			CONFIGDATA cf
    char *filespec
    , STATUS s )
{
//	char filespec[FILESPECLEN];

	/*	snprintf(filespec,FILESPECLEN
				,"data/%s%d.%s"
				,cf.telemfilebase
				,s.nPlatformID
				,cf.telemfileext);
	*/
	WriteEngStatus( filespec, s );

	WriteSciDbl( filespec, "Humidity"     , 2 , s.dHumidPct );
	WriteSciDbl( filespec, "AirTemp"      , 1 , s.dAirTempDegC );
	WriteSciDbl( filespec, "WaterTemp"    , 1 , s.dWaterTempDegC );
	WriteSciDbl( filespec, "Windspeed"    , 1 , s.dWindSpeed_mps );
	WriteSciDbl( filespec, "WindAngle"    , 1 , s.dWindAngleDeg );
	WriteSciDbl( filespec, "AirPressure"  , 0 , s.dAirPress_mbar );
	WriteSciDbl( filespec, "Salinity"     , 2 , s.dSalinity_psu );
	WriteSciDbl( filespec, "Fluorescence" , 3 , s.dFluorescence_V );
	WriteSciDbl( filespec, "Bathymetry"   , 2 , s.dBathymetry );
	WriteSciDbl( filespec, "CurrentSpeed" , 1 , s.dCurrentSpeedMPS );
	WriteSciDbl( filespec, "CurrentAngle" , 1 , s.dCurrentAngleDeg );

	PopGnomePrefix( filespec );

	return;
}


/************************************************
*
* telemfile_WriteEngStatus_FileDirect()
*/
/**
*
* @brief
* Writes engineering telemetry to a file
*
* Parameters:
*   - fp  - file pointer
*   - st  - status structure
*
* Returns:
*    none
*
************************************************/
static void telemfile_WriteEngStatus_FileDirect(
    FILE *fp
    , STATUS st )
{
	fprintf( fp, "[general]\n" );
	fprintf( fp, "platform=%s\n", st.sPlatformName );
	fprintf( fp, "platformID=%d\n", st.nPlatformID );
	fprintf( fp, "mode=%d\n", st.nPlatformMode );
	fprintf( fp, "modename=%s\n", st.sPlatformModeName );

	fprintf( fp, "\n[engineering]\n" );
	fprintf( fp, "time=%s\n", st.sTime );
	fprintf( fp, "time_secs=%ld\n", st.nTime );
	fprintf( fp, "lat_deg=%.6f\n", st.dLatDeg );
	fprintf( fp, "lon_deg=%.6f\n", st.dLonDeg );
	fprintf( fp, "altitude_m=%.1f\n", st.dAltMeters );
	fprintf( fp, "speed_mps=%.1f\n", st.dSpeed_mps );
	fprintf( fp, "waterspeed_mps=%.1f\n", st.dWaterspeed_mps );
	fprintf( fp, "heading_deg=%.1f\n", st.dHdgDeg );
	fprintf( fp, "pitch_deg=%.0f\n", st.dPitchDeg );
	fprintf( fp, "roll_deg=%.1f\n", st.dRollDeg );

	if ( st.dBattVolts > -250 ) {
		fprintf( fp, "battery_volts=%.1f\n", st.dBattVolts );
	}

	return;
}


/************************************************
*
* telemfile_WriteSciDbl_FileDirect()
*/
/**
*
* @brief
* Writes a SCIENCE_DATA struct to a file
*
* Parameters:
*   - fp      - file pointer
*   - name    - the name of the science value
*   - places  - number of decimal places to use for float
*   - s       - SCI_DBL structure to write
*
* Returns:
*    none
*
************************************************/
static void telemfile_WriteSciDbl_FileDirect(
    FILE *fp,
    char *name
    , int places
    , SCI_DBL s )
{
	char str2[FILESPECLEN];

	if ( s.dValue > -250 ) {

		snprintf( str2, FILESPECLEN, "value=%%.%df\n", places );

		fprintf( fp, "\n[%s]\n", name );
		fprintf( fp, str2, s.dValue );
		fprintf( fp, "units=%s\n", s.sUnits );
		fprintf( fp, "time=%s\n", s.sTime );
		fprintf( fp, "lat_deg=%.6f\n", s.position.dLatDeg );
		fprintf( fp, "lon_deg=%.6f\n", s.position.dLonDeg );

	}

	return;
}


/************************************************
*
* WriteFileDirect()
*/
/**
*
* @brief
* Writes telemetry to a file
*
* Parameters:
*   - fp  - file pointer
*   - st  - status structure
*
* Returns:
*    none
*
************************************************/
static void WriteFileDirect(
    FILE *fp
    , STATUS st )
{

	telemfile_WriteEngStatus_FileDirect( fp, st );

	telemfile_WriteSciDbl_FileDirect( fp, "Humidity"     , 2 , st.dHumidPct );
	telemfile_WriteSciDbl_FileDirect( fp, "AirTemp"      , 1 , st.dAirTempDegC );
	telemfile_WriteSciDbl_FileDirect( fp, "WaterTemp"    , 1 , st.dWaterTempDegC );
	telemfile_WriteSciDbl_FileDirect( fp, "Windspeed"    , 1 , st.dWindSpeed_mps );
	telemfile_WriteSciDbl_FileDirect( fp, "WindAngle"    , 1 , st.dWindAngleDeg );
	telemfile_WriteSciDbl_FileDirect( fp, "AirPressure"  , 0 , st.dAirPress_mbar );
	telemfile_WriteSciDbl_FileDirect( fp, "Salinity"     , 2 , st.dSalinity_psu );
	telemfile_WriteSciDbl_FileDirect( fp, "Fluorescence" , 3 , st.dFluorescence_V );
	telemfile_WriteSciDbl_FileDirect( fp, "Bathymetry"   , 2 , st.dBathymetry );
	telemfile_WriteSciDbl_FileDirect( fp, "CurrentSpeed" , 1 , st.dCurrentSpeedMPS );
	telemfile_WriteSciDbl_FileDirect( fp, "CurrentAngle" , 1 , st.dCurrentAngleDeg );

	return;
}


/************************************************
*
* telemfile_WriteFileDirect()
*/
/**
*
* @brief
* Writes telemetry to a file
*
* Parameters:
*   - filespec - filename
*   - st       - status structure
*
* Returns:
*    none
*
************************************************/
void telemfile_WriteFileDirect(
    char *filespec
    , STATUS st )
{
	FILE *fp;

	fp = fopen( filespec, "w" );

	if ( fp != NULL ) {
		WriteFileDirect( fp, st );
	}

	fclose( fp );

	return;
}

/************************************************
*
* telemfile_WriteTelemServer()
*/
/**
*
* @brief
* Opens a socket connection to the telemetry
*    server and writes STATUS struct to that
*    socket
*
* Parameters:
*   - sIPaddr   - string containing ip address of server
*   - sPortNum  - string containing port number of server
*   - st        - status structure
*
* Returns:
*    none
*
************************************************/
void telemfile_WriteTelemServer(
    char *sIPaddr
    , char *sPortNum
    , STATUS st )
{
	FILE *fpW;
	int s;

	s = socket_OpenSocket( sIPaddr, sPortNum );

#ifdef DEBUG
	logfile_LogPrint( "telemfile_WriteTelemServer() - opening socket file" );
#endif
	fpW = fdopen( s, "w" );

	if ( fpW != NULL ) {

#ifdef DEBUG
		logfile_LogPrint( "telemfile_WriteTelemServer() - writing telemetry" );
#endif

		WriteFileDirect( fpW, st );
		fclose( fpW );
	}
	else {
#ifdef DEBUG
		logfile_LogPrint( "telemfile_WriteTelemServer() - can't open socket file" );
#endif
	}

	close( s );

	return;
}


/********************************************************
*
*  telemfile_FetchTelemServer()
*
*  Pulls telemetry data from the dataserver via socket
*    and writes to a file
*
*
********************************************************/
void telemfile_FetchTelemServer( char *sIPaddr, char *sPortNum, char *sOutfile )
{
	socket_FetchFile( sIPaddr, sPortNum, sOutfile );

	return;
}
