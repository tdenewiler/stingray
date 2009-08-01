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
#include <dirent.h>
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
#include "task.h"


/* Global file descriptors. Only global so that vision_exit() can close them. */
int server_fd;
CvCapture *f_cam;
CvCapture *b_cam;
IplImage *bin_img;
DIR *dirp;


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
 *****************************************************************************/

void visiond_exit( )
{
    printf("\nVISIOND_EXIT: Shutting down visiond program ... ");
    /* Sleep to let things shut down properly. */
    usleep( 200000 );

    /* Close the open file descriptors. */
    if( server_fd > 0 ) {
        close( server_fd );
    }

    /* Close the cameras and windows. */
    if( f_cam ) {
        cvReleaseCapture( &f_cam );
    }

    if( b_cam ) {
        cvReleaseCapture( &b_cam );
    }

    cvReleaseImage( &bin_img );
    closedir( dirp );

    printf("<OK>\n\n");
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
    int xrot = -1;
    int yrot = -1;
    int fence_center = -1;
	int y_max = -1;
    int ii = 0;
    int lineWidth = 75;
    int camera = 0;
    int saving_fvideo = FALSE;
    int saving_bvideo = FALSE;
    IplImage *img = NULL;
    IplImage *img_eq = NULL;
	const char *win = "Image";
	const char *binwin = "Binary";
    CvVideoWriter *f_writer = 0;
    CvVideoWriter *b_writer = 0;
    int is_color = TRUE;
    struct timeval ctime;
    struct tm ct;
    char write_time[80] = {0};
	int vision_mode = VISIOND_NONE;
	int task = TASK_NONE;
	HSV hsv_buoy;
	HSV hsv_pipe;
	HSV hsv_fence;
	int bouyTouchCount = 0;
	double angleFrontCam = VISIOND_FRONT_CAM_ANGLE_OFFSET * M_PI /  180;

	/* Temporary variable to make it easier to switch between using HSV
	 *  olding or boxes to try and find pipe. HSV = 1, Boxes = 2. */
	int pipe_type = VISION_PIPE_HSV;
	int pipex = -10000;
	int pipey = -10000;
	double bearing = -10000;

	/* Variables to hold box centroid sequence and vertex sequence. */
	CvMemStorage *storage1 = 0;
	storage1 = cvCreateMemStorage(0);
	CvSeq *boxes = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage1 );
	CvSeqReader reader1;
	CvMemStorage *storage2 = 0;
	storage2 = cvCreateMemStorage(0);
	CvSeq *squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage2 );
	CvSeqReader reader2;
	CvPoint box_pt;
	CvPoint pt[4];
	CvPoint *rect = pt;
	int count = 4;

	/* Timers. */
	struct timeval fps_time = {0, 0};
	struct timeval fps_start = {0, 0};
	struct timeval save_time = {0, 0};
	struct timeval save_start = {0, 0};
	struct timeval open_time = {0, 0};
	struct timeval open_start = {0, 0};
	int time1s = 0;
	int time1ms = 0;
	int time2s = 0;
	int time2ms = 0;
	int dt = 0;
	int nframes = 0;
    double fps = 0.0;

	/* Variables for opening, reading and using directories and files. */
	struct dirent *dfile = NULL;
	//const char *dirname = "images/buoy/";
	const char *dirname = "../../../pics/practice_side_wed_1pm/images/buoy/";
	int diropen = FALSE;
	char filename[STRING_SIZE * 2];

	/* Initialize timers. */
	gettimeofday( &fps_time, NULL );
	gettimeofday( &fps_start, NULL );
	gettimeofday( &save_time, NULL );
	gettimeofday( &save_start, NULL );
	gettimeofday( &open_time, NULL );
	gettimeofday( &open_start, NULL );

    printf( "MAIN: Starting Vision daemon ...\n" );

    /* Initialize variables. */
    server_fd = -1;
    memset( &msg, 0, sizeof(MSG_DATA) );
	messages_init( &msg );

    /* Parse command line arguments. */
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, STINGRAY, (const char *)VISIOND_FILENAME );

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

	/* Initialize HSV structs to configuration values. */
    hsv_pipe.hL = cf.pipe_hL;
    hsv_pipe.hH = cf.pipe_hH;
    hsv_pipe.sL = cf.pipe_sL;
    hsv_pipe.sH = cf.pipe_sH;
    hsv_pipe.vL = cf.pipe_vL;
    hsv_pipe.vH = cf.pipe_vH;
    hsv_buoy.hL = cf.buoy_hL;
    hsv_buoy.hH = cf.buoy_hH;
    hsv_buoy.sL = cf.buoy_sL;
    hsv_buoy.sH = cf.buoy_sH;
    hsv_buoy.vL = cf.buoy_vL;
    hsv_buoy.vH = cf.buoy_vH;
    hsv_fence.hL = cf.fence_hL;
    hsv_fence.hH = cf.fence_hH;
    hsv_fence.sL = cf.fence_sL;
    hsv_fence.sH = cf.fence_sH;
    hsv_fence.vL = cf.fence_vL;
    hsv_fence.vH = cf.fence_vH;

	/* Open directory. */
	if( cf.open_rate ) {
		if( (dirp = opendir(dirname)) ) {
			printf("MAIN: Image directory opened OK.\n");
			diropen = TRUE;
		}
		else {
			printf("MAIN: WARNING!!! Image directory not opened.\n");
		}
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

    /* Need to have a config about what cameras if any to open. */
    if( cf.op_mode == 99 ) {
    	/* Special case for bad camera. */
    	printf("MAIN: Skipping camera opening because op mode = 99 in "
			   "configuration file.\n");
	}
	else {
		if( diropen ) {
			/* Load an image from disk. */
			dfile = readdir( dirp );
			if( dfile == NULL ) {
				closedir( dirp );
				dirp = opendir( dirname );
			}
			else {
				/* Check that we don't try to open directories. */
				if( strncmp( dfile->d_name, ".", 1 ) == 0 ) {
					dfile = readdir( dirp );
				}
				if( strncmp( dfile->d_name, "..", 2 ) == 0 ) {
					dfile = readdir( dirp );
				}
				/* Load image here and set up other images. */
				strncpy( filename, dirname, STRING_SIZE );
				printf("MAIN: Loading dir %s\n", filename);
				//strncat( filename, dfile->d_name, STRING_SIZE );
				//strncat( filename, "20090730_135401.54264.jpg", STRING_SIZE );
				strncat( filename, "20090730_135359.819759.jpg", STRING_SIZE );

				printf("MAIN: Loading file %s\n", filename);
				img = cvLoadImage( filename );
				bin_img = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
				img_eq  = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3 );
				fence_center = img->width / 2;
				if( img ) {
					printf("MAIN: img created OK.\n");
				}
				if( bin_img ) {
					printf("MAIN: bin_img created OK.\n");
				}
			}
		}
		else {
			/* Open front camera. */
			f_cam = cvCaptureFromCAM( camera );
			if( !f_cam ) {
				cvReleaseCapture( &f_cam );
				printf("MAIN: WARNING!!! Could not open f_cam.\n");
			}
			else {
				img = cvQueryFrame( f_cam );
				bin_img = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
				img_eq  = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3 );
				fence_center = img->width / 2;
				printf("MAIN: Front camera opened OK.\n");
			}

			/* Open bottom camera. */
			camera = 1;
			b_cam = cvCaptureFromCAM( camera );
			if( !b_cam ) {
				cvReleaseCapture( &b_cam );
				printf("MAIN: WARNING!!! Could not open b_cam.\n");
			}
			else {
				img = cvQueryFrame( b_cam );
				bin_img = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
				img_eq  = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3 );
				printf("MAIN: Bottom camera opened OK.\n");
			}
		}
	}

    /* Create windows to display video if set in configuration file. */
	if( cf.vision_window ) {
		cvNamedWindow( win, CV_WINDOW_AUTOSIZE );
		cvNamedWindow( binwin, CV_WINDOW_AUTOSIZE );
	}
    printf("MAIN: Vision server running now.\n");

    /* Main loop. */
    int loop_counter = 0;
    while( 1 ) {
    	if( loop_counter == 1000 ) {
    		loop_counter = 0;
		}
		else {
			loop_counter++;
		}

		/* Look for a file. Check for NULL or directory. */
		if( diropen ) {
			/* Check timer. */
			time1s =  open_time.tv_sec;
			time1ms = open_time.tv_usec;
			time2s =  open_start.tv_sec;
			time2ms = open_start.tv_usec;
			dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms ) / 1000000;

			if( dt > cf.open_rate ) {
				dfile = readdir( dirp );
				if( dfile == NULL ) {
					/* Start over at the beginning of the directory. */
					closedir( dirp );
					dirp = opendir( dirname );
				}
				else {
					if( strncmp( dfile->d_name, ".", 1 ) == 0 ) {
						dfile = readdir( dirp );
					}
					if( strncmp( dfile->d_name, "..", 2 ) == 0 ) {
						dfile = readdir( dirp );
					}
					if( dfile == NULL ) {
						/* Start over at the beginning of the directory. */
						closedir( dirp );
						dirp = opendir( dirname );
						dfile = readdir( dirp );
					}
				}
				/* Reset the open image timer. */
				gettimeofday( &open_start, NULL );
			}
			/* Load image here. */
			strncpy( filename, dirname, STRING_SIZE );
			//strncat( filename, dfile->d_name, STRING_SIZE );
			strncat( filename, "20090730_135359.819759.jpg", STRING_SIZE );

			img = cvLoadImage( filename );
		}

    	/* Do vision processing based on task */
    	if( task == TASK_NONE ) {
    		/* Do nothing and give cleared values. */
    		msg.vision.data.front_x = 0;
    		msg.vision.data.front_y = 0;
    		msg.vision.data.bottom_x = 0;
    		msg.vision.data.bottom_y = 0.0;
		} /* end TASK_NONE */

        else if( task == TASK_GATE && (f_cam || diropen) ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( f_cam );
				/* Flip the source image. */
				cvFlip( img, img );
			}

			/* Look for the gate . */
			status = vision_find_gate( &dotx, &doty, cf.vision_angle, img,
				bin_img, &hsv_buoy );
			if( status == 1 || status == 2 ) {
				/* We have detected the gate. */
				//printf("MAIN: Gate Status = %d\n", status);
				msg.vision.data.status = TASK_GATE_DETECTED;

				/* The subtractions are opposite of each other on purpose. This
				 * is so that they match the way the depth sensor and yaw sensor
				 * work. */
				msg.vision.data.front_x = dotx - (img->width / 2);
				msg.vision.data.front_y = (img->height / 2) - doty;

				xrot =  msg.vision.data.front_x * cos( angleFrontCam ) +
						msg.vision.data.front_y * sin( angleFrontCam );
				yrot = -msg.vision.data.front_x * sin( angleFrontCam ) +
						msg.vision.data.front_y * cos( angleFrontCam );

				msg.vision.data.front_x = xrot;
				msg.vision.data.front_y = yrot;

				/* Draw a circle at the centroid location. */
				cvCircle( img, cvPoint(dotx, doty),
					10, cvScalar(0, 255, 0), 5, 8 );

				if( status == 2 ) {
					msg.vision.data.status = TASK_GATE_CLEARED;
				}
			}
		} /* end TASK_GATE */

        else if( task == TASK_BUOY && (f_cam || diropen) ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( f_cam );
				/* Flip the source image. */
				cvFlip( img, img );
			}

			/* Look for the buoy. */
			status = vision_find_dot( &dotx, &doty, cf.vision_angle, img,
				bin_img, &hsv_buoy );
			if( status == 1 || status == 2 ) {
				/* We have detected the buoy. */
				//printf("MAIN: Bouy Status = %d\n", status);
				msg.vision.data.status = TASK_BUOY_DETECTED;

				/* The subtractions are opposite of each other on purpose. This
				 * is so that they match the way the depth sensor and yaw sensor
				 * work. */
				msg.vision.data.front_x = dotx - (img->width / 2);
				msg.vision.data.front_y = (img->height / 2) - doty;

				xrot =  msg.vision.data.front_x * cos( angleFrontCam ) +
						msg.vision.data.front_y * sin( angleFrontCam );
				yrot = -msg.vision.data.front_x * sin( angleFrontCam ) +
						msg.vision.data.front_y * cos( angleFrontCam );

				msg.vision.data.front_x = xrot;
				msg.vision.data.front_y = yrot;

				/* Draw a circle at the centroid location. */
				cvCircle( img, cvPoint(dotx, doty),
					10, cvScalar(0, 255, 0), 5, 8 );

				if( status == 2 ) {
					msg.vision.data.status = TASK_BUOY_TOUCHED;
				}
			}
		} /* end TASK_BUOY */

		else if( task == TASK_PIPE && (b_cam || diropen) &&
			pipe_type == VISION_PIPE_HSV ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( b_cam );
			}

			/* Look for the pipe. */
			status = vision_find_pipe( &pipex, &pipey, &bearing, img, bin_img,
				&hsv_pipe );

			/* If we get a positive status message, render the box and populate
			 * the network message. */
			if( status == 1 ) {
				/* Set the detection status of vision */
				//printf("MAIN Pipe Status = %d\n", status);
				msg.vision.data.status = TASK_PIPE_DETECTED;

				msg.vision.data.bearing = bearing;
				msg.vision.data.bottom_x = pipex - img->width / 2;
				msg.vision.data.bottom_y = pipey - img->height / 2;

				/* Draw a circle at the centroid location. */
				cvCircle( img, cvPoint(pipex, pipey),
					10, cvScalar(255, 0, 0), 5, 8 );

				/* Draw the bearing if it is there. */
				if( bearing != 0 ) {
					for( ii = 0; ii < 25; ii++ ) {
						cvCircle( img,
							cvPoint(img->width / 2 + ((int)(bearing * ii)),
							img->width / 2 + ii), 2, cvScalar(255, 255, 0), 2 );
					}
				}
				else {
					for( ii = 0; ii < 25; ii++ ) {
						cvCircle( img,
							cvPoint(img->width / 2 + ((int)(bearing * ii)),
							img->width / 2 + ii), 2, cvScalar(0, 0, 255), 2 );
					}
				}
			}
		} /* end TASK_PIPE -- hsv */

		else if( task == TASK_PIPE && (b_cam || diropen) &&
			pipe_type == VISION_PIPE_BOX ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( b_cam );
			}

			/* Look for the pipe. */
			status = vision_find_boxes( img, boxes, squares, VISION_PIPE,
				&msg.vision.data.bearing );

			/* If we get a positive status message, render the box
			 * and populate the network message. */
			if( status > 0 ) {
				/* Initialize the centroid sequence reader. */
				cvStartReadSeq( boxes, &reader1, 0 );
				/* Read four sequence elements at a time. */
				for( ii = 0; ii < boxes->total; ii += 2 ) {
					/* Read centroid x and y coordinates. */
					CV_READ_SEQ_ELEM( box_pt, reader1 );
					/* Draw the centroid as a circle. */
					cvCircle( img, box_pt,
						10, cvScalar(0, 0, 255), 5, 8 );
				}
				/* Initialize the vertex sequence reader. */
				cvStartReadSeq( squares, &reader2, 0 );
				for( ii = 0; ii < squares->total; ii += 4 ) {
					/* Read vertex x and y coordinates. */
					CV_READ_SEQ_ELEM( pt[0], reader2 );
					CV_READ_SEQ_ELEM( pt[1], reader2 );
					CV_READ_SEQ_ELEM( pt[2], reader2 );
					CV_READ_SEQ_ELEM( pt[3], reader2 );
					/* Draw the square as a closed polyline. */
					cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0),
						3, CV_AA, 0 );
				}
				/* Set target offsets in network message. */
				if( boxes->total > 0 ) {
					/* Set the detection status of vision */
		    		msg.vision.data.status = TASK_PIPE_DETECTED;

					msg.vision.data.bearing = 0; /* !!! TODO: Fix this value !!! */
					msg.vision.data.bottom_x = box_pt.x - img->width / 2;
					msg.vision.data.bottom_y = box_pt.y - img->height / 2;
				}
				/* Clear out the sequences so that next time we only draw newly
				 * found squares and centroids. */
				cvClearSeq( boxes );
				cvClearSeq( squares );
			}
		} /* end TASK_PIPE -- boxes */

		else if( task == TASK_FENCE && (f_cam || diropen) ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( f_cam );
				/* Flip the source image. */
				cvFlip( img, img );
			}

			/* Look for the fence. */
            status = vision_find_fence( &fence_center, &y_max, img,	bin_img,
				&hsv_fence );
            if( status == 1 ) {
            	/* Set the detection status of vision. */
				msg.vision.data.status = TASK_FENCE_DETECTED;

				/* Draw a circle at the centroid. */
				cvCircle( img, cvPoint(fence_center, y_max),
					10, cvScalar(0, 0, 255), 5, 8 );

				/* Draw a horizontal line indicating the lowest point of the
				 * fence. */
				y_max = vision_get_fence_bottom(bin_img);
				for( ii = 0; ii < lineWidth; ii++ ) {
					cvCircle( img, cvPoint(img->width / 2 + ii, y_max),
						2, cvScalar(100, 255, 20), 2 );
				}
				/* Set target offsets in network message. The subtractions are
				 * opposite of each other on purpose. This is so that they
				 * match the way the depth sensor and yaw sensor work. */
				msg.vision.data.front_x = fence_center - img->width / 2;
				msg.vision.data.front_y = y_max - img->height / 4;
            }
        } /* end TASK_FENCE */

        else if( task == TASK_GATE && (f_cam || diropen) ) {
        	/* Look for the gate. */

        	/* Default to fail detection until code is written. */
        	msg.vision.data.status = TASK_NOT_DETECTED;
		} /* end TASK_GATE */

		else if( task == TASK_BOXES && (b_cam || diropen) ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( b_cam );
			}

			/* Look for the boxes. */
			status = vision_find_boxes( img, boxes, squares, VISION_BOX,
				&msg.vision.data.bearing );

			/* If we get a positive status message, render the box and populate
			 * the network message. */
			if( status > 0 ) {
				/* Initialize the centroid sequence reader. */
				cvStartReadSeq( boxes, &reader1, 0 );
				/* Read two sequence elements at a time. */
				for( ii = 0; ii < boxes->total; ii += 2 ) {
					/* Read centroid x and y coordinates. */
					CV_READ_SEQ_ELEM( box_pt, reader1 );
					/* Draw the centroid as a circle. */
					cvCircle( img, box_pt,
						10, cvScalar(0, 0, 255), 5, 8 );
				}
				/* Initialize the vertex sequence reader. */
				cvStartReadSeq( squares, &reader2, 0 );
				/* Read four sequence elements at a time. */
				for( ii = 0; ii < squares->total; ii += 4 ) {
					/* Read vertex x and y coordinates. */
					CV_READ_SEQ_ELEM( pt[0], reader2 );
					CV_READ_SEQ_ELEM( pt[1], reader2 );
					CV_READ_SEQ_ELEM( pt[2], reader2 );
					CV_READ_SEQ_ELEM( pt[3], reader2 );
					/* Draw the square as a closed polyline. */
					cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0 );
				}
				/* Set target offsets in network message. */
				if( boxes->total > 0 ) {
					/* Set the detection status of vision */
		    		msg.vision.data.status = TASK_BOXES_DETECTED;

					/* Set the message variables to the center of the detected
					 * box. */
					msg.vision.data.box1_x = box_pt.x - img->width / 2;
					msg.vision.data.box1_y = box_pt.y - img->height / 2;
				}
				/* Clear out the sequences so that next time we only draw newly
				 * found squares and centroids. */
				cvClearSeq( boxes );
				cvClearSeq( squares );
			}
		} /* end TASK_BOXES */

		else if( task == TASK_SUITCASE && (b_cam || diropen) ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Get a new image. */
			if( !diropen ) {
				img = cvQueryFrame( f_cam );
			}

			/* Look for the suitcase. */
			status = vision_suitcase( img, boxes, squares );
			if( status > 0 ) {

				/* Initialize the centroid sequence reader. */
				cvStartReadSeq( boxes, &reader1, 0 );
				/* Read four sequence elements at a time. */
				for( ii = 0; ii < boxes->total; ii += 2 ) {
					/* Read centroid x and y coordinates. */
					CV_READ_SEQ_ELEM( box_pt, reader1 );
					/* Draw the centroid as a circle. */
					cvCircle( img, box_pt,
						10, cvScalar(0, 0, 255), 5, 8 );
				}
				/* Initialize the vertex sequence reader. */
				cvStartReadSeq( squares, &reader2, 0 );
				for( ii = 0; ii < squares->total; ii += 4 ) {
					/* Read vertex x and y coordinates. */
					CV_READ_SEQ_ELEM( pt[0], reader2 );
					CV_READ_SEQ_ELEM( pt[1], reader2 );
					CV_READ_SEQ_ELEM( pt[2], reader2 );
					CV_READ_SEQ_ELEM( pt[3], reader2 );
					/* Draw the square as a closed polyline. */
					cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0 );
				}
				/* Set target offsets in network message. */
				if( boxes->total > 0 ) {
					/* Set the detection status of vision */
		    		msg.vision.data.status = TASK_SUITCASE_DETECTED;

					msg.vision.data.suitcase_x = box_pt.x - img->width / 2;
					msg.vision.data.suitcase_y = box_pt.y - img->height / 2;
				}
				/* Clear out the sequences so that next time we only draw newly
				 * found squares and centroids. */
				cvClearSeq( boxes );
				cvClearSeq( squares );
			}
		} /* end TASK_SUITCASE */

		else {
			/* Clear the detection status */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Clear the bouy touch count */
			bouyTouchCount = 0;

			/* No mode or no valid cameras -- Simulate. */
			if( loop_counter % 100 == 0 ) {
				msg.vision.data.front_x = loop_counter;
				msg.vision.data.front_y = loop_counter;
			}
		}

        /* Get network data. */
        if( (cf.enable_server) && (server_fd > 0) ) {
            recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_VISION );
            if( recv_bytes > 0 ) {
                recv_buf[recv_bytes] = '\0';
                messages_decode( server_fd, recv_buf, &msg, recv_bytes );

				/* Update local variables using network variables. */
                task = msg.task.data.task;
				hsv_pipe.hL = msg.vsetting.data.pipe_hsv.hL;
				hsv_pipe.hH = msg.vsetting.data.pipe_hsv.hH;
				hsv_pipe.sL = msg.vsetting.data.pipe_hsv.sL;
				hsv_pipe.sH = msg.vsetting.data.pipe_hsv.sH;
				hsv_pipe.vL = msg.vsetting.data.pipe_hsv.vL;
				hsv_pipe.vH = msg.vsetting.data.pipe_hsv.vH;
				hsv_buoy.hL = msg.vsetting.data.buoy_hsv.hL;
				hsv_buoy.hH = msg.vsetting.data.buoy_hsv.hH;
				hsv_buoy.sL = msg.vsetting.data.buoy_hsv.sL;
				hsv_buoy.sH = msg.vsetting.data.buoy_hsv.sH;
				hsv_buoy.vL = msg.vsetting.data.buoy_hsv.vL;
				hsv_buoy.vH = msg.vsetting.data.buoy_hsv.vH;
				hsv_fence.hL = msg.vsetting.data.fence_hsv.hL;
				hsv_fence.hH = msg.vsetting.data.fence_hsv.hH;
				hsv_fence.sL = msg.vsetting.data.fence_hsv.sL;
				hsv_fence.sH = msg.vsetting.data.fence_hsv.sH;
				hsv_fence.vL = msg.vsetting.data.fence_hsv.vL;
				hsv_fence.vH = msg.vsetting.data.fence_hsv.vH;

                /* Force vision to look for the pipe no matter which pipe
			     * subtask we are currently searching for. */
				if( task == TASK_PIPE1 || task == TASK_PIPE2 ||
					task == TASK_PIPE3 || task == TASK_PIPE4 ) {
					task = TASK_PIPE;
				}
            }
        }

		/* Show the image in a window. */
		if( cf.vision_window ) {
			vision_mode = msg.vision.data.mode;
			/* OpenCV needs a little pause here. */
			if( cvWaitKey( 5 ) >= 0 );
			/* Determine which image to show in which window. */
			switch( vision_mode ) {
			case VISIOND_FCOLOR:
				cvShowImage( win, img );
				if( task != TASK_NONE ) {
					cvShowImage( binwin, bin_img );
				}
				break;
			case VISIOND_FBINARY:
				cvShowImage( win, bin_img );
				if( task != TASK_NONE ) {
					cvShowImage( binwin, bin_img );
				}
				break;
			case VISIOND_BCOLOR:
				cvShowImage( win, img );
				if( task != TASK_NONE ) {
					cvShowImage( binwin, bin_img );
				}
				break;
			case VISIOND_BBINARY:
				cvShowImage( win, bin_img );
				if( task != TASK_NONE ) {
					cvShowImage( binwin, bin_img );
				}
				break;
			case VISIOND_NONE:
				break;
			}
			/* Just show color video without any processing of it. */
			if( msg.task.data.task == TASK_NONE ) {
				switch( vision_mode ) {
				case VISIOND_FCOLOR:
					if( !diropen ) {
						img = cvQueryFrame( f_cam );
					}
					cvShowImage( win, img );
					break;
				case VISIOND_BCOLOR:
					if( !diropen ) {
						img = cvQueryFrame( b_cam );
					}
					cvShowImage( win, img );
					break;
				}
			}
		}

		/* Calculate frames per second. */
		nframes++;
		time1s =  fps_time.tv_sec;
		time1ms = fps_time.tv_usec;
		time2s =  fps_start.tv_sec;
		time2ms = fps_start.tv_usec;
		dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );

		if( dt > 1000000 ) {
			fps = (double)nframes * (1000000 / dt);
			gettimeofday( &fps_start, NULL );
			nframes = 0;
			msg.vision.data.fps = fps;
			//printf("MAIN: fps = %lf\n", fps);
		}

		/* Save an image from both the front and bottom cameras using timer. */
		if( cf.save_image_rate == 0 ) {
			/* Do nothing. */
		}
		else if( !diropen ) {
			/* Check save frame timer. */
			time1s =  save_time.tv_sec;
			time1ms = save_time.tv_usec;
			time2s =  save_start.tv_sec;
			time2ms = save_start.tv_usec;
			dt = util_calc_dt( &time1s, &time1ms, &time2s, &time2ms );
			if( dt > 1000000 / cf.save_image_rate ) {
				if( f_cam ) {
					img = cvQueryFrame( f_cam );
					vision_save_frame( img );
				}
				if( b_cam ) {
					img = cvQueryFrame( b_cam );
					vision_save_frame( img );
				}
				gettimeofday( &save_start, NULL );
			}
		}

		/* Check to see if we should load an image from disk for simulation. */
		if( cf.open_rate ) {
		}

        /* Check state of save frames and video messages. */
        if( msg.vsetting.data.save_fframe && f_cam ) {
            /* Save image to disk. */
			vision_save_frame( img );
			msg.vsetting.data.save_fframe = FALSE;
        }
        if( msg.vsetting.data.save_bframe && b_cam ) {
            /* Save image to disk. */
			vision_save_frame( img );
			msg.vsetting.data.save_bframe = FALSE;
        }
        if( msg.vsetting.data.save_fvideo && !saving_fvideo && f_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "stream/f20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.avi", ctime.tv_usec );
            fps = cvGetCaptureProperty( f_cam, CV_CAP_PROP_FPS );
            f_writer = cvCreateVideoWriter( write_time, CV_FOURCC('M', 'J', 'P', 'G'),
                fps, cvGetSize( img ), is_color );
            saving_fvideo = TRUE;
        }
        else if( !msg.vsetting.data.save_fvideo && saving_fvideo ) {
            cvReleaseVideoWriter( &f_writer );
            saving_fvideo = FALSE;
        }
		else if( msg.vsetting.data.save_fvideo && saving_fvideo ) {
			cvWriteFrame( f_writer, img );
		}
        if( msg.vsetting.data.save_bvideo && !saving_bvideo && b_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "stream/b20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.avi", ctime.tv_usec );
            fps = cvGetCaptureProperty( b_cam, CV_CAP_PROP_FPS );
            b_writer = cvCreateVideoWriter( write_time, CV_FOURCC('M', 'J', 'P', 'G'),
                fps, cvGetSize( img ), is_color );
            saving_bvideo = TRUE;
        }
        else if( !msg.vsetting.data.save_bvideo && saving_bvideo ) {
            cvReleaseVideoWriter( &b_writer );
            saving_bvideo = FALSE;
        }
		else if( msg.vsetting.data.save_bvideo && saving_bvideo ) {
			cvWriteFrame( b_writer, img );
		}

		/* Update timers. */
		gettimeofday( &fps_time, NULL );
		gettimeofday( &save_time, NULL );
		gettimeofday( &open_time, NULL );

		/* If image was loaded from file, release it. Don't need to release if
		 * image is captured from camera. */
		if( diropen ) {
			cvReleaseImage( &img );
		}
    }

    exit( 0 );
} /* end main() */
