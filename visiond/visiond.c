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
#include "task.h"


/* Global file descriptors. Only global so that vision_exit() can close them. */
int server_fd;
CvCapture *f_cam;
CvCapture *b_cam;
IplImage *bin_img;


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
    CvVideoWriter *f_writer = 0;
    CvVideoWriter *b_writer = 0;
    int is_color = TRUE;
    double fps = 0.0;
    struct timeval ctime;
    struct tm ct;
    char write_time[80] = {0};
	int vision_mode = VISIOND_NONE;
	int task = TASK_NONE;

	/* Variables to hold box centroid sequence and vertex sequence. */
	CvMemStorage *storage1 = 0;
	storage1 = cvCreateMemStorage(0);
	CvSeq *boxes = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage1 );
	CvSeq *circles = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage1 );
	float *circle_pt = NULL;
	CvSeqReader reader1;
	CvMemStorage *storage2 = 0;
	storage2 = cvCreateMemStorage(0);
	CvSeq *squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage2 );
	CvSeqReader reader2;
	CvPoint box_pt;
	CvPoint pt[4];
	CvPoint *rect = pt;
	int count = 4;

    printf( "MAIN: Starting Vision daemon ...\n" );

    /* Initialize variables. */
    server_fd = -1;
    memset( &msg, 0, sizeof( MSG_DATA ) );
	messages_init( &msg );

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

    /* Need to have a config about what cameras if any to open */
    if( cf.op_mode == 99 ) {
    	/* Special case for bad camera. */
    	printf("MAIN: Skipping camera opening because op mode = 99 in configuration file.\n");
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
			//img = cvLoadImage( "../../../../pics/stingrayBackup/images/b20090717_135344.719209.jpg" );
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

    /* Create windows to display video if set in configuration file. */
	if( cf.vision_window ) {
		cvNamedWindow( win, CV_WINDOW_AUTOSIZE );
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

    	/* Do vision processing based on task */
    	if( task == TASK_NONE ) {
    		/* Do nothing and give cleared values. */
    		msg.vision.data.front_x = 0;
    		msg.vision.data.front_y = 0;
    		msg.vision.data.bottom_x = 0;
    		msg.vision.data.bottom_y = 0.0;
		} /* end TASK_NONE */

        else if( task == TASK_BUOY && f_cam ) {
			/* Look for the buoy. */
			status = vision_find_dot( &dotx, &doty,
					cf.vision_angle, f_cam, img, bin_img,
					msg.vsetting.data.buoy_hsv.hL,
					msg.vsetting.data.buoy_hsv.hH,
					msg.vsetting.data.buoy_hsv.sL,
					msg.vsetting.data.buoy_hsv.sH,
					msg.vsetting.data.buoy_hsv.vL,
					msg.vsetting.data.buoy_hsv.vH );
			if( status == 1 ) {
				/* Set the detection status of vision */
				msg.vision.data.status = TASK_BOUY_DETECTED;

				/* The subtractions are opposite of each other on purpose. This
				 * is so that they match the way the depth sensor and yaw sensor
				 * work. */
				msg.vision.data.front_x = dotx - (img->width / 2);
				msg.vision.data.front_y = (img->height / 2) - doty;

				/* Draw a circle at the centroid location. */
				cvCircle( img, cvPoint(dotx, doty),
					10, cvScalar(255, 0, 0), 5, 8 );

				/* Requires some sore of exit criteria. */
			}

			/* Try to detect circles using cvHoughCircles(). */
			//status = vision_find_circle( f_cam, img, circles );
			//if( status > 0 ) {
				//for( ii = 0; ii < circles->total; ii++ ) {
					//circle_pt = (float *)cvGetSeqElem( circles, ii );
					//cvCircle( img, cvPoint(cvRound(circle_pt[0]),
						//cvRound(circle_pt[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
					//cvCircle( img, cvPoint(cvRound(circle_pt[0]),
						//cvRound(circle_pt[1])), cvRound(circle_pt[2]),
						//CV_RGB(255,0,0), 3, 8, 0 );
				//}
			//}
			else {
				/* No positive detection. */
				msg.vision.data.status = TASK_NOT_DETECTED;
			}
		} /* end TASK_BUOY */

		else if( task == TASK_PIPE && f_cam ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Look for the pipe. */
			status = vision_find_boxes( f_cam, img, boxes, squares, VISION_PIPE,
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
					cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0 );
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
		        else {
				/* No positive detection. */
				msg.vision.data.status = TASK_NOT_DETECTED;
			}
		} /* end TASK_PIPE */

		else if( task == TASK_FENCE && f_cam ) {

			/* Initialize to no positive detection. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Look for the fence. */
            status = vision_find_fence( &fence_center, &y_max, f_cam, img, bin_img,
                    msg.vsetting.data.fence_hsv.hL,
                    msg.vsetting.data.fence_hsv.hH,
                    msg.vsetting.data.fence_hsv.sL,
                    msg.vsetting.data.fence_hsv.sH,
                    msg.vsetting.data.fence_hsv.vL,
                    msg.vsetting.data.fence_hsv.vH );
            if( status == 1 ) {
            	/* Set the detection status of vision. */
				msg.vision.data.status = TASK_FENCE_DETECTED;

				/* Draw a circle at the centroid. */
				cvCircle( img, cvPoint(fence_center, img->height / 2),
					10, cvScalar(0, 0, 255), 5, 8 );
				/* Draw a horizontal line indicating the lowest point of the
				 * fence. */
				for( ii = 0; ii < lineWidth; ii++ ) {
					cvCircle( img, cvPoint(img->width / 2 + ii, y_max),
						2, cvScalar(0, 255, 0), 2 );
				}
				/* Set target offsets in network message. */
				/* The subtractions are opposite of each other on purpose. This
				 * is so that they match the way the depth sensor and yaw sensor
				 * work. */
				msg.vision.data.front_x = fence_center - img->width / 2;
				msg.vision.data.front_y = y_max - img->height / 4;
            }

        } /* end TASK_FENCE */

        else if( task == TASK_GATE && f_cam ) {
        	/* Look for the gate */

        	/* Default to fail detection until code is written. */
        	msg.vision.data.status = TASK_NOT_DETECTED;
		} /* end TASK_GATE */

		else if( task == TASK_BOXES && b_cam ) {
			/* Set to not detected to start and reset if we get a hit. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Look for the boxes. */
			status = vision_find_boxes( b_cam, img, boxes, squares, VISION_BOX,
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
					cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0 );
				}
				/* Set target offsets in network message. */
				if( boxes->total > 0 ) {
					/* Set the detection status of vision */
		    		msg.vision.data.status = TASK_BOXES_DETECTED;

					msg.vision.data.box1_x = box_pt.x - img->width / 2;
					msg.vision.data.box1_y = box_pt.y - img->height / 2;
				}
				/* Clear out the sequences so that next time we only draw newly
				 * found squares and centroids. */
				cvClearSeq( boxes );
				cvClearSeq( squares );
			}

		} /* end TASK_BOXES */

		else if( task == TASK_SUITCASE && b_cam ) {

			/* No positive detection. */
			msg.vision.data.status = TASK_NOT_DETECTED;

			/* Look for the suitcase. */
			status = vision_suitcase( b_cam, img, boxes, squares );
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

                task = msg.task.data.task;

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
				break;
			case VISIOND_FBINARY:
				cvShowImage( win, bin_img );
				break;
			case VISIOND_BCOLOR:
				cvShowImage( win, img );
				break;
			case VISIOND_BBINARY:
				cvShowImage( win, bin_img );
				break;
			case VISIOND_NONE:
				break;
			}
			if( msg.task.data.task == TASK_NONE ) {
				switch( vision_mode ) {
				case VISIOND_FCOLOR:
					img = cvQueryFrame( f_cam );
					cvShowImage( win, img );
					break;
				case VISIOND_BCOLOR:
					img = cvQueryFrame( b_cam );
					cvShowImage( win, img );
					break;
				}
			}
		}

        /* Check state of save frames and video messages. */
        if( msg.vsetting.data.save_fframe && f_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "images/f20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.jpg", ctime.tv_usec );
            cvSaveImage( write_time, img );
            msg.vsetting.data.save_fframe = FALSE;
        }
        if( msg.vsetting.data.save_bframe && b_cam ) {
            /* Get a timestamp and use for filename. */
            gettimeofday( &ctime, NULL );
            ct = *( localtime ((const time_t*) &ctime.tv_sec) );
            strftime( write_time, sizeof(write_time), "images/b20%y%m%d_%H%M%S", &ct);
            snprintf( write_time + strlen(write_time),
            		strlen(write_time), ".%03ld.jpg", ctime.tv_usec );
            cvSaveImage( write_time, img );
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
    }

    exit( 0 );
} /* end main() */
