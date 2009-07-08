/******************************************************************************
 *
 *  Title:        client.c
 *
 *  Description:  A client GUI program for running specific commands to the UUV
 *                for testing and data collection purposes.
 *
 *****************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>
#include <gtk/gtk.h>

#include "client.h"
#include "gui.h"
#include "network.h"
#include "util.h"
#include "messages.h"
#include "parser.h"


int planner_fd;
int vision_fd;
int nav_fd;
CONF_VARS cf;
MSG_DATA msg;


/******************************************************************************
 *
 * Title:       int main( int argc, char *argv[] )
 *
 * Description: Initialize data.
 *
 * Input:       Command line arguments -- argc, argv.
 *
 * Output:      None.
 *
 * Globals:     kill_app
 *
 *****************************************************************************/

int main( int argc, char *argv[] )
{
	printf( "MAIN: Starting GUI ...\n" );
	
    memset( &msg, 0, sizeof(MSG_DATA) );
	messages_init( &msg );

    /* Parse command line arguments. */
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, GUI, ( const char * )CLIENT_FILENAME );

    /* Initialize the targets and the gains. */
    msg.target.data.pitch   = cf.target_pitch;
    msg.target.data.roll    = cf.target_roll;
    msg.target.data.yaw     = cf.target_yaw;
    msg.target.data.depth   = cf.target_depth;
    msg.target.data.fx      = cf.target_fx;
    msg.target.data.fy      = cf.target_fy;
    msg.target.data.speed   = cf.target_speed;
    //msg.gain.data.kp_pitch  = cf.kp_pitch;
    //msg.gain.data.ki_pitch  = cf.ki_pitch;
    //msg.gain.data.kd_pitch  = cf.kd_pitch;
    //msg.gain.data.kp_roll   = cf.kp_roll;
    //msg.gain.data.ki_roll   = cf.ki_roll;
    //msg.gain.data.kd_roll   = cf.kd_roll;
    //msg.gain.data.kp_yaw    = cf.kp_yaw;
    //msg.gain.data.ki_yaw    = cf.ki_yaw;
    //msg.gain.data.kd_yaw    = cf.kd_yaw;
    //msg.gain.data.kp_depth  = cf.kp_depth;
    //msg.gain.data.ki_depth  = cf.ki_depth;
    //msg.gain.data.kd_depth  = cf.kd_depth;
    msg.vsetting.data.pipe_hsv.hL = cf.pipe_hL;
    msg.vsetting.data.pipe_hsv.hH = cf.pipe_hH;
    msg.vsetting.data.pipe_hsv.sL = cf.pipe_sL;
    msg.vsetting.data.pipe_hsv.sH = cf.pipe_sH;
    msg.vsetting.data.pipe_hsv.vL = cf.pipe_vL;
    msg.vsetting.data.pipe_hsv.vH = cf.pipe_vH;
    msg.vsetting.data.buoy_hsv.hL = cf.buoy_hL;
    msg.vsetting.data.buoy_hsv.hH = cf.buoy_hH;
    msg.vsetting.data.buoy_hsv.sL = cf.buoy_sL;
    msg.vsetting.data.buoy_hsv.sH = cf.buoy_sH;
    msg.vsetting.data.buoy_hsv.vL = cf.buoy_vL;
    msg.vsetting.data.buoy_hsv.vH = cf.buoy_vH;
    msg.vsetting.data.fence_hsv.hL = cf.fence_hL;
    msg.vsetting.data.fence_hsv.hH = cf.fence_hH;
    msg.vsetting.data.fence_hsv.sL = cf.fence_sL;
    msg.vsetting.data.fence_hsv.sH = cf.fence_sH;
    msg.vsetting.data.fence_hsv.vL = cf.fence_vL;
    msg.vsetting.data.fence_hsv.vH = cf.fence_vH;

    /* Connect to the planner server. */
    if( cf.enable_planner ) {
        planner_fd = net_client_setup( cf.planner_IP, cf.planner_port );
		if( planner_fd > 0 ) {
			printf( "MAIN: Planner client setup OK.\n" );
		}
		else {
			printf( "MAIN: WARNING!!! Planner client setup failed.\n" );
		}
    }

	/* Connect to the vision server. */
    if( cf.enable_vision ) {
        vision_fd = net_client_setup( cf.vision_IP, cf.vision_port );
		if( vision_fd > 0 ) {
			printf( "MAIN: Vision client setup OK.\n" );
		}
		else {
			printf( "MAIN: WARNING!!! Vision client setup failed.\n" );
		}
    }

	/* Connect to the nav server. */
    if( cf.enable_nav ) {
        nav_fd = net_client_setup( cf.nav_IP, cf.nav_port );
		if( nav_fd > 0 ) {
			printf( "MAIN: Nav client setup OK.\n" );
		}
		else {
			printf( "MAIN: WARNING!!! Nav client setup failed.\n" );
		}
    }

    /* Set up the GUI. */
    gtk_init( &argc, &argv );
	printf( "MAIN: Gtk initialized OK.\n" );
    gui_init( );
	printf( "MAIN: GUI initialized OK.\n" );
    gui_set_timers( );
	printf( "MAIN: Timers setup OK.\n" );
	printf( "MAIN: GUI running now.\n" );
    gtk_main( );

    return 0;
} /* end main() */
