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


int client_fd;
int planner_fd;
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
	memset( &msg, 0, sizeof( MSG_DATA ) );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, GUI, ( const char * )CLIENT_FILENAME );

	/* Initialize the targets and the gains. */
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
		client_fd = net_client_setup( cf.server_IP, cf.api_port );
	}
	if ( cf.enable_planner ) {
		planner_fd = net_client_setup( cf.planner_IP, cf.planner_port );
	}

	/* Set up the GUI. */
	gtk_init( &argc, &argv );

	gui_init( );

	gui_set_timers( );

	gtk_main( );

	return 0;
} /* end main() */
