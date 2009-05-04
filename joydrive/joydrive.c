/******************************************************************************
 *
 *  Title:        joydrive.c
 *
 *  Description:  Main program for joydrive daemon.
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

#include "joydrive.h"
#include "network.h"
#include "labjack.h"
#include "util.h"
#include "messages.h"
#include "parser.h"
#include "joy.h"


/* Global file descriptors. Only global so that labjackd_exit() can close them. */
int joy_fd;
int nav_fd;


/******************************************************************************
 *
 * Title:       void joydrive_sigint( int signal )
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

void joydrive_sigint( int signal )
{
    exit( 0 );
} /* end joydrive_sigint() */


/******************************************************************************
 *
 * Title:       void joydrive_exit( )
 *
 * Description: Exit function for main program. Closes all file descriptors.
 *              This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     File descriptors: joy_fd, net_fd.
 *
 *****************************************************************************/

void joydrive_exit( )
{
    /* Sleep to let things shut down properly. */
    usleep( 200000 );

    /* Close the open file descriptors. */
    if ( joy_fd > 0 ) {
        close( joy_fd );
    }
    if ( nav_fd > 0 ) {
        close( nav_fd );
    }
} /* end joydrive_exit() */


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
    exit_ptr = joydrive_exit;
    atexit( exit_ptr );

    struct sigaction sigint_action;
    sigint_action.sa_handler = joydrive_sigint;
    sigint_action.sa_flags = 0;
    sigaction( SIGINT, &sigint_action, NULL );

    int recv_bytes = 0;
    int status = -1;
    int mode = MODE_JOY;
    char recv_buf[MAX_MSG_SIZE];
    MSG_DATA msg;
    CONF_VARS cf;
    JOY_DATA joy;

    /* Initialize variables. */
    joy_fd = -1;
    nav_fd = -1;

    memset( &msg, 0, sizeof( MSG_DATA ) );
    memset( &cf, 0, sizeof( CONF_VARS ) );
    memset( &joy, 0, sizeof( JOY_DATA ) );

    /* Parse command line arguments. */
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, STINGRAY, ( const char * )JOYDRIVE_FILENAME );

    /* Set up communications. */
    //nav_fd = net_client_setup( cf.server_IP, cf.api_port );

    /* Set up the joystick. */
    joy_fd = joy_setup( );
    printf( "MAIN: joy_fd = %d\n", joy_fd );

    /* Main loop. */
    while ( 1 ) {
        /* Get network data. */
        if ( nav_fd > 0 ) {
            recv_bytes = net_client( nav_fd, recv_buf, &msg, mode );
            if ( recv_bytes > 0 ) {
                recv_buf[recv_bytes] = '\0';
                messages_decode( nav_fd, recv_buf, &msg );
            }
        }
        /* Reset the target change values back to zero. */
        msg.teleop.data.pitch = 0;
        msg.teleop.data.roll = 0;
        msg.teleop.data.yaw = 0;
        msg.teleop.data.depth = 0;
        msg.teleop.data.fx = 0;
        msg.teleop.data.fy = 0;
        msg.teleop.data.speed = 0;

        /* Get joystick data. */
        if ( joy_fd > 0 ) {
            status = joy_get_data( joy_fd, &joy );
        }

        /* Set target change values based on joystick input. */
        if ( joy.joy_axis == JOY_A5 ) {
            if ( joy.axis_value == -32767 ) {
                msg.teleop.data.fy = 1;
                printf( "MAIN: fy + 1.\n" );
            }
            else if ( joy.axis_value == 32767 ) {
                msg.teleop.data.fy = -1;
                printf( "MAIN: fy - 1.\n" );
            }
        }
        if ( joy.joy_axis == JOY_A4 ) {
            if ( joy.axis_value == 32767 ) {
                msg.teleop.data.fx = 1;
                printf( "MAIN: fx + 1.\n" );
            }
            else if ( joy.axis_value == -32767 ) {
                msg.teleop.data.fx = -1;
                printf( "MAIN: fx - 1.\n" );
            }
        }
        if ( joy.joy_button == JOY_B0 ) {
            msg.teleop.data.yaw = -1;
                printf( "MAIN: yaw - 1.\n" );
        }
        else if ( joy.joy_button == JOY_B2 ) {
            msg.teleop.data.yaw = 1;
                printf( "MAIN: yaw + 1.\n" );
        }
        if ( joy.joy_button == JOY_B1 ) {
            msg.teleop.data.depth = -0.01;
                printf( "MAIN: depth - 0.01.\n" );
        }
        else if ( joy.joy_button == JOY_B3 ) {
            msg.teleop.data.depth = 0.01;
                printf( "MAIN: depth + 0.01.\n" );
        }
        if ( joy.joy_axis == JOY_A0 ) {
            if ( joy.axis_value == 32767 ) {
                msg.teleop.data.roll = 1;
                printf( "MAIN: roll + 1.\n" );
            }
            else if ( joy.axis_value == -32767 ) {
                msg.teleop.data.roll = -1;
                printf( "MAIN: roll - 1.\n" );
            }
        }
        if ( joy.joy_axis == JOY_A1 ) {
            if ( joy.axis_value == -32767 ) {
                msg.teleop.data.pitch = 1;
                printf( "MAIN: pitch + 1.\n" );
            }
            else if ( joy.axis_value == 32767 ) {
                msg.teleop.data.pitch = -1;
                printf( "MAIN: pitch - 1.\n" );
            }
        }
        if ( joy.joy_button == JOY_B4 ) {
            msg.teleop.data.speed = -1;
                printf( "MAIN: speed - 1.\n" );
        }
        else if ( joy.joy_button == JOY_B5 ) {
            msg.teleop.data.speed = 1;
                printf( "MAIN: speed + 1.\n" );
        }
        if (joy.joy_button == JOY_B6 ) {
            if ( joy.button_value == 1 ) {
                if ( joy.joy_button == JOY_B7 ) {
                    if ( joy.button_value == 1 ) {
                        printf( "MAIN: send estop\n" );
                    }
                }
            }
        }
    }

    exit( 0 );
} /* end main() */
