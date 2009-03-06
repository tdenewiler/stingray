/*************************************************************************
*
*	logfile.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module writes debug statements to a log file.
*
**************************************************************************
*
*  version 1.2.1, 1/8/09, SBS
*    - change (level < loglevel)  to (level <= loglevel)
*
*  version 1.2, 12/3/08, SBS
*    - add logLevelPrint(), logLevelSet()
*    - add logfile_OpenSyslog() ; modify logprint to use syslog or local file
*
*  version 1.1.5, 10/4/08, SBS
*    - documentation
*
*  version 1.1.4, 7/12/08, SBS
*    - change to using gettimeofday() instead of time(), gives us output in logs
*    - add copy to stdout with #ifdef DEBUGSCREEN
*
*  version 1.1.3, 4/1/08, SBS
*    - minor cleanup / documentation
*
*  version 1.1.2, 1/28/08, SBS
*    - minor cleanup
*
*  version 1.1.1, 12/17/07, SBS
*    - change interface so file pointer is held in this module
*
*  version 1.1, 11/29/07, SBS
*    - add logprint()
*
*  version 1.0, 6/14/07, SBS
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
*  This module writes debug statements to a log file.
*  <hr>
**/

#ifdef DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <syslog.h>

#include "logfile.h"

#define LOGLEVELDEFAULT	LOGLEVEL_INFO

#define FILESPECLEN 256

static FILE *fp;
static int loglevel;
static int syslogfacility;


/************************************************
*
*  logfile_OpenSyslog()
*
*
************************************************/
void logfile_OpenSyslog( int facility )
{
	loglevel = LOGLEVELDEFAULT;
	syslogfacility = facility;

	printf( "===SSA=== Logging using syslog facility %d\n", facility );

	return;
}


/************************************************
*
*  logfile_OpenFile()
*
*  Opens the log file, saving the fp in a static
*    variable for later use.
*
*  Parameters:
*    filename - the log file name
*
*  Returns:
*    none
*
************************************************/
void logfile_OpenFile( char *filename )
{
	loglevel = LOGLEVELDEFAULT;
	syslogfacility = -1;

	printf( "===SSA=== Attempting to open log file %s\n", filename );

	fp = fopen( filename, "a" );

	if ( fp == NULL ) {
		printf( "===SSA=== Error opening log file, using STDOUT\n" );
		fp = stdout;
	}
	else {
		printf( "===SSA=== Successfully opened log file\n" );
	}

	return;
}


/************************************************
*
*  logfile_CloseFile()
*
*  Closes the log file (never actually called
*    since everything runs in a forever loop.)
*
*  Parameters:
*    none
*
*  Returns:
*    none
*
************************************************/
void logfile_CloseFile()
{
	if ( syslogfacility == -1 ) {
		fclose( fp );
	}

	return;
}


/************************************************
*
*  logfile_LogLevelSet()
*
*  Sets the global log level threshold.
*    Only messages with levels less than this
*    value will be printed.
*
************************************************/
void logfile_LogLevelSet( int level )
{
	char logstr[LOGSTRLEN];

	loglevel = level;

	snprintf( logstr, LOGSTRLEN
	          , "logfile_LogLevelSet() - setting level to %d"
	          , level );

	logfile_LogLevelPrint( LOGLEVEL_IMPORTANT, logstr );

	printf( "===SSA=== Setting log level to %d\n", level );

	return;
}


/********************************************************
*
*  logfile_LogLevelPrint()
*
*  Prints a string to the log file, prefixed with a timestamp.
*    Only prints if level is greater than the preset loglevel;
*
*
*  Parameters:
*    level - level of this message
*    str - the string
*
*  Returns:
*    none
*
********************************************************/
void logfile_LogLevelPrint( int level, char *str )
{

	static struct tm *tmp;

	static struct timeval tv;
	static time_t t;
	static char timestr[26];

	if ( level <= loglevel ) {
		gettimeofday( &tv, NULL );

		t = tv.tv_sec;

		tmp = localtime( &t );

		strftime( timestr, sizeof( timestr ), "%b %d %T", tmp );

		if ( syslogfacility == -1 ) {

			fprintf( fp, "%s.%06d %s \n", timestr, ( int )( tv.tv_usec ), str );
			fflush( fp );
		}
		else {

			syslog( syslogfacility, "%s.%06d %s \n", timestr, ( int )( tv.tv_usec ), str );
		}

#ifdef DEBUGSCREEN
		printf( "%s\n", str );

		fflush( stdout );

#endif
	}

	return;
}


/********************************************************
*
*  logfile_LogPrint()
*
*  Prints a string to the log file, prefixed with a timestamp
*
*
*  Parameters:
*    str - the string
*
*  Returns:
*    none
*
********************************************************/
void logfile_LogPrint( char *str )
{
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, str );

	return;
}

#endif


