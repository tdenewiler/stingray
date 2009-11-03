/******************************************************************************
 *
 *  Title:        visiond.c
 *
 *  Description:  Main program for vision daemon.
 *
 *****************************************************************************/

#include "visiond.h"

/* Global file descriptors. Only global so that visiond_exit() can close them. */
int server_fd;
CvCapture *f_cam;
CvCapture *b_cam;
IplImage *bin_img;
IplImage *save_img;
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
} /// end visiond_sigint()


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
    
    /// Sleep to let things shut down properly.
    usleep( 200000 );

    /// Close the open file descriptors.
    if( server_fd > 0 ) {
        close( server_fd );
    }

    /// Close the cameras and windows.
    if( f_cam ) {
        cvReleaseCapture( &f_cam );
    }

    if( b_cam ) {
        cvReleaseCapture( &b_cam );
    }

    cvReleaseImage( &bin_img );
    if ( save_img ) {
    	cvReleaseImage( &save_img );
	}

    if ( dirp ) {
    	closedir( dirp );
	}

    printf("<OK>\n\n");
} /// end visiond_exit()


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
    /// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
    void( *exit_ptr )( void );
    exit_ptr = visiond_exit;
    atexit( exit_ptr );

    struct sigaction sigint_action;
    sigint_action.sa_handler = visiond_sigint;
    sigint_action.sa_flags = 0;
    sigaction( SIGINT, &sigint_action, NULL );

    /// Set up state variables and initialize them.
    int recv_bytes = 0;
    char recv_buf[MAX_MSG_SIZE];
    CONF_VARS cf;
    MSG_DATA msg;

    /// Set up image/window variables and initialize them.
    IplImage *img = NULL;
	const char *win = "Image";
	const char *binwin = "Binary";
	save_img = NULL;

	/// Set up video variables and initialize them.
	int saving_fvideo = FALSE;
    int saving_bvideo = FALSE;
    CvVideoWriter *f_writer = 0;
    CvVideoWriter *b_writer = 0;
    int is_color = TRUE;
    struct timeval ctime;
    struct tm ct;

    /// Set up file access variables and initialize them.
	struct dirent *dfile = NULL;
	int diropen = FALSE;
	char filename[STRING_SIZE * 2];
	char write_time[80] = {0};
    char curr_save_dir[STRING_SIZE];
    int vision_classified = 0;
    int vision_considered = 0;

	/// Set up timer variables and intialize them.
	TIMING timer_fps;
	TIMING timer_save;
	TIMING timer_open;
	int dt = 0;
	int nframes = 0;
    double fps = 0.0;

	/// Actually initialize timers.
	timing_set_timer(&timer_fps);
	timing_set_timer(&timer_save);
	timing_set_timer(&timer_open);

    printf( "MAIN: Starting Vision daemon ...\n" );

    /// Initialize variables.
    server_fd = -1;
    memset( &msg, 0, sizeof(MSG_DATA) );
	messages_init( &msg );

	/// Initialize task to none.
	msg.task.data.task = TASK_NONE;

    /// Parse command line arguments.
    parse_default_config( &cf );
    parse_cla( argc, argv, &cf, STINGRAY, (const char *)VISIOND_FILENAME );

    /// Initialize HSV message data to configuration values.
	visiond_msg_cf_init( &msg, &cf );

    /// Set up server.
    if( cf.enable_server ) {
        server_fd = net_server_setup( cf.server_port );
		if( server_fd > 0 ) {
			printf("MAIN: Server setup OK.\n");
		}
		else {
			printf("MAIN: WARNING!!! Server setup failed.\n");
		}
    }
    
    /// Make sure open image directory is formatted correctly.
	if ( cf.open_image_dir[strlen( cf.open_image_dir ) - 1] != '/' ) {
		cf.open_image_dir[strlen( cf.open_image_dir )] = '/';
	}
	
	/// Make sure save image directory is formatted correctly.
	if ( cf.save_image_dir[strlen( cf.save_image_dir ) - 1] != '/' ) {
		cf.save_image_dir[strlen( cf.save_image_dir )] = '/';
	}
	
	/// Make sure the hardcoded save sub directories exist. Create if not.
	strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
	if ( !opendir ( strncat( curr_save_dir, "post/", 5 ) ) ) {
		if ( mkdir( curr_save_dir, S_IRWXU ) == -1 ) {
			printf( "MAIN: WARNING!!! Cannot setup save post image directory.\n" );
			
			/// Turn off saving post processed image.
			cf.save_image_post = 0;
		}
	}
	
	strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
	if ( !opendir ( strncat( curr_save_dir, "front/", 6 ) ) ) {
		if ( mkdir( curr_save_dir, S_IRWXU ) == -1 ) {
			printf( "MAIN: WARNING!!! Cannot setup front image directory.\n" );
			
			/// Turn off saving post processed image.
			cf.save_image_front = 0;
		}
	}
	
	strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
	if ( !opendir ( strncat( curr_save_dir, "bottom/", 7 ) ) ) {
		if ( mkdir( curr_save_dir, S_IRWXU ) == -1 ) {
			printf( "MAIN: WARNING!!! Cannot setup bottom image directory.\n" );
			
			/// Turn off saving post processed image.
			cf.save_image_bottom = 0;
		}
	}

    /// Decide whether to use source images or cameras.
	if ( cf.open_image_rate && strcmp( cf.open_image_dir, "" ) != 0 ) {

		if ( ( diropen = visiond_open_image_init( cf.open_image_dir, filename ) ) ) {
    		img = cvLoadImage( filename );
    		bin_img = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
    		save_img = cvCreateImage( cvSize( img->width * 2, img->height ), IPL_DEPTH_8U, img->nChannels );
			printf( "MAIN: Load from file as camera OK.\n" );
		}
		else {
			exit( 0 );
		}
	}
	else if( cf.op_mode == 99 ) {
    	/// Special case for bad camera.
    	printf( "MAIN: Skipping camera opening because op mode = 99 in "
		   	"configuration file.\n" );
	}
	else {
		/// Using actual cameras.
    	printf( "MAIN: Using actual cameras for vision...\n" );

		/// Open front camera.
		f_cam = cvCaptureFromCAM( 0 );
		if( !f_cam ) {
			cvReleaseCapture( &f_cam );
			printf( "MAIN: WARNING!!! Could not open f_cam.\n" );
		}
		else {
			img = cvQueryFrame( f_cam );
			bin_img = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
			printf( "MAIN: Front camera opened OK.\n" );
		}

		/// Open bottom camera.
		b_cam = cvCaptureFromCAM( 1 );
		if( !b_cam ) {
			cvReleaseCapture( &b_cam );
			printf( "MAIN: WARNING!!! Could not open b_cam.\n" );
		}
		else {
			img = cvQueryFrame( b_cam );
			bin_img = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
			printf( "MAIN: Bottom camera opened OK.\n" );
		}
	}

    /// Create windows to display video if set in configuration file.
	if( cf.vision_window ) {
		cvNamedWindow( win, CV_WINDOW_AUTOSIZE );
		cvNamedWindow( binwin, CV_WINDOW_AUTOSIZE );
	}

	/// Show the image in a window.
	if( cf.vision_window ) {

		/// OpenCV needs a little pause here.
		if( cvWaitKey( 500 ) >= 0 );

		/// Actually show the image.
		if ( img )
		{
			cvShowImage( win, img );
		}
		if( bin_img )
		{
			cvShowImage( binwin, bin_img );
		}

		/// OpenCV needs a little pause here.
		if( cvWaitKey( 500 ) >= 0 );
	}
	
	/// Need a way to force task from file.
	if ( diropen ) {
		msg.task.data.task = visiond_translate_task( cf.vision_task );
	}

    printf( "MAIN: Vision server running now.\n" );

    /// Main loop.
    while( 1 ) {

    	/// Get network data.
        if( (cf.enable_server) && (server_fd > 0) ) {
            recv_bytes = net_server( server_fd, recv_buf, &msg, MODE_VISION );
            if( recv_bytes > 0 ) {
                recv_buf[recv_bytes] = '\0';
                messages_decode( server_fd, recv_buf, &msg, recv_bytes );

                /// Force vision to look for the pipe no matter which pipe
			    /// subtask we are currently searching for.
				if( msg.task.data.task == TASK_PIPE1 || msg.task.data.task == TASK_PIPE2 ||
					msg.task.data.task == TASK_PIPE3 || msg.task.data.task == TASK_PIPE4 ) {
					msg.task.data.task = TASK_PIPE;
				}
            }
        }

		/// Reset "THE" image to be null. This way we know if there is a new image or not.
		img = NULL;

    	/// Do vision processing based on task.
    	if( msg.task.data.task == TASK_NONE ) {
    		/// Do nothing and give cleared values.
    		msg.vision.data.front_x = 0;
    		msg.vision.data.front_y = 0;
    		msg.vision.data.bottom_x = 0;
    		msg.vision.data.bottom_y = 0.0;
		}
		else {
			/// Check if time to pull a new image.
			if( diropen && timing_check_period(&timer_open, cf.open_image_rate)) {
				/// Get the next file pointer.
				dfile = readdir( dirp );

				/// Find the next file by ignoring directories.
				while ( dfile != NULL && strstr( dfile->d_name, ".jpg" ) == NULL ) {
					/// Iterate to next file.
					dfile = readdir( dirp );
				}

				if( dfile != NULL ) {
					/// This is a file so use it.

					/// Load image here.
					strncpy( filename, cf.open_image_dir, STRING_SIZE );
					strncat( filename, dfile->d_name, STRING_SIZE );
					img = cvLoadImage( filename );
				}
				else {
					/// Finished the directory.
					printf( "MAIN: Finished loading from source directory.\n\n" );
					printf( "VISION RESULTS:\n" );
					printf( "  %d/%d ---> %f%%\n", vision_classified, vision_considered,
								((double)vision_classified/vision_considered) * 100 );
					exit( 0 );
				}

				/// Reset the open image timer.
				timing_set_timer(&timer_open);
			}
			else if ( f_cam && ( msg.task.data.task == TASK_GATE ||
								msg.task.data.task == TASK_BUOY ||
								msg.task.data.task == TASK_FENCE ) ) {
				img = cvQueryFrame( f_cam );
				cvFlip( img, img );
			}
			else if ( b_cam && ( msg.task.data.task == TASK_PIPE ||
								msg.task.data.task == TASK_BOXES ||
								msg.task.data.task == TASK_SUITCASE ) ) {
				img = cvQueryFrame( b_cam );
			}
		}

		/// Only handle the image if there is a valid one.
		if ( img != NULL ) {

			/// Process the image according to the task.
			if ( visiond_process_image( img, bin_img, &msg ) ) {
				vision_classified++;
			}
			vision_considered++;

			/// Show the image in a window.
			if( cf.vision_window ) {

				/// OpenCV needs a little pause here.
				if( cvWaitKey( 5 ) >= 0 );

				/// Actually show the image.
				if ( img )
				{
					cvShowImage( win, img );
				}
				if( bin_img )
				{
					cvShowImage( binwin, bin_img );
				}
			}

			/// Calculate frames per second.
			nframes++;
			if( timing_check_period(&timer_fps, 1.) ) {
				timing_get_dt(&timer_fps, &timer_fps);
				dt = timing_s2us(&timer_fps);
				fps = (double)nframes * (1000000 / dt);
				timing_set_timer(&timer_fps);
				nframes = 0;
				msg.vision.data.fps = fps;
			}

			if ( diropen ) {
				/// The processed images need to be saved before the live feeds.
				if ( cf.save_image_post ) {
					vision_concat_images( img, bin_img, save_img );
					strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
					vision_save_frame( save_img, strncat( curr_save_dir, "post/", 5 ) );
				}
			}

			/// If image was loaded from file, release it. Don't need to release if
		 	/// image is captured from camera.
			if( diropen ) {
				cvReleaseImage( &img );
			}
		}

		/// Save the front and/or bottom images. (CONFIG)
		if ( !diropen && timing_check_period(&timer_save, cf.save_image_rate) ) {
			if ( cf.save_image_front && f_cam ) {
				/// Save an image from the front camera.
				img = cvQueryFrame( f_cam );
				strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
				vision_save_frame( img, strncat( curr_save_dir, "front/", 6 ) );
			}

			if ( cf.save_image_bottom && b_cam ) {
				/// Save an image from the bottom camera.
				img = cvQueryFrame( b_cam );
				strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
				vision_save_frame( img, strncat( curr_save_dir, "bottom/", 7 ) );
			}

			timing_set_timer(&timer_save);
		}


		/// Save the front or bottom image. (GUI)
		if( msg.vsetting.data.save_fframe && f_cam ) {
			/// Save image to disk.
			img = cvQueryFrame( f_cam );
			strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
			vision_save_frame( img, strncat( curr_save_dir, "front/", 6 ) );
			msg.vsetting.data.save_fframe = FALSE;
		}
		if( msg.vsetting.data.save_bframe && b_cam ) {
			/// Save image to disk.
			img = cvQueryFrame( b_cam );
			strncpy( curr_save_dir, cf.save_image_dir, STRING_SIZE );
			vision_save_frame( img, strncat( curr_save_dir, "bottom/", 7 ) );
			msg.vsetting.data.save_bframe = FALSE;
		}

		/// Save the front or bottom video. (GUI)
		if( msg.vsetting.data.save_fvideo && !saving_fvideo && f_cam ) {
			/// Get a timestamp and use for filename.
			gettimeofday( &ctime, NULL );
			img = cvQueryFrame( f_cam );
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
			/// Get a timestamp and use for filename.
			gettimeofday( &ctime, NULL );
			img = cvQueryFrame( b_cam );
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
} /// end main()


/******************************************************************************
 *
 * Title:       int visiond_visiond_open_image_init( char *dir, char *filename )
 *
 * Description: Tests if we should and can load images from file.
 *
 *****************************************************************************/

int visiond_open_image_init( char *dir, char *filename )
{
	struct dirent *dfile = NULL;

	/// Load an images from disk.
	//printf( "visiond_open_image_init: Using source images from directory for vision...\n" );

	/// Open directory.
	if ( (dirp = opendir( dir ) ) ) {

		dfile = readdir( dirp );
		if ( dfile == NULL ) {
			printf( "visiond_open_image_init: WARNING!!! Image directory not valid.\n" );
			return FALSE;
		}
		else {
			/// Find the next file by ignoring directories.
			while ( dfile != NULL && strstr( dfile->d_name, ".jpg" ) == NULL ) {

				/// Iterate to next file.
				dfile = readdir( dirp );
			}

			/// Setup the images.
			if( dfile != NULL ) {
				/// This is a file so use it.

				/// Load image here.
				strncpy( filename, dir, STRING_SIZE );
				strncat( filename, dfile->d_name, STRING_SIZE );

				/// Close and Open the directory so we start from the beginning.
				closedir( dirp );
				if ( (dirp = opendir( dir ) ) ) {
					/// Reopen OK.
					//printf( "visiond_open_image_init: Image source directory OK.\n" );
					return TRUE;
				}
				else {
					/// Reopen failed.
					printf( "visiond_open_image_init: WARNING!!! Source directory reopen failed.\n" );
					return FALSE;
				}
			}
			else {
				/// No images so exit the module.
				printf( "visiond_open_image_init: WARNING!!! No image files in directory.\n" );
				return FALSE;
			}
		}
	}
	else {
		/// Directory was not opened so exit the module.
		printf( "visiond_open_image_init: WARNING!!! Image directory '%s' not opened.\n", dir );
		return FALSE;
	}

	return TRUE;
} /// end visiond_open_image_init()


/******************************************************************************
 *
 * Title:       int visiond_translate_task( char *task_name )
 *
 * Description: Translates the task name to the task ID.
 *
 * Input:      	task_name: The name of the task.
 *
 * Output:      None.
 *
 *****************************************************************************/
int visiond_translate_task( char *task_name )
{
	if ( strcmp( task_name, "buoy" ) == 0 ) {
		return TASK_BUOY;
	}
	else if ( strcmp( task_name, "gate" ) == 0 ) {
		return TASK_GATE;
	}
	else if ( strcmp( task_name, "fence" ) == 0 ) {
		return TASK_FENCE;
	}
	else if ( strcmp( task_name, "pipe" ) == 0 ) {
		return TASK_PIPE;
	}
	else if ( strcmp( task_name, "boxes" ) == 0 ) {
		return TASK_BOXES;
	}
	else if ( strcmp( task_name, "suitcase" ) == 0 ) {
		return TASK_SUITCASE;
	}

	return TASK_NONE;
} /// end visiond_translate_task()

/******************************************************************************
 *
 * Title:       int visiond_process_image( IplImage *img, IplImage *bin_img,
 * 									MSG_DATA *msg )
 *
 * Description: Processes the given image based on the current task. Fills the
 * 				given binary image file with the result of processing and
 * 				draws a circle in the object spot on the given image.
 *
 * Input:      	img: The given image to be processed.
 * 				bin_img: The place to put the resulting binary image.
 * 				msg: The message structure that holds relevant information
 * 					including hsv values, vision_angle, and task.
 *
 * Output:      Draws cirlce on img if object found and draws bin_img.
 *
 *****************************************************************************/
int visiond_process_image( IplImage *img, IplImage *bin_img, MSG_DATA *msg )
{
	int status = -1;

	/// BUOY VARIABLES.
	int dotx = -1;
    int doty = -1;
	int xrot = -1;
    int yrot = -1;

    double angleFrontCam = VISIOND_FRONT_CAM_ANGLE_OFFSET * M_PI /  180;

    /// PIPE VARIABLES.
    /// Temporary variable to make it easier to switch between using HSV
	/// olding or boxes to try and find pipe. HSV = 1, Boxes = 2.
	int pipe_type = VISION_PIPE_HSV;
	int pipex = -10000;
	int pipey = -10000;
	double bearing = -10000;
	int ii = 0;

	/// BOXES VARIABLES.
	/// Variables to hold box centroid sequence and vertex sequence.
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

	/// FENCE VARIABLES.
    int fence_center = img->width / 2;
	int y_max = -1;
	int lineWidth = 75;


	if ( msg->task.data.task == TASK_GATE ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the gate .
		status = vision_find_gate( &dotx, &doty, msg->vsetting.data.vision_angle, img,
			bin_img, &msg->vsetting.data.buoy_hsv );
		if( status == 1 || status == 2 ) {
			/// We have detected the gate.
			//printf("MAIN: Gate Status = %d\n", status);
			msg->vision.data.status = TASK_GATE_DETECTED;

			/// The subtractions are opposite of each other on purpose. This is 
			/// so that they match the way the depth sensor and yaw sensor work.
			msg->vision.data.front_x = dotx - (img->width / 2);
			msg->vision.data.front_y = (img->height / 2) - doty;

			xrot =  msg->vision.data.front_x * cos( angleFrontCam ) +
					msg->vision.data.front_y * sin( angleFrontCam );
			yrot = -msg->vision.data.front_x * sin( angleFrontCam ) +
					msg->vision.data.front_y * cos( angleFrontCam );

			msg->vision.data.front_x = xrot;
			msg->vision.data.front_y = yrot;

			/// Draw a circle at the centroid location.
			cvCircle( img, cvPoint(dotx, doty),
				10, cvScalar(0, 255, 0), 5, 8 );

			if( status == 2 ) {
				msg->vision.data.status = TASK_GATE_CLEARED;
			}
		}
	} /// end TASK_GATE

	else if ( msg->task.data.task == TASK_BUOY ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the buoy.
		status = vision_find_dot( &dotx, &doty, msg->vsetting.data.vision_angle, img,
			bin_img, &msg->vsetting.data.buoy_hsv );

		if( status == 1 || status == 2 ) {
			/// We have detected the buoy.
			//printf("MAIN: Bouy Status = %d\n", status);
			msg->vision.data.status = TASK_BUOY_DETECTED;

			/// The subtractions are opposite of each other on purpose. This is 
			/// so that they match the way the depth sensor and yaw sensor work.
			msg->vision.data.front_x = dotx - (img->width / 2);
			msg->vision.data.front_y = (img->height / 2) - doty;

			xrot =  msg->vision.data.front_x * cos( angleFrontCam ) +
					msg->vision.data.front_y * sin( angleFrontCam );
			yrot = -msg->vision.data.front_x * sin( angleFrontCam ) +
					msg->vision.data.front_y * cos( angleFrontCam );

			msg->vision.data.front_x = xrot;
			msg->vision.data.front_y = yrot;

			/// Draw a circle at the centroid location.
			cvCircle( img, cvPoint(dotx, doty),
				10, cvScalar(0, 255, 0), 5, 8 );

			if( status == 2 ) {
				msg->vision.data.status = TASK_BUOY_TOUCHED;
			}
		}
	} /// end TASK_BUOY

	else if ( msg->task.data.task == TASK_PIPE && pipe_type == VISION_PIPE_HSV ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the pipe.
		status = vision_find_pipe( &pipex, &pipey, &bearing, img, bin_img,
			&msg->vsetting.data.pipe_hsv );

		/// If we get a positive status message, render the box and populate
		/// the network message.
		if( status == 1 ) {
			/// Set the detection status of vision.
			//printf("MAIN Pipe Status = %d\n", status);
			msg->vision.data.status = TASK_PIPE_DETECTED;

			msg->vision.data.bearing = bearing;
			msg->vision.data.bottom_x = pipex - img->width / 2;
			msg->vision.data.bottom_y = pipey - img->height / 2;

			/// Draw a circle at the centroid location.
			cvCircle( img, cvPoint(pipex, pipey),
				10, cvScalar(255, 0, 0), 5, 8 );

			/// Draw the bearing if it is there.
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
	} /// end TASK_PIPE -- hsv

	else if ( msg->task.data.task == TASK_PIPE && pipe_type == VISION_PIPE_BOX ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the pipe.
		status = vision_find_boxes( img, boxes, squares, VISION_PIPE,
			&msg->vision.data.bearing );

		/// If we get a positive status message, render the box
		/// and populate the network message.
		if( status > 0 ) {
			/// Initialize the centroid sequence reader.
			cvStartReadSeq( boxes, &reader1, 0 );
			
			/// Read four sequence elements at a time.
			for( ii = 0; ii < boxes->total; ii += 2 ) {
				/// Read centroid x and y coordinates.
				CV_READ_SEQ_ELEM( box_pt, reader1 );
				
				/// Draw the centroid as a circle.
				cvCircle( img, box_pt,
					10, cvScalar(0, 0, 255), 5, 8 );
			}
			
			/// Initialize the vertex sequence reader.
			cvStartReadSeq( squares, &reader2, 0 );
			for( ii = 0; ii < squares->total; ii += 4 ) {
				/// Read vertex x and y coordinates.
				CV_READ_SEQ_ELEM( pt[0], reader2 );
				CV_READ_SEQ_ELEM( pt[1], reader2 );
				CV_READ_SEQ_ELEM( pt[2], reader2 );
				CV_READ_SEQ_ELEM( pt[3], reader2 );
				
				/// Draw the square as a closed polyline.
				cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0),
					3, CV_AA, 0 );
			}
			
			/// Set target offsets in network message.
			if( boxes->total > 0 ) {
				/// Set the detection status of vision.
				msg->vision.data.status = TASK_PIPE_DETECTED;

				msg->vision.data.bearing = 0; //// !!! TODO: Fix this value !!!
				msg->vision.data.bottom_x = box_pt.x - img->width / 2;
				msg->vision.data.bottom_y = box_pt.y - img->height / 2;
			}
			/// Clear out the sequences so that next time we only draw newly
			/// found squares and centroids.
			cvClearSeq( boxes );
			cvClearSeq( squares );
		}
	} /// end TASK_PIPE -- boxes

	else if ( msg->task.data.task == TASK_FENCE ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the fence.
		status = vision_find_fence( &fence_center, &y_max, img,	bin_img,
			&msg->vsetting.data.fence_hsv );
			
		if( status == 1 ) {
			/// Set the detection status of vision.
			msg->vision.data.status = TASK_FENCE_DETECTED;

			/// Draw a circle at the centroid.
			cvCircle( img, cvPoint(fence_center, y_max),
				10, cvScalar(0, 0, 255), 5, 8 );

			/// Draw a horizontal line indicating the lowest point of the fence.
			y_max = vision_get_fence_bottom(bin_img);
			for( ii = 0; ii < lineWidth; ii++ ) {
				cvCircle( img, cvPoint(img->width / 2 + ii, y_max),
					2, cvScalar(100, 255, 20), 2 );
			}
			
			/// Set target offsets in network message. The subtractions are
			/// opposite of each other on purpose. This is so that they
			/// match the way the depth sensor and yaw sensor work.
			msg->vision.data.front_x = fence_center - img->width / 2;
			msg->vision.data.front_y = y_max - img->height / 4;
		}
	} /// end TASK_FENCE

	else if ( msg->task.data.task == TASK_BOXES ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the boxes.
		status = vision_find_boxes( img, boxes, squares, VISION_BOX,
			&msg->vision.data.bearing );

		/// If we get a positive status message, render the box and populate
		/// the network message.
		if( status > 0 ) {
			/// Initialize the centroid sequence reader.
			cvStartReadSeq( boxes, &reader1, 0 );
			
			/// Read two sequence elements at a time.
			for( ii = 0; ii < boxes->total; ii += 2 ) {
				/// Read centroid x and y coordinates.
				CV_READ_SEQ_ELEM( box_pt, reader1 );
				
				/// Draw the centroid as a circle.
				cvCircle( img, box_pt,
					10, cvScalar(0, 0, 255), 5, 8 );
			}
			
			/// Initialize the vertex sequence reader.
			cvStartReadSeq( squares, &reader2, 0 );
			
			/// Read four sequence elements at a time.
			for( ii = 0; ii < squares->total; ii += 4 ) {
				/// Read vertex x and y coordinates.
				CV_READ_SEQ_ELEM( pt[0], reader2 );
				CV_READ_SEQ_ELEM( pt[1], reader2 );
				CV_READ_SEQ_ELEM( pt[2], reader2 );
				CV_READ_SEQ_ELEM( pt[3], reader2 );
				
				/// Draw the square as a closed polyline.
				cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0 );
			}
			
			/// Set target offsets in network message.
			if( boxes->total > 0 ) {
				/// Set the detection status of vision.
				msg->vision.data.status = TASK_BOXES_DETECTED;

				/// Set the message variables to the center of the detected box.
				msg->vision.data.box1_x = box_pt.x - img->width / 2;
				msg->vision.data.box1_y = box_pt.y - img->height / 2;
			}
			/// Clear out the sequences so that next time we only draw newly
			/// found squares and centroids.
			cvClearSeq( boxes );
			cvClearSeq( squares );
		}
	} /// end TASK_BOXES

	else if ( msg->task.data.task == TASK_SUITCASE ) {
		/// Set to not detected to start and reset if we get a hit.
		msg->vision.data.status = TASK_NOT_DETECTED;

		/// Look for the suitcase.
		status = vision_suitcase( img, boxes, squares );
		if( status > 0 ) {

			/// Initialize the centroid sequence reader.
			cvStartReadSeq( boxes, &reader1, 0 );
			
			/// Read four sequence elements at a time.
			for( ii = 0; ii < boxes->total; ii += 2 ) {
				/// Read centroid x and y coordinates.
				CV_READ_SEQ_ELEM( box_pt, reader1 );
				
				/// Draw the centroid as a circle.
				cvCircle( img, box_pt,
					10, cvScalar(0, 0, 255), 5, 8 );
			}
			
			/// Initialize the vertex sequence reader.
			cvStartReadSeq( squares, &reader2, 0 );
			for( ii = 0; ii < squares->total; ii += 4 ) {
				/// Read vertex x and y coordinates.
				CV_READ_SEQ_ELEM( pt[0], reader2 );
				CV_READ_SEQ_ELEM( pt[1], reader2 );
				CV_READ_SEQ_ELEM( pt[2], reader2 );
				CV_READ_SEQ_ELEM( pt[3], reader2 );
				
				/// Draw the square as a closed polyline.
				cvPolyLine( img, &rect, &count, 1, 1, CV_RGB(0, 255, 0), 3, CV_AA, 0 );
			}
			
			/// Set target offsets in network message.
			if( boxes->total > 0 ) {
				/// Set the detection status of vision.
				msg->vision.data.status = TASK_SUITCASE_DETECTED;

				msg->vision.data.suitcase_x = box_pt.x - img->width / 2;
				msg->vision.data.suitcase_y = box_pt.y - img->height / 2;
			}
			/// Clear out the sequences so that next time we only draw newly
			/// found squares and centroids.
			cvClearSeq( boxes );
			cvClearSeq( squares );
		}
	} /// end TASK_SUITCASE

	else {
		/// Clear the detection status.
		msg->vision.data.status = TASK_NOT_DETECTED;

	}
	
	if ( msg->vision.data.status == TASK_GATE_DETECTED || msg->vision.data.status == TASK_BUOY_DETECTED ||
			msg->vision.data.status == TASK_PIPE_DETECTED || msg->vision.data.status == TASK_FENCE_DETECTED ||
			msg->vision.data.status == TASK_BOXES_DETECTED || msg->vision.data.status == TASK_SUITCASE_DETECTED ) {
		return 1;
	}
	
	return 0;
} /// end visiond_process_image()


/******************************************************************************
 *
 * Title:       int visiond_msg_cf_init( MSG_DATA *msg, CONF_VARS *cf )
 *
 * Description: Initializes the msg variables using config file variables.
 *
 *****************************************************************************/

int visiond_msg_cf_init( MSG_DATA *msg, CONF_VARS *cf )
{
    msg->vsetting.data.pipe_hsv.hL  = cf->pipe_hL;
    msg->vsetting.data.pipe_hsv.hH  = cf->pipe_hH;
    msg->vsetting.data.pipe_hsv.sL  = cf->pipe_sL;
    msg->vsetting.data.pipe_hsv.sH  = cf->pipe_sH;
    msg->vsetting.data.pipe_hsv.vL  = cf->pipe_vL;
    msg->vsetting.data.pipe_hsv.vH  = cf->pipe_vH;
    msg->vsetting.data.buoy_hsv.hL  = cf->buoy_hL;
    msg->vsetting.data.buoy_hsv.hH  = cf->buoy_hH;
    msg->vsetting.data.buoy_hsv.sL  = cf->buoy_sL;
    msg->vsetting.data.buoy_hsv.sH  = cf->buoy_sH;
    msg->vsetting.data.buoy_hsv.vL  = cf->buoy_vL;
    msg->vsetting.data.buoy_hsv.vH  = cf->buoy_vH;
    msg->vsetting.data.fence_hsv.hL = cf->fence_hL;
    msg->vsetting.data.fence_hsv.hH = cf->fence_hH;
    msg->vsetting.data.fence_hsv.sL = cf->fence_sL;
    msg->vsetting.data.fence_hsv.sH = cf->fence_sH;
    msg->vsetting.data.fence_hsv.vL = cf->fence_vL;
    msg->vsetting.data.fence_hsv.vH = cf->fence_vH;
    msg->vsetting.data.vision_angle = cf->vision_angle;

	return TRUE;
} /// end visiond_msg_cf_init()
