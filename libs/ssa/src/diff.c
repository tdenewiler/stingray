/**********************************************************
*
*	diff.c
*
*  Copyright 2007,2008 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module provides a diff function which compares
*    two files byte by byte.
*
**************************************************************************
*
*  version 1.0.2, 10/4/08, SBS
*    - documentation
*
* version 1.0.1, 12/7/07, SBS
*    - add rval/exit for can't open file
*
* version 1.0.0, 12/4/07, SBS
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
*  This module provides a diff function which compares
*    two files byte by byte.
*  <hr>
**/

#include <stdio.h>
#include <stdlib.h>

#include "logfile.h"

#include "diff.h"


/********************************************************
*
* diff()
*
* Compares two files byte by byte
*
* Parameters:
*    file1, file2  - names of the two files to compare
*
* Returns:
*    0 = files are identical
*   <0 = files are different
*   >0 = error
*
********************************************************/
int diff( char *file1, char *file2 )
{
	FILE *fp1, *fp2;
	char c1, c2;

	fp1 = fopen( file1, "r" );
	fp2 = fopen( file2, "r" );

	if ( fp1 == NULL ) {
		printf( "DIFF() : Failed to open file %s\n", file1 );
		return( 1 );
	}

	if ( fp2 == NULL ) {
		printf( "DIFF() : Failed to open file %s\n", file2 );
		return( 1 );
	}

	while ( 1 ) {
		c1 = fgetc( fp1 );
		c2 = fgetc( fp2 );

		if ( c1 == EOF ) {
			if ( c2 == EOF ) {

#ifdef DEBUG
				logfile_LogPrint( "DIFF() : Both files at EOF, no differences" );
#endif

				fclose( fp1 );
				fclose( fp2 );
				return( 0 );
			}
			else {

#ifdef DEBUG
				logfile_LogPrint( "DIFF() : Only file 1 at EOF, files different" );
#endif

				fclose( fp1 );
				fclose( fp2 );
				return( -1 );
			}
		}

		if ( c2 == EOF ) {

#ifdef DEBUG
			logfile_LogPrint( "DIFF() : Only file 2 at EOF, files different" );
#endif

			fclose( fp1 );
			fclose( fp2 );
			return( -2 );
		}

		if ( c1 != c2 ) {

#ifdef DEBUG
			logfile_LogPrint( "DIFF () : Characters are different, files different." );
#endif

			fclose( fp1 );
			fclose( fp2 );
			return( -3 );
		}
	}

#ifdef DEBUG
	logfile_LogPrint( "DIFF() : Oops.  Shouldn't be here!\n" );

#endif

	fclose( fp1 );

	fclose( fp2 );

	return( 2 );
}
