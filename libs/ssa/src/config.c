/**********************************************************
*
*	config.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module provides functions which read configuration
*    parameters from the command line and config file.
*
**************************************************************************
*
*  version 1.2.1, 1/8/09, SBS
*    - LogPrint --> LogLevelPrint
*
*  version 1.2, 12/31/08, SBS
*    - add config items to support new socket comms
*    - cleanup
*
*  version 1.1.1, 12/3/08, SBS
*    - add loglevel
*
*  version 1.1, 11/13/08, SBS
*    - revisions to support new rsync/wget config data
*
*  version 1.0.6, 10/22/08, SBS
*    - add localpath to CONFIGDATA
*    - cleanup / doc'n
*
*  version 1.0.5, 10/4/08, SBS
*    - documentation
*
*  version 1.0.4, 6/4/08, SBS
*    - change to readconfig_str2
*
*  version 1.0.3, 4/1/08, SBS
*    - minor cleanup / documentation
*
*  version 1.0.2, 12/07/07, SBS
*    - add log file name to config file/data
*
*  version 1.0.1, 11/30/07, SBS
*    - add logfile output
*
*  version 1.0.a, 5/8/07, SBS
*    - irc_client-specific version
*
*	version 1.0, 4/5/07, SBS
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
*  This module provides functions which read configuration
*    parameters from the command line and config file.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "types.h"

#include "readconfigfile.h"
#include "logfile.h"

#include "config.h"

#define MAXPATHLENGTH 512

/************************************************
*  Default configuration values
************************************************/

//#define DEFSENDCOMMAND "scp -p -i"
//#define DEFSENDCOMMAND "rsync"
//#define DEFRETRCOMMAND "wget -N"
//#define DEFIDENTITYKEY "taosfscpkey"

//#define DEFTIMEPERIOD 1.01
//#define DEFNUMFILES 1

#define DEFREPORTPERIOD 1

//#define DEFUSERNAME "taosfscponly"
//#define DEFSERVERNAME "taosfdataserver"

//#define DEFRSYNCHOST	"localhost"
//#define DEFRSYNCSHARE "taosf"

#define DEFLOCALPATH "data"
#define DEFLOCALCOPYDIR "/var/www/taosf/gsfc"

//#define DEFWGETHOST "localhost"
//#define DEFWGETPATH "taosf/datafiles"

#define DEFCMDFILEBASE "command"
#define DEFCMDFILEEXT "txt"

#define DEFTELEMFILEBASE "telemetry"
#define DEFTELEMFILEEXT "txt"

//#define DEFREMOTEPATH "taosf/gsfc"
//#define DEFLOCALPATH "data"

#define DEFNUMROBOTS 1
#define DEFDOCOMMAND 0

#define DEFJAVAUSERDIR "C:\\code\\asf\\project"
#define DEFJAVACLASSPATH ".;C:\\code\\asf\\project;C:\\code\\irc;C:\\code\\asf\\project\\classes;C:\\code\\asf\\project\\lib\\asf.jar;C:\\code\\asf\\project\\examples"

#define DEFIRCRESOURCE "-Presources/ocean/config/ssa/ssa.plist"

#define DEFMOCUDIR "e:\\devel\\ssc-sd\\Mocu\\"
#define DEFARCHIVEDIR "/var/www/taosf/archive"

#define DEFDBSERVER "localhost"
#define DEFDBNAME "taosf"
#define DEFDBUSER "taosf"
#define DEFDBPWD  "foo"

#define DEFLOGFILENAME "logfile.log"

#define DEFLOGLEVEL LOGLEVEL_DEBUG

#define DEFTELEMSERVERIPADDR "127.0.0.1"
#define DEFCMDSERVERIPADDR "127.0.0.1"


#define DEFCMDSERVERBASEPORTPUT 8000
#define DEFCMDSERVERBASEPORTFETCH 8050

#define DEFTELEMSERVERBASEPORTPUT 9000
#define DEFTELEMSERVERBASEPORTFETCH 9050

#define DEFROBOTNUMBER 1
#define DEFROBOTNAME "nameless robot"

/********************************************************
*
*  readcommandline()
*/
/**
*
* @brief
*  Uses getopt() to read command line options
*
*  Parameters:
*    - argc, argv - arguments from main()
*
*    - configfilespec - if the -f option is used the
*       corresponding filename will be copied into
*		  this variable
*
*  Returns:
*    - none
*
********************************************************/
static void readcommandline(
    int argc
    , char *argv[]
    , char *configfilespec
)
{
	int opt;

	configfilespec[0] = '\0';

	while ( ( opt = getopt( argc, argv, "f:nh" ) ) > 0 ) {
		switch ( opt ) {

			case 'f':	// filespec
				printf( "===SSA=== Command line option \"-f %s\" found.\n", optarg );
				snprintf( configfilespec, MAXPATHLENGTH, optarg );
				printf( "===SSA=== Using config file '%s'\n\n", configfilespec );
				break;

			case 'h':	// help

			case '?':

			default:
				// print usage instructions and exit
				printf( "\n===SSA=== usage: %s [options]\n", argv[0] );
				printf( "===SSA===   -f <filespec>\tUse config file <filespec>.\n\n" );
				exit( -1 );
		}
	}

	return;
}


/***********************************************
*
*  config_readconfig()
*/
/**
*
* @brief
*   Reads configuration data from a file
*
*   The name of the config file is read from the
*      command line (-f option)
*
*   If there is no cmd line arg, then a default
*     filename provided by the caller is used.
*
*
* Parameters:
*   - argc, argv - arguments from main()
*
*   - defcfgfilename - the config file to use if none given on command line
*
*
*  Returns:
*    - none
*
***********************************************/
CONFIGDATA config_readconfig(
    int argc
    , char *argv[]
    , char *defcfgfilename
)
{
	CONFIGDATA cf;
	char configfilespec[MAXPATHLENGTH];

	readcommandline( argc, argv, configfilespec );

	if ( strlen( configfilespec ) == 0 ) {
		snprintf( configfilespec, sizeof( configfilespec ), defcfgfilename );

#ifdef DEBUG
		printf( "===SSA=== Using config file '%s'\n", configfilespec );
#endif
	}


	cf.numrobots = readconfig_float(

	                   configfilespec
	                   , "general"
	                   , "numrobots"
	                   , DEFNUMROBOTS );

	cf.docommand = readconfig_int(
	                   configfilespec
	                   , "general"
	                   , "docommand"
	                   , DEFDOCOMMAND );

	cf.reportperiod = readconfig_int(
	                      configfilespec
	                      , "general"
	                      , "reportperiod"
	                      , DEFREPORTPERIOD );

	/*
		readconfig_str2(
					cf.rsynchostname
					,sizeof(cf.rsynchostname)
					,configfilespec
					,"rsync"
					,"hostname"
					,DEFRSYNCHOST);

		readconfig_str2(
					cf.rsyncsharename
					,sizeof(cf.rsyncsharename)
					,configfilespec
					,"rsync"
					,"hostshare"
					,DEFRSYNCSHARE);

		readconfig_str2(
					cf.wgethostname
					,sizeof(cf.wgethostname)
					,configfilespec
					,"wget"
					,"hostname"
					,DEFWGETHOST);

		readconfig_str2(
					cf.wgethostpath
					,sizeof(cf.wgethostpath)
					,configfilespec
					,"wget"
					,"hostpath"
					,DEFWGETPATH);
	*/
	readconfig_str2(
	    cf.localcopydir
	    , sizeof( cf.localcopydir )
	    , configfilespec
	    , "rsync"
	    , "localcopydir"
	    , DEFLOCALCOPYDIR );

	readconfig_str2(
	    cf.localdatapath
	    , sizeof( cf.localdatapath )
	    , configfilespec
	    , "rsync"
	    , "localpath"
	    , DEFLOCALPATH );

	readconfig_str2(
	    cf.cmdfilebase
	    , sizeof( cf.cmdfilebase )
	    , configfilespec
	    , "file"
	    , "cmdfilebase"
	    , DEFCMDFILEBASE );

	readconfig_str2(
	    cf.cmdfileext
	    , sizeof( cf.cmdfileext )
	    , configfilespec
	    , "file"
	    , "cmdfileext"
	    , DEFCMDFILEEXT );

	readconfig_str2(
	    cf.telemfilebase
	    , sizeof( cf.telemfilebase )
	    , configfilespec
	    , "file"
	    , "telemfilebase"
	    , DEFTELEMFILEBASE );

	readconfig_str2(
	    cf.telemfileext
	    , sizeof( cf.telemfileext )
	    , configfilespec
	    , "file"
	    , "telemfileext"
	    , DEFTELEMFILEEXT );


	readconfig_str2(
	    cf.javauserdir
	    , sizeof( cf.javauserdir )
	    , configfilespec
	    , "java"
	    , "userdir"
	    , DEFJAVAUSERDIR );

	readconfig_str2(
	    cf.javaclasspath
	    , sizeof( cf.javaclasspath )
	    , configfilespec
	    , "java"
	    , "classpath"
	    , DEFJAVACLASSPATH );

	readconfig_str2(
	    cf.ircresource
	    , sizeof( cf.ircresource )
	    , configfilespec
	    , "irc"
	    , "resource"
	    , DEFIRCRESOURCE );

	readconfig_str2(
	    cf.mocudir
	    , sizeof( cf.mocudir )
	    , configfilespec
	    , "file"
	    , "mocudirectory"
	    , DEFMOCUDIR );

	readconfig_str2(
	    cf.archivedir
	    , sizeof( cf.archivedir )
	    , configfilespec
	    , "file"
	    , "archivedir"
	    , DEFARCHIVEDIR );

	readconfig_str2(
	    cf.dbusername
	    , sizeof( cf.dbusername )
	    , configfilespec
	    , "database"
	    , "username"
	    , DEFDBUSER );

	readconfig_str2(
	    cf.dbdatabasename
	    , sizeof( cf.dbdatabasename )
	    , configfilespec
	    , "database"
	    , "name"
	    , DEFDBNAME );

	readconfig_str2(
	    cf.dbservername
	    , sizeof( cf.dbservername )
	    , configfilespec
	    , "database"
	    , "servername"
	    , DEFDBSERVER );

	readconfig_str2(
	    cf.dbuserpasswd
	    , sizeof( cf.dbuserpasswd )
	    , configfilespec
	    , "database"
	    , "password"
	    , DEFDBPWD );

	readconfig_str2(
	    cf.logfilename
	    , sizeof( cf.logfilename )
	    , configfilespec
	    , "general"
	    , "logfile"
	    , DEFLOGFILENAME );

	cf.loglevel = readconfig_int(
	                  configfilespec
	                  , "general"
	                  , "loglevel"
	                  , DEFLOGLEVEL );

	readconfig_str2(
	    cf.telemserver_ipaddr
	    , sizeof( cf.telemserver_ipaddr )
	    , configfilespec
	    , "server"
	    , "telemipaddr"
	    , DEFTELEMSERVERIPADDR );

	cf.telemserver_baseportfetch = readconfig_int(
	                                   configfilespec
	                                   , "server"
	                                   , "telembaseportfetch"
	                                   , DEFTELEMSERVERBASEPORTFETCH );

	cf.telemserver_baseportput = readconfig_int(
	                                 configfilespec
	                                 , "server"
	                                 , "telembaseportput"
	                                 , DEFTELEMSERVERBASEPORTPUT );

	readconfig_str2(
	    cf.cmdserver_ipaddr
	    , sizeof( cf.cmdserver_ipaddr )
	    , configfilespec
	    , "server"
	    , "cmdipaddr"
	    , DEFCMDSERVERIPADDR );

	cf.cmdserver_baseportfetch = readconfig_int(
	                                 configfilespec
	                                 , "server"
	                                 , "cmdbaseportfetch"
	                                 , DEFCMDSERVERBASEPORTFETCH );

	cf.cmdserver_baseportput = readconfig_int(
	                               configfilespec
	                               , "server"
	                               , "cmdbaseportput"
	                               , DEFCMDSERVERBASEPORTPUT );

	cf.robotnumber = readconfig_int(
	                     configfilespec
	                     , "general"
	                     , "robotnumber"
	                     , DEFROBOTNUMBER );

	readconfig_str2(
	    cf.robotname
	    , sizeof( cf.robotname )
	    , configfilespec
	    , "general"
	    , "robotname"
	    , DEFROBOTNAME );

	return( cf );
}


/***********************************************
*
*  config_PrintConfig()
*/
/**
*
* @brief
*  Prints a CONFIGDATA struct to the log file
*
*  Parameters:
*    - cf - the struct to print
*
*  Returns:
*    - none
*
***********************************************/
void config_PrintConfig(
    CONFIGDATA cf
)
{
#ifdef DEBUG
	char temp[LOGSTRLEN];

	logfile_LogLevelPrint( LOGLEVEL_INFO, "===== Configuration Information =====" );

	snprintf( temp, LOGSTRLEN, "numrobots = %d", cf.numrobots );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "docommand = %d", cf.docommand );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "robot number = %d", cf.robotnumber );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "robot name = %s", cf.robotname );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	/*
		snprintf(temp,LOGSTRLEN,"rsync hostname = %s",cf.rsynchostname);
		logfile_LogLevelPrint(LOGLEVEL_INFO,temp);

		snprintf(temp,LOGSTRLEN,"rsync sharename = %s",cf.rsyncsharename);
		logfile_LogLevelPrint(LOGLEVEL_INFO,temp);

		snprintf(temp,LOGSTRLEN,"wget hostname = %s",cf.wgethostname);
		logfile_LogLevelPrint(LOGLEVEL_INFO,temp);

		snprintf(temp,LOGSTRLEN,"wget pathname = %s",cf.wgethostpath);
		logfile_LogLevelPrint(LOGLEVEL_INFO,temp);
	*/
	snprintf( temp, LOGSTRLEN, "local data path = %s", cf.localdatapath );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );


	snprintf( temp, LOGSTRLEN, "cmdfilebase = %s", cf.cmdfilebase );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "cmdfileext = %s", cf.cmdfileext );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "telemfilebase = %s", cf.telemfilebase );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "telemfileext = %s", cf.telemfileext );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "javauserdir = %s", cf.javauserdir );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "javaclasspath = %s", cf.javaclasspath );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "ircresource = %s", cf.ircresource );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "mocu directory = %s", cf.mocudir );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "archive directory = %s", cf.archivedir );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "database" );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "   server = %s", cf.dbservername );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "   database = %s", cf.dbdatabasename );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "   user = %s", cf.dbusername );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "   password = %s", cf.dbuserpasswd );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "logfile = %s", cf.logfilename );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "loglevel = %d", cf.loglevel );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "telemetry server address = %s", cf.telemserver_ipaddr );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "telemetry base port (fetch) = %d", cf.telemserver_baseportfetch );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "telemetry base port (put) = %d", cf.telemserver_baseportput );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "command server address = %s", cf.cmdserver_ipaddr );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "command base port (fetch) = %d", cf.cmdserver_baseportfetch );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );

	snprintf( temp, LOGSTRLEN, "command base port (put) = %d", cf.cmdserver_baseportput );
	logfile_LogLevelPrint( LOGLEVEL_INFO, temp );


	logfile_LogLevelPrint( LOGLEVEL_INFO, " " );

	logfile_LogLevelPrint( LOGLEVEL_INFO, "===== End Configuration Information =====" );
	logfile_LogLevelPrint( LOGLEVEL_INFO, " " );
#endif

	return;
}



