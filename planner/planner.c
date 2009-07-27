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
#include <cv.h>

#include "planner.h"
#include "microstrain.h"
#include "network.h"
#include "parser.h"
#include "labjack.h"
#include "pololu.h"
#include "kalman.h"
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
int bKF;
FILE *f_log;


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
	if( server_fd > 0 ) {
		close( server_fd );
	}
	if( vision_fd > 0 ) {
		close( vision_fd );
	}
	if( lj_fd > 0 ) {
		close( lj_fd );
	}
	if( nav_fd > 0 ) {
		close( nav_fd );
	}

	/* Close the open file pointers. */
	if( f_log ) {
		fclose( f_log );
	}

	/* Close the Kalman filter. */
	if( bKF > 0 ) {
		close_kalman();
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

	/* Declare variables. */
	int recv_bytes = 0;
	char recv_buf[MAX_MSG_SIZE];
	char vision_buf[MAX_MSG_SIZE];
	char lj_buf[MAX_MSG_SIZE];
	char nav_buf[MAX_MSG_SIZE];
	CONF_VARS cf;
	MSG_DATA msg;
	PID pid;
	LABJACK_DATA lj;
	//float accel_count = 0.0;
	//float accel_inc = 0.1;
	int old_task = 0;
	int old_dropper = 0;
	int old_subtask = 0;
	CvPoint3D32f loc;
	int task = TASK_NONE;
	int subtask = SUBTASK_CONTINUING;
	int status = TASK_CONTINUING;

	struct timeval vision_time = {0, 0};
	struct timeval vision_start = {0, 0};
	struct timeval plan_time = {0, 0};
	struct timeval plan_start = {0, 0};
	struct timeval task_time = {0, 0};
	struct timeval task_start = {0, 0};
	struct timeval log_time = {0, 0};
	struct timeval log_start = {0, 0};
	struct timeval kalman_time = {0, 0};
	struct timeval kalman_start = {0,0};
	struct timeval subtask_time = {0, 0};
	struct timeval subtask_start = {0, 0};
	int time1s = 0;
	int time1ms = 0;
	int time2s = 0;
	int time2ms = 0;
	int task_dt = 0;
	int subtask_dt = 0;
	int dt = 0;

	/* Declare timestamp variables. */
	struct timeval ctime;
    struct tm ct;
    char write_time[80] = {0};

	printf("MAIN: Starting Planner ... \n");

	/* Initialize variables. */
	server_fd = -1;
	vision_fd = -1;
	lj_fd = -1;
	nav_fd = -1;
	bKF = -1;

	memset( &msg, 0, sizeof(MSG_DATA) );
	memset( &pid, 0, sizeof(PID) );
	memset( &lj,  0, sizeof(LABJACK_DATA) );
	messages_init( &msg );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, ( const char * )PLANNER_FILENAME );

	/* Set up default values for the targets, gains and tasks. */
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

    /* Initialize tasks */
    /* Never used because of lines ~332  -- FIX ME */
    task = cf.task_start;
    subtask = cf.subtask_start;

	msg.task.data.task = task;
	msg.task.data.subtask = subtask;
	msg.task.data.course = TASK_COURSE_OFF;

    /* Set up Kalman filter. */
    bKF = init_kalman();
    if( bKF > 0 ) {
		printf("MAIN: Kalman filter setup OK.\n");
		//kalman_print_test();
	}
	else {
		printf("MAIN: WARNING!!! Kalman filter setup failed.\n");
	}

	/* Set up server. */
	if( cf.enable_server ) {
		server_fd = net_server_setup( cf.server_port );
		if( server_fd > 0 ) {
			printf("MAIN: Server setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Server setup failed.\n");
		}
	}

	/* Set up the vision network client. */
	if( cf.enable_vision ) {
		vision_fd = net_client_setup( cf.vision_IP, cf.vision_port );
		if( vision_fd > 0 ) {
			printf("MAIN: Vision client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Vision client setup failed.\n");
		}
	}

	/* Connect to the labjack daemon. */
    if( cf.enable_labjack ) {
        lj_fd = net_client_setup( cf.labjackd_IP, cf.labjackd_port );
		if( lj_fd > 0 ) {
			printf("MAIN: Labjack client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Labjack client setup failed.\n");
		}
    }

    /* Set up the nav network client. */
	if( cf.enable_nav ) {
        nav_fd = net_client_setup( cf.nav_IP, cf.nav_port );
		if( nav_fd > 0 ) {
			printf("MAIN: Nav client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Nav client setup failed.\n");
		}
    }

    /* Open log file if flag set. */
    if( cf.enable_log ) {
    	f_log = fopen( "log.csv", "w" );
    	if( f_log ) {
   		fprintf( f_log, "time,pitch,roll,yaw,depth,accel1,accel2,accel3,ang1,"
				"ang2,ang3,fx,fy,tpitch,troll,tyaw,tdepth,tfx,tfy\n" );
		}
	}

	/* Initialize timers. */
	gettimeofday( &vision_time, NULL );
	gettimeofday( &vision_start, NULL );
	gettimeofday( &plan_time, NULL );
	gettimeofday( &plan_start, NULL );
	gettimeofday( &task_time, NULL );
	gettimeofday( &task_start, NULL );
	gettimeofday( &subtask_time, NULL );
	gettimeofday( &subtask_start, NULL );
	gettimeofday( &log_time, NULL );
	gettimeofday( &log_start, NULL );
	gettimeofday( &kalman_time, NULL );
	gettimeofday( &kalman_start, NULL );

	printf("MAIN: Planner running now.\n");
	printf( "\n" );

	/* Main loop. */
	while( 1 ) {
		/* Get network data. */
		if( (cf.enable_server) && (server_fd > 0) ) {
			recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_PLANNER );
			if( recv_bytes > 0 ) {
				recv_buf[recv_bytes] = '\0';
				messages_decode( server_fd, recv_buf, &msg, recv_bytes );
			}
		}

		/* Get vision data. */
		if( (cf.enable_vision) && (vision_fd > 0) ) {
			time1s =    vision_time.tv_sec;
			time1ms =   vision_time.tv_usec;
			time2s =    vision_start.tv_sec;
			time2ms =   vision_start.tv_usec;
			dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );

			if( dt > cf.period_vision ) {
				recv_bytes = net_client( vision_fd, vision_buf, &msg, MODE_OPEN );
				vision_buf[recv_bytes] = '\0';
				if( recv_bytes > 0 ) {
					messages_decode( vision_fd, vision_buf, &msg, recv_bytes );
                    gettimeofday( &vision_start, NULL );
					msg.status.data.fps = msg.vision.data.fps;
				}
			}
		}

		/* If there is a new task then send to vision. */
		/* This will break the algorithm below */
		task = msg.task.data.task;
		subtask = msg.task.data.subtask;
		if( old_task != msg.task.data.task ) {
			if( (cf.enable_vision) && (vision_fd > 0) ) {
				messages_send( vision_fd, TASK_MSGID, &msg );
			}
			old_task = msg.task.data.task;
			/* Reset the task and subtask start timers. */
			gettimeofday( &task_start, NULL );
			gettimeofday( &subtask_start, NULL );
		}
		else if( old_subtask != msg.task.data.subtask ) {
			if( (cf.enable_vision) && (vision_fd > 0) ) {
				messages_send( vision_fd, TASK_MSGID, &msg );
			}
			old_subtask = msg.task.data.subtask;
		}

        /* Get nav data. */
		if( nav_fd > 0 ) {
			recv_bytes = net_client( nav_fd, nav_buf, &msg, MODE_PLANNER );
			nav_buf[recv_bytes] = '\0';
			if( recv_bytes > 0 ) {
				messages_decode( nav_fd, nav_buf, &msg, recv_bytes );
			}
			/* Check to send dropper servo value to nav here. Use old_dropper
			 * variable to see if new value needs to be sent. */
			if( msg.client.data.dropper != old_dropper ) {
				messages_send( nav_fd, CLIENT_MSGID, &msg );
				old_dropper = msg.client.data.dropper;
			}
		}

		/* Update Kalman filter. */
		if( bKF ) {
			time1s =    kalman_time.tv_sec;
			time1ms =   kalman_time.tv_usec;
			time2s =    kalman_start.tv_sec;
			time2ms =   kalman_start.tv_usec;
			dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );

			/* If it has been long enough, update the filter. */
			if( dt > 0.1 * 1000000 ) {
				STAT cs = msg.status.data;
				float ang[] = { cs.pitch, cs.roll, cs.yaw };
				float real_accel[] = { cs.accel[0], cs.accel[1], cs.accel[2] - 9.86326398 };

				/* Update the Kalman filter. */
				kalman_update( ((float)dt)/1000000, msg.lj.data.pressure, ang,
						real_accel, cs.ang_rate );
				gettimeofday( &kalman_start, NULL );
			}

			/* Get current location estimation. */
			kalman_get_location( loc );
		}

		/* Get labjack data. */
        if( (cf.enable_labjack) && (lj_fd > 0) ) {
            recv_bytes = net_client( lj_fd, lj_buf, &msg, MODE_OPEN );
            lj_buf[recv_bytes] = '\0';
            if( recv_bytes > 0 ) {
                messages_decode( lj_fd, lj_buf, &msg, recv_bytes );
				msg.status.data.depth = msg.lj.data.pressure;
            }
        }

        /* Log if flag is set. */
        if( cf.enable_log && f_log ) {
        	time1s =    log_time.tv_sec;
			time1ms =   log_time.tv_usec;
			time2s =    log_start.tv_sec;
			time2ms =   log_start.tv_usec;
			dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );

			/* Get a timestamp and use for log. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
			strftime( write_time, sizeof(write_time), "20%y-%m-%d_%H:%M:%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%06ld", ctime.tv_usec );

			/* Log every (enable_log) seconds. */
			if( dt > (cf.enable_log * 100000) ) {
				STAT cs = msg.status.data;
				TARGET target = msg.target.data;
				fprintf( f_log, "%s, %.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,"
					"%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,%.06f,"
					"%.06f,%.06f\n",
					write_time, cs.pitch, cs.roll, cs.yaw, msg.lj.data.pressure,
					cs.accel[0], cs.accel[1], cs.accel[2],
					cs.ang_rate[0], cs.ang_rate[1], cs.ang_rate[2], cs.fx, cs.fy,
					target.pitch, target.roll, target.yaw, target.depth,
					target.fx, target.fy );
				gettimeofday( &log_start, NULL );
			}
		}

		/* Update the task dt. dt value is in seconds */
		time1s =    task_time.tv_sec;
		time1ms =   task_time.tv_usec;
		time2s =    task_start.tv_sec;
		time2ms =   task_start.tv_usec;
		task_dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms ) / 1000000;

		/* Update the subtask dt. dt value is in seconds */
		time1s =    subtask_time.tv_sec;
		time1ms =   subtask_time.tv_usec;
		time2s =    subtask_start.tv_sec;
		time2ms =   subtask_start.tv_usec;
		subtask_dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms ) / 1000000;

		/* Run the current task. */
		status = task_run( &msg, &cf, task_dt, subtask_dt );
		if( msg.task.data.course == TASK_COURSE_ON ) {
			/* Set the subtask in the network message. */
			msg.task.data.subtask = subtask;
			if( status == TASK_SUCCESS || status == TASK_FAILURE ) {
				/* Move on to the next task. Initialize the subtask. */
				msg.task.data.task++;
				msg.task.data.subtask = SUBTASK_SEARCH_DEPTH;
				/* Re-initialize the task and subtask timers. */
				gettimeofday( &task_start, NULL );
				gettimeofday( &subtask_start, NULL );
			}
			else if( status == SUBTASK_SUCCESS || status == SUBTASK_FAILURE ) {
				/* Move on to the next subtask. */
				msg.task.data.subtask++;
				/* Re-initialize the subtask timer. */
				gettimeofday( &subtask_start, NULL);
			}
		}

		/* Update timers. */
		gettimeofday( &vision_time, NULL );
		gettimeofday( &plan_time, NULL );
		gettimeofday( &task_time, NULL );
		gettimeofday( &subtask_time, NULL );
		gettimeofday( &log_time, NULL );
		gettimeofday( &kalman_time, NULL );
	}

	exit( 0 );
} /* end main() */
