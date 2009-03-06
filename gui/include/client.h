/**
 *  \file client.h
 *  \brief Simple client program for testing UUV API.
 */

#ifndef _CLIENT_H_
#define _CLIENT_H_


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

#ifndef STRING_SIZE
#define STRING_SIZE 64
#endif /* STRING_SIZE */

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 65536
#endif /* MAX_MSG_SIZE */

/** @name Defualt filename for the client configuration file. */
//@{
#ifndef CLIENT_FILENAME
#define CLIENT_FILENAME "../../conf/gui.conf"
#endif /* CLIENT_FILENAME */
//@}

#ifndef SSC_SYNC
#define SSC_SYNC 255
#endif /* SSC_SYNC */


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

//! Main function for the client program.
//! \param argc The number of command line arguments.
//! \param argv An array containing the command line arguments.
//! \return Always returns 0.
int main( int argc, char *argv[] );


#endif /* _CLIENT_H_ */

