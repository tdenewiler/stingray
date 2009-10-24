/**
 *  \file visiond.h
 *  \brief Main program for vision daemon.
 */

#ifndef _VISIOND_H_
#define _VISIOND_H_

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

#include "vision.h"
#include "messages.h"
#include "network.h"
#include "parser.h"
#include "util.h"
#include "microstrain.h"
#include "labjack.h"
#include "task.h"
#include "timing.h"

/******************************
**
** #defines
**
******************************/

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

/** @name Default filename for the vision configuration file. */
//@{
#ifndef VISIOND_FILENAME
#define VISIOND_FILENAME "../../conf/visiond.conf"
#endif /* VISIOND_FILENAME */
//@}

#ifndef STRING_SIZE
#define STRING_SIZE 64
#endif /* STRING_SIZE */

#ifndef MAX_IP_LEN
#define MAX_IP_LEN 15
#endif /* MAX_IP_LEN */

#ifndef MAX_PORT_LEN
#define MAX_PORT_LEN 8
#endif /* MAX_PORT_LEN */

#ifndef VISIOND_MODES
#define VISIOND_MODES	1
#define VISIOND_FBINARY	2
#define VISIOND_FCOLOR	3
#define VISIOND_BBINARY	4
#define VISIOND_BCOLOR	5
#define VISIOND_NONE	6
#endif /* VISIOND_MODES */

#define VISIOND_FRONT_CAM_ANGLE_OFFSET  (8)


/******************************
**
** Data types
**
******************************/



/******************************
**
** Function prototypes
**
******************************/

//! This function is called when SIGINT (ctrl-c) is invoked.
//! \param signal The SIGINT signal.
void visiond_sigint( int signal );

//! Exit function for main program. Sets actuators to safe values and closes
//! all file descriptors. This function is called when SIGINT (ctrl-c) is
//! invoked.
void visiond_exit( );

//! Main function for the uuv program.
//! \param argc Number of command line arguments.
//! \param argv Array of command line arguments.
//! \return Always returns 0.
int main( int argc, char *argv[] );

//! Function to set up the open image directory.
//! \param dir The directory to load images from.
//! \param filename Resulting first file name.
//! \return Returns TRUE if successful and FALSE if failure.
int visiond_open_image_init( char *dir, char *filename );

//! Translates the task name to the task ID.
//! \param task_name The name of the task to be translated.
//! \return The task ID.
int visiond_translate_task( char *task_name );

//! Processes the given image based on the current task.
//! \param img The given image to be processed.
//! \param bin_img The place to put the resulting binary image.
//! \param msg The message structure that holds relevant information
//! \return The task ID.
int visiond_process_image( IplImage *img, IplImage *bin_img, MSG_DATA *msg );

//! Initializes the msg variables using config file variables.
//! \param msg The message variables.
//! \param cf The configuration file variables.
//! \return TRUE.
int visiond_msg_cf_init( MSG_DATA *msg, CONF_VARS *cf );


#endif /* _VISIOND_H_ */
