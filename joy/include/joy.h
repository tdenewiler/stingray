/**
 *  \file joy.h
 *  \brief This library enables us to get inputs from Logitech Dual Action
 *          and Microsoft Sidewinder joysticks.
 */

#ifndef _JOY_H_
#define _JOY_H_

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

/*
 * The Logitech Dual Action has 6 Axes:
 *
 * 00 - leftAnalogLeft = -32767, leftAnalogCenter = 0, leftAnalogRight = 32767
 * 01 - leftAnalogUp = -32767, leftAnalogCenter = 0, leftAnalogDown = 32767
 * 02 - rightAnalogLeft = -32767, rightAnalogCenter = 0, rightAnalogRight = 32767
 * 03 - rightAnalogUp = -32767, rightAnalogCenter = 0, rightAnalogDown = 32767
 * 04 - padLeft = -32767, padCenter = 0, padRight = 32767
 * 05 - padUp = -32767, padCenter = 0, padDown = 32767
 *
 * NOTE: The current values are stored in axis[]
 *
 * Here's an example:
 *
 * if(axis[0] == 0 && axis[1] == 0) {
 * 	// left analog is centered
 * }
 */

/*
 * The Logitech Dual Action has 12 Buttons:
 *
 * 00 - button01 = 0 or 1
 * 01 - button02 = 0 or 1
 * 02 - button03 = 0 or 1
 * 03 - button04 = 0 or 1
 * 04 - button05 = 0 or 1
 * 05 - button06 = 0 or 1
 * 06 - button07 = 0 or 1
 * 07 - button08 = 0 or 1
 * 08 - button09 = 0 or 1
 * 09 - button10 = 0 or 1
 * 10 - buttonLeftAnalog = 0 or 1
 * 11 - buttonRightAnalog = 0 or 1
 *
 * NOTE: The current values are stored in button[]
 *
 * Here's an example:
 *
 * if( button[0] == 0 && button[1] == 1 ) {
 * 	// button 1 is not currently pressed and button 2 is currently pressed
 * }
 *
 * A Micorsoft Sidewinder joystick has 3 axes:
 * 00 - x direction [-32767, 32767]
 * 01 - y direction [-32767, 32767]
 * 02 - slider [-32767, 32767]
 *
 * and 8 buttons:
 * 00 - button01 = 0 or 1
 * 01 - button01 = 0 or 1
 * 02 - button01 = 0 or 1
 * 03 - button01 = 0 or 1
 * 04 - button01 = 0 or 1
 * 05 - button01 = 0 or 1
 * 06 - button01 = 0 or 1
 * 07 - button01 = 0 or 1
 *
 */


/******************************
 *
 * #defines
 *
 *****************************/

#ifndef JOY_BUTTONS
#define JOY_BUTTONS
#define JOY_A0	0
#define JOY_A1	1
#define JOY_A2 	2
#define JOY_A3	3
#define JOY_A4	4
#define JOY_A5	5
#define JOY_B0	0
#define JOY_B1	1
#define JOY_B2	2
#define JOY_B3	3
#define JOY_B4	4
#define JOY_B5	5
#define JOY_B6	6
#define JOY_B7	7
#define JOY_B8	8
#define JOY_B9	9
#endif /* JOY_BUTTONS */


/******************************
 *
 * Data types
 *
 *****************************/

#ifndef _JOY_DATA_
#define _JOY_DATA_

typedef struct {
	unsigned int joy_axis;		//!< Specifies which axis input was found on.
	unsigned int axis_value;	//!< Specifies the value of the axis input.
	unsigned int joy_button;	//!< Specifies which button input was found on.
	unsigned int button_value;	//!< Specifies the value of the button input.
} JOY_DATA;

#endif /* _JOY_DATA_ */


/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Sets up a serial port for use with a joystick.
//! \return A file descriptor for the serial port.
int joy_setup( );

//! Gets joystick input from the USB port. Uses the Linux joystick interface.
//! \param fd A file descriptor for the serial port.
//! \param joy_data A pointer to hold joystick data that was input.
//! \return The number of bytes read.
int joy_get_data( int fd, JOY_DATA* joy_data );


#endif /* _JOY_H_ */
