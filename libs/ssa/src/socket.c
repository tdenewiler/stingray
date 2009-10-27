/*************************************************************************
*
*   socket.c
*
*  Copyright 2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*
*  This module provides functions that deal with socket i/o.
*
**************************************************************************
*
*  version 1.1, 2/2/09, SBS
*    - add _FetchToString
*
*  version 1.0.2, 1/8/09, SBS
*    - LOGLEVEL_INFO --> LOGLEVEL_DEBUG
*
*  version 1.0.1, 1/2/09, SBS
*    - LogPrint --> LogLevelPrint
*
*  version 1.0, 12/31/08, SBS
*    - split from commandfile.c
*
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
//#include <time.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <netdb.h>

#ifdef DEBUG
#include "logfile.h"
#endif


/**************************************************************************
*
*  socket_FetchToString()
*
*  Pulls data from the dataserver via socket
*    and writes to a file
*
*
**************************************************************************/
void socket_FetchToString( char *sIPaddr, char *sPortNum, char *sOutString, int nSize )
{

	struct sockaddr_in adr_srvr;
//  struct timeval tv;
	FILE *fpR;
//  socklen_t lon;
//  fd_set set;
	char buf[5096];
//  long arg;
	int len_inet;
//  int valopt;
	int z, s;


	strncpy( sOutString, "", nSize );

#ifdef DEBUG
	char logstr[LOGSTRLEN];
	snprintf( logstr, LOGSTRLEN
	          , "socket_FetchFile() - opening socket to %s:%s"
	          , sIPaddr, sPortNum );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, logstr );
#endif

	s = socket( PF_INET, SOCK_STREAM, 0 );

	if ( s == -1 ) {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - socket() failed" );
		printf( "%s : %s\n", strerror( errno ), "socket failed" );
		fflush( stdout );
#endif
		return;
	}

	/*
	    #ifdef DEBUG
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - setting non-blocking");
	    #endif

	    arg = fcntl(s, F_GETFL, NULL);
	    arg |= O_NONBLOCK;
	    fcntl(s, F_SETFL, arg);
	*/

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - setting address" );

#endif

	memset( &adr_srvr, 0, sizeof( adr_srvr ) );

	adr_srvr.sin_family = AF_INET;

	adr_srvr.sin_port = htons( atoi( sPortNum ) );

	adr_srvr.sin_addr.s_addr = inet_addr( sIPaddr );

	if ( adr_srvr.sin_addr.s_addr == INADDR_NONE ) {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - bad address" );
		printf( "Socket Error : %s : %s\n", strerror( errno ), "address bad" );
		fflush( stdout );
#endif
		close( s );
		return;
	}

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - connecting" );

#endif

	len_inet = sizeof( adr_srvr );

	z = connect( s, ( struct sockaddr * ) & ( adr_srvr ), len_inet );

#ifdef DEBUG
	snprintf( logstr, sizeof( logstr ), "socket_FetchFile() - connect() rval = %d", z );

	logfile_LogLevelPrint( LOGLEVEL_DEBUG, logstr );

	snprintf( logstr, sizeof( logstr ), "socket_FetchFile() - errno = %d", errno );

	logfile_LogLevelPrint( LOGLEVEL_DEBUG, logstr );

	snprintf( logstr, sizeof( logstr ), "socket_FetchFile() - EINPROGRESS = %d", EINPROGRESS );

	logfile_LogLevelPrint( LOGLEVEL_DEBUG, logstr );

#endif

	if ( z < 0 ) {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - connect() failed" );
		printf( "Connect Error : %s : %s\n", strerror( errno ), "connect failed" );
		fflush( stdout );
#endif
		close( s );
		return;
	}

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - opening file from socket" );

#endif

	fpR = fdopen( s, "r" );

	if ( fpR != NULL ) {

#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - reading command" );
#endif

		// read from socket

		while ( fgets( buf, sizeof( buf ), fpR ) ) {

			//write to file
			//fprintf(fpW,"%s",buf);

//              printf("%s\n",buf);

			strncat( sOutString, buf, nSize - strlen( sOutString ) );

		}

		shutdown( fileno( fpR ), SHUT_RDWR );

		fclose( fpR );
	}
	else {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_FetchFile() - failed" );
#endif
	}

	close( s );

	return;
}


/**************************************************************************
*
*  socket_FetchFile()
*
*  Pulls data from the dataserver via socket
*    and writes to a file
*
*
**************************************************************************/
void socket_FetchFile( char *sIPaddr, char *sPortNum, char *sOutfile )
{
	/*  struct sockaddr_in adr_srvr;
	    struct timeval tv;
	    FILE *fpW,*fpR;
	    socklen_t lon;
	    fd_set set;
	    char buf[5096];
	    long arg;
	    int len_inet,valopt;
	    int z,s;

	    #ifdef DEBUG
	    char logstr[LOGSTRLEN];
	    snprintf(logstr,LOGSTRLEN
	        ,"socket_FetchFile() - opening socket to %s:%s"
	        ,sIPaddr,sPortNum);
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,logstr);
	    #endif

	    s = socket(PF_INET,SOCK_STREAM,0);
	    if(s == -1){
	        #ifdef DEBUG
	        logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - socket() failed");
	        printf("%s : %s\n",strerror(errno),"socket failed");
	        fflush(stdout);
	        #endif
	        return;
	    }

	    #ifdef DEBUG
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - setting non-blocking");
	    #endif

	    arg = fcntl(s, F_GETFL, NULL);
	    arg |= O_NONBLOCK;
	    fcntl(s, F_SETFL, arg);

	    #ifdef DEBUG
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - setting address");
	    #endif

	    memset(&adr_srvr,0,sizeof(adr_srvr));
	    adr_srvr.sin_family = AF_INET;
	    adr_srvr.sin_port = htons(atoi(sPortNum));
	    adr_srvr.sin_addr.s_addr = inet_addr(sIPaddr);

	    if(adr_srvr.sin_addr.s_addr == INADDR_NONE){
	        #ifdef DEBUG
	        logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - bad address");
	        printf("%s : %s\n",strerror(errno),"address bad");
	        fflush(stdout);
	        #endif
	        close(s);
	        return;
	    }

	    #ifdef DEBUG
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - connecting");
	    #endif

	    len_inet = sizeof(adr_srvr);
	    z = connect(s,(struct sockaddr *)&(adr_srvr),len_inet);

	    #ifdef DEBUG
	    snprintf(logstr,sizeof(logstr),"socket_FetchFile() - connect() rval = %d",z);
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,logstr);
	    snprintf(logstr,sizeof(logstr),"socket_FetchFile() - errno = %d",errno);
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,logstr);
	    snprintf(logstr,sizeof(logstr),"socket_FetchFile() - EINPROGRESS = %d",EINPROGRESS);
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,logstr);
	    #endif

	    if(z < 0){
	        if(errno == EINPROGRESS){
	            int zz;

	            // wait for connection for 0.075 second
	            tv.tv_sec = 0;
	            tv.tv_usec = 75000;

	            FD_ZERO(&set);
	            FD_SET(s, &set);

	            zz = select(s+1, NULL, &set, NULL, &tv);
	            if(zz > 0) {
	                #ifdef DEBUG
	                logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - here!");
	                #endif
	                lon = sizeof(int);
	                getsockopt(s, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
	                if (valopt) {
	                    #ifdef DEBUG
	                    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - Error in connect()");
	                    #endif
	                    close(s);
	                    return;
	                }else{
	                    #ifdef DEBUG
	                    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - here 2!");
	                    #endif
	                }
	            }else {
	                #ifdef DEBUG
	                logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - Timeout or error in connect()");
	                #endif
	                close(s);
	                return;
	            }

	//      }else if(z == -1){
	        }else{
	            #ifdef DEBUG
	            logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - connect() failed");
	            printf("%s : %s\n",strerror(errno),"connect failed");
	            fflush(stdout);
	            #endif
	            close(s);
	            return;
	        }
	    }

	    #ifdef DEBUG
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - setting back to blocking");
	    #endif

	    arg = fcntl(s, F_GETFL, NULL);
	    arg &= (~O_NONBLOCK);
	    fcntl(s, F_SETFL, arg);


	    #ifdef DEBUG
	    logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - opening localfile");
	    #endif

	    fpW = fopen(sOutfile,"w");
	    if(fpW != NULL){

	        #ifdef DEBUG
	        logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - opening file from socket");
	        #endif

	        fpR = fdopen(s,"r");

	        if(fpR != NULL){

	            #ifdef DEBUG
	            logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - reading command");
	            #endif

	            // read from socket
	            while(fgets(buf,sizeof(buf),fpR)){

	                //write to file
	                fprintf(fpW,"%s",buf);
	            }
	            shutdown(fileno(fpR),SHUT_RDWR);
	            fclose(fpR);
	        }else{
	            #ifdef DEBUG
	            logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - failed");
	            #endif
	        }
	        fclose(fpW);
	    }else{
	        #ifdef DEBUG
	        logfile_LogLevelPrint(LOGLEVEL_DEBUG,"socket_FetchFile() - failed");
	        #endif
	    }

	    close(s);
	*/

	char cmd[512];

	snprintf( cmd, sizeof( cmd ), "telnet  %s %s > %s 2>/dev/null", sIPaddr, sPortNum, sOutfile );
	system( cmd );

	snprintf( cmd, sizeof( cmd ), "sed -i -e '1,3d' %s", sOutfile );
	system( cmd );

	return;
}


/**************************************************************************
*
* socket_OpenSocket()
*/
/**
*
* @brief
* Opens a socket connection to the dataserver
*
* Parameters:
*   - sIPaddr   - string containing ip address of server
*   - sPortNum  - string containing port number of server
*
* Returns:
*    none
*
**************************************************************************/
int socket_OpenSocket(
    char *sIPaddr
    , char *sPortNum )
{

	struct sockaddr_in adr_srvr;

	struct timeval tv;
	socklen_t lon;
	int len_inet, valopt;
	fd_set set;
	int z, s;
	long arg;

#ifdef DEBUG
	char logstr[LOGSTRLEN];
	snprintf( logstr, LOGSTRLEN
	          , "socket_OpenSocket() - opening socket to %s:%s"
	          , sIPaddr, sPortNum );
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, logstr );
#endif

	s = socket( PF_INET, SOCK_STREAM, 0 );

	if ( s == -1 ) {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - socket() failed" );
		printf( "%s : %s\n", strerror( errno ), "socket failed" );
		fflush( stdout );
#endif
		return( -1 );
	}


#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - setting non-blocking" );

#endif

	arg = fcntl( s, F_GETFL, NULL );

	arg |= O_NONBLOCK;

	fcntl( s, F_SETFL, arg );

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - setting address" );

#endif

	memset( &adr_srvr, 0, sizeof( adr_srvr ) );

	adr_srvr.sin_family = AF_INET;

	adr_srvr.sin_port = htons( atoi( sPortNum ) );

	adr_srvr.sin_addr.s_addr = inet_addr( sIPaddr );

	if ( adr_srvr.sin_addr.s_addr == INADDR_NONE ) {
#ifdef DEBUG
		logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - bad address" );
		printf( "%s : %s\n", strerror( errno ), "address bad" );
		fflush( stdout );
#endif
		close( s );
		return( -1 );
	}

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - connecting" );

#endif

	len_inet = sizeof( adr_srvr );

	z = connect( s, ( struct sockaddr * ) & ( adr_srvr ), len_inet );

	if ( z < 0 ) {
		if ( errno == EINPROGRESS ) {
#ifdef DEBUG
			logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - EINPROGRESS ; doing select()" );
#endif
			// wait for connection for 1 second
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			FD_ZERO( &set );
			FD_SET( s, &set );

			if ( select( s + 1, NULL, &set, NULL, &tv ) > 0 ) {
				lon = sizeof( int );
				getsockopt( s, SOL_SOCKET, SO_ERROR, ( void* )( &valopt ), &lon );

				if ( valopt ) {
#ifdef DEBUG
					logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - Error in connect()" );
#endif
					close( s );
					return( -1 );
				}
			}
			else {
#ifdef DEBUG
				logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - Timeout or error in connect()" );
#endif
				close( s );
				return( -1 );
			}

		}
		else
			if ( z == -1 ) {
#ifdef DEBUG
				logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - connect() failed" );
				printf( "%s : %s\n", strerror( errno ), "connect failed" );
				fflush( stdout );
#endif
				close( s );
				return( -1 );
			}
	}

#ifdef DEBUG
	logfile_LogLevelPrint( LOGLEVEL_DEBUG, "socket_OpenSocket() - setting back to blocking" );

#endif

	arg = fcntl( s, F_GETFL, NULL );

	arg &= ( ~O_NONBLOCK );

	fcntl( s, F_SETFL, arg );

	return( s );
}
