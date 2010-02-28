/******************************************************************************
 *
 *  Title:        nav.c
 *
 *  Description:  Main program for Stingray UUV.
 *
 *****************************************************************************/

#include "nav.h"

/// Global file descriptors. Only global so that nav_exit() can close them.
int server_fd;
int pololu_fd;
int imu_fd;
int lj_fd;

/*------------------------------------------------------------------------------
 * void nav_sigint()
 * Callback for when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void nav_sigint(int signal)
{
    exit(0);
} /* end nav_sigint() */


/*------------------------------------------------------------------------------
 * void nav_exit()
 * Exit function for main program. Sets actuators to safe values and closes all
 * open file descriptors. Callback for when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void nav_exit()
{
    printf("\nNAV_EXIT: Shutting down nav program ... ");
    /// Sleep to let things shut down properly.
    usleep(200000);

    /// Close the open file descriptors.
    if (pololu_fd > 0) {
        /// Set all the actuators to safe positions.
        pololu_initialize_channels(pololu_fd);
        usleep(200000);
        close(pololu_fd);
    }
    if (imu_fd > 0) {
        close(imu_fd);
    }
    if (server_fd > 0) {
        close(server_fd);
    }
	if (lj_fd > 0) {
		close(lj_fd);
	}

    printf("<OK>\n\n");
} /* end nav_exit() */


/*------------------------------------------------------------------------------
 * int main()
 * Initialize data. Open ports. Run main program loop.
 *----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    /// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
    void(*exit_ptr)(void);
    exit_ptr = nav_exit;
    atexit(exit_ptr);

	/// Attach signals to exit function.
    struct sigaction sigint_action;
    sigint_action.sa_handler = nav_sigint;
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);
    sigaction(SIGTERM, &sigint_action, NULL);
    sigaction(SIGQUIT, &sigint_action, NULL);
    sigaction(SIGHUP, &sigint_action, NULL);

	/// Declare variables.
    int status = -1;
	int pololu_initialized = FALSE;
	int pololu_starting = FALSE;
    int recv_bytes = 0;
    int mode = MODE_STATUS;
	int mstrain_serial = 0;
    char recv_buf[MAX_MSG_SIZE];
	char lj_buf[MAX_MSG_SIZE];
    CONF_VARS cf;
    MSG_DATA msg;
    PID pid;
	float dt = 0.;
    TIMING timer_pitch;
    TIMING timer_roll;
    TIMING timer_yaw;
    TIMING timer_depth;
	TIMING timer_pololu;
	TIMING timer_print;
	int count_pitch = 0;
	int count_roll = 0;
	int count_yaw = 0;
	int count_depth = 0;
	int count_mstrain = 0;

    printf("MAIN: Starting Navigation ... \n");

    /// Initialize variables.
    server_fd = -1;
    pololu_fd = -1;
    imu_fd = -1;
	lj_fd = 0;
    memset(&msg, 0, sizeof(MSG_DATA));
    memset(&pid, 0, sizeof(PID));
    memset(&recv_buf, 0, MAX_MSG_SIZE);
    memset(&lj_buf, 0, MAX_MSG_SIZE);
	messages_init(&msg);

    /// Parse command line arguments.
    parse_default_config(&cf);
    parse_cla(argc, argv, &cf, STINGRAY, (const char *)STINGRAY_FILENAME);

    /// Initialize the PID controllers with configuration file values.
    status = pid_init(&pid, &cf);

    /// Set up server.
    if (cf.enable_server) {
        server_fd = net_server_setup(cf.server_port);
		if (server_fd > 0) {
			printf("MAIN: Nav server setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Nav server setup failed.\n");
		}
    }

    /// Set up the Microstrain IMU.
    if (cf.enable_imu) {
        imu_fd = mstrain_setup(cf.imu_port , cf.imu_baud);
		status = mstrain_serial_number(imu_fd, &mstrain_serial);
		if (mstrain_serial == MSTRAIN_SERIAL) {
			printf("MAIN: IMU setup OK.\n");
		}
		else {
			printf("MAIN: SIMULATION MODE!!! IMU data is simulated.\n");
			/// Seed the random number generator.
			srand((unsigned int)time(NULL));
			imu_fd = 0;
		}
    }
	else {
		printf("MAIN: SIMULATION MODE!!! IMU data is simulated.\n");
		/// Seed the random number generator.
		srand((unsigned int)time(NULL));
	}

    /// Set up the Pololu servo controller.
    if (cf.enable_pololu) {
        pololu_fd = pololu_setup(cf.pololu_port, cf.pololu_baud);
		if (pololu_fd > 0) {
			/// Initialize the pololu.
			pololu_initialize_channels(pololu_fd);
			printf("MAIN: Pololu setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Pololu setup failed.\n");
		}
    }

	/// Connect to the labjack daemon.
	if ((cf.enable_labjack > 0) && (cf.enable_pololu > 0)) {
        lj_fd = net_client_setup(cf.labjackd_IP, cf.labjackd_port);
		if (lj_fd > 0) {
			printf("MAIN: Labjack client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Labjack client setup failed.\n");
		}
	}

    /// Initialize timers.
    timing_set_timer(&timer_pitch);
    timing_set_timer(&timer_roll);
    timing_set_timer(&timer_yaw);
    timing_set_timer(&timer_depth);
    timing_set_timer(&timer_pololu);
    timing_set_timer(&timer_print);

	printf("MAIN: Nav running now.\n");

    /// Main loop. Will exit on <ctrl-c>.
    while (1) {
		/// Check labjack data to see if kill switch has been closed.
		if ((cf.enable_pololu > 0) && (cf.enable_labjack > 0) && (lj_fd > 0)) {
			recv_bytes = net_client(lj_fd, lj_buf, &msg, mode);
			lj_buf[recv_bytes] = '\0';
			if (recv_bytes > 0) {
				messages_decode(lj_fd, lj_buf, &msg, recv_bytes);
				msg.status.data.depth = msg.lj.data.pressure;
			}
			if (pololu_initialized == FALSE) {
				/// Get the state of the kill switch.
				if (msg.lj.data.battery1 > BATT1_THRESH) {
					if (pololu_starting == FALSE) {
						pololu_initialize_channels(pololu_fd);
						pololu_starting = TRUE;
						/// Start the timer.
						timing_set_timer(&timer_pololu);
					}
					/// Check that 7 seconds have elapsed since initializing Pololu.
					if ((dt = timing_check_period(&timer_pololu, (float)POLOLU_INIT_TIME))) {
						pololu_initialized = TRUE;
						pololu_starting = FALSE;
						pid.pitch.ierr = 0;
						pid.roll.ierr = 0;
						pid.yaw.ierr = 0;
						pid.depth.ierr = 0;
						pid.fx.ierr = 0;
						pid.fy.ierr = 0;
						msg.target.data.yaw = msg.status.data.yaw;
						printf("MAIN: Pololu initialized.\n");
					}
				}
				else {
					pololu_initialized = FALSE;
					pololu_starting = FALSE;
				}
			}
			else {
				/// Get the state of the kill switch.
				if (msg.lj.data.battery1 > BATT1_THRESH) {
					pololu_initialized = TRUE;
				}
				else {
					pololu_initialized = FALSE;
				}
			}
		}

        /// Get network data.
        if ((cf.enable_server) && (server_fd > 0)) {
            recv_bytes = net_server(server_fd, recv_buf, &msg, MODE_PLANNER);
            if (recv_bytes > 0) {
                recv_buf[recv_bytes] = '\0';
                messages_decode(server_fd, recv_buf, &msg, recv_bytes);
				/// Make sure that the servo and speed commands are zero if Pololu is not initialized.
				if (pololu_initialized == FALSE) {
					msg.target.data.fx = 0;
					msg.target.data.fy = 0;
					msg.target.data.speed = 0;
				}
            }
        }

        /// Get Microstrain data.
        if ((cf.enable_imu) && (imu_fd > 0)) {
            //recv_bytes = mstrain_euler_angles(imu_fd, &msg.mstrain.data.pitch, &msg.mstrain.data.roll, &msg.mstrain.data.yaw);
			//recv_bytes = mstrain_vectors(imu_fd, 0, msg.mstrain.data.mag, msg.mstrain.data.accel, msg.mstrain.data.ang_rate);
			recv_bytes = mstrain_euler_vectors(imu_fd, &msg.mstrain.data.pitch, &msg.mstrain.data.roll, &msg.mstrain.data.yaw, msg.mstrain.data.accel, msg.mstrain.data.ang_rate);
			count_mstrain++;
        }
		else {
			/// Simulation Mode. This is where the simulated data is generated.
			msg.mstrain.data.pitch       = cf.target_pitch + rand() / (float)RAND_MAX;
			msg.mstrain.data.roll        = cf.target_roll  + rand() / (float)RAND_MAX;
			msg.mstrain.data.yaw         = cf.target_yaw   + rand() / (float)RAND_MAX;
			msg.mstrain.data.accel[0]    = 0 + rand() / (float)RAND_MAX;
			msg.mstrain.data.accel[1]    = 0 + rand() / (float)RAND_MAX;
			msg.mstrain.data.accel[2]    = 0 + rand() / (float)RAND_MAX;
			msg.mstrain.data.ang_rate[0] = 0 + rand() / (float)RAND_MAX;
			msg.mstrain.data.ang_rate[1] = 0 + rand() / (float)RAND_MAX;
			msg.mstrain.data.ang_rate[2] = 0 + rand() / (float)RAND_MAX;
		}

        /// Perform PID loops.
        if (msg.stop.data.state == FALSE) {
            /// Pitch.
			if ((dt = timing_check_period(&timer_pitch, cf.period_pitch))) {
				//printf("MAIN: Hit pitch timer at %fs.\n", dt);
				timing_set_timer(&timer_pitch);
                pid_loop(pololu_fd, &pid, &cf, &msg, dt, PID_PITCH, pololu_initialized);
				count_pitch++;
            }

            /// Roll.
			if ((dt = timing_check_period(&timer_roll, cf.period_roll))) {
				timing_set_timer(&timer_roll);
                pid_loop(pololu_fd, &pid, &cf, &msg, dt, PID_ROLL, pololu_initialized);
                count_roll++;
            }

            /// Yaw.
			if ((dt = timing_check_period(&timer_yaw, cf.period_yaw))) {
				timing_set_timer(&timer_yaw);
                pid_loop(pololu_fd, &pid, &cf, &msg, dt, PID_YAW, pololu_initialized);
                count_yaw++;
            }

            /// Depth.
			if ((dt = timing_check_period(&timer_depth, cf.period_depth))) {
				timing_set_timer(&timer_depth);
                pid_loop(pololu_fd, &pid, &cf, &msg, dt, PID_DEPTH, pololu_initialized);
                count_depth++;
            }
        }

		// Temporary check to see how fast PID loop is reached.
		if (timing_check_period(&timer_print, 1.)) {
			printf("MAIN: %d %d %d %d PID loops and %d Microstrain reads per second.\n",
			    count_pitch, count_roll, count_yaw, count_depth, count_mstrain);
			count_pitch = 0;
			count_roll = 0;
			count_yaw = 0;
			count_depth = 0;
			count_mstrain = 0;
			timing_set_timer(&timer_print);
		}

        /// Update status message.
        messages_update(&msg);
    }

    exit(0);
} /* end main() */
