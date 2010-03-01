/**
 *  \file joydrive.h
 *  \brief Main program for joydrive.
 */

#ifndef _JOYDRIVE_H_
#define _JOYDRIVE_H_

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
#ifndef JOYDRIVE_FILENAME
#define JOYDRIVE_FILENAME "../../conf/joydrive.conf"
#endif /* JOYDRIVE_FILENAME */
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

/* Joy axis defines */
#define JOYDRIVE_AXIS_MOVE_1 0x7FFF /* +fx , -fy */
#define JOYDRIVE_AXIS_MOVE_2 0x8001 /* -fx , +fy */


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
void joydrive_sigint( int signal );

//! Exit function for main program. Sets actuators to safe values and closes
//! all file descriptors. This function is called when SIGINT (ctrl-c) is
//! invoked.
void joydrive_exit( );

//! Main function for the joydrive program.
//! \param argc Number of command line arguments.
//! \param argv Array of command line arguments.
//! \return Always returns 0.
int main( int argc, char *argv[] );


#endif /* _JOYDRIVE_H_ */
