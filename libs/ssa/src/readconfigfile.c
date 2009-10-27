/*************************************************************************
*
*	readconfigfile.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module provides wrappers for gnome_config_get_foo,
*    constructing the configuration file description from
*    passed parameters
*
**************************************************************************
*
*  version 1.0.5, 1/1/09, SBS
*    - printf -> logfile_LogLevelPrint
*
*  version 1.0.4, 10/4/08, SBS
*    - documentation
*
*  version 1.0.3, 6/18/08, SBS
*    - add conditional usage of g_free() / free() depending on Linux / cygwin
*
*  version 1.0.2, 6/4/08, SBS
*    - add readconfig_str2
*
*	version 1.0.1, 4/1/08, SBS
*    - minor clereaanup / documentation
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
*  This module provides wrappers for gnome_config_get_foo,
*    constructing the configuration file description from
*    passed parameters
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgnome/gnome-config.h>

#include "logfile.h"

#include "readconfigfile.h"

#define MAXLABELSTRLENGTH 1024


/**********************************************************
*
*  readconfig_int()
*
*  wrapper for gnome_config_get_int_with_default()
*
*  Parameters:
*    filespec - config file name
*    section  - section name
*    varname  - variable name
*    defval   - default value used if variable is not found
*
*  Returns:
*    the int from the config file
*
**********************************************************/
int readconfig_int( char *filespec, char *section, char *varname, int defval )
{
	char name[MAXLABELSTRLENGTH];

#ifdef DEBUG
	char temp[LOGSTRLEN];
#endif

	snprintf( name, MAXLABELSTRLENGTH
	          , "=%s=/%s/%s=%d"
	          , filespec
	          , section
	          , varname
	          , defval );

#ifdef DEBUG
	snprintf( temp, LOGSTRLEN, "varname is '%s'", name );
	logfile_LogLevelPrint( LOGLEVEL_VERBOSE, temp );
#endif

	return( gnome_config_get_int_with_default( name, NULL ) );
}


/**********************************************************
*
*  readconfig_float()
*
*  wrapper for gnome_config_get_float_with_default()
*
*  Parameters:
*    filespec - config file name
*    section  - section name
*    varname  - variable name
*    defval   - default value used if variable is not found
*
*  Returns:
*    the float from the config file
*
**********************************************************/
float readconfig_float( char *filespec, char *section, char *varname, float defval )
{
	char name[MAXLABELSTRLENGTH];

#ifdef DEBUG
	char temp[LOGSTRLEN];
#endif

	snprintf( name, MAXLABELSTRLENGTH
	          , "=%s=/%s/%s=%f"
	          , filespec
	          , section
	          , varname
	          , defval );

#ifdef DEBUG
	snprintf( temp, LOGSTRLEN, "varname is '%s'", name );
	logfile_LogLevelPrint( LOGLEVEL_VERBOSE, temp );
#endif

	return( gnome_config_get_float_with_default( name, NULL ) );
}


/**********************************************************
*
*  readconfig_str()
*
*  wrapper for gnome_config_get_string_with_default()
*
*  Parameters:
*    filespec - config file name
*    section  - section name
*    varname  - variable name
*    defval   - default value used if variable is not found
*
*  Returns:
*    the string from the config file
*
**********************************************************/
char * readconfig_str( char *filespec, char *section, char *varname, char *defval )
{
	char name[MAXLABELSTRLENGTH];

#ifdef DEBUG
	char temp[LOGSTRLEN];
#endif

	snprintf( name, MAXLABELSTRLENGTH
	          , "=%s=/%s/%s=%s"
	          , filespec
	          , section
	          , varname
	          , defval );

#ifdef DEBUG
	snprintf( temp, LOGSTRLEN, "varname is '%s'", name );
	logfile_LogLevelPrint( LOGLEVEL_VERBOSE, temp );
	snprintf( temp, LOGSTRLEN, "result is '%s'", gnome_config_get_string_with_default( name, NULL ) );
	logfile_LogLevelPrint( LOGLEVEL_VERBOSE, temp );
#endif

	return( gnome_config_get_string_with_default( name, NULL ) );
}


/**********************************************************
*
*  readconfig_str2()
*
*  wrapper for gnome_config_get_string_with_default()
*    this version fixes memory leaks by taking an output
*    string pointer as an argument
*
*  Parameters:
*    outstr   - output string
*    outstrlen - length of output string
*    filespec - config file name
*    section  - section name
*    varname  - variable name
*    defval   - default value used if variable is not found
*
*  Returns:
*    none
*
**********************************************************/
void readconfig_str2(
    char *outstr
    , int outstrlen
    , char *filespec
    , char *section
    , char *varname
    , char *defval )
{
	char name[MAXLABELSTRLENGTH];
	char *tempstr;

#ifdef DEBUG
	char temp[LOGSTRLEN];
#endif

	snprintf( name, MAXLABELSTRLENGTH
	          , "=%s=/%s/%s=%s"
	          , filespec
	          , section
	          , varname
	          , defval );

#ifdef DEBUG
	snprintf( temp, LOGSTRLEN, "varname is '%s'", name );
	logfile_LogLevelPrint( LOGLEVEL_VERBOSE, temp );
	snprintf( temp, LOGSTRLEN, "result is '%s'", gnome_config_get_string_with_default( name, NULL ) );
	logfile_LogLevelPrint( LOGLEVEL_VERBOSE, temp );
#endif

	tempstr = gnome_config_get_string_with_default( name, NULL );

	strncpy( outstr, tempstr, outstrlen );

#ifdef __CYGWIN__
	free( tempstr );
#else
	g_free( tempstr );
#endif

	return;
}
