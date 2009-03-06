#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "joy.h"


int main()
{
	int joy_fd = -1;
	int status = -1;
	JOY_DATA joy_data;

	/* Don't pass pointers of locals variables because of stack swaps. Instead
	 * use the heap. */
	JOY_DATA *pjoy_data = ( JOY_DATA * )calloc( 1, sizeof( JOY_DATA ) );
	pjoy_data = &joy_data;

	joy_fd = joy_setup( );
	printf( "joy_fd = %d\n\n", joy_fd );

	if ( joy_fd < 0 ) {
		printf( "No joystick, quitting...\n" );
		return 0;
	}

	while ( 1 ) {
		/* Fill in JOY_DATA struct. */
		status = joy_get_data( joy_fd, pjoy_data );

		if ( status > 0 ) {
			printf( "joy_axis = %d  axis_value = %d\n",
			        joy_data.joy_axis,
			        joy_data.axis_value
			      );
			printf( "joy_button = %d  button_value = %d\n",
			        joy_data.joy_button,
			        joy_data.button_value
			      );
		}
	}

	/* Free used heap memory and throw away the keys. */

	/* Close the serial port. */
	close( joy_fd );

	return 0;
} /* end main() */
