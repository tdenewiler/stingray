/******************************************************************************
 *
 *  Title:        visiond.c
 *
 *  Description:  Main program for vision daemon.
 *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "visiond.h"
#include "vision.h"
#include "network.h"
#include "parser.h"
#include "util.h"
#include "messages.h"
#include "microstrain.h"
#include "labjack.h"


/* Global file descriptors. Only global so that vision_exit() can close them. */
int server_fd;
CvCapture *f_cam;
CvCapture *b_cam;
IplImage *f_out_img;
IplImage *b_out_img;


/******************************************************************************
 *
 * Title:       void visiond_sigint( int signal )
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

void visiond_sigint( int signal )
{
	exit( 0 );
} /* end visiond_sigint() */


/******************************************************************************
 *
 * Title:       void visiond_exit( )
 *
 * Description: Exit function for main program. Closes all file descriptors.
 *              This function is called when SIGINT (ctrl-c) is invoked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     File descriptors: server_fd, pololu_fd, labjack_fd, imu_fd.
 *
 *****************************************************************************/

void visiond_exit( )
{
	printf( "\nVISIOND_EXIT: Shutting down visiond program ... " );
	/* Sleep to let things shut down properly. */
	usleep( 200000 );

	/* Close the open file descriptors. */

	if ( server_fd > 0 ) {
		close( server_fd );
	}

	/* Close the cameras and windows. */
	if ( f_cam ) {
		cvReleaseCapture( &f_cam );
	}

	if ( b_cam ) {
		cvReleaseCapture( &b_cam );
	}

	cvReleaseImage( &f_out_img );

	cvReleaseImage( &b_out_img );

	printf( "<OK>\n\n" );
} /* end visiond_exit() */


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
	exit_ptr = visiond_exit;
	atexit( exit_ptr );

	struct sigaction sigint_action;
	sigint_action.sa_handler = visiond_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	/* Set up variables and initialize them. */
	int status = -1;
	int recv_bytes = 0;
	char recv_buf[MAX_MSG_SIZE];
	CONF_VARS cf;
	MSG_DATA msg;
	int dotx = -1;
	int doty = -1;
	int pipex = -1;
	float bearing = -1;
	int amt = 1;
	int camera = 0;
	IplImage *f_img = NULL;
	IplImage *b_img = NULL;
	const char *f_win = "Front";
	const char *b_win = "Bottom";

	printf( "MAIN: Starting Vision daemon ... " );

	/* Initialize variables. */
	server_fd = -1;
	memset( &msg, 0, sizeof( MSG_DATA ) );

	/* Parse command line arguments. */
	parse_default_config( &cf );
	parse_cla( argc, argv, &cf, STINGRAY, ( const char * )VISIOND_FILENAME );

	/* Set up communications. */
	if ( cf.enable_net ) {
		server_fd = net_server_setup( cf.vision_port );
	}

	/* Open cameras. */
	f_cam = cvCaptureFromCAM( camera );
	if ( !f_cam ) {
		cvReleaseCapture( &f_cam );
		printf( "MAIN: Could not open f_cam.\n" );
	}
	else {
		f_img = cvQueryFrame( f_cam );
		f_out_img = cvCreateImage( cvSize( f_img->width, f_img->height ), IPL_DEPTH_8U, 1 );
	}

	camera = 1;
	b_cam = cvCaptureFromCAM( camera );
	if ( !b_cam ) {
		cvReleaseCapture( &b_cam );
		printf( "MAIN: Could not open b_cam.\n" );
	}
	else {
		b_img = cvQueryFrame( b_cam );
		b_out_img = cvCreateImage( cvSize( b_img->width, b_img->height ), IPL_DEPTH_8U, 1 );
	}

	/* Create a window if specified in configuration file. */
	if ( cf.vision_window ) {
		cvNamedWindow( f_win, CV_WINDOW_AUTOSIZE );
		cvNamedWindow( b_win, CV_WINDOW_AUTOSIZE );
	}

	printf( "<OK>\n" );

	/* Main loop. */
	while ( 1 ) {
		/* Get vision data. */
		if ( b_cam ) {
			status = vision_find_pipe( &pipex, &bearing, b_cam, b_img, b_out_img );
			if ( status == 1 ) {
				msg.vision.data.bottom_x = pipex;
				msg.vision.data.bottom_y = bearing;
				if ( cf.vision_window ) {
					if ( cvWaitKey( 3 ) >= 0 );
					cvShowImage( b_win, b_img );
				}
			}
			printf( "MAIN: bottom %d %d\n", dotx, doty );
		}

		if ( f_cam ) {
			status = vision_find_dot( &dotx, &doty, amt, f_cam, f_img, f_out_img );
			if ( status == 1 ) {
				msg.vision.data.front_x = dotx;
				msg.vision.data.front_y = doty;
				if ( cf.vision_window ) {
					if ( cvWaitKey( 3 ) >= 0 );
					cvShowImage( f_win, f_img );
				}
			}
			printf( "MAIN: front %d %d\n", dotx, doty );
		}

		/* Get network data. */
		if ( ( cf.enable_net ) && ( server_fd > 0 ) ) {
			recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_VISION );
			if ( recv_bytes > 0 ) {
				recv_buf[recv_bytes] = '\0';
				messages_decode( server_fd, recv_buf, &msg );
			}
		}
	}

	exit( 0 );
} /* end main() */
