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
#include <time.h>
#include <sys/time.h>
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
IplImage *f_bin_img;
IplImage *b_bin_img;


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

    cvReleaseImage( &b_bin_img );
    cvReleaseImage( &f_bin_img );

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
    int width = -1;
    int height = -1;
    int pipex = -1;
    int fence_center = -1;
	int y_max = -1;
    double bearing = -1;
    int amt = 2;
    int ii = 0;
    int lineWidth = 75;
    int camera = 0;
    int saving_fvideo = FALSE;
    int saving_bvideo = FALSE;
    IplImage *f_img = NULL;
    IplImage *b_img = NULL;
    const char *f_win = "Front";
    const char *b_win = "Bottom";
    CvVideoWriter *f_writer = 0;
    CvVideoWriter *b_writer = 0;
    int is_color = TRUE;
    double fps = 0.0;
    struct timeval ctime;
    struct tm ct;
    char write_time[80] = {0};

    printf( "MAIN: Starting Vision daemon ...\n" );
    /* Initialize variables. */
    server_fd = -1;
    memset( &msg, 0, sizeof( MSG_DATA ) );

    /* Parse command line arguments. */
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, STINGRAY, ( const char * )VISIOND_FILENAME );

    /* Initialize HSV message data to configuration values. */
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

    /* Set up communications. */
    if ( cf.enable_net ) {
        server_fd = net_server_setup( cf.vision_port );
		if ( server_fd > 0 ) {
			printf( "MAIN: Server setup OK.\n" );
		}
		else {
			printf( "MAIN: WARNING!!! Server setup failed.\n" );
		}
    }

    /* Open front camera. */
    /* Need to have a config about what cameras if any to open */
    /* Commenting this out because my default camera does not work */
    f_cam = cvCaptureFromCAM( camera );
    if ( !f_cam ) {
        cvReleaseCapture( &f_cam );
        printf( "MAIN: Could not open f_cam.\n" );
    }
    else {
        f_img = cvQueryFrame( f_cam );
        f_bin_img = cvCreateImage( cvGetSize( f_img ), IPL_DEPTH_8U, 1 );
    	fence_center = f_img->width / 2;
		printf( "MAIN: Front camera opened OK.\n" );
    }

    /* Open bottom camera. */
    camera = 1;
    b_cam = cvCaptureFromCAM( camera );
    if ( !b_cam ) {
        cvReleaseCapture( &b_cam );
        printf( "MAIN: Could not open b_cam.\n" );
    }
    else {
        b_img = cvQueryFrame( b_cam );
        b_bin_img = cvCreateImage( cvGetSize( b_img ), IPL_DEPTH_8U, 1 );
		printf( "MAIN: Bottom camera opened OK.\n" );
    }

    /* Create windows to display video if set in configuration file. */
    if ( cf.vision_window ) {
        cvNamedWindow( f_win, CV_WINDOW_AUTOSIZE );
        cvNamedWindow( b_win, CV_WINDOW_AUTOSIZE );
    }
    printf( "MAIN: Vision server running now.\n" );

    /* Main loop. */
    int loop_counter = 0;
    while ( 1 ) {
    	if ( loop_counter == 1000 ) {
    		loop_counter = 0;
		}
		else {
			loop_counter++;
		}

    	/* Do vision processing based on task */
    	if ( msg.task.data.num == TASK_NONE ) {
    		/* Do nothing and give cleared values. */
    		msg.vision.data.front_x = 0;
    		msg.vision.data.front_y = 0;
    		msg.vision.data.bottom_x = 0;
    		msg.vision.data.bottom_y = 0.0;
		}
        else if ( msg.task.data.num == TASK_BUOY && f_cam ) {
			/* Look for the buoy. */
			status = vision_find_dot( &dotx, &doty, &width, &height, amt,
					f_cam, f_img, f_bin_img,
					msg.vsetting.data.buoy_hsv.hL,
					msg.vsetting.data.buoy_hsv.hH,
					msg.vsetting.data.buoy_hsv.sL,
					msg.vsetting.data.buoy_hsv.sH,
					msg.vsetting.data.buoy_hsv.vL,
					msg.vsetting.data.buoy_hsv.vH );

			if ( status == 1 ) {
				msg.vision.data.front_x = -1 * (dotx - f_img->width / 2);
				msg.vision.data.front_y = -1 * (doty - f_img->height / 2);
				if ( cf.vision_window ) {
					if ( cvWaitKey( 5 ) >= 0 );
					cvCircle( f_img, cvPoint(dotx, doty),
						10, cvScalar(255, 0, 0), 5, 8 );
					cvShowImage( f_win, f_img );
				}
			}
		}
		else if ( msg.task.data.num == TASK_PIPE && b_cam ) {
			/* Look for the pipe */
			status = vision_find_pipe( &pipex, &bearing, b_cam, b_img, b_bin_img,
                    msg.vsetting.data.pipe_hsv.hL,
                    msg.vsetting.data.pipe_hsv.hH,
                    msg.vsetting.data.pipe_hsv.sL,
                    msg.vsetting.data.pipe_hsv.sH,
                    msg.vsetting.data.pipe_hsv.vL,
                    msg.vsetting.data.pipe_hsv.vH );

            if ( status == 1 ) {
                if ( cf.vision_window ) {
                    if ( cvWaitKey( 5 ) >= 0 );
                        for ( ii = 0; ii < lineWidth; ii++ ) {
                            if( bearing != 0 ) {
                                cvCircle( b_img,
                                        cvPoint( b_img->width / 2 + ((int)(bearing * ii)),
											(b_img->width / 2) + ii ),
											2, cvScalar(255, 255, 0), 2 );
                            }
                            else {
                                cvCircle( b_img,
                                        cvPoint( b_img->width / 2 + ((int)(bearing * ii)),
                                            (b_img->width / 2) + ii ),
											2, cvScalar(0, 0, 255), 2 );
                            }
                            cvShowImage( b_win, b_img );
                        }
                }
                bearing = atan(bearing) * 180 / M_PI;
                msg.vision.data.bottom_x = pipex;
                msg.vision.data.bottom_y = bearing;
            }
		}
		else if ( msg.task.data.num == TASK_FENCE && f_cam ) {
			/* Look for the fence. */
            status = vision_find_fence( &fence_center, &y_max, f_cam, f_img, f_bin_img,
                    msg.vsetting.data.fence_hsv.hL,
                    msg.vsetting.data.fence_hsv.hH,
                    msg.vsetting.data.fence_hsv.sL,
                    msg.vsetting.data.fence_hsv.sH,
                    msg.vsetting.data.fence_hsv.vL,
                    msg.vsetting.data.fence_hsv.vH );

            if ( status == 1 ) {
                if ( cf.vision_window ) {
                    if ( cvWaitKey( 5 ) >= 0 );
					cvCircle( f_img, cvPoint(fence_center, f_img->height / 2),
						10, cvScalar(0, 0, 255), 5, 8 );
					for ( ii = 0; ii < lineWidth; ii++ ) {
						cvCircle( f_img, cvPoint(f_img->width / 2 + ii, y_max),
								2, cvScalar(0, 255, 0), 2 );
					}
                	cvShowImage( f_win, f_img );
				}
            }
        }
        else if ( msg.task.data.num == TASK_GATE && f_cam ) {
        	/* Look for the gate */

		}
		else {
			/* No mode or no valid cameras - Simulate. */
			if ( loop_counter % 100 == 0 ) {
				msg.vision.data.front_x = loop_counter;
				msg.vision.data.front_y = loop_counter;
			}
		}


        /* Get network data. */
        if ( ( cf.enable_net ) && ( server_fd > 0 ) ) {
            recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_VISION );
            if ( recv_bytes > 0 ) {
                recv_buf[recv_bytes] = '\0';
                messages_decode( server_fd, recv_buf, &msg );
            }
        }

        /* Check state of save frames and video messages. */
        if ( msg.vsetting.data.save_fframe && f_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "images/f20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.jpg", ctime.tv_usec );
            cvSaveImage( write_time, f_img );
            msg.vsetting.data.save_fframe = FALSE;
        }
        if ( msg.vsetting.data.save_bframe && b_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "images/b20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.jpg", ctime.tv_usec );
            cvSaveImage( write_time, b_img );
            msg.vsetting.data.save_bframe = FALSE;
        }
        if ( msg.vsetting.data.save_fvideo && !saving_fvideo && f_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "stream/f20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.avi", ctime.tv_usec );
            fps = cvGetCaptureProperty( f_cam, CV_CAP_PROP_FPS );
            printf( "MAIN: fps = %lf\n", fps );
            f_writer = cvCreateVideoWriter( write_time, CV_FOURCC('M', 'J', 'P', 'G'),
                fps, cvGetSize( f_img ), is_color );
            saving_fvideo = TRUE;
        }
        else if ( !msg.vsetting.data.save_fvideo && saving_fvideo ) {
            cvReleaseVideoWriter( &f_writer );
            saving_fvideo = FALSE;
        }
        if ( msg.vsetting.data.save_bvideo && !saving_bvideo && b_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "stream/b20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.avi", ctime.tv_usec );
            fps = cvGetCaptureProperty( b_cam, CV_CAP_PROP_FPS );
            b_writer = cvCreateVideoWriter( write_time, CV_FOURCC('M', 'J', 'P', 'G'),
                fps, cvGetSize( b_img ), is_color );
            saving_bvideo = TRUE;
        }
        else if ( !msg.vsetting.data.save_bvideo && saving_bvideo ) {
            cvReleaseVideoWriter( &b_writer );
            saving_bvideo = FALSE;
        }
    }

    exit( 0 );
} /* end main() */