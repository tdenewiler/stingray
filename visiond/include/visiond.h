/**
 *  \file visiond.h
 *  \brief Main program for vision daemon.
 */

#ifndef _VISIOND_H_
#define _VISIOND_H_

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


#endif /* _VISIOND_H_ */
