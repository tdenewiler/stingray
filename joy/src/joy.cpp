/*------------------------------------------------------------------------------
 *
 *  Title:        joy.c
 *
 *----------------------------------------------------------------------------*/

#include "joy.h"

/*------------------------------------------------------------------------------
 * int joy_setup()
 * Sets up a serial port to use with joystick.
 *----------------------------------------------------------------------------*/

int joy_setup( )
{
    int fd = -1;
    fd = open("/dev/input/js0", O_RDONLY);

    return fd;
} /* end joy_setup() */


/*------------------------------------------------------------------------------
 * int joy_get_data()
 * Gets joystick input from the USB port. Uses the Linux joystick interface.
 *----------------------------------------------------------------------------*/

int joy_get_data( int fd, JOY_DATA *joy_data )
{
	/// Declare variables.
    int read_bytes = 0;
    unsigned char axes = 0;
    unsigned char buttons = 0;
    ioctl( fd, JSIOCGAXES, &axes );
    ioctl( fd, JSIOCGBUTTONS, &buttons );
    int *axis;
    int *button;
    struct js_event js;
	
	/// Initialize variables.
    axis = (int *)calloc( axes, sizeof(int) );
    button = (int *)calloc( buttons, sizeof(int) );
    read_bytes = read( fd, &js, sizeof(struct js_event) );

	/// Switch on captured joystick event.
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
