/******************************************************************************
 *
 *  Title:        labjackd.c
 *
 *  Description:  Main program for labjack daemon.
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

#include "labjackd.h"
#include "network.h"
#include "labjack.h"
#include "util.h"
#include "messages.h"
#include "parser.h"


/* Global file descriptors. Only global so that labjackd_exit() can close them. */
int labjack_fd;
int labjackd_fd;


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
 *****************************************************************************/

void labjackd_exit( )
{
	printf("LABJACKD_EXIT: Shutting down ... ");

	/* Sleep to let things shut down properly. */
	usleep( 200000 );

	/* Close the open file descriptors. */
	if ( labjackd_fd > 0 ) {
		close( labjackd_fd );
	}
	if ( labjack_fd > 0 ) {
		close( labjack_fd );
	}

	printf("<OK>\n");
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
	int status = -1;
	char recv_buf[MAX_MSG_SIZE];
	MSG_DATA msg;
	LABJACK_DATA lj;
	CONF_VARS cf;

	struct timeval sim_time = {0, 0};
	struct timeval sim_start = {0, 0};
	int time1s = 0;
    int time1ms = 0;
    int time2s = 0;
    int time2ms = 0;
    int dt = 0;

	printf("MAIN: Starting Labjack daemon ...\n");

	/* Initialize variables. */
	labjack_fd = -1;
	labjackd_fd = -1;

	memset( &msg, 0, sizeof( MSG_DATA ) );
	memset( &cf, 0, sizeof( CONF_VARS ) );
	memset( &lj, 0, sizeof( LABJACK_DATA ) );
	messages_init( &msg );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, ( const char * )LABJACKD_FILENAME );

	/* Set up server. */
	labjackd_fd = net_server_setup( cf.server_port );
	if ( labjackd_fd > 0 ) {
		printf("MAIN: Server setup OK.\n");
	}
	else {
		printf("MAIN: WARNING!!! Server setup failed.\n");
	}

	/* Set up the labjack. */
	labjack_fd = init_labjack( );
	if ( labjack_fd ) {
		status = query_labjack( );
		printf("MAIN: Labjack setup OK.\n", status);
	}
	else
	{
		printf("MAIN: SIMULATION MODE!!! Labjack data is simulated.\n");
		/* Seed the random variable. */
		srand((unsigned int) time(NULL) );
	}

	 /* Initialize timers. */
    gettimeofday( &sim_time, NULL );
    gettimeofday( &sim_start, NULL );

	printf("MAIN: Labjack server running now.\n");

	/* Main loop. */
	while ( 1 ) {
		/* Get network data. */
		if ( labjackd_fd > 0 ) {
			recv_bytes = net_server( labjackd_fd, recv_buf, &msg, MODE_LJ );
			if ( recv_bytes > 0 ) {
				recv_buf[recv_bytes] = '\0';
				messages_decode( labjackd_fd, recv_buf, &msg, recv_bytes );
			}
		}

		/* Get Labjack data. */
		if ( labjack_fd > 0 ) {
			status = query_labjack( );
			if ( status > 0 ) {
				lj.battery1 = getBatteryVoltage( AIN_0 );
				lj.battery2 = getBatteryVoltage( AIN_1 );
				lj.pressure = getBatteryVoltage( AIN_2 );
				lj.water    = getBatteryVoltage( AIN_3 );

				msg.lj.data.battery1 = lj.battery1;
				msg.lj.data.battery2 = lj.battery2;
				msg.lj.data.pressure = lj.pressure;
				msg.lj.data.water    = lj.water;
			}
		}
		else {
			/* Simulation Mode. This is where the simulated data is generated. */
			time1s =    sim_time.tv_sec;
            time1ms =   sim_time.tv_usec;
            time2s =    sim_start.tv_sec;
            time2ms =   sim_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
			if ( dt > 100000 ) {
				msg.lj.data.battery1 = 10.0  + rand() / (float)RAND_MAX;
				msg.lj.data.battery2 = 14.0  + rand() / (float)RAND_MAX;
				msg.lj.data.pressure = 0.543 + rand() / (float)RAND_MAX;
				msg.lj.data.water    = 0.289 + rand() / (float)RAND_MAX;
				gettimeofday( &sim_start, NULL );
			}
		}

		/* Check battery voltage. Make sure it is connected. If too low then
		 * have the computer shut down so that the battery is not damaged. */
		//if ( (lj.battery1 > BATT1_THRESH) && (lj.battery1 < BATT1_MIN) ) {
			//status = system("shutdown -h now \"Labjackd: Motor battery has low voltage.\"");
		//}
		if ( (lj.battery2 > BATT2_THRESH) && (lj.battery2 < BATT2_MIN) ) {
			status = system("shutdown -h now \"Labjackd: Computer battery has low voltage.\"");
		}

		/* Update timers. */
        gettimeofday( &sim_time, NULL );
	}

	exit( 0 );
} /* end main() */
