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


/* Global file descriptors. Only global so that nav_exit() can close them. */
int server_fd;
int pololu_fd;
int lj_fd;
int imu_fd;
int vision_fd;
int planner_fd;


#ifdef USE_SSA
/******************************************************************************
 *
 * Title:       void ssa_update_telemetry( MSG_DATA msg )
 *
 * Description: Uploads data to the SSA dataserver
 *
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
    if ( msg->stop.data.state == TRUE ) {
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
 * Globals:     None.
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
 * Globals:     File descriptors: server_fd, pololu_fd, labjack_fd, imu_fd.
 *
 *****************************************************************************/

void nav_exit( )
{
    printf( "\nNAV_EXIT: Shutting down nav program ... " );
    /* Sleep to let things shut down properly. */
    usleep( 200000 );

    /* Close the open file descriptors. */
    if ( pololu_fd > 0 ) {
        /* Set all the actuators to safe positions. */
        pololuInitializeChannels( pololu_fd );
        usleep( 200000 );
        close( pololu_fd );
    }
    if ( imu_fd > 0 ) {
        close( imu_fd );
    }
    if ( lj_fd > 0 ) {
        close( lj_fd );
    }
    if ( server_fd > 0 ) {
        close( server_fd );
    }
    if ( vision_fd > 0 ) {
        close( vision_fd );
    }
    if ( planner_fd > 0 ) {
        close( planner_fd );
    }

    printf( "<OK>\n\n" );
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
 * Globals:     None.
 *
 *****************************************************************************/

int main( int argc, char *argv[] )
{
    /* Setup exit function. It is called when SIGINT (ctrl-c) is invoked. */
    void( *exit_ptr )( void );
    exit_ptr = nav_exit;
    atexit( exit_ptr );

    struct sigaction sigint_action;
    sigint_action.sa_handler = nav_sigint;
    sigint_action.sa_flags = 0;
    sigaction( SIGINT, &sigint_action, NULL );
    sigaction( SIGTERM, &sigint_action, NULL );
    sigaction( SIGQUIT, &sigint_action, NULL );
    sigaction( SIGHUP, &sigint_action, NULL );

    int status = -1;
    int recv_bytes = 0;
    int mode = MODE_STATUS;
    char recv_buf[MAX_MSG_SIZE];
    char vision_buf[MAX_MSG_SIZE];
    char planner_buf[MAX_MSG_SIZE];
    char lj_buf[MAX_MSG_SIZE];
    CONF_VARS cf;
    MSG_DATA msg;
    PID pid;
    LABJACK_DATA lj;

    struct timeval pitch_time = {0, 0};
    struct timeval roll_time = {0, 0};
    struct timeval yaw_time = {0, 0};
    struct timeval depth_time = {0, 0};
    struct timeval vision_time = {0, 0};
    struct timeval planner_time = {0, 0};
    struct timeval pitch_start = {0, 0};
    struct timeval roll_start = {0, 0};
    struct timeval yaw_start = {0, 0};
    struct timeval depth_start = {0, 0};
    struct timeval vision_start = {0, 0};
    struct timeval planner_start = {0, 0};
    int time1s = 0;
    int time1ms = 0;
    int time2s = 0;
    int time2ms = 0;
    int dt = 0;

    printf( "MAIN: Starting Navigation ... \n" );
    printf( "MAIN: Open kill switch and then press key.\n" );
    getchar( );

    /* Initialize variables. */
    server_fd = -1;
    pololu_fd = -1;
    lj_fd = -1;
    imu_fd = -1;
    vision_fd = -1;

    memset( &msg, 0, sizeof( MSG_DATA ) );
    memset( &pid, 0, sizeof( PID ) );
    //memset( &lj, 0, sizeof( LABJACK_DATA ) );
    memset( &recv_buf, 0, MAX_MSG_SIZE );
    memset( &vision_buf, 0, MAX_MSG_SIZE );
    memset( &planner_buf, 0, MAX_MSG_SIZE );
    memset( &lj_buf, 0, MAX_MSG_SIZE );

    /* Parse command line arguments. */
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, STINGRAY, ( const char * )STINGRAY_FILENAME );

    /* Initialize the PID controllers with configuration file values. */
    status = pid_init( &pid, &cf );
    msg.target.data.pitch   = cf.target_pitch;
    msg.target.data.roll    = cf.target_roll;
    msg.target.data.yaw     = cf.target_yaw;
    msg.target.data.depth   = cf.target_depth;
    msg.gain.data.kp_pitch  = cf.kp_pitch;
    msg.gain.data.ki_pitch  = cf.ki_pitch;
    msg.gain.data.kd_pitch  = cf.kd_pitch;
    msg.gain.data.kp_roll   = cf.kp_roll;
    msg.gain.data.ki_roll   = cf.ki_roll;
    msg.gain.data.kd_roll   = cf.kd_roll;
    msg.gain.data.kp_yaw    = cf.kp_yaw;
    msg.gain.data.ki_yaw    = cf.ki_yaw;
    msg.gain.data.kd_yaw    = cf.kd_yaw;
    msg.gain.data.kp_depth  = cf.kp_depth;
    msg.gain.data.ki_depth  = cf.ki_depth;
    msg.gain.data.kd_depth  = cf.kd_depth;

    /* Set up communications. */
    if ( cf.enable_net ) {
        server_fd = net_server_setup( cf.api_port );
    }

    /* Set up the vision network client. */
    if ( cf.enable_vision ) {
        vision_fd = net_client_setup( cf.vision_IP, cf.vision_port );
    }

    /* Set up the planner network client. */
    if ( cf.enable_planner ) {
        planner_fd = net_client_setup( cf.planner_IP, cf.planner_port );
    }

    /* Set up the Microstrain IMU. */
    if ( cf.enable_imu ) {
        imu_fd = mstrain_setup( cf.imu_port , cf.imu_baud );
    }

    /* Set up the labjack. */
    if ( cf.enable_labjack ) {
        lj_fd = init_labjack( );
        lj_fd = 1;
    }
    if ( lj_fd ) {
        status = query_labjack( );
    }

    /* Connect to the labjack daemon. */
    //if ( cf.enable_labjack ) {
        //lj_fd = net_client_setup( cf.labjackd_IP, cf.labjackd_port );
    //}

    /* Set up the Pololu servo controller. */
    if ( cf.enable_pololu ) {
        pololu_fd = pololuSetup( cf.pololu_port, cf.pololu_baud );
        pololuInitializeChannels( pololu_fd );
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
    gettimeofday( &vision_time, NULL );
    gettimeofday( &vision_start, NULL );

    printf( "MAIN: Close the kill switch now.\n" );

    status = -1;
    //if ( ( cf.enable_labjack ) && ( lj_fd > 0 ) ) {
        //while ( status < 0 ) {
            //recv_bytes = net_client( lj_fd, lj_buf, &msg, mode );
            //lj_buf[recv_bytes] = '\0';
            //if ( recv_bytes > 0 ) {
                //messages_decode( lj_fd, lj_buf, &msg );
            //}
            //status = msg.lj.data.battery1;
        //}
        //printf( "MAIN: Kill switch is closed.\n" );
    //}
    if ( (cf.enable_labjack) && (lj_fd > 0) ) {
    	while ( getBatteryVoltage(AIN_0) < 10.0 ) {
    		query_labjack( );
    		usleep( 100000 );
		}
		printf( "MAIN: Kill switch is closed.\n" );
	}
    printf( "MAIN: Waiting for motors to arm ... " );
    if ( ( cf.enable_labjack ) && ( lj_fd > 0 ) ) {
        sleep( 7 );
    }
    printf( "<OK>\n" );

    /* Main loop. */
    while ( 1 ) {
        /* Get network data. */
        if ( ( cf.enable_net ) && ( server_fd > 0 ) ) {
            recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_NAV );
            if ( recv_bytes > 0 ) {
                recv_buf[recv_bytes] = '\0';
                messages_decode( server_fd, recv_buf, &msg );
            }
        }
        printf( "MAIN:\n%lf %lf %lf %lf %lf\n\n"
        	, msg.gain.data.kp_pitch
        	, msg.gain.data.ki_pitch
        	, msg.gain.data.kd_pitch
        	, msg.gain.data.kp_roll
        	, msg.gain.data.kp_yaw
        );

        /* Check state of emergency stop value. */
        if ( msg.stop.data.state == TRUE ) {
            printf( "MAIN:  EMERGENCY STOP received. Setting actuators to\n"
                    "       safe positions!\n" );
            /* Set all the actuators to safe positions. */
            pololuInitializeChannels( pololu_fd );
        }

        /* Send dropper servo command. */
        if ( pololu_fd > 0 ) {
            status = pololuSetPosition7Bit( pololu_fd, 11, msg.client.data.dropper );
        }

        /* Check for assisted teleop commands. */
        if ( msg.target.data.mode == MANUAL ) {
            msg.target.data.pitch   += msg.teleop.data.pitch;
            msg.target.data.roll    += msg.teleop.data.roll;
            msg.target.data.yaw     += msg.teleop.data.yaw;
            msg.target.data.depth   += msg.teleop.data.depth;
            msg.target.data.fx      += msg.teleop.data.fx;
            msg.target.data.fy      += msg.teleop.data.fy;
            msg.target.data.speed   += msg.teleop.data.speed;

            /* Reset the target change values back to zero. */
            msg.teleop.data.pitch = 0;
            msg.teleop.data.roll = 0;
            msg.teleop.data.yaw = 0;
            msg.teleop.data.depth = 0;
            msg.teleop.data.fx = 0;
            msg.teleop.data.fy = 0;
            msg.teleop.data.speed = 0;

            //printf( "%f\n", msg.target.data.pitch );
            //printf( "%f\n", msg.target.data.roll );
            //printf( "%f\n", msg.target.data.yaw );
            //printf( "%f\n", msg.target.data.depth );
            //printf( "%f\n", msg.target.data.fx );
            //printf( "%f\n", msg.target.data.fy );
            //printf( "%f\n", msg.target.data.speed );
        }

        /* Get vision data. */
        if ( ( cf.enable_vision ) && ( vision_fd > 0 ) ) {
            time1s =    vision_time.tv_sec;
            time1ms =   vision_time.tv_usec;
            time2s =    vision_start.tv_sec;
            time2ms =   vision_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if ( dt > cf.period_vision ) {
                recv_bytes = net_client( vision_fd, vision_buf, &msg, mode );
                vision_buf[recv_bytes] = '\0';
                if ( recv_bytes > 0 ) {
                    messages_decode( vision_fd, vision_buf, &msg );
                    /* Set target values based on current orientation and pixel error. */
                    msg.target.data.yaw = msg.mstrain.data.yaw + (float)msg.vision.data.front_x / 10.;
                    msg.target.data.pitch = msg.mstrain.data.pitch + (float)msg.vision.data.front_y / 10.;
                    //msg.target.data.yaw = msg.mstrain.data.yaw + (float)msg.vision.data.bottom_y;
                    //msg.target.data.fx = (float)msg.vision.data.bottom_x;
                    gettimeofday( &vision_start, NULL );
                }
            }
            //printf( "MAIN:\n%f\n%f\n\n", msg.target.data.pitch, msg.target.data.yaw );
        }

        /* Get labjack daemon data. */
        //if ( ( cf.enable_labjack ) && ( lj_fd > 0 ) ) {
            //recv_bytes = net_client( lj_fd, lj_buf, &msg, mode );
            //lj_buf[recv_bytes] = '\0';
            //if ( recv_bytes > 0 ) {
                //messages_decode( lj_fd, lj_buf, &msg );
            //}
        //}
        if ( (cf.enable_labjack) && (lj_fd > 0) ) {
        	query_labjack( );
        	lj.battery1 = getBatteryVoltage( AIN_0 );
        	lj.battery2 = getBatteryVoltage( AIN_1 );
        	lj.pressure = getBatteryVoltage( AIN_2 );
        	lj.water    = getBatteryVoltage( AIN_3 );
		}

        /* Get planner data. */
        if ( ( cf.enable_planner ) && ( planner_fd > 0 ) ) {
            time1s =    planner_time.tv_sec;
            time1ms =   planner_time.tv_usec;
            time2s =    planner_start.tv_sec;
            time2ms =   planner_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if ( dt > cf.period_planner ) {
                recv_bytes = net_client( planner_fd, planner_buf, &msg, mode );
                planner_buf[recv_bytes] = '\0';
                if ( recv_bytes > 0 ) {
                    messages_decode( planner_fd, vision_buf, &msg );
                    gettimeofday( &planner_start, NULL );
                }
            }
        }

        /* Get Microstrain data. */
        if ( ( cf.enable_imu ) && ( imu_fd > 0 ) ) {
            recv_bytes = mstrain_euler_vectors( imu_fd,
                                                &msg.mstrain.data.pitch,
                                                &msg.mstrain.data.roll,
                                                &msg.mstrain.data.yaw,
                                                msg.mstrain.data.accel,
                                                msg.mstrain.data.ang_rate
                                              );
        }
        //printf( "MAIN:\n%f %f %f\n\n"
        	//, msg.mstrain.data.pitch
        	//, msg.mstrain.data.roll
        	//, msg.mstrain.data.yaw
        //);

        /* Get Labjack data. */
        //if ( ( cf.enable_labjack ) && ( lj_fd > 0 ) ) {
            //query_labjack( );
            //lj.battery1 = getBatteryVoltage( AIN_0 );
            //lj.battery2 = getBatteryVoltage( AIN_1 );
            //lj.pressure = getBatteryVoltage( AIN_2 );
            //lj.water    = getBatteryVoltage( AIN_3 );
        //}

        /* Perform PID loops. */
        if ( msg.stop.data.state == FALSE ) {
            /* Pitch. */
            time1s =    pitch_time.tv_sec;
            time1ms =   pitch_time.tv_usec;
            time2s =    pitch_start.tv_sec;
            time2ms =   pitch_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if ( dt > pid.pitch.period ) {
                if ( ( cf.enable_pololu ) && ( pololu_fd > 0 ) ) {
                    pid_loop( pololu_fd, &pid, &cf, &msg, &lj, dt, PID_PITCH );
                }
                msg.status.data.pitch_period = dt;
                gettimeofday( &pitch_start, NULL );
            }

            /* Roll. */
            time1s =    roll_time.tv_sec;
            time1ms =   roll_time.tv_usec;
            time2s =    roll_start.tv_sec;
            time2ms =   roll_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if ( dt > pid.roll.period ) {
                if ( ( cf.enable_pololu ) && ( pololu_fd > 0 ) ) {
                    pid_loop( pololu_fd, &pid, &cf, &msg, &lj, dt, PID_ROLL );
                }
                msg.status.data.roll_period = dt;
                gettimeofday( &roll_start, NULL );
            }

            /* Yaw. */
            time1s =    yaw_time.tv_sec;
            time1ms =   yaw_time.tv_usec;
            time2s =    yaw_start.tv_sec;
            time2ms =   yaw_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if ( dt > pid.yaw.period ) {
                if ( ( cf.enable_pololu ) && ( pololu_fd > 0 ) ) {
                    pid_loop( pololu_fd, &pid, &cf, &msg, &lj, dt, PID_YAW );
                }
                msg.status.data.yaw_period = dt;
                gettimeofday( &yaw_start, NULL );
            }

            /* Depth. */
            time1s =    depth_time.tv_sec;
            time1ms =   depth_time.tv_usec;
            time2s =    depth_start.tv_sec;
            time2ms =   depth_start.tv_usec;
            dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
            if ( dt > pid.depth.period ) {
                if ( ( cf.enable_pololu ) && ( pololu_fd > 0 ) ) {
                    pid_loop( pololu_fd, &pid, &cf, &msg, &lj, dt, PID_DEPTH );
                }
                msg.status.data.depth_period = dt;
                gettimeofday( &depth_start, NULL );
            }
        }

        /* Update status message. */
        messages_update( &msg, &lj );

        /* Update and send the SSA data. */
        #ifdef USE_SSA
        ssa_update_telemetry( &msg );
        usleep( 500000 );
        #endif /* USE_SSA */

        /* Update timers. */
        gettimeofday( &pitch_time, NULL );
        gettimeofday( &roll_time, NULL );
        gettimeofday( &yaw_time, NULL );
        gettimeofday( &depth_time, NULL );
        gettimeofday( &vision_time, NULL );
        gettimeofday( &planner_time, NULL );
    }

    exit( 0 );
} /* end main() */
