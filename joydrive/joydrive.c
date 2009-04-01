/******************************************************************************
 *
 *  Title:        joydrive.c
 *
 *  Description:  Main program for joydrive daemon.
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

#include "joydrive.h"
#include "network.h"
#include "labjack.h"
#include "util.h"
#include "messages.h"
#include "parser.h"
#include "joy.h"


/* Global file descriptors. Only global so that labjackd_exit() can close them. */
int joy_fd;
int nav_fd;


/******************************************************************************
 *
 * Title:       void joydrive_sigint( int signal )
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

void joydrive_sigint( int signal )
{
	exit( 0 );
} /* end joydrive_sigint() */


/******************************************************************************
 *
 * Title:       void joydrive_exit( )
 *
 * Description: Exit function for main program. Closes all file descriptors.
 *              This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     File descriptors: joy_fd, net_fd.
 *
 *****************************************************************************/

void joydrive_exit( )
{
	/* Sleep to let things shut down properly. */
	usleep( 200000 );

	/* Close the open file descriptors. */
	if ( joy_fd > 0 ) {
		close( joy_fd );
	}
	if ( nav_fd > 0 ) {
		close( nav_fd );
	}
} /* end joydrive_exit() */


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
	exit_ptr = joydrive_exit;
	atexit( exit_ptr );

	struct sigaction sigint_action;
	sigint_action.sa_handler = joydrive_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	int recv_bytes = 0;
	int status = -1;
	int mode = MODE_JOY;
	char recv_buf[MAX_MSG_SIZE];
	MSG_DATA msg;
	CONF_VARS cf;
	JOY_DATA joy;

	/* Initialize variables. */
	joy_fd = -1;
	nav_fd = -1;

	memset( &msg, 0, sizeof( MSG_DATA ) );
	memset( &cf, 0, sizeof( CONF_VARS ) );
	memset( &joy, 0, sizeof( JOY_DATA ) );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, ( const char * )JOYDRIVE_FILENAME );

	/* Set up communications. */
	nav_fd = net_client_setup( cf.server_IP, cf.api_port );

	/* Set up the joystick. */
	joy_fd = joy_setup( );

	/* Main loop. */
	while ( 1 ) {
		/* Get network data. */
		if ( nav_fd > 0 ) {
			recv_bytes = net_client( nav_fd, recv_buf, &msg, mode );
			recv_buf[recv_bytes] = '\0';
		}
		if ( recv_bytes > 0 ) {
			messages_decode( nav_fd, recv_buf, &msg );
		}

		/* Get joystick data. */
		if ( joy_fd > 0 ) {
			status = joy_get_data( joy_fd, &joy );
		}

		/* Set target change values based on joystick input. */
		if ( joy.joy_button == JOY_A5 ) {
			if ( joy.axis_value > 0 ) {
				msg.target.data.pitchd = 1;
			}
			else if ( joy.axis_value < 0 ) {
				msg.target.data.pitchd = -1;
			}
		}
		if ( joy.joy_button == JOY_A4 ) {
			if ( joy.axis_value > 0 ) {
				msg.target.data.rolld = 1;
			}
			else if ( joy.axis_value < 0 ) {
				msg.target.data.rolld = -1;
			}
		}
		if ( joy.joy_button == JOY_B0 ) {
			msg.target.data.yawd = -1;
		}
		else if ( joy.joy_button == JOY_B2 ) {
			msg.target.data.yawd = 1;
		}
		if ( joy.joy_button == JOY_B1 ) {
			msg.target.data.depthd = -1;
		}
		else if ( joy.joy_button == JOY_B3 ) {
			msg.target.data.depthd = 1;
		}
		if ( joy.joy_button == JOY_A0 ) {
			if ( joy.axis_value > 0 ) {
				msg.target.data.fxd = 1;
			}
			else if ( joy.axis_value < 0 ) {
				msg.target.data.fxd = -1;
			}
		}
		if ( joy.joy_button == JOY_A2 ) {
			if ( joy.axis_value > 0 ) {
				msg.target.data.fyd = 1;
			}
			else if ( joy.axis_value < 0 ) {
				msg.target.data.fyd = -1;
			}
		}
		if ( joy.joy_button == JOY_B4 ) {
			msg.target.data.speedd = -1;
		}
		else if ( joy.joy_button == JOY_B5 ) {
			msg.target.data.speedd = 1;
		}
		if (joy.joy_button == JOY_B6 ) {
			if ( joy.button_value == 1 ) {
				if ( joy.joy_button == JOY_B7 ) {
					if ( joy.button_value == 1 ) {
						printf( "MAIN: send estop\n" );
					}
				}
			}
		}
		printf( "MAIN: targets\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n"
				, msg.target.data.pitchd
				, msg.target.data.rolld
				, msg.target.data.yawd
				, msg.target.data.depthd
				, msg.target.data.fxd
				, msg.target.data.fyd
				, msg.target.data.speedd
		);
		/* Reset the target change values back to zero. */
		msg.target.data.pitchd = 0;
		msg.target.data.rolld = 0;
		msg.target.data.yawd = 0;
		msg.target.data.depthd = 0;
		msg.target.data.fxd = 0;
		msg.target.data.fyd = 0;
		msg.target.data.speedd = 0;
	}

	exit( 0 );
} /* end main() */
