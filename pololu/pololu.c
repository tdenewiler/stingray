/******************************************************************************
 *
 *  Title:        pololu.c
 *
 *  Description:  This library implements the Pololu protocol for the
 *                Pololu 16-Channel USB servo controller.  The user's
 *                manual is available at: www.pololu.com
 *
 *  NOTES:
 *    POLOLU_SLEEP may not be necessay now.  More experiments are needed.
 *
 *****************************************************************************/

#include "pololu.h"
#include "serial.h"


/******************************************************************************
 *
 * Title:       int pololuSetup( char *portname,
 *                               int baud
 *                             )
 *
 * Description: Initializes communications with the Pololu. Sets up a file
 *              descriptor for further communications.  Baud rates from
 *              2000 to 40000 Hz are accepted.
 *
 * Input:       portname: The name of the port that the Pololu is plugged into.
 *              baud:     The baud rate to use for the Pololu serial port.
 *
 * Output:      fd: A file descriptor for the Pololu.  Returns -1 for failure.
 *
 *****************************************************************************/

int pololuSetup( char *portname,
                 int baud
               )
{
	/* Set up variables. */
	int fd = -1;
	int result = 0;
	
	/* Open a serial port connection to the Pololu. */
	if( portname != NULL ) {
		fd = setup_serial( portname, baud );
	}

	/* Check that port opened correctly. Initialize the channels. */
	if( fd < 0 ) {
		return fd;
	}
	else {
		/* Initialize channels. */
		result = pololuInitializeChannels( fd );
		if( result < 0 ) {
			close( fd );
			return POLOLU_FAILURE;
		}

		return fd;
	}
} /* end pololu_setup() */


/******************************************************************************
 *
 * Title:       int pololuSetParameters( int fd,
 *                                       int channel,
 *                                       int channelOn,
 *                                       int direction,
 *                                       int range
 *                                      )
 *
 * Description: Sets the parameters for a channel.  If values out of range are
 *              given the function fails and returns 0 bytes sent.
 *
 * Input:       fd        -file descriptor of Pololu
 *              channel   -0-15 for the servo channel
 *              channelOn -0 for OFF (default), any other value for ON
 *              direction -0 for REVERSE, any other value for FORWARD (default)
 *              range     -0-15 for the servo range multiplier (15 default)
 *
 * Output:      Returns the number of bytes sent.
 *
 *****************************************************************************/

int pololuSetParameters( int fd,
                         int channel,
                         int channelOn,
                         int direction,
                         int range
                       )
{
	/* Set up variables. */
	int result = 0;
	unsigned char msg[5];
	unsigned char c = 0;
	unsigned char r = 0;

	/* Check range on the parameters. */
	if( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (range >= 0)
	        && (range <= 15) ) {

		/* Typecast the ints to chars. */
		c = (unsigned char)channel;
		r = (unsigned char)range;

		/* Set the parameters. */
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_COMMAND_BYTE;
		msg[3] = c;

		/* Set up masks. 0 is default for forward direction in Pololu protocol.
		 * 0 is default for channel on. */
		unsigned char bit5 = 0;
		if( !direction ) {
			bit5 = 32;
		}
		unsigned char bit6 = 0;
		if( channelOn ) {
			bit6 = 64;
		}

		/* Set the bit mask. */
		msg[4] = ( (r & 15) | (bit5 & 32) | (bit6 & 64) ) & 127;

		/* Send the parameters to the Pololu. */
		result = send_serial( fd, &msg, 5 );
	}

	return result;
} /* end pololuSetParameters() */


/******************************************************************************
 *
 * Title:       int pololuSetSpeed( int fd,
 *                                  int channel,
 *                                  int speed
 *                                 )
 *
 * Description: Sets the speed of a channel. If values out of range are given
 *              the function fails and returns 0 bytes sent.
 *
 * Input:       fd        -file descriptor of Pololu
 *              channel   -0-15 for the servo channel
 *              speed     -0-127 Sets the speed that the channel changes from
 *                          one value to another.  (0 is default for instant)
 *
 * Output:      Returns the number of bytes sent.
 *
 *****************************************************************************/

int pololuSetSpeed( int fd,
                    int channel,
                    int speed
                  )
{
	/* Set up variables. */
	int result = 0;
	unsigned char msg[5];
	unsigned char c = 0;
	unsigned char s = 0;

	/* Check bounds on speed and channel. */
	if( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (speed >= 0) &&
	        (speed <= 127) ) {

		/* Typecast the ints to chars. */
		c = (unsigned char)channel;
		s = (unsigned char)speed;

		/* Set the parameters. */
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_SPEED_BYTE;
		msg[3] = c;

		/* Set the bit mask. */
		msg[4] = s & 127;

		/* Send the command to the Pololu. */
		result = send_serial( fd, &msg, 5 );
	}

	return result;
} /* end pololuSetSpeed() */


/******************************************************************************
 *
 * Title:       int pololuSetPosition7Bit( int fd,
 *                                         int channel,
 *                                         int position
 *                                        )
 *
 * Description: Sets the relative position of a channel.  If values out of
 *              range are given the function fails and returns 0 bytes sent.
 *
 * Input:       fd        -file descriptor of Pololu
 *              channel   -0-15 for the servo channel
 *              position  -0-127 Sets the relative position of the channel.
 *
 * Output:      Returns the number of bytes sent.
 *
 *****************************************************************************/

int pololuSetPosition7Bit( int fd,
                           int channel,
                           int position
                         )
{
	/* Set up variables. */
	int result = 0;
	unsigned char msg[5];
	unsigned char c = 0;
	unsigned char p = 0;

	/* Check the bounds on the channel and position. */
	if( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 0) &&
	        (position <= 127) ) {

		/* Typecast the ints to chars. */
		c = (unsigned char)channel;
		p = (unsigned char)position;

		/* Set the parameters. */
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_SPEED_BYTE;
		msg[3] = c;

		/* Set the bit mask. */
		msg[4] = p & 127;

		/* Send the command to the Pololu. */
		result = send_serial( fd, &msg, 5 );
	}

	return result;
} /* end pololuSetPosition7Bit() */


/******************************************************************************
 *
 * Title:       int pololuSetPosition8Bit( int fd,
 *                                         int channel,
 *                                         int position
 *                                        )
 *
 * Description: Sets the relative position of a channel.  If values out of
 *              range are given the function fails and returns 0 bytes sent.
 *
 * Input:       fd        -file descriptor of Pololu
 *              channel   -0-15 for the servo channel
 *              position  -0-255 Sets the relative position of the channel.
 *
 * Output:      Returns the number of bytes sent.
 *
 *****************************************************************************/

int pololuSetPosition8Bit( int fd,
                           int channel,
                           int position
                         )
{
	/* Set up variables. */
	int result = 0;
	unsigned char msg[6];
	unsigned char c = 0;
	unsigned char p = 0;

	/* Check the bounds on the channel and position. */
	if( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 0) &&
	        (position <= 255) ) {

		/* Typecast the ints to chars. */
		c = (unsigned char)channel;
		p = (unsigned char)position;
		
		/* Set the parameters. */
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_8BIT_POS_BYTE;
		msg[3] = c;

		/* Set the bit mask. */
		if( p & 128 ) {
			msg[4] = 1;
		}
		else {
			msg[4] = 0;
		}
		msg[5] = ( p & 127 );

		/* Send the command to the Pololu. */
		result = send_serial( fd, &msg, 6 );
	}

	return result;
} /* end pololuSetPosition8Bit() */


/******************************************************************************
 *
 * Title:       int pololuSetPositionAbsolute( int fd,
 *                                             int channel,
 *                                             int position
 *                                            )
 *
 * Description: Sets the absolute position of a channel.  If values out of
 *              range are given the function fails and returns 0 bytes sent.
 *
 * Input:       fd        -file descriptor of Pololu
 *              channel   -0-15 for the servo channel
 *              position  -500-5500 Sets the absolute position of the channel.
 *
 * Output:      Returns the number of bytes sent.
 *
 *****************************************************************************/
int pololuSetPositionAbsolute( int fd,
                               int channel,
                               int position
                             )
{
	/* Set up variables. */
	int result = 0;
	unsigned char msg[6];
	unsigned char c = 0;
	unsigned int p = 0;

	/* Check the bounds on the channel and position. */
	if( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 500) &&
	        (position <= 5500) ) {

		/* Typecast the channel to char and position to int. */
		c = (unsigned char)channel;
		p = (unsigned int)position;

		/* Set the parameters. */
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_ABS_POS_BYTE;
		msg[3] = c;

		/* Set the bit mask. */
		msg[4] = ( p >> 7 ) & 127;
		msg[5] = ( p & 127 );

		/* Send the command to the Pololu. */
		result = send_serial( fd, &msg, 6 );
	}

	return result;
} /* end pololuSetPositionAbsolute() */


/******************************************************************************
 *
 * Title:       int pololuSetNeutral( int fd,
 *                                    int channel,
 *                                    int position
 *                                   )
 *
 * Description: Sets the absolute neutral position of a channel.  If values
 *              out of range are given the function fails and returns 0 bytes
 *              sent.
 *
 * Input:       fd        -file descriptor of Pololu
 *              channel   -0-15 for the servo channel
 *              position  -500-5500 Sets the absolute neutral position of the
 *                         channel.  (3000 default)
 *
 * Output:      Returns the number of bytes sent.
 *
 *****************************************************************************/
int pololuSetNeutral( int fd,
                      int channel,
                      int position
                    )
{
	/* Set up variables. */
	int result = 0;
	unsigned char msg[6];
	unsigned char c = 0;
	unsigned int p = 0;

	/* Check the bounds on the channel and position. */
	if( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 500) &&
	        (position <= 5500) ) {

		/* Typecast the channel to char and position to int. */
		c = (unsigned char)channel;
		p = (unsigned int)position;

		/* Set the parameters. */
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_NEUTRAL_BYTE;
		msg[3] = c;

		/* Set the bit mask. */
		msg[4] = ( p >> 7 ) & 127;
		msg[5] = ( p & 127 );

		/* Send the command to the Pololu. */
		result = send_serial( fd, &msg, 6 );
	}

	return result;
} /* end pololuSetNeutral() */


/******************************************************************************
 *
 * Title:       int pololuInitializeChannels( int fd )
 *
 * Description: Initializes all channels and sets all channels to neutral.
 *
 * Input:       fd file descriptor of Pololu device.
 *
 * Output:      Retuns 1 for success and -1 for failure.
 *
 *****************************************************************************/

int pololuInitializeChannels( int fd )
{
	/* Set up variables. */
	int result = 0;

	/* Check for a valid file descriptor. */
	if( fd < 0 ) {
		return POLOLU_FAILURE;
	}

	/* Set up the default profile as determined from testing servos and motors. */
	result += pololuSetParameters(   fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetSpeed(        fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_CH0_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetSpeed(        fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_CH3_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_LEFT_WING_MOTOR, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetSpeed(        fd, POLOLU_LEFT_WING_MOTOR, POLOLU_LWING_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_LEFT_WING_MOTOR, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetSpeed(        fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_RWING_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_TAIL_MOTOR, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetSpeed(        fd, POLOLU_TAIL_MOTOR, POLOLU_TAIL_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_TAIL_MOTOR, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_LEFT_SERVO1, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetNeutral(      fd, POLOLU_LEFT_SERVO1, POLOLU_CH1_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_LEFT_SERVO1, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_LEFT_SERVO2, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetNeutral(      fd, POLOLU_LEFT_SERVO2, POLOLU_CH2_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_LEFT_SERVO2, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_RIGHT_SERVO1, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetNeutral(      fd, POLOLU_RIGHT_SERVO1, POLOLU_CH4_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_RIGHT_SERVO1, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_RIGHT_SERVO2, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetNeutral(      fd, POLOLU_RIGHT_SERVO2, POLOLU_CH5_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_RIGHT_SERVO2, POLOLU_7BIT_NEUTRAL );
	result += pololuSetParameters(   fd, POLOLU_DROPPER_SERVO, POLOLU_CHANNEL_ON, POLOLU_DIR_FORWARD, POLOLU_RANGE );
	result += pololuSetNeutral(      fd, POLOLU_DROPPER_SERVO, POLOLU_DROPPER_NEUTRAL );
	result += pololuSetPosition7Bit( fd, POLOLU_DROPPER_SERVO, POLOLU_7BIT_NEUTRAL );

	/* Check the number of bytes sent and return success or failure. There are
	 * 5 bytes for each command and 6 bytes for each neutral command:
	 * --> 5 * 23 + 6 * 4 = 139. */
	if( result == 139 ) {
		result = POLOLU_SUCCESS;
	}
	else {
		result = POLOLU_FAILURE;
	}

	return result;
} /* end pololuInitializeChannels() */


/*************************HIGHER LEVEL CONTROL FUNCTIONS**********************/

/******************************************************************************
 *
 * Title:   int pololuControlVoiths( int voithThrust,
 *                             float thrustAngle,
 *                             int thrust,
 *                             int yawTorque
 *                            )
 *
 * Description: This is a higher level function that controls both
 *              VSPs together.
 *
 * Input:
 *	voithThrust: between 0 and 100
 *	thrustAngle: between 0 and 360
 *	thrust: between -100 and 100
 *	yawTorque: between -100 and 100
 *
 * Output:      1 for success and -1 for failure
 *
 *****************************************************************************/
int pololuControlVoiths( int fd,
                   int voithThrust,
                   float thrustAngle,
                   int thrust,
                   int yawTorque
                 )
{
	/* The forces need to be in the range [-100,100]. They will then be scaled
	 * to the range [0,128] before being sent to the Pololu. If any force is
	 * outside the acceptable range then an error code will be returned. */
	if( (fd < 0) ||
	    (voithThrust < 0) ||
	    (voithThrust > POLOLU_SERVO_BOUND) ||
	    (thrust < -1 * POLOLU_SERVO_BOUND) ||
	    (thrust > POLOLU_SERVO_BOUND) ||
	    (yawTorque < -1 * POLOLU_SERVO_BOUND) ||
	    (yawTorque > POLOLU_SERVO_BOUND) ) {
		return POLOLU_FAILURE;
	}

	/* Set up angle variables. */
	int bytes = 0;
	float angle = 0;
	angle = thrustAngle; // * ( M_PI / 180 );

	/* Set up the angle offset in the range [0,360]. */
	float leftAngleOffset = POLOLU_LEFT_ANGLE_OFFSET * ( M_PI / 180 );
	float rightAngleOffset = POLOLU_RIGHT_ANGLE_OFFSET * ( M_PI / 180 );

	/* Calculate the radius values for where the Voith pins should be. */
	int radius1 = yawTorque + thrust;
	int radius2 = yawTorque - thrust;

	/* Calculate the commands to send to the servos to control Voith direction. */
	int leftCmd1  = (int)( POLOLU_SERVO_NEUTRAL + POLOLU_SERVO_GAIN * radius1 * cos(angle + leftAngleOffset) );
	int leftCmd2  = (int)( POLOLU_SERVO_NEUTRAL + POLOLU_SERVO_GAIN * radius1 * sin(angle + leftAngleOffset) );
	int rightCmd1 = (int)( POLOLU_SERVO_NEUTRAL - POLOLU_SERVO_GAIN * radius2 * cos(angle + rightAngleOffset) );
	int rightCmd2 = (int)( POLOLU_SERVO_NEUTRAL - POLOLU_SERVO_GAIN * radius2 * sin(angle + rightAngleOffset) );

	/* Scale the Voith thrust value. */
	int scaledVoithThrust = voithThrust * POLOLU_VOITH_GAIN + POLOLU_VOITH_NEUTRAL;

	/* Send the commands to the Pololu to control servo and motor positions. */
	bytes += pololuSetPosition7Bit( fd, POLOLU_LEFT_SERVO1, leftCmd1 );
	bytes += pololuSetPosition7Bit( fd, POLOLU_LEFT_SERVO2, leftCmd2 );
	bytes += pololuSetPosition7Bit( fd, POLOLU_RIGHT_SERVO1, rightCmd1 );
	bytes += pololuSetPosition7Bit( fd, POLOLU_RIGHT_SERVO2, rightCmd2 );
	bytes += pololuSetPosition7Bit( fd, POLOLU_LEFT_VOITH_MOTOR, scaledVoithThrust );
	bytes += pololuSetPosition7Bit( fd, POLOLU_RIGHT_VOITH_MOTOR, scaledVoithThrust );

	/* Check the number of bytes sent and return success or failure. There are
	 * 5 bytes for each command --> 5 * 6 = 30. */
	if( bytes == POLOLU_BYTES_VOITH ) {
		return POLOLU_SUCCESS;
	}

	return POLOLU_FAILURE;
} /* end pololuControlVoiths() */


/******************************************************************************
 *
 * Title:   int pololuControlVertical( int fd,
 *                               int vertForce,
 *                               int rollTorque,
 *                               int pitchTorque
 *                              )
 *
 * Description: This is a higher level function that controls the vertical
 *              thrusters together.
 *
 * Input:
 *
 * Output:
 *
 *****************************************************************************/

int pololuControlVertical( int fd,
                     int vertForce,
                     int rollTorque,
                     int pitchTorque
                   )
{
	/* The forces need to be in the range [-100,100]. They will then be scaled
	 * to the range [0,128] before being sent to the Pololu. If any force is
	 * outside the acceptable range then an error code will be returned. */
	if( (fd < 0) ||
	    (vertForce < -1 * POLOLU_SERVO_BOUND) ||
	    (vertForce > POLOLU_SERVO_BOUND) ||
	    (rollTorque < -1 * POLOLU_SERVO_BOUND) ||
	    (rollTorque > POLOLU_SERVO_BOUND) ||
	    (pitchTorque < -1 * POLOLU_SERVO_BOUND) ||
	    (pitchTorque > POLOLU_SERVO_BOUND) ) {
		return POLOLU_FAILURE;
	}

	/* Set up motor command variables. */
	int bytes = 0;
	int leftCmd = 0;
	int rightCmd = 0;
	int tailCmd = 0;
	float left = 0;
	float right = 0;
	float tail = 0;

	/* Set up the motor dead zone variables. */
	float dzRadius = POLOLU_DEADZONE;
	float leftNeutral = POLOLU_SERVO_NEUTRAL;
	float rightNeutral = POLOLU_SERVO_NEUTRAL;
	float tailNeutral = POLOLU_SERVO_NEUTRAL;

	/* Calculate the servo commands. */
	left = vertForce + rollTorque;
	right = vertForce - rollTorque;
	tail = pitchTorque;

	/* Check the bounds on the servo commands. */
	if( left > POLOLU_SERVO_BOUND ) {
		left = POLOLU_SERVO_BOUND;
	}
	if( left < -1 * POLOLU_SERVO_BOUND ) {
		left = -1 * POLOLU_SERVO_BOUND;
	}
	if( right > POLOLU_SERVO_BOUND ) {
		right = POLOLU_SERVO_BOUND;
	}
	if( right < -1 * POLOLU_SERVO_BOUND ) {
		right = -1 * POLOLU_SERVO_BOUND;
	}

	/* Need a very short sleep before sending out commands. */
	usleep( POLOLU_SLEEP );

	/* The range [-0.1,0.1] is our new dead zone. */
	if( left > dzRadius ) {
		leftNeutral += POLOLU_DZ_NEUTRAL;
	}
	if( left < -1 * dzRadius ) {
		leftNeutral -= POLOLU_DZ_NEUTRAL;
	}
	if( right > dzRadius ) {
		rightNeutral += POLOLU_DZ_NEUTRAL;
	}
	if( right < -1 * dzRadius ) {
		rightNeutral -= POLOLU_DZ_NEUTRAL;
	}
	if( tail > dzRadius ) {
		tailNeutral += POLOLU_DZ_NEUTRAL;
	}
	if( tail < -1 * dzRadius ) {
		tailNeutral -= POLOLU_DZ_NEUTRAL;
	}

	/* Actually calculate the values to send to the Pololu now that we have the
	 * dead zone accounted for. */
	leftCmd = (int)( leftNeutral + POLOLU_NEUTRAL_GAIN * left );
	rightCmd = (int)( rightNeutral + POLOLU_NEUTRAL_GAIN * right );
	tailCmd = (int)( tailNeutral + POLOLU_NEUTRAL_GAIN * tail );

	/* Send the commands to the Pololu to control servo and motor positions. */
	bytes += pololuSetPosition7Bit( fd, POLOLU_LEFT_WING_MOTOR, leftCmd );
	bytes += pololuSetPosition7Bit( fd, POLOLU_RIGHT_WING_MOTOR, rightCmd );
	bytes += pololuSetPosition7Bit( fd, POLOLU_TAIL_MOTOR, tailCmd );

	/* Check the number of bytes sent and return success or failure. There are
	 * 5 bytes for each command --> 5 * 3 = 15. */
	if( bytes == POLOLU_BYTES_VERTICAL ) {
		return POLOLU_SUCCESS;
	}

	return POLOLU_FAILURE;
} /* end pololuControlVertical() */
