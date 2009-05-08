/******************************************************************************
 *
 *  Title:        pololuCalibrate.c
 *
 *  Description:  Main program for calibrating the Pololu motor controller.
 * 				  Sets the neutral position for the controllers to 63 for the
 * 				  Electronic Servo Controllers. The number 63 was found by
 * 				  127 / 2 = 63. Channels 0 and 3 are the motors for the Voiths.
 *
 *****************************************************************************/


#include <stdio.h>
#include <string.h>
#include "pololu.h"

int main( void )
{
	/* Create variables. */
	int pololu_fd = -1;
	char pololu_port[25];
	strncpy( pololu_port, "/dev/ttyUSB0", 25 );

	/* Set up the Pololu port. */
	pololu_fd = pololuSetup( pololu_port, 9600 );

	/* Send full throttle commands to channels 0 and 3. */
	printf("MAIN: Sending full throttle commands.\n");
	pololuSetPosition7Bit(pololu_fd,0,127);
	pololuSetPosition7Bit(pololu_fd,3,127);
	printf("MAIN: Waiting for you to press a key before setting neutral position ... ");
	getchar( );
	printf( "<OK>\n" );

	/* Set the desired neutral position for channels 0 and 3. */
	pololuSetPosition7Bit(pololu_fd,0,63);
	pololuSetPosition7Bit(pololu_fd,3,63);
	printf("MAIN: Waiting for you to press a key to exit ... ");
	getchar( );
	printf("<OK>\n");

	return 0;
} /* end main() */
