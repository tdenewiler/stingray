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
int lj_fd;
int nav_fd;


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
 *****************************************************************************/

void planner_exit( )
{
	printf("PLANNER_EXIT: Shutting down planner program ... ");
	/* Sleep to let things shut down properly. */
	usleep( 200000 );

	/* Close the open file descriptors. */
	if ( server_fd > 0 ) {
		close( server_fd );
	}
	if ( vision_fd > 0 ) {
		close( vision_fd );
	}
	if ( lj_fd > 0 ) {
		close( lj_fd );
	}
	if ( nav_fd > 0 ) {
		close( nav_fd );
	}

	printf("<OK>\n\n");
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
	char lj_buf[MAX_MSG_SIZE];
	char nav_buf[MAX_MSG_SIZE];
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

	int old_task = 0;

	printf("MAIN: Starting Planner ... \n");

	/* Initialize variables. */
	server_fd = -1;
	vision_fd = -1;
	lj_fd = -1;
	nav_fd = -1;

	memset( &msg, 0, sizeof( MSG_DATA ) );
	memset( &pid, 0, sizeof( PID ) );
	memset( &lj, 0, sizeof( LABJACK_DATA ) );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, ( const char * )PLANNER_FILENAME );

	/* Set up the default values for the target. */
	msg.target.data.pitch   = cf.target_pitch;
    msg.target.data.roll    = cf.target_roll;
    msg.target.data.yaw     = cf.target_yaw;
    msg.target.data.depth   = cf.target_depth;

	/* Set up communications. */
	if ( cf.enable_server ) {
		server_fd = net_server_setup( cf.server_port );
		if ( server_fd > 0 ) {
			printf("MAIN: Server setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Server setup failed.\n");
		}
	}

	/* Set up the vision network client. */
	if ( cf.enable_vision ) {
		vision_fd = net_client_setup( cf.vision_IP, cf.vision_port );
		if ( vision_fd > 0 ) {
			printf("MAIN: Vision client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Vision client setup failed.\n");
		}
	}

	/* Connect to the labjack daemon. */
    if ( cf.enable_labjack ) {
        lj_fd = net_client_setup( cf.labjackd_IP, cf.labjackd_port );
		if ( lj_fd > 0 ) {
			printf("MAIN: Labjack client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Labjack client setup failed.\n");
		}
    }

    /* Set up the nav network client. */
	if ( cf.enable_nav ) {
        nav_fd = net_client_setup( cf.nav_IP, cf.nav_port );
		if ( nav_fd > 0 ) {
			printf("MAIN: Nav client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Nav client setup failed.\n");
		}
    }

	/* Initialize timers. */
	gettimeofday( &vision_time, NULL );
	gettimeofday( &vision_start, NULL );
	gettimeofday( &plan_time, NULL );
	gettimeofday( &plan_start, NULL );
	gettimeofday( &task_time, NULL );
	gettimeofday( &task_start, NULL );
	printf("MAIN: Planner running now.\n");

	/* Main loop. */
	while ( 1 ) {
		/* Get network data. */
		if ( ( cf.enable_server ) && ( server_fd > 0 ) ) {
			recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_PLANNER );
			if ( recv_bytes > 0 ) {
				recv_buf[recv_bytes] = '\0';
				messages_decode( server_fd, recv_buf, &msg );
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
				recv_bytes = net_client( vision_fd, vision_buf, &msg, MODE_OPEN );
				vision_buf[recv_bytes] = '\0';
				if ( recv_bytes > 0 ) {
					messages_decode( vision_fd, vision_buf, &msg );

                    gettimeofday( &vision_start, NULL );
				}
			}

			/* If there is a new task then send to vision. */
			if ( old_task != msg.task.data.num ) {
				messages_send( vision_fd, TASK_MSGID, &msg );
				old_task = msg.task.data.num;
			}
		}

        /* Get nav data. */
		if ( nav_fd > 0 ) {
			recv_bytes = net_client( nav_fd, nav_buf, &msg, MODE_PLANNER );
			nav_buf[recv_bytes] = '\0';
			if ( recv_bytes > 0 ) {
				messages_decode( nav_fd, nav_buf, &msg );
			}
		}

		/* Get labjack data. */
        if ( (cf.enable_labjack) && (lj_fd > 0) ) {
            recv_bytes = net_client( lj_fd, lj_buf, &msg, MODE_OPEN );
            lj_buf[recv_bytes] = '\0';
            if ( recv_bytes > 0 ) {
                messages_decode( lj_fd, lj_buf, &msg );
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
