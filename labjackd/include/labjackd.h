/**
 *  \file labjackd.h
 *  \brief Main program for labjack daemon.
 */

#ifndef _LABJACKD_H_
#define _LABJACKD_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "network.h"
#include "labjack.h"
#include "util.h"
#include "messages.h"
#include "parser.h"

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
#ifndef LABJACKD_FILENAME
#define LABJACKD_FILENAME "conf/labjackd.conf"
#endif /* LABJACKD_FILENAME */
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

#ifndef BATT_LIMITS
#define BATT_LIMITS
#define BATT1_THRESH	3
#define BATT1_MIN		10
#define BATT2_THRESH	3
#define BATT2_MIN		13.7
#endif /* BATT_LIMITS */

#ifndef PRESSURE_CALIBRATION
#define PRESSURE_CALIBRATION
#define PRESSURE_SLOPE			9.1566
#define PRESSURE_BIAS			(-4.3913)
#endif /* PRESSURE_CALIBRATION */


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
void labjackd_sigint( int signal );

//! Exit function for main program. Sets actuators to safe values and closes
//! all file descriptors. This function is called when SIGINT (ctrl-c) is
//! invoked.
void labjackd_exit( );

//! Main function for the labjackd program.
//! \param argc Number of command line arguments.
//! \param argv Array of command line arguments.
//! \return Always returns 0.
int main( int argc, char *argv[] );


#endif /* _LABJACKD_H_ */
