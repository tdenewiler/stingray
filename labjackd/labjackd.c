/******************************************************************************
 *
 *  Title:        labjackd.c
 *
 *  Description:  Main program for labjack server.
 *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "network.h"
#include "labjack.h"
#include "util.h"
#include "messages.h"

/* This program is meant to be used system wide. Therefore it is more difficult
 * to use a configuration file and the port is being hard coded for now.
 */
#define LABJACKD_PORT 2010


/* Global file descriptors. Only global so that planner_exit() can close them. */
int server_fd;
int labjackd_fd;
int labjack_fd;


/******************************************************************************
 *
 * Title:       void labjackd_sigint( int signal )
 *
 * Description: This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       signal: The SIGINT signal.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void labjackd_sigint( int signal )
{
	exit( 0 );
} /* end labjackd_sigint() */


/******************************************************************************
 *
 * Title:       void labjackd_exit( )
 *
 * Description: Exit function for main program. Closes all file descriptors.
 *              This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     File descriptors: labjackd_fd.
 *
 *****************************************************************************/

void labjackd_exit( )
{
	/* Sleep to let things shut down properly. */
	usleep( 200000 );

	/* Close the open file descriptors. */

	if ( labjackd_fd > 0 ) {
		close( labjackd_fd );
	}

	if ( labjack_fd > 0 ) {
		close( labjack_fd );
	}
} /* end labjackd_exit() */


/******************************************************************************
 *
 * Title:       int main( int argc, char *argv[] )
 *
 * Description: Initialize data. Open ports. Run main program loop.
 *
 * Input:       argc: Number of command line arguments.
 *              argv: Array of command line arguments.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int main( int argc, char *argv[] )
{
	/* Setup exit function. It is called when SIGINT (ctrl-c) is invoked. */
	void( *exit_ptr )( void );
	exit_ptr = labjackd_exit;
	atexit( exit_ptr );

	struct sigaction sigint_action;
	sigint_action.sa_handler = labjackd_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	int recv_bytes = 0;
	char recv_buf[MAX_MSG_SIZE];
	MSG_DATA msg;
	LABJACK_DATA lj;

	/* Initialize variables. */
	labjackd_fd = -1;

	memset( &msg, 0, sizeof( MSG_DATA ) );
	memset( &lj, 0, sizeof( LABJACK_DATA ) );

	/* Set up communications. */
	server_fd = net_server_setup( LABJACKD_PORT );

	/* Set up the labjack. */
	labjack_fd = init_labjack( );

	if ( labjack_fd ) {
		query_labjack( );
	}

	labjack_fd = 1; /* Fake it until that function returns an fd. */

	/* Main loop. */

	while ( 1 ) {
		/* Get network data. */
		if ( labjackd_fd > 0 ) {
			recv_bytes = net_server( labjackd_fd, recv_buf, &msg, MODE_NAV );

			if ( recv_bytes > 0 ) {
				recv_buf[recv_bytes] = '\0';
				messages_decode( labjackd_fd, recv_buf, &msg );
			}
		}

		/* Get Labjack data. */
		if ( labjack_fd > 0 ) {
			query_labjack( );
			lj.battery1 = getBatteryVoltage( AIN_0 );
			lj.battery2 = getBatteryVoltage( AIN_1 );
			lj.pressure = getBatteryVoltage( AIN_2 );
			lj.water    = getBatteryVoltage( AIN_3 );
			msg.status.data.battery1 = lj.battery1;
			msg.status.data.battery2 = lj.battery2;
			msg.status.data.pressure = lj.pressure;
			msg.status.data.water = lj.water;
		}
	}

	exit( 0 );
} /* end main() */
