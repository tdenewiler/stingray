/******************************************************************************
 *
 *  Title:        serial.c
 *
 *  Description:  Handles setting up serial connections and serial I/O.
 *
 *****************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

#include "serial.h"


/******************************************************************************
 *
 * Title:       int recv_serial( int fd, char *response, int length )
 *
 * Description: Get data from the serial port. If SERIAL_DEBUG is defined then
 *              read system call errors are printed to screen.
 *
 * Input:       fd:       File descriptor of serial port.
 *              response: Pointer to a buffer to hold response data.
 *              length:   Expected length of message.
 *
 * Output:      status: Number of bytes received. Read system call errors
 *                      return -1.  If serial port is not available then -2 is
 *                      returned.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int recv_serial( int fd,
                 void *response,
                 int length
               )
{
	int status = -1;
	int port_count = 0;

	struct timeval timeout;
	fd_set serial_fds;

	timeout.tv_sec = SERIAL_TIMEOUT_SECS;
	timeout.tv_usec = SERIAL_TIMEOUT_USECS;

	FD_ZERO( &serial_fds );
	FD_SET( fd, &serial_fds );

	port_count = select( SERIAL_MAX_PORTS, &serial_fds, NULL, NULL, &timeout );

	if ( ( port_count == 0 ) || ( !FD_ISSET( fd, &serial_fds ) ) ) {
		status = -2;
	}

	status = read( fd, response, length );

#ifdef SERIAL_DEBUG

	if ( status == -1 ) {
		perror( "read" );
	}

#endif /* SERIAL_DEBUG */

	return status;
} /* end recv_serial() */


/******************************************************************************
 *
 * Title:       int send_serial( int fd, void *command, int length )
 *
 * Description: Writes commands to serial port. If SERIAL_DEBUG is defined then
 *              write system call errors are printed to screen.
 *
 * Input:       fd:      Pointer to a file descriptor for the serial port.
 *              command: Pointer to the data to send.
 *              length:  Length of command.
 *
 * Output:      status: Number of bytes written to serial port if successful,
 *                      else write system call error code.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int send_serial( int fd,
                 void *command,
                 int length
               )
{
	int status;

	status = write( fd, command, length );

	tcdrain( fd );

#ifdef SERIAL_DEBUG

	if ( status < 0 ) {
		perror( "write" );
	}

#endif /* SERIAL_DEBUG */

	return status;
} /* end send_serial() */


/******************************************************************************
 *
 * Title:       int setup_serial( char *port_name, int baud )
 *
 * Description: Opens a serial port. If SERIAL_DEBUG is defined then open
 *              system call errors are printed to screen.
 *
 * Input:       port_name: String containing the name of the physical port.
 *              baud:      The desired baud rate.
 *
 * Output:      fd: The new file descriptor when successful. Open error returns
 *                  -1. Baud rate error returns -2.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int setup_serial( char *port_name,
                  int baud
                )
{
	int fd;

	struct termios options;

	fd = open( port_name, O_RDWR | O_NOCTTY | O_NONBLOCK );

	if ( fd < 0 ) {
#ifdef SERIAL_DEBUG
		perror( "open" );
#endif /* SERIAL_DEBUG */
		return fd;
	}

	if ( fd > 0 ) {
		tcgetattr( fd, &options );
		tcflush( fd, TCIFLUSH );

		// attempt # 2
		options.c_cflag = ( options.c_cflag & ~ CSIZE ) | CS8;
		options.c_cflag |= CLOCAL | CREAD | CS8;
		options.c_cflag &= ~ CRTSCTS;
		options.c_cflag &= ~ CSTOPB;
		options.c_cflag &= ~( PARENB | PARODD );
		options.c_iflag = IGNBRK;
		options.c_iflag &= ~( IXON | IXOFF | IXANY );
		options.c_oflag = 0;
		options.c_lflag = 0;

		switch ( baud ) {

			case 1200:
				cfsetispeed( &options, B1200 );
				cfsetospeed( &options, B1200 );
				break;

			case 2400:
				cfsetispeed( &options, B2400 );
				cfsetospeed( &options, B2400 );
				break;

			case 4800:
				cfsetispeed( &options, B4800 );
				cfsetospeed( &options, B4800 );
				break;

			case 9600:
				cfsetispeed( &options, B9600 );
				cfsetospeed( &options, B9600 );
				break;

			case 19200:
				cfsetispeed( &options, B19200 );
				cfsetospeed( &options, B19200 );
				break;

			case 38400:
				cfsetispeed( &options, B38400 );
				cfsetospeed( &options, B38400 );
				break;

			case 57600:
				cfsetispeed( &options, B57600 );
				cfsetospeed( &options, B57600 );
				break;

			case 115200:
				cfsetispeed( &options, B115200 );
				cfsetospeed( &options, B115200 );
				break;

			case 230400:
				cfsetispeed( &options, B230400 );
				cfsetospeed( &options, B230400 );
				break;

			default: /* Bad baud rate passed. */
				close( fd );
				return -2;
		}

		tcsetattr( fd, TCSANOW, &options );
	}

	return fd;
} /* end setup_serial() */


/******************************************************************************
 *
 * Title:      	int serial_bytes_available( int fd )
 *
 * Description:	Checks to see how many bytes are available to read on the
 *              serial stack. If SERIAL_DEBUG is defined then the ioctl system
 *              call error is printed to the screen.
 *
 * Input:      	fd: A file descriptor for the port.
 *
 * Output:     	bytes_available: Bytes available on the serial stack.
 *
 * Globals:		None.
 *
 *****************************************************************************/

int serial_bytes_available( int fd )
{
	int bytes_available;
	int status = 0;

	status = ioctl( fd, FIONREAD, &bytes_available );
#ifdef SERIAL_DEBUG

	if ( status == -1 ) {
		perror( "ioctl" );
	}

#endif /* SERIAL_DEBUG */

	return bytes_available;
} /* end serial_bytes_available() */
