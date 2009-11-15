/*------------------------------------------------------------------------------
 *
 *  Title:	estimate.c
 *
 *----------------------------------------------------------------------------*/

#include "estimate.h"

/// Global file descriptors. Only global so that planner_exit() can close them.
int nav_fd;
FILE *f_log;

/*------------------------------------------------------------------------------
 * int estimate_sigint()
 * Called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void estimate_sigint( int signal )
{
	exit( 0 );
} /* end estimate_sigint() */


/*------------------------------------------------------------------------------
 * int estimate_exit()
 * Exit function for main program. Closes all file descriptors. This function
 * is called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void estimate_exit( )
{
	printf("ESTIMATE_EXIT: Shutting down planner program ... ");
	/// Sleep to let things shut down properly.
	usleep( 200000 );

	/// Close the open file descriptors.
	if (nav_fd > 0) {
		close(nav_fd);
	}

	/// Close the open file pointers.
	if (f_log) {
		fclose(f_log);
	}

	printf("<OK>\n\n");
} /* end estimate_exit() */


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
	/// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
	void( *exit_ptr )( void );
	exit_ptr = estimate_exit;
	atexit( exit_ptr );

	struct sigaction sigint_action;
	sigint_action.sa_handler = estimate_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	/// Declare variables.
	int recv_bytes = 0;
	char nav_buf[MAX_MSG_SIZE];
	CONF_VARS cf;
	MSG_DATA msg;
	int ks_closed = FALSE;

	/// Declare timers.
	TIMING timer_input;

	printf("MAIN: Starting Estimate ... \n");

	/// Initialize variables.
	nav_fd = -1;

	memset( &msg, 0, sizeof(MSG_DATA) );
	messages_init( &msg );

	/// Parse command line arguments.
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, (const char *)ESTIMATE_FILENAME );

	/// Set up default values for the targets, gains and tasks.
    msg.target.data.pitch   	 = cf.target_pitch;
    msg.target.data.roll    	 = cf.target_roll;
    msg.target.data.yaw     	 = cf.target_yaw;
    msg.target.data.depth   	 = cf.target_depth;
    msg.gain.data.kp_pitch  	 = cf.kp_pitch;
    msg.gain.data.ki_pitch  	 = cf.ki_pitch;
    msg.gain.data.kd_pitch  	 = cf.kd_pitch;
    msg.gain.data.kp_roll    	 = cf.kp_roll;
    msg.gain.data.ki_roll   	 = cf.ki_roll;
    msg.gain.data.kd_roll   	 = cf.kd_roll;
    msg.gain.data.kp_yaw    	 = cf.kp_yaw;
    msg.gain.data.ki_yaw    	 = cf.ki_yaw;
    msg.gain.data.kd_yaw    	 = cf.kd_yaw;
    msg.gain.data.kp_depth  	 = cf.kp_depth;
    msg.gain.data.ki_depth  	 = cf.ki_depth;
    msg.gain.data.kd_depth  	 = cf.kd_depth;

    /// Set up the nav network client.
	if (cf.enable_nav) {
        nav_fd = net_client_setup(cf.nav_IP, cf.nav_port);
		if (nav_fd > 0) {
			printf("MAIN: Nav client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Nav client setup failed.\n");
		}
    }

    /// Open log file if flag set.
    if (cf.enable_log) {
    	f_log = fopen("logestimate.csv", "w");
    	if (f_log) {
			sysid_log_init(f_log);
		}
	}

	/// Initialize timers.
	timing_set_timer(&timer_input);

	/// Get input sequences for each axis.

	printf("MAIN: Planner running now.\n");
	printf( "\n" );

	/// Main loop.
	while (1) {
        /// Get nav data.
		if( nav_fd > 0 ) {
			msg.target.data.task = msg.task.data.task;
			msg.target.data.vision_status = msg.vision.data.status;
			recv_bytes = net_client( nav_fd, nav_buf, &msg, MODE_PLANNER );
			nav_buf[recv_bytes] = '\0';
			if( recv_bytes > 0 ) {
				messages_decode( nav_fd, nav_buf, &msg, recv_bytes );
			}
			/// Check the kill switch state.
			if (!ks_closed) {
				if (msg.lj.data.battery1 > 5) {
					ks_closed = TRUE;
				}
				else {
					ks_closed = FALSE;
				}
			}
			if (ks_closed) {
				if (msg.lj.data.battery1 < 5) {
					ks_closed = FALSE;
				}
			}
		}

        /// Log if flag is set.
        if (cf.enable_log && f_log) {
			sysid_log(&msg, f_log);
		}

		/// Send new input if input timer has elapsed.
	}

	exit( 0 );
} /* end main() */
