/*************************************************************************
*
*	commandfile.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module provides funtions to retrieve a command file from
*    the dataserver and parse it.
*
**************************************************************************
*
*  version 1.1, 2/2/09, SBS
*    - add reading/parsing thread ; this allows the socket calls to block
*        which solves problems we've had with incomplete messages
*    - cleanup
*
*  version 1.0.10, 2/1/09, SBS
*    - rename nBoatId -> nPlatformID
*       sBoatName -> sPlatformName
*       nCmd --> nCmdID
*
*  version 1.0.9, 1/27/09, SBS
*    - add throttle, rudder, heading to COMMAND struct
*    - add closeenough to WAYPT struct
*
*  version 1.0.8, 1/2/09, SBS
*    - add platformID and commandID processing to ParseCommandFile()
*    - LogPrint --> LogLevelPrint
*
*  version 1.0.7, 12/31/08, SBS
*    - move guts of FetchCommandServer into socket.c
*    - cleanup
*
*  version 1.0.6, 12/2/08, SBS
*    - add logging output to FetchCommandServer
*    - change to nonblocking connect() with timeout in FetchCommandServer
*
*  version 1.0.5, 11/28/08, SBS
*    - add FetchCommandServer
*    - change ParseCommandFile to take filespec instead of CONFIGDATA
*    - cleanup
*    - rename from readcommandfile.c
*
*  version 1.0.4, 10/4/08, SBS
*    - documentation
*
*  version 1.0.3, 6/4/08, SBS
*    - change to readconfig_str2
*
*  version 1.0.2, 5/30/08, SBS
*    - add parsing of pause and resume commands
*    - create #defined CMDSTR_xxx in command.h
*
*  version 1.0.1, 4/1/08, SBS
*    - minor cleanup
*
*  version 1.0, 12/9/07, SBS
*    - derived from commandfile.c in irc_client and telemfile.c in common
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
*  This module provides functions to retrieve a command file from
*    the dataserver and parse it.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/timeb.h>
#include <pthread.h>


#include "delay.h"
#include "waypoints.h"
#include "platform_types.h"
#include "platform_modes.h"
#include "status.h"

#include "readconfigfile.h"
#include "diff.h"

#ifdef DEBUG
#include "logfile.h"
#endif

#include "config.h"
#include "socket.h"

#define NEED_CMDNAMES
#include "command.h"
#undef NEED_CMDNAMES

#include "commandfile.h"


//**************************************************************************

#define CMDSTRLENGTH 1024
#define PLATSTRLENGTH 128
#define FILENAMELENGTH 128

#define SLEEPDELAY (0.025)
#define LOOPINTERVAL (0.25)

//**************************************************************************

#ifdef NEEDCMDFILETHREAD
static COMMAND global_cmd;
static pthread_mutex_t command_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
	char sIPaddr[32];
	char sPortNum[8];
	char sOutfile[128];
}COMMANDTHREADPARAMS;

#endif

/**************************************************************************
*
*  Min()
*/
/**
*
* @brief
* Returns the lesser of two ints
*
**************************************************************************/
static int Min(
    int a
    , int b )
{
	if ( a < b ) {
		return( a );
	}

	return( b );
}


/**************************************************************************
*
* ParseCommandFile()
*/
/**
*
* @brief
* Reads data from command file into COMMAND struct
*
* Parameters:
*   - cf     - config struct
*   - robnum - which robot
*
* Returns:
*   - none
*
**************************************************************************/
COMMAND commandfile_ParseCommandFile(
    char *filespec )
{
	COMMAND c;
	register int i;
	char tmp[64];
	int j;

#ifdef DEBUG
	char temp[LOGSTRLEN];
	snprintf( temp, LOGSTRLEN
	          , "ParseCmdFile() - parsing file '%s'", filespec );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );
#endif

	c = command_InitCommand();

	c.nPlatformID = readconfig_int( filespec, "global", "platformID", -255 );

//	printf("platform ID = %d\n",c.nPlatformID);

	c.nCmdID = readconfig_int( filespec, "global", "commandID", -255 );

	readconfig_str2( c.sPlatformName, sizeof( c.sPlatformName ), filespec
	                 , "global", "platform", "-255" );


	if ( c.nCmdID == -255 ) {
		// older code doesn't use cmd number, so parse command name
		readconfig_str2( c.sCmdName, sizeof( c.sCmdName ), filespec
		                 , "global", "command", "-255" );

		if ( strcasecmp( c.sCmdName, cmdnames[CMD_NULL] ) == 0 ) {
			c.nCmdID = CMD_NULL;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_HALT] ) == 0 ) {
			c.nCmdID = CMD_HALT;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_WPT] ) == 0 ) {
			c.nCmdID = CMD_WPT;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_PAUSE] ) == 0 ) {
			c.nCmdID = CMD_PAUSE;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_RESUME] ) == 0 ) {
			c.nCmdID = CMD_RESUME;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_INTERRUPT] ) == 0 ) {
			c.nCmdID = CMD_INTERRUPT;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_UNINTERRUPT] ) == 0 ) {
			c.nCmdID = CMD_UNINTERRUPT;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_MANUAL] ) == 0 ) {
			c.nCmdID = CMD_MANUAL;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_CALIBRATE] ) == 0 ) {
			c.nCmdID = CMD_CALIBRATE;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_STATIONKEEP] ) == 0 ) {
			c.nCmdID = CMD_STATIONKEEP;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_FREEDRIFT] ) == 0 ) {
			c.nCmdID = CMD_FREEDRIFT;
		}
		else if ( strcasecmp( c.sCmdName, cmdnames[CMD_HEADINGHOLD] ) == 0 ) {
			c.nCmdID = CMD_HEADINGHOLD;
		}

	}
	else {
		snprintf( c.sCmdName, sizeof( c.sCmdName ), cmdnames[c.nCmdID] );
	}


	if ( c.nPlatformID == -255 ) {
		// older code doesn't use boat number, so extract from boat name

		char *n;

		n = strrchr( c.sPlatformName, ' ' );

		if ( n != NULL ) {
			sscanf( n, "%d", &( c.nPlatformID ) );
		}

	}


	c.nRudder = readconfig_int( filespec, "manual", "rudder", 0 );

	c.nThrottle = readconfig_int( filespec, "manual", "throttle", 0 );
	c.nHeading = readconfig_int( filespec, "headinghold", "heading", 0 );


	c.nWaypointCount = readconfig_int( filespec, "global", "numwaypoints", 0 );

	if ( c.nWaypointCount > 0 ) {

		j = Min( c.nWaypointCount, MAXWAYPOINTS );

		for ( i = 0; i < j; i++ ) {

			snprintf( tmp, sizeof( tmp ), "waypoint%d", i );

			c.Waypoints[i].dLatDeg
			= readconfig_float( filespec, tmp, "lat_deg", -255 );
			c.Waypoints[i].dLonDeg
			= readconfig_float( filespec, tmp, "lon_deg", -255 );
			c.Waypoints[i].dSpeedMps
			= readconfig_float( filespec, tmp, "speed_mps", -255 );
			c.Waypoints[i].dCloseEnoughM
			= readconfig_float( filespec, tmp, "closeenough_m", -255 );
		}
	}

	return( c );
}


/**************************************************************************
*
* commandfile_WriteFileDirect()
*/
/**
*
* @brief
* Writes COMMAND struct to a file
*
* Parameters:
*   - fp  - file pointer
*   - cmd - command structure
*
* Returns:
*   - none
*
**************************************************************************/
void commandfile_WriteFileDirect(
    FILE *fp
    , COMMAND cmd )
{
	char temp[2048];

	command_WriteString( temp, sizeof( temp ), cmd );

	fprintf( fp, temp );

	return;
}


/**************************************************************************
*
* commandfile_WriteCommandServer()
*/
/**
*
* @brief
* Opens a socket connection to the command
*    server and writes COMMAND struct to that
*    socket
*
* Parameters:
*   - sIPaddr   - string containing ip address of server
*   - sPortNum  - string containing port number of server
*   - cmd       - command structure
*
* Returns:
*   - none
*
**************************************************************************/
void commandfile_WriteCommandServer(
    char *sIPaddr
    , char *sPortNum
    , COMMAND cmd )
{
	FILE *fpW;
	int s;

//	printf("%s : %s\n",sIPaddr,sPortNum);

	s = socket_OpenSocket( sIPaddr, sPortNum );

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_INFO
	                       , "commandfile_WriteCommandServer() - opening socket file" );
#endif
	fpW = fdopen( s, "w" );

	if ( fpW != NULL ) {

#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_INFO
		                       , "commandfile_WriteCommandServer() - writing command" );
#endif

		commandfile_WriteFileDirect( fpW, cmd );
		fclose( fpW );
	}
	else {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_INFO
		                       , "commandfile_WriteCommandServer() - can't open socket file" );
#endif
	}

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_INFO
	                       , "commandfile_WriteCommandServer() - closing socket" );

#endif
	close( s );

	return;
}


/********************************************************
*
*  command_FetchCommandServer()
*/
/**
*
* @brief
* Pulls command data from the dataserver via socket
*    and writes to a file
*
*
********************************************************/
/*
void commandfile_FetchCommandServer2(
			char *sIPaddr
			,char *sPortNum
			,char *sOutfile)
{
//	socket_FetchFile(sIPaddr,sPortNum,sOutfile);

	char sCmdStr[2048];
	FILE *fp;

	socket_FetchToString(sIPaddr,sPortNum,sCmdStr,2048);

	printf("=====\n");
	printf("commandfile_FetchCommandServer() :\n");
	printf(sCmdStr);
	printf("=====\n");

	fp = fopen(sOutfile,"w");
	if(fp != NULL){
		fprintf(fp,sCmdStr);
		fclose(fp);
	}

	return;
}
*/

#ifdef NEEDCMDFILETHREAD
/**************************************************************************
*
*  commandfile_mainloop()
*/
/**
*
* @brief
*
*
**************************************************************************/
void * commandfile_mainloop(
    void *ptr )
{

	struct timeb starttime, endtime;
	TIMESPEC delay;
	double delta;
//	int *robnum;
	COMMANDTHREADPARAMS *cp;

	char sCmdStr[2048];
//	char sIPaddr[32] = "128.2.181.103";
//	char sPortNum[16] = "8051";
//	char sOutfile[128] = "data/command1.txt";
	char sIPaddr[32];
	char sPortNum[16];
	char sOutfile[128];
	FILE *fp;

	COMMAND tempcmd;

//	robnum = (int *)ptr;
	cp = ( COMMANDTHREADPARAMS * )ptr;

	strncpy( sIPaddr, cp->sIPaddr, sizeof( sIPaddr ) );
	strncpy( sPortNum, cp->sPortNum, sizeof( sPortNum ) );
	strncpy( sOutfile, cp->sOutfile, sizeof( sOutfile ) );


	delay = delay_setdelay( SLEEPDELAY );


	while ( 1 ) {

		ftime( &starttime );

		socket_FetchToString( sIPaddr, sPortNum, sCmdStr, 2048 );

		if ( strlen( sCmdStr ) > 1 ) {
			pthread_mutex_lock( &file_mutex );
			fp = fopen( sOutfile, "w" );

			if ( fp != NULL ) {
				fprintf( fp, sCmdStr );
				fclose( fp );
			}

			pthread_mutex_unlock( &file_mutex );

			tempcmd = commandfile_ParseCommandFile( sOutfile );

			pthread_mutex_lock( &command_mutex );
			global_cmd = tempcmd;
			pthread_mutex_unlock( &command_mutex );
		}

		ftime( &endtime );

		delta = delay_deltatime( starttime, endtime );

		while ( delta < ( LOOPINTERVAL - SLEEPDELAY ) ) {
			delay_sleep( &delay );
			ftime( &endtime );
			delta = delay_deltatime( starttime, endtime );
		}
	}

}


/**************************************************************************
*
*  commandfile_GetStatus()
*/
/**
*
* @brief
*
*
**************************************************************************/
COMMAND commandfile_GetStatus( void )
{
	COMMAND c;

	pthread_mutex_lock( &command_mutex );
	c = global_cmd;
	pthread_mutex_unlock( &command_mutex );

	return( c );
}


/**************************************************************************
*
*  commandfile_StartThread()
*/
/**
*
* @brief
*
*
**************************************************************************/
int commandfile_StartThread( CONFIGDATA cf )
{
	pthread_t thread;
	int rval;
	COMMAND tempcmd;

	COMMANDTHREADPARAMS cp;

	printf( "commandfile_StartThread()\n" );

	tempcmd = command_InitCommand();

	pthread_mutex_lock( &command_mutex );
	global_cmd = tempcmd;
	pthread_mutex_unlock( &command_mutex );

	snprintf( cp.sIPaddr, sizeof( cp.sIPaddr ), "%s", cf.cmdserver_ipaddr );
	snprintf( cp.sPortNum, sizeof( cp.sPortNum ), "%d"
	          , cf.cmdserver_baseportfetch + cf.robotnumber );
	snprintf( cp.sOutfile, sizeof( cp.sOutfile ), "%s/%s%d.%s"
	          , cf.localdatapath
	          , cf.cmdfilebase
	          , cf.robotnumber
	          , cf.cmdfileext );

	rval = pthread_create( &thread, NULL, commandfile_mainloop, ( void* ) & cp );

	return( rval );
}

#endif
