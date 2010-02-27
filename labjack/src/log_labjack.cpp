/******************************************************************************
 * file: log_labjack.c
 * desc:
 * date: nov 21, 2008
 * auth: chris boynton <bboynton@ucsd.edu>
 *****************************************************************************/

#include "labjack.h"

static char *ljLogFilePath;

void setupLabjackLog( char *filePathName )
{
	ljLogFilePath = filePathName;
}

void logLabjack()
{
	return;
}

