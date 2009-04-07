/******************************************************************************
 *
 *  Title:        planner.c
 *
 *  Description:  Main program for planner.
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

#include "planner.h"
#include "microstrain.h"
#include "network.h"
#include "parser.h"
#include "labjack.h"
#include "pololu.h"
#include "util.h"
#include "serial.h"
#include "messages.h"
#include "pid.h"
#include "task.h"


/* Global file descriptors. Only global so that planner_exit() can close them. */
int server_fd;
int vision_fd;


/******************************************************************************
 *
 * Title:       void planner_sigint( int signal )
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

void planner_sigint( int signal )
{
	exit( 0 );
} /* end planner_sigint() */


/******************************************************************************
 *
 * Title:       void planner_exit( )
 *
 * Description: Exit function for main program. Closes all file descriptors.
 *              This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     File descriptors: server_fd, pololu_fd, labjack_fd, imu_fd.
 *
 *****************************************************************************/

void planner_exit( )
{
	printf( "PLANNER_EXIT: Shutting down planner program ... " );
	/* Sleep to let things shut down properly. */
	usleep( 200000 );

	/* Close the open file descriptors. */

	if ( server_fd > 0 ) {
		close( server_fd );
	}

	if ( vision_fd > 0 ) {
		close( vision_fd );
	}

	printf( "<OK>\n\n" );
} /* end planner_exit() */


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
	exit_ptr = planner_exit;
	atexit( exit_ptr );

	struct sigaction sigint_action;
	sigint_action.sa_handler = planner_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	int recv_bytes = 0;
	char recv_buf[MAX_MSG_SIZE];
	char vision_buf[MAX_MSG_SIZE];
	CONF_VARS cf;
	MSG_DATA msg;
	PID pid;
	LABJACK_DATA lj;

	struct timeval vision_time = {0, 0};

	struct timeval vision_start = {0, 0};

	struct timeval plan_time = {0, 0};

	struct timeval plan_start = {0, 0};

	struct timeval task_time = {0, 0};

	struct timeval task_start = {0, 0};
	int time1s = 0;
	int time1ms = 0;
	int time2s = 0;
	int time2ms = 0;
	int dt = 0;

	printf( "MAIN: Starting Planner ... \n" );

	/* Initialize variables. */
	server_fd = -1;
	vision_fd = -1;

	memset( &msg, 0, sizeof( MSG_DATA ) );
	memset( &pid, 0, sizeof( PID ) );
	memset( &lj, 0, sizeof( LABJACK_DATA ) );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, ( const char * )PLANNER_FILENAME );

	/* Set up communications. */
	if ( cf.enable_planner ) {
		server_fd = net_server_setup( cf.planner_port );
	}

	/* Set up the vision network client. */
	if ( cf.enable_vision ) {
		vision_fd = net_client_setup( cf.vision_IP, cf.vision_port );
	}

	/* Initialize timers. */
	gettimeofday( &vision_time, NULL );
	gettimeofday( &vision_start, NULL );
	gettimeofday( &plan_time, NULL );
	gettimeofday( &plan_start, NULL );
	gettimeofday( &task_time, NULL );
	gettimeofday( &task_start, NULL );
	printf( "<OK>\n" );

	/* Main loop. */
	while ( 1 ) {
		/* Get network data. */
		if ( ( cf.enable_net ) && ( server_fd > 0 ) ) {
			recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_PLANNER );
			if ( recv_bytes > 0 ) {
				recv_buf[recv_bytes] = '\0';
				messages_decode( server_fd, recv_buf, &msg );
				printf( "MAIN: task = %d\n", msg.task.data.num );
			}
		}

		/* Get vision data. */
		if ( ( cf.enable_vision ) && ( vision_fd > 0 ) ) {
			time1s =    vision_time.tv_sec;
			time1ms =   vision_time.tv_usec;
			time2s =    vision_start.tv_sec;
			time2ms =   vision_start.tv_usec;
			dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );

			if ( dt > cf.period_vision ) {
				recv_bytes = net_client( vision_fd, vision_buf, &msg, MODE_STATUS );
				vision_buf[recv_bytes] = '\0';

				if ( recv_bytes > 0 ) {
					messages_decode( vision_fd, vision_buf, &msg );
					printf( "MAIN: vision = %d %d\n", msg.vision.data.front_x,
					        msg.vision.data.front_y );
					gettimeofday( &vision_start, NULL );
				}
			}
		}

		/* Update the task dt. */
		time1s =    task_time.tv_sec;

		time1ms =   task_time.tv_usec;

		time2s =    task_start.tv_sec;

		time2ms =   task_start.tv_usec;

		dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );

		/* Run the current task. */
		task_run( &msg, dt );

		/* Update timers. */
		gettimeofday( &vision_time, NULL );

		gettimeofday( &plan_time, NULL );

		gettimeofday( &task_time, NULL );
	}

	exit( 0 );
} /* end main() */
