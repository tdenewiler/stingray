/*------------------------------------------------------------------------------
 *
 *  Title:        serial.c
 *
 *  Description:  Handles setting up serial connections and serial I/O.
 *
 *----------------------------------------------------------------------------*/

#include "serial.h"

/*------------------------------------------------------------------------------
 * int recv_serial()
 * Get data from the serial port.
 *----------------------------------------------------------------------------*/

int recv_serial(int fd, void *response, int length)
{
	/// Declare variables.
	int status = -1;
	int port_count = 0;

	struct timeval timeout;
	fd_set serial_fds;

	timeout.tv_sec = SERIAL_TIMEOUT_SECS;
	timeout.tv_usec = SERIAL_TIMEOUT_USECS;

	FD_ZERO(&serial_fds);
	FD_SET(fd, &serial_fds);

	port_count = select(SERIAL_MAX_PORTS, &serial_fds, NULL, NULL, &timeout);

	if ((port_count == 0) || (!FD_ISSET(fd, &serial_fds))) {
		status = -2;
	}

	status = read(fd, response, length);

	if (status == -1) {
		perror("read");
	}

	return status;
} /* end recv_serial() */


/*------------------------------------------------------------------------------
 * int send_serial()
 * Writes commands to serial port.
 *----------------------------------------------------------------------------*/

int send_serial(int fd, void *command, int length)
{
	/// Declare variables.
	int status = 0;
	status = write(fd, command, length);
	tcdrain(fd);

	if (status < 0) {
		perror("write");
	}

	return status;
} /* end send_serial() */


/*------------------------------------------------------------------------------
 * int setup_serial()
 * Opens a serial port.
 *----------------------------------------------------------------------------*/

int setup_serial(char *port_name, int baud)
{
	/// Declare variables.
	int fd = -1;
	struct termios options;
	fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (fd < 1) {
		perror("open");
		return fd;
	}

	/// Set serial port options.
	tcgetattr(fd, &options);
	tcflush(fd, TCIFLUSH);
	options.c_cflag = (options.c_cflag & ~ CSIZE) | CS8;
	options.c_cflag |= CLOCAL | CREAD | CS8;
	options.c_cflag &= ~ CRTSCTS;
	options.c_cflag &= ~ CSTOPB;
	options.c_cflag &= ~(PARENB | PARODD);
	options.c_iflag = IGNBRK;
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	options.c_oflag = 0;
	options.c_lflag = 0;

	switch (baud) {
	case 1200:
		cfsetispeed(&options, B1200);
		cfsetospeed(&options, B1200);
		break;

	case 2400:
		cfsetispeed(&options, B2400);
		cfsetospeed(&options, B2400);
		break;

	case 4800:
		cfsetispeed(&options, B4800);
		cfsetospeed(&options, B4800);
		break;

	case 9600:
		cfsetispeed(&options, B9600);
		cfsetospeed(&options, B9600);
		break;

	case 19200:
		cfsetispeed(&options, B19200);
		cfsetospeed(&options, B19200);
		break;

	case 38400:
		cfsetispeed(&options, B38400);
		cfsetospeed(&options, B38400);
		break;

	case 57600:
		cfsetispeed(&options, B57600);
		cfsetospeed(&options, B57600);
		break;

	case 115200:
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
		break;

	case 230400:
		cfsetispeed(&options, B230400);
		cfsetospeed(&options, B230400);
		break;

	default: ///Bad baud rate passed.
		close(fd);
		return -2;
	}

	tcsetattr(fd, TCSANOW, &options);

	return fd;
} /* end setup_serial() */


/*------------------------------------------------------------------------------
 * int serial_bytes_available()
 * Checks to see how many bytes are available to read on the serial stack.
 *----------------------------------------------------------------------------*/

int serial_bytes_available(int fd)
{
	/// Declare variables.
	int bytes_available;
	int status = 0;

	status = ioctl(fd, FIONREAD, &bytes_available);
	if (status == -1) {
		perror("ioctl");
	}

	return bytes_available;
} /* end serial_bytes_available() */
