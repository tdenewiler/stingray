#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/input.h>
#include <linux/joystick.h>

#include "joy.h"


/******************************************************************************
 *
 * Title:       int joy_setup( )
 *
 * Description: Sets up a serial port to use with joystick.
 *
 * Input:       None.
 *
 * Output:      fd: A file descriptor for the joystick.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int joy_setup( )
{
	int fd = -1;

	fd = open( "/dev/input/js0", O_RDONLY );

	return fd;
} /* end joy_setup() */


/******************************************************************************
 *
 * Title:       int joy_get_data( int fd, JOY_DATA *joy_data )
 *
 * Description: Gets joystick input.
 *
 * Input:       fd: The file descriptor for the joystick.
 * 				joy_data: A pointer to a struct to hold the data.
 *
 * Output:      read_bytes: Number of bytes read from fd.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int joy_get_data( int fd, JOY_DATA *joy_data )
{
	int read_bytes = 0;
	unsigned char axes = 0;
	unsigned char buttons = 0;
	ioctl( fd, JSIOCGAXES, &axes );
	ioctl( fd, JSIOCGBUTTONS, &buttons );
	int *axis;
	int *button;

	struct js_event js;
	axis = ( int * )calloc( axes, sizeof( int ) );
	button = ( int * )calloc( buttons, sizeof( int ) );
	read_bytes = read( fd, &js, sizeof( struct js_event ) );

	switch ( js.type & ~JS_EVENT_INIT ) {

		case JS_EVENT_AXIS:
			axis[js.number] = js.value;
			joy_data->joy_axis = js.number;
			joy_data->axis_value = js.value;

			break;

		case JS_EVENT_BUTTON:
			button[js.number] = js.value;
			joy_data->joy_button = js.number;
			joy_data->button_value = js.value;

			break;
	}

	return read_bytes;
} /* end joy_input() */
