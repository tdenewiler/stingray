/******************************************************************************
 *
 *  Title:        planner.c
 *
 *  Description:  Main program for planner.
 *
 *****************************************************************************/

#include "planner.h"

/// Global file descriptors. Only global so that planner_exit() can close them.
int server_fd;
int vision_fd;
int lj_fd;
int nav_fd;
int bKF;
FILE *f_log;


/******************************************************************************
 *
 * Title:       void planner_sigint(int signal)
 *
 * Description: This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       signal: The SIGINT signal.
 *
 * Output:      None.
 *
 *****************************************************************************/

void planner_sigint(int signal)
{
	exit(0);
} /* end planner_sigint() */


/******************************************************************************
 *
 * Title:       void planner_exit()
 *
 * Description: Exit function for main program. Closes all file descriptors.
 *              This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/

void planner_exit()
{
	printf("PLANNER_EXIT: Shutting down planner program ... ");
	/// Sleep to let things shut down properly.
	usleep(200000);

	/// Close the open file descriptors.
	if (server_fd > 0) {
		close(server_fd);
	}
	if (vision_fd > 0) {
		close(vision_fd);
	}
	if (lj_fd > 0) {
		close(lj_fd);
	}
	if (nav_fd > 0) {
		close(nav_fd);
	}

	/// Close the open file pointers.
	if (f_log) {
		fclose(f_log);
	}

	/// Close the Kalman filter.
	if (bKF > 0) {
		close_kalman();
	}

	printf("<OK>\n\n");
} /* end planner_exit() */


/******************************************************************************
 *
 * Title:       int main(int argc, char *argv[])
 *
 * Description: Initialize data. Open ports. Run main program loop.
 *
 * Input:       argc: Number of command line arguments.
 *              argv: Array of command line arguments.
 *
 * Output:      None.
 *
 *****************************************************************************/

int main(int argc, char *argv[])
{
	/// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
	void(*exit_ptr)(void);
	exit_ptr = planner_exit;
	atexit(exit_ptr);

	struct sigaction sigint_action;
	sigint_action.sa_handler = planner_sigint;
	sigint_action.sa_flags = 0;
	sigaction(SIGINT, &sigint_action, NULL);

	/// Declare variables.
	int recv_bytes = 0;
	char recv_buf[MAX_MSG_SIZE];
	char vision_buf[MAX_MSG_SIZE];
	char lj_buf[MAX_MSG_SIZE];
	char nav_buf[MAX_MSG_SIZE];
	CONF_VARS cf;
	MSG_DATA msg;
	PID pid;
	LABJACK_DATA lj;
	int old_task = 0;
	int old_dropper = 0;
	int old_subtask = 0;
	int task = TASK_NONE;
	int subtask = SUBTASK_CONTINUING;
	int status = TASK_CONTINUING;
	int ks_closed = FALSE;
	int buoy_touched = FALSE;
	int buoy_success = FALSE;
	CvPoint3D32f loc;

	/// Declare timers.
	TIMING timer_vision;
	TIMING timer_plan;
	TIMING timer_task;
	TIMING timer_log;
	TIMING timer_kalman;
	TIMING timer_subtask;

	/// Declare timestamp variables.
	struct timeval ctime;
    struct tm ct;
    char write_time[80] = {0};

	printf("MAIN: Starting Planner ... \n");

	/// Initialize variables.
	server_fd = -1;
	vision_fd = -1;
	lj_fd = -1;
	nav_fd = -1;
	bKF = -1;

	memset(&msg, 0, sizeof(MSG_DATA));
	memset(&pid, 0, sizeof(PID));
	memset(&lj,  0, sizeof(LABJACK_DATA));
	messages_init(&msg);

	/// Parse command line arguments.
	parse_default_config(&cf);
	parse_cla(argc, argv, &cf, STINGRAY, (const char *)PLANNER_FILENAME);

	/// Set up default values for the targets, gains and tasks.
    msg.target.data.pitch   	 = cf.target_pitch;
    msg.target.data.roll    	 = cf.target_roll;
    msg.target.data.yaw     	 = cf.target_yaw;
    msg.target.data.yaw_previous = TASK_YAW_PREVIOUS_NOT_SET;
    msg.target.data.yaw_detected = TASK_YAW_DETECTED_NOT_SET;
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

    /// Initialize tasks.
    task = cf.task_start;
	old_task = task;
    subtask = cf.subtask_start;
	old_subtask = subtask;

	msg.task.data.task = task;
	msg.task.data.subtask = subtask;
	msg.task.data.course = cf.course_start;

    /// Set up Kalman filter.
    bKF = init_kalman();
    if (bKF > 0) {
		printf("MAIN: Kalman filter setup OK.\n");
		//kalman_print_test();
	}
	else {
		printf("MAIN: WARNING!!! Kalman filter setup failed.\n");
	}

	/// Set up server.
	if (cf.enable_server) {
		server_fd = net_server_setup(cf.server_port);
		if (server_fd > 0) {
			printf("MAIN: Server setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Server setup failed.\n");
		}
	}

	/// Set up the vision network client.
	if (cf.enable_vision) {
		vision_fd = net_client_setup(cf.vision_IP, cf.vision_port);
		if (vision_fd > 0) {
			printf("MAIN: Vision client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Vision client setup failed.\n");
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
    	f_log = fopen("log.csv", "w");
    	if (f_log) {
   		fprintf(f_log, "time,pitch,roll,yaw,depth,accel1,accel2,accel3,ang1,"
				"ang2,ang3,fx,fy,tpitch,troll,tyaw,tdepth,tfx,tfy\n");
		}
	}

	/// Initialize timers.
	timing_set_timer(&timer_vision);
	timing_set_timer(&timer_plan);
	timing_set_timer(&timer_task);
	timing_set_timer(&timer_subtask);
	timing_set_timer(&timer_log);
	timing_set_timer(&timer_kalman);

	printf("MAIN: Planner running now.\n");
	printf("\n");

	/// Main loop.
	while (1) {
		/// Get network data.
		if ((cf.enable_server) && (server_fd > 0)) {
			recv_bytes = net_server(server_fd, recv_buf, &msg, MODE_PLANNER);
			if (recv_bytes > 0) {
				recv_buf[recv_bytes] = '\0';
				messages_decode(server_fd, recv_buf, &msg, recv_bytes);
			}
		}

		/// Get vision data.
		if ((cf.enable_vision) && (vision_fd > 0)) {
			if (timing_check_period(&timer_vision, cf.period_vision)) {
				recv_bytes = net_client(vision_fd, vision_buf, &msg, MODE_OPEN);
				vision_buf[recv_bytes] = '\0';
				if (recv_bytes > 0) {
					messages_decode(vision_fd, vision_buf, &msg, recv_bytes);
                    timing_set_timer(&timer_vision);
					msg.status.data.fps = msg.vision.data.fps;
				}
			}
		}

		/// If there is a new task then send to vision.
		task = msg.task.data.task;
		subtask = msg.task.data.subtask;
		if (old_task != msg.task.data.task) {
			if ((cf.enable_vision) && (vision_fd > 0)) {
				messages_send(vision_fd, TASK_MSGID, &msg);
			}
			old_task = msg.task.data.task;
			/// Reset the task and subtask start timers.
			timing_set_timer(&timer_task);
			timing_set_timer(&timer_subtask);
		}
		else if (old_subtask != msg.task.data.subtask) {
			if ((cf.enable_vision) && (vision_fd > 0)) {
				messages_send(vision_fd, TASK_MSGID, &msg);
			}
			old_subtask = msg.task.data.subtask;
			timing_set_timer(&timer_subtask);
		}

        /// Get nav data.
		if (nav_fd > 0) {
			msg.target.data.task = msg.task.data.task;
			msg.target.data.vision_status = msg.vision.data.status;
			recv_bytes = net_client(nav_fd, nav_buf, &msg, MODE_PLANNER);
			nav_buf[recv_bytes] = '\0';
			if (recv_bytes > 0) {
				messages_decode(nav_fd, nav_buf, &msg, recv_bytes);
			}
			/// Check to send dropper servo value to nav here. Use old_dropper variable to see if new value needs to be sent.
			if (msg.client.data.dropper != old_dropper) {
				messages_send(nav_fd, CLIENT_MSGID, &msg);
				old_dropper = msg.client.data.dropper;
			}
			/// Check the kill switch state.
			if (!ks_closed) {
				if (msg.lj.data.battery1 > 5) {
					ks_closed = TRUE;
					timing_set_timer(&timer_task);
					if (msg.task.data.course) {
						task = TASK_GATE;
						msg.task.data.task = task;
						timing_set_timer(&timer_subtask);
					}
					cf.task_init_yaw = msg.status.data.yaw;
					cf.target_yaw = cf.task_init_yaw;
					msg.target.data.yaw = cf.task_init_yaw;
					printf("MAIN: task = %d     %lf\n", task, cf.task_init_yaw);
				}
				else {
					ks_closed = FALSE;
				}
			}
			if (ks_closed) {
				if (msg.lj.data.battery1 < 5) {
					ks_closed = FALSE;
					//task = TASK_NONE;
					//msg.task.data.task = task;
				}
			}
		}

		/// Update Kalman filter.
		if (bKF) {
			/// If it has been long enough, update the filter.
			if (timing_check_period(&timer_kalman, 0.1)) {
				STAT cs = msg.status.data;
				float ang[] = { cs.pitch, cs.roll, cs.yaw };
				float real_accel[] = { cs.accel[0], cs.accel[1], cs.accel[2] - 9.86326398 };

				/// Update the Kalman filter.
				kalman_update(timing_check_period(&timer_kalman, 0.), msg.lj.data.pressure, ang, real_accel, cs.ang_rate);
				timing_set_timer(&timer_kalman);
			}

			/// Get current location estimation.
			kalman_get_location(loc);
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

        /// Log if flag is set.
        if (cf.enable_log && f_log) {
			/// Get a timestamp and use for log.
            gettimeofday(&ctime, NULL);
            ct = *(localtime ((const time_t*) &ctime.tv_sec));
			strftime(write_time, sizeof(write_time), "20%y-%m-%d_%H:%M:%S", &ct);
            snprintf(write_time + strlen(write_time),
            		strlen(write_time), ".%06ld", ctime.tv_usec);

			/// Log enable_log times every second.
			if (timing_check_period(&timer_log, cf.enable_log)) {
				STAT cs = msg.status.data;
				TARGET target = msg.target.data;
				fprintf(f_log, "%s, %.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,"
					"%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,"
					"%.06f,%.06f\n",
					write_time, cs.pitch, cs.roll, cs.yaw, msg.lj.data.pressure,
					cs.accel[0], cs.accel[1], cs.accel[2],
					cs.ang_rate[0], cs.ang_rate[1], cs.ang_rate[2], cs.fx, cs.fy,
					target.pitch, target.roll, target.yaw, target.depth,
					target.fx, target.fy);
				timing_set_timer(&timer_log);
			}
		}

		/// Update the task and subtask elapsed times.
		timing_get_dt(&timer_task, &timer_task);
		timing_get_dt(&timer_subtask, &timer_subtask);

		/// Run the current task.
		status = task_run(&msg, &cf, timer_task.s, timer_subtask.s);
		if (msg.task.data.course) {
			/// Set the subtask in the network message.
			msg.task.data.subtask = subtask;
			if (status == TASK_SUCCESS || status == TASK_FAILURE) {
				if (task == TASK_BUOY && buoy_success) {
					if (!buoy_touched) {
						buoy_touched = TRUE;
					}
					else if (buoy_touched && timer_subtask.s < TASK_BUOY_WAIT_TIME) {
						/// Do nothing here.
					}
					else {
						msg.task.data.task++;
						task++;
						printf("MAIN: task = %d\n", task);
						msg.task.data.subtask = SUBTASK_SEARCH_DEPTH;
						/// Re-initialize the task and subtask timers.
						timing_set_timer(&timer_task);
						timing_set_timer(&timer_subtask);
					}
				}
				else if (task == TASK_BUOY && status == TASK_SUCCESS) {
					buoy_success = TRUE;
					timing_set_timer(&timer_subtask);
				}
				else {
					/// Move on to the next task. Initialize the subtask.
					msg.task.data.task++;
					task++;
					printf("MAIN: task = %d\n", task);
					msg.task.data.subtask = SUBTASK_SEARCH_DEPTH;
					/// Re-initialize the task and subtask timers.
					timing_set_timer(&timer_task);
					timing_set_timer(&timer_subtask);
				}
			}
			else if (task == TASK_BUOY && buoy_success) {
				if (timer_subtask.s < TASK_BUOY_WAIT_TIME) {
					/// Do nothing here.
				}
				else {
					msg.task.data.task++;
					task++;
					printf("MAIN: task = %d\n", task);
					msg.task.data.subtask = SUBTASK_SEARCH_DEPTH;
					/// Re-initialize the task and subtask timers.
					timing_set_timer(&timer_task);
					timing_set_timer(&timer_subtask);
				}
			}
		}
		else {
			/// If we are not in course mode and get a status of something other than task continue, reset the timers.
			if (status == TASK_SUCCESS || status == TASK_FAILURE) {
				timing_set_timer(&timer_task);
				timing_set_timer(&timer_subtask);
			}
		}
	}

	exit(0);
} /* end main() */
