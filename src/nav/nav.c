/******************************************************************************
 *
 *  Title:        nav.c
 *
 *  Description:  Main program for Stingray UUV.
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

#include "nav.h"
#include "microstrain.h"
#include "network.h"
#include "parser.h"
#include "labjack.h"
#include "pololu.h"
#include "util.h"
#include "serial.h"
#include "messages.h"
#include "pid.h"
#include "labjackd.h"

#ifdef USE_SSA
#include <sys/timeb.h>
#include "waypoints.h"
#define NEED_TYPENAMES
#include "platform_types.h"
#undef NEED_TYPENAMES
#define NEED_MODENAMES
#include "platform_modes.h"
#undef NEED_MODENAMES
#include "status.h"
#include "telemfile.h"
#endif /* USE_SSA */

#define YAW_HIST_SIZE 30

/* Global file descriptors. Only global so that nav_exit() can close them. */
int server_fd;
int pololu_fd;
int imu_fd;
int lj_fd;
float yaw_hist[YAW_HIST_SIZE];
int yaw_head = 0;


#ifdef USE_SSA
/******************************************************************************
 *
 * Title:       void ssa_update_telemetry( MSG_DATA msg )
 *
 * Description: Uploads data to the SSA dataserver
 *
 *****************************************************************************/

static void ssa_update_telemetry(
    MSG_DATA *msg )
{
    STATUS s;
    time_t t;
    struct tm *tmp;
    s = status_InitStatus();

    //**********
    // engineering data time
    //**********
    t = time( NULL );
    tmp = localtime( &t );
    s.nTime = t;
    status_SetTimeString( s.sTime, sizeof( s.sTime ), tmp );

    //**********
    // platform identification
    //**********
    s.nPlatformID = 15;
    snprintf( s.sPlatformName, sizeof( s.sPlatformName )
              , "UCSD Stingray" );
    s.nPlatformType = PLATFORMTYPE_UUV;
    snprintf( s.sPlatformTypeName, sizeof( s.sPlatformTypeName )
              , platformtypenames[s.nPlatformType] );

    //**********
    // engineering telemetry
    //**********
    if( msg->stop.data.state == TRUE ) {
        s.nPlatformMode = MODE_PAUSE;
    }
    else {
        s.nPlatformMode = MODE_TRANSIT;
    }

    snprintf( s.sPlatformModeName, sizeof( s.sPlatformModeName ), "%s"
              , modenames[s.nPlatformMode] );
    s.nCommsLatency = 0;

    //settime(&s);
    s.dHdgDeg = msg->mstrain.data.yaw;
    s.dPitchDeg = msg->mstrain.data.pitch;
    s.dRollDeg = msg->mstrain.data.roll;
    s.dBattVolts = msg->status.data.battery2;
    s.dAltMeters = -msg->target.data.depth;
    telemfile_WriteFileDirect( "ssatelemetry.txt", s );
    telemfile_WriteTelemServer( "128.2.181.103", "9015", s );

    return;
} /* end ssa_update_telemetry() */

#endif /* USE_SSA */


/******************************************************************************
 *
 * Title:       void nav_sigint( int signal )
 *
 * Description: This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       signal: The SIGINT signal.
 *
 * Output:      None.
 *
 *****************************************************************************/

void nav_sigint( int signal )
{
    exit( 0 );
} /* end nav_sigint() */


/******************************************************************************
 *
 * Title:       void nav_exit( )
 *
 * Description: Exit function for main program. Sets actuators to safe values
 *              and closes all file descriptors. This function is called when
 *              SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/

void nav_exit( )
{
    printf("\nNAV_EXIT: Shutting down nav program ... ");
    /* Sleep to let things shut down properly. */
    usleep( 200000 );

    /* Close the open file descriptors. */
    if( pololu_fd > 0 ) {
        /* Set all the actuators to safe positions. */
        pololuInitializeChannels( pololu_fd );
        usleep( 200000 );
        close( pololu_fd );
    }
    if( imu_fd > 0 ) {
        close( imu_fd );
    }
    if( server_fd > 0 ) {
        close( server_fd );
    }
	if( lj_fd > 0 ) {
		close( lj_fd );
	}

    printf("<OK>\n\n");
} /* end nav_exit() */


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
    exit_ptr = nav_exit;
    atexit( exit_ptr );

	/* Attach signals to exit function. */
    struct sigaction sigint_action;
    sigint_action.sa_handler = nav_sigint;
    sigint_action.sa_flags = 0;
    sigaction( SIGINT, &sigint_action, NULL );
    sigaction( SIGTERM, &sigint_action, NULL );
    sigaction( SIGQUIT, &sigint_action, NULL );
    sigaction( SIGHUP, &sigint_action, NULL );

	/* Declare variables. */
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
    struct timeval pitch_time = {0, 0};
    struct timeval roll_time = {0, 0};
    struct timeval yaw_time = {0, 0};
    struct timeval depth_time = {0, 0};
	struct timeval pololu_time = {0, 0};
    struct timeval pitch_start = {0, 0};
    struct timeval roll_start = {0, 0};
    struct timeval yaw_start = {0, 0};
    struct timeval depth_start = {0, 0};
	struct timeval pololu_start = {0, 0};
    int time1s = 0;
    int time1ms = 0;
    int time2s = 0;
    int time2ms = 0;
    int dt = 0;
	short int accel_gain = 0;
	short int mag_gain = 0;
	short int bias_gain = 0;

    printf("MAIN: Starting Navigation ... \n");

    /* Initialize variables. */
    server_fd = -1;
    pololu_fd = -1;
    imu_fd = -1;
	lj_fd = 0;
    memset( &msg, 0, sizeof( MSG_DATA ) );
    memset( &pid, 0, sizeof( PID ) );
    memset( &recv_buf, 0, MAX_MSG_SIZE );
	messages_init( &msg );
	memset( &yaw_hist, 0, sizeof( yaw_hist ) );

    /* Parse command line arguments. */
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, STINGRAY, ( const char * )STINGRAY_FILENAME );

    /* Initialize the PID controllers with configuration file values. */
    status = pid_init( &pid, &cf );

    /* Set up server. */
    if( cf.enable_server ) {
        server_fd = net_server_setup( cf.server_port );
		if( server_fd > 0 ) {
			printf("MAIN: Nav server setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Nav server setup failed.\n");
		}
    }

    /* Set up the Microstrain IMU. */
    if( cf.enable_imu ) {
        imu_fd = mstrain_setup( cf.imu_port , cf.imu_baud );
		status = mstrain_serial_number( imu_fd, &mstrain_serial );
		if( mstrain_serial == MSTRAIN_SERIAL ) {
			printf("MAIN: IMU setup OK.\n");
		}
		else {
			printf("MAIN: SIMULATION MODE!!! IMU data is simulated.\n");
			/* Seed the random variable. */
			srand((unsigned int) time(NULL) );
			imu_fd = 0;
		}
		if( imu_fd > 0 ) {
			/* Zero the mag gains. */
			//status = mstrain_zero_mag_gain( imu_fd );
		}
    }
	else {
		printf("MAIN: SIMULATION MODE!!! IMU data is simulated.\n");
		/* Seed the random variable. */
		srand((unsigned int) time(NULL) );
	}

    /* Set up the Pololu servo controller. */
    if( cf.enable_pololu ) {
        pololu_fd = pololuSetup( cf.pololu_port, cf.pololu_baud );
		if( pololu_fd > 0 ) {
			/* Initialize the pololu. */
			pololuInitializeChannels( pololu_fd );
			printf("MAIN: Pololu setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Pololu setup failed.\n");
		}
    }

	/* Connect to the labjack daemon. */
	if( (cf.enable_labjack > 0) && (cf.enable_pololu > 0) ) {
        lj_fd = net_client_setup( cf.labjackd_IP, cf.labjackd_port );
		if( lj_fd > 0 ) {
			printf("MAIN: Labjack client setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Labjack client setup failed.\n");
		}
	}

    /* Initialize timers. */
    gettimeofday( &pitch_time, NULL );
    gettimeofday( &pitch_start, NULL );
    gettimeofday( &roll_time, NULL );
    gettimeofday( &roll_start, NULL );
    gettimeofday( &yaw_time, NULL );
    gettimeofday( &yaw_start, NULL );
    gettimeofday( &depth_time, NULL );
    gettimeofday( &depth_start, NULL );
    gettimeofday( &pololu_time, NULL );
    gettimeofday( &pololu_start, NULL );

	printf("MAIN: Nav running now.\n");

    /* Main loop. */
    while( 1 ) {
		/* Check labjack data to see if kill switch has been closed. */
		if( (cf.enable_pololu > 0) && (cf.enable_labjack > 0) && (lj_fd > 0) ) {
			recv_bytes = net_client( lj_fd, lj_buf, &msg, mode );
			lj_buf[recv_bytes] = '\0';
			if( recv_bytes > 0 ) {
				messages_decode( lj_fd, lj_buf, &msg, recv_bytes );
				msg.status.data.depth = msg.lj.data.pressure;
			}
			if( pololu_initialized == FALSE ) {
				/* Get the state of the kill switch. */
				if( msg.lj.data.battery1 > BATT1_THRESH ) {
					if( pololu_starting == FALSE ) {
						pololuInitializeChannels( pololu_fd );
						pololu_starting = TRUE;
						/* Start the timer. */
						gettimeofday( &pololu_start, NULL );
					}
					/* Check that 7 seconds have elapsed since initializing Pololu. */
		            time1s =    pololu_time.tv_sec;
					time1ms =   pololu_time.tv_usec;
					time2s =    pololu_start.tv_sec;
					time2ms =   pololu_start.tv_usec;
					dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
					if( dt > POLOLU_INIT_TIME ) {
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
				/* Get the state of the kill switch. */
				if( msg.lj.data.battery1 > BATT1_THRESH ) {
					pololu_initialized = TRUE;
				}
				else {
					pololu_initialized = FALSE;
				}
			}
		}

        /* Get network data. */
        if( (cf.enable_server) && (server_fd > 0) ) {
            recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_PLANNER );
            if( recv_bytes > 0 ) {
                recv_buf[recv_bytes] = '\0';
                messages_decode( server_fd, recv_buf, &msg, recv_bytes );
				/* Make sure that the servo and speed commands are zero if
				 * Pololu is not initialized. */
				if( pololu_initialized == FALSE ) {
					msg.target.data.fx = 0;
					msg.target.data.fy = 0;
					msg.target.data.speed = 0;
				}
            }
        }

        /* Send dropper servo command. Check that Pololu is initialized. */
        //if( (pololu_fd > 0) && (pololu_initialized) ) {
            //status = pololuSetPosition7Bit( pololu_fd, POLOLU_DROPPER_SERVO, msg.client.data.dropper );
            //printf("MAIN: dropper = %d\n", msg.client.data.dropper);
        //}

        /* Check for assisted teleop commands. */
        //if( msg.target.data.mode == MANUAL ) {
            //msg.target.data.pitch   += msg.teleop.data.pitch;
            //msg.target.data.roll    += msg.teleop.data.roll;
            //msg.target.data.yaw     += msg.teleop.data.yaw;
            //msg.target.data.depth   += msg.teleop.data.depth;
            //msg.target.data.fx      += msg.teleop.data.fx;
            //msg.target.data.fy      += msg.teleop.data.fy;
            //msg.target.data.speed   += msg.teleop.data.speed;

            ///* Reset the target change values back to zero. */
            //msg.teleop.data.pitch = 0;
            //msg.teleop.data.roll = 0;
            //msg.teleop.data.yaw = 0;
            //msg.teleop.data.depth = 0;
            //msg.teleop.data.fx = 0;
            //msg.teleop.data.fy = 0;
            //msg.teleop.data.speed = 0;
        //}

         /* Get Microstrain data. */
        if( (cf.enable_imu) && (imu_fd > 0) ) {
            recv_bytes = mstrain_euler_angles( imu_fd, &msg.mstrain.data.pitch,
				&msg.mstrain.data.roll, &msg.mstrain.data.yaw );
				//&msg.mstrain.data.roll, &msg.mstrain.data.yaw, msg.mstrain.data.accel,
				//msg.mstrain.data.ang_rate );

			/* Calculate yaw average from history. */
			//msg.mstrain.data.yaw = calc_yaw_avg( msg.mstrain.data.yaw );

			recv_bytes = mstrain_vectors( imu_fd, 0, msg.mstrain.data.mag,
				msg.mstrain.data.accel, msg.mstrain.data.ang_rate );
        }
		else {
			/* Simulation Mode. This is where the simulated data is generated. */
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

        /* Perform PID loops. */
        if( msg.stop.data.state == FALSE ) {
            /* Pitch. */
            time1s =    pitch_time.tv_sec;
            time1ms =   pitch_time.tv_usec;
            time2s =    pitch_start.tv_sec;
            time2ms =   pitch_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if( dt > pid.pitch.period ) {
                pid_loop( pololu_fd, &pid, &cf, &msg, dt, PID_PITCH, pololu_initialized );
                msg.status.data.pitch_period = dt;
                gettimeofday( &pitch_start, NULL );
            }

            /* Roll. */
            time1s =    roll_time.tv_sec;
            time1ms =   roll_time.tv_usec;
            time2s =    roll_start.tv_sec;
            time2ms =   roll_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if( dt > pid.roll.period ) {
                pid_loop( pololu_fd, &pid, &cf, &msg, dt, PID_ROLL, pololu_initialized );
                msg.status.data.roll_period = dt;
                gettimeofday( &roll_start, NULL );
            }

            /* Yaw. */
            time1s =    yaw_time.tv_sec;
            time1ms =   yaw_time.tv_usec;
            time2s =    yaw_start.tv_sec;
            time2ms =   yaw_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if( dt > pid.yaw.period ) {
                pid_loop( pololu_fd, &pid, &cf, &msg, dt, PID_YAW, pololu_initialized );
				msg.status.data.yaw_period = dt;
				gettimeofday( &yaw_start, NULL );
            }

            /* Depth. */
            time1s =    depth_time.tv_sec;
            time1ms =   depth_time.tv_usec;
            time2s =    depth_start.tv_sec;
            time2ms =   depth_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if( dt > pid.depth.period ) {
                pid_loop( pololu_fd, &pid, &cf, &msg, dt, PID_DEPTH, pololu_initialized );
                msg.status.data.depth_period = dt;
                gettimeofday( &depth_start, NULL );
            }
        }

        /* Update status message. */
        messages_update( &msg );

        /* Update and send the SSA data. */
        #ifdef USE_SSA
        ssa_update_telemetry( &msg );
        usleep( SSA_SLEEP );
        #endif /* USE_SSA */

        /* Update timers. */
        gettimeofday( &pitch_time, NULL );
        gettimeofday( &roll_time, NULL );
        gettimeofday( &yaw_time, NULL );
        gettimeofday( &depth_time, NULL );
        gettimeofday( &pololu_time, NULL );
    }

    exit( 0 );
} /* end main() */

float calc_yaw_avg( float curr_yaw )
{
	int i = 0;
	float yaw_total = 0;
	float yaw_avg = 0;

	/* Update yaw history. */
	yaw_hist[yaw_head] = curr_yaw;
	yaw_head++;
	if ( yaw_head > YAW_HIST_SIZE )
	{
		yaw_head = 0;
	}

	/* Calculate yaw average. */
	for ( i = 0; i < YAW_HIST_SIZE; i++ )
	{
		yaw_total += yaw_hist[i];
	}

	yaw_avg = yaw_total / YAW_HIST_SIZE;

	printf( "yaw_avg = %f\n", yaw_avg );

	return yaw_avg;
}
