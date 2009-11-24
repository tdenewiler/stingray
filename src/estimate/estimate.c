/*------------------------------------------------------------------------------
 *
 *  Title:	estimate.c
 *
 *----------------------------------------------------------------------------*/

#include "estimate.h"

/// Global file descriptors. Only global so that planner_exit() can close them.
int nav_fd;
int lj_fd;
FILE *f_log;


/*------------------------------------------------------------------------------
 * int estimate_sigint()
 * Called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void estimate_sigint(int signal)
{
	exit(0);
} /* end estimate_sigint() */


/*------------------------------------------------------------------------------
 * int estimate_exit()
 * Exit function for main program. Closes all file descriptors. This function
 * is called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void estimate_exit()
{
	printf("ESTIMATE_EXIT: Shutting down estimate program ... ");
	/// Sleep to let things shut down properly.
	usleep(200000);

	/// Close the open file descriptors.
	if (nav_fd > 0) {
		close(nav_fd);
	}
	if (lj_fd > 0) {
		close(lj_fd);
	}

	/// Close the open file pointers.
	if (f_log) {
		fclose(f_log);
	}

	printf("<OK>\n\n");
} /* end estimate_exit() */


/*------------------------------------------------------------------------------
 * int main()
 * Initialize data. Open ports. Run main program loop.
 *----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
	/// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
	void(*exit_ptr)(void);
	exit_ptr = estimate_exit;
	atexit(exit_ptr);

	struct sigaction sigint_action;
	sigint_action.sa_handler = estimate_sigint;
	sigint_action.sa_flags = 0;
	sigaction(SIGINT, &sigint_action, NULL);

	/// Declare variables.
	int recv_bytes = 0;
	char nav_buf[MAX_MSG_SIZE];
	char lj_buf[MAX_MSG_SIZE];
	CONF_VARS cf;
	MSG_DATA msg;
	int ks_closed = FALSE;

	/// Declare timers.
	TIMING timer_input;
	TIMING timer_log;
	TIMING timer_axis;

	printf("MAIN: Starting Estimate ... \n");

	/// Initialize variables.
	nav_fd = -1;
	memset(&msg, 0, sizeof(MSG_DATA));
	messages_init(&msg);

	/// Parse command line arguments.
	parse_default_config(&cf);
	parse_cla(argc, argv, &cf, STINGRAY, (const char *)ESTIMATE_FILENAME);

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

	/// Connect to the labjack daemon.
    if (cf.enable_labjack) {
        lj_fd = net_client_setup(cf.labjackd_IP, cf.labjackd_port);
		if (lj_fd > 0) {
			printf("MAIN: Labjack client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Labjack client setup failed.\n");
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
	timing_set_timer(&timer_log);
	timing_set_timer(&timer_axis);

	/// Get input sequences for each axis.
	float seq_pitch[cf.input_size];
	memset(&seq_pitch, 0, cf.input_size);
	if (cf.input_type == INPUT_PRB) {
		sysid_get_prb_seq(seq_pitch, -5., 5., cf.input_size);
	}
	else {
		sysid_get_step_seq(seq_pitch, -5., 5., cf.input_size);
	}
	usleep(10000);

	float seq_roll[cf.input_size];
	memset(&seq_roll, 0, cf.input_size);
	if (cf.input_type == INPUT_PRB) {
		sysid_get_prb_seq(seq_roll, -5., 5., cf.input_size);
	}
	else {
		sysid_get_step_seq(seq_roll, -5., 5., cf.input_size);
	}
	usleep(10000);

	float seq_yaw[cf.input_size];
	memset(&seq_yaw, 0, cf.input_size);
	if (cf.input_type == INPUT_PRB) {
		sysid_get_prb_seq(seq_yaw, -5., 5., cf.input_size);
	}
	else {
		sysid_get_step_seq(seq_yaw, -5., 5., cf.input_size);
	}
	usleep(10000);

	float seq_depth[cf.input_size];
	memset(&seq_depth, 0, cf.input_size);
	if (cf.input_type == INPUT_PRB) {
		sysid_get_prb_seq(seq_depth, 0.1, 0.3, cf.input_size);
	}
	else {
		sysid_get_step_seq(seq_depth, 0.1, 0.3, cf.input_size);
	}

	int seq_num = 0;
	if (cf.input_type == INPUT_RAMP) {
		seq_num = floor(cf.input_size / 2);
	}
	int seq = 1;
	int direction = 1;

	for (int i = 0; i < cf.input_size; i++) {
		printf("MAIN: seq_pitch = %f\n", seq_pitch[i]);
	}

	printf("MAIN: Estimate running now.\n");
	printf( "\n" );

	///
	/// Main loop.
	///
	while (1) {
        /// Get nav data.
		if (nav_fd > 0) {
			msg.target.data.task = msg.task.data.task;
			msg.target.data.vision_status = msg.vision.data.status;
			recv_bytes = net_client(nav_fd, nav_buf, &msg, MODE_PLANNER);
			nav_buf[recv_bytes] = '\0';
			if (recv_bytes > 0) {
				messages_decode(nav_fd, nav_buf, &msg, recv_bytes);
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

		/// Get labjack data.
        if ((cf.enable_labjack) && (lj_fd > 0)) {
            recv_bytes = net_client(lj_fd, lj_buf, &msg, MODE_OPEN);
            lj_buf[recv_bytes] = '\0';
            if (recv_bytes > 0) {
                messages_decode(lj_fd, lj_buf, &msg, recv_bytes);
				msg.status.data.depth = msg.lj.data.pressure;
            }
        }

        /// Log if flag is set and timer has elapsed.
		if (timing_check_period(&timer_log, cf.period_log) && f_log) {
			sysid_log(&msg, f_log);
			timing_set_timer(&timer_log);
		}

		/// Send new input if input timer has elapsed.
		// Might still need to wait for input period after setting axis to cf value so two things aren't happening at once.
		if ((cf.input_type == INPUT_STEP) || (cf.input_type == INPUT_PRB)) {
			if (timing_check_period(&timer_input, cf.period_input)) {
				switch (seq) {
				case 1:
				printf("MAIN: Hit input timer at %fs using pitch target %f\n", timing_get_dts(&timer_input), seq_pitch[seq_num]);
				msg.target.data.pitch = seq_pitch[seq_num];
				timing_set_timer(&timer_input);
				seq_num++;
				if (seq_num == cf.input_size) {
					msg.target.data.pitch = cf.target_pitch;
					seq_num = 0;
					seq++;
				}
				break;

				case 2:
				printf("MAIN: Hit input timer at %fs using roll target %f\n", timing_get_dts(&timer_input), seq_roll[seq_num]);
				msg.target.data.roll = seq_roll[seq_num];
				timing_set_timer(&timer_input);
				seq_num++;
				if (seq_num == cf.input_size) {
					msg.target.data.roll = cf.target_roll;
					seq_num = 0;
					seq++;
				}
				break;

				case 3:
				printf("MAIN: Hit input timer at %fs using yaw target %f\n", timing_get_dts(&timer_input), seq_yaw[seq_num]);
				msg.target.data.yaw = seq_yaw[seq_num];
				timing_set_timer(&timer_input);
				seq_num++;
				if (seq_num == cf.input_size) {
					msg.target.data.yaw = cf.target_yaw;
					seq_num = 0;
					seq++;
				}
				break;

				case 4:
				printf("MAIN: Hit input timer at %fs using depth target %f\n", timing_get_dts(&timer_input), seq_depth[seq_num]);
				msg.target.data.depth = seq_depth[seq_num];
				timing_set_timer(&timer_input);
				seq_num++;
				if (seq_num == cf.input_size) {
					msg.target.data.depth = cf.target_depth;
					seq_num = 0;
					seq++;
				}
				break;

				default:
				exit(0);
				}
			}
		}
		/// Use ramp input where switching is done using a probability.
		else if (cf.input_type == INPUT_RAMP) {
			if (timing_check_period(&timer_axis, cf.period_axis)) {
				/// Reset the current axis back to cf value before moving on to next axis.
				switch (seq) {
				case 1:
				msg.target.data.pitch = cf.target_pitch;
				break;
				
				case 2:
				msg.target.data.roll = cf.target_roll;
				break;

				case 3:
				msg.target.data.yaw = cf.target_yaw;
				break;

				case 4:
				msg.target.data.depth = cf.target_depth;
				break;
				}
				
				/// Move on to next axis.
				seq++;
				seq_num = floor(cf.input_size / 2);
				timing_set_timer(&timer_axis);
			}
			if (timing_check_period(&timer_input, cf.period_input)) {
				if (sysid_switch(cf.input_prob) == TRUE) {
					direction *= -1;
				}
				seq_num += direction;
				if (seq_num > cf.input_size - 1) {
					seq_num = cf.input_size - 1;
					direction *= -1;
				}
				else if (seq_num < 0) {
					seq_num = 0;
					direction *= -1;
				}
				switch (seq) {
				case 1:
				printf("MAIN: Hit input timer at %fs using pitch target %f at seq_num %d\n", timing_get_dts(&timer_input), seq_pitch[seq_num], seq_num);
				msg.target.data.pitch = seq_pitch[seq_num];
				timing_set_timer(&timer_input);
				break;

				case 2:
				printf("MAIN: Hit input timer at %fs using roll target %f at seq_num %d\n", timing_get_dts(&timer_input), seq_roll[seq_num], seq_num);
				msg.target.data.roll = seq_roll[seq_num];
				timing_set_timer(&timer_input);
				break;

				case 3:
				printf("MAIN: Hit input timer at %fs using yaw target %f at seq_num %d\n", timing_get_dts(&timer_input), seq_yaw[seq_num], seq_num);
				msg.target.data.yaw = seq_yaw[seq_num];
				timing_set_timer(&timer_input);
				break;

				case 4:
				printf("MAIN: Hit input timer at %fs using depth target %f at seq_num %d\n", timing_get_dts(&timer_input), seq_depth[seq_num], seq_num);
				msg.target.data.depth = seq_depth[seq_num];
				timing_set_timer(&timer_input);
				break;

				default:
				exit(0);
				}
			}
		}

	}

	exit(0);
} /* end main() */
