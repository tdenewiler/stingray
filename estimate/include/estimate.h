/**
 *  \file estimate.h
 *  \brief Main program for running estimation and system identification.
 */

#ifndef _ESTIMATE_H_
#define _ESTIMATE_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "network.h"
#include "parser.h"
#include "msgtypes.h"
#include "timing.h"
#include "sysid.h"


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

/** @name Default filename for the configuration file. */
//@{
#ifndef ESTIMATE_FILENAME
#define ESTIMATE_FILENAME "../../conf/estimate.conf"
#endif /* ESTIMATE_FILENAME */
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

#ifndef ESTIMATE_TYPES
#define INPUT_STEP	1
#define INPUT_PRB	2
#define INPUT_RAMP	3
#endif /* ESTIMATE_TYPES */


/******************************
 *
 * #defines
 *
 *****************************/

/******************************
 *
 * Data types
 *
 *****************************/

/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Called when SIGINT (ctrl-c) is invoked.
//! \param signal The SIGINT signal.
void estimate_sigint( int signal );

//! Exit function for main program. Sets actuators to safe values and closes
//! all file descriptors. This function is called when SIGINT (ctrl-c) is
//! invoked.
void estimate_exit( );

//! Main function for the program.
//! \param argc Number of command line arguments.
//! \param argv Array of command line arguments.
//! \return Always returns 0.
int main( int argc, char *argv[] );

#endif /* _ESTIMATE_H_ */
