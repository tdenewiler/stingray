/**
 *  \file nav.h
 *  \brief Main program for Stingray UUV.
 */

#ifndef _NAV_H_
#define _NAV_H_

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

/** @name Default filename for the uuv configuration file. */
//@{
#ifndef STINGRAY_FILENAME
#define STINGRAY_FILENAME "../../conf/nav.conf"
#endif /* STINGRAY_FILENAME */
//@}

/** @name Serial number of Microstrain IMU. */
//@{
#ifndef MSTRAIN_SERIAL
#define MSTRAIN_SERIAL 2104
#endif /* MSTRAIN_SERIAL */
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

#ifndef SSA_SLEEP
#define SSA_SLEEP 500000
#endif /* SSA_SLEEP */


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
void nav_sigint( int signal );

//! Exit function for main program. Sets actuators to safe values and closes
//! all file descriptors. This function is called when SIGINT (ctrl-c) is
//! invoked.
void nav_exit( );

//! Main function for the uuv program.
//! \param argc Number of command line arguments.
//! \param argv Array of command line arguments.
//! \return Always returns 0.
int main( int argc, char *argv[] );


float calc_yaw_avg( float curr_yaw );

#endif /* _NAV_H_ */
