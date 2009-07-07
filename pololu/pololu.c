/******************************************************************************
 *
 *  Title:        pololu.c
 *
 *  Description:  This library implements the Pololu protocol for the
 *                Pololu 16-Channel USB servo controller.  The user's
 *                manual is available at: www.pololu.com
 *
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
	int fd = -1;
	int result = 0;

	if ( portname != NULL ) {
		fd = setup_serial( portname, baud );
	}

	// if fd<0 setup_serial failed
	// NOTE THAT THIS IS FAILURE VALUE FROM setup_serial
	if ( fd < 0 ) {
		return fd;
	}
	else {
		// initialize channels
		result = pololuInitializeChannels( fd );
		// if the channels don't initialize then failure

		if ( result < 0 ) {
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
	// sleep for a bit
	// usleep( POLOLU_SLEEP );
	// check ranges
	int result = 0;

	if ( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (range >= 0)
	        && (range <= 15) ) {

		// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned char r = (unsigned char)range;

		// set parameters
		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x00;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]

		// setup some masks
		unsigned char bit5 = 0; // 0 is default for forward in protocol

		if ( !direction ) {
			bit5 = 32;  // 16 is reverse
		}

		unsigned char bit6 = 0; // 0 is for off
		if ( channelOn ) {
			bit6 = 64;  // 32 is for on
		}

		/* This should do it. */
		msg[4] = ( (r & 15) | (bit5 & 32) | (bit6 & 64) ) & 127;

		// send the message
		// printf( "sp: %x %x %x %x %x \n",msg[0],msg[1],msg[2],msg[3],msg[4] );
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
	// sleep for a bit
	// usleep( POLOLU_SLEEP );

	// set speed
	int result = 0;
	// check ranges

	if ( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (speed >= 0) &&
	        (speed <= 127) ) {

		// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned char s = (unsigned char)speed;

		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x01;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		msg[4] = s & 127;

		// send the message
		//printf( "sp: %x %x %x %x %x \n",msg[0],msg[1],msg[2],msg[3],msg[4] );
		result = send_serial( fd, &msg, 5 );
		// result = bytes received
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
	// sleep for a bit
	// usleep( POLOLU_SLEEP );

	// check ranges
	int result = 0;

	if ( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 0) &&
	        (position <= 127) ) {

		// set position7bit
		// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned char p = (unsigned char)position;

		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x02;  // command #  result+=pololuSetPosition7Bit(fd,1,63);
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		msg[4] = p & 127;

		// send the message
		//printf( "sp: %x %x %x %x %x \n",msg[0],msg[1],msg[2],msg[3],msg[4] );
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
	// sleep for a bit
	// usleep( POLOLU_SLEEP );

	// check ranges
	int result = 0;

	if ( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 0) &&
	        (position <= 255) ) {

		// set position8bit
		// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned char p = (unsigned char)position;
		//printf( "sp8: p=%x\n",p );
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x03;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		// 0th bit of msg[4] is 7th bit of position

		if ( p & 128 ) {
			msg[4] = 1;
		}
		else {
			msg[4] = 0;
		}

		msg[5] = ( p & 127 );

		// send the message
		//printf("sp8: %x %x %x %x %x %x \n",msg[0],msg[1],msg[2],msg[3],msg[4],msg[5]);
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
	// sleep for a bit
	// usleep( POLOLU_SLEEP );

	// check ranges
	int result = 0;

	if ( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 500) &&
	        (position <= 5500) ) {

		// set positionAbsolute
		// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned int p = (unsigned int)position;
		unsigned char msg[6];
		//printf( "spA: %x\n",p );
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x04;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		// msg[4] is upper 7 bits of position
		msg[4] = ( p >> 7 ) & 127;
		msg[5] = ( p & 127 );

		// send the message
		//printf("spA: %x %x %x %x %x %x \n",msg[0],msg[1],msg[2],msg[3],msg[4],msg[5]);
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
	// sleep for a bit
	// usleep( POLOLU_SLEEP );

	// set neutral
	// check ranges
	int result = 0;

	if ( (fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 500) &&
	        (position <= 5500) ) {

		// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned int p = (unsigned int)position;
		unsigned char msg[6];
		//printf("spA: %x\n",p);
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x05;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		// msg[4] is upper 7 bits of position
		msg[4] = ( p >> 7 ) & 127;
		msg[5] = ( p & 127 );

		// send the message
		//printf("sp: %x %x %x %x %x %x \n",msg[0],msg[1],msg[2],msg[3],msg[4],msg[5]);
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
	int result = 0;

	// if invalid fd return failure
	if ( fd < 0 ) {
		return POLOLU_FAILURE;
	}

	// THIS IS OUR DEFAULT PROFILE
	result += pololuSetParameters( fd, 0, 1, 1, 15 );
	result += pololuSetParameters( fd, 3, 1, 1, 15 );
	result += pololuSetSpeed( fd, 0, 10 );
	result += pololuSetSpeed( fd, 3, 10 );
	result += pololuSetPosition7Bit( fd, 0, 63 );
	result += pololuSetPosition7Bit( fd, 3, 63 );
	result += pololuSetParameters( fd, 7, 1, 1, 15 );
	result += pololuSetParameters( fd, 8, 1, 1, 15 );
	result += pololuSetParameters( fd, 10, 1, 1, 15 );
	result += pololuSetSpeed( fd, 7, 0 );
	result += pololuSetSpeed( fd, 8, 0 );
	result += pololuSetSpeed( fd, 10, 0 );
	result += pololuSetPosition7Bit( fd, 7, 63 );
	result += pololuSetPosition7Bit( fd, 8, 63 );
	result += pololuSetPosition7Bit( fd, 10, 63 );
	result += pololuSetParameters( fd, 1, 1, 1, 15 ) ;
	result += pololuSetNeutral( fd, 1, POLOLU_CH1_NEUTRAL );
	result += pololuSetPosition7Bit( fd, 1, 63 );
	result += pololuSetParameters( fd, 2, 1, 1, 15 );
	result += pololuSetNeutral( fd, 2, POLOLU_CH2_NEUTRAL );
	result += pololuSetPosition7Bit( fd, 2, 63 );
	result += pololuSetParameters( fd, 4, 1, 1, 15 );
	result += pololuSetNeutral( fd, 4, POLOLU_CH4_NEUTRAL );
	result += pololuSetPosition7Bit( fd, 4, 63 );
	result += pololuSetParameters( fd, 5, 1, 1, 15 );
	result += pololuSetNeutral( fd, 5, POLOLU_CH5_NEUTRAL );
	result += pololuSetPosition7Bit( fd, 5, 63 );

	// the total number of bytes sent
	// 5 for each normal command and 6 for each pololuSetNeutral command
	if ( result == 139 ) {
		result = POLOLU_SUCCESS;
	}
	else {
		result = POLOLU_FAILURE;
	}

	// needs a little extra sleep in case this function is followed directly by
	// close()
	usleep( POLOLU_SLEEP );

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
	if( left < -dzRadius ) {
		leftNeutral -= POLOLU_DZ_NEUTRAL;
	}
	if( right > dzRadius ) {
		rightNeutral += POLOLU_DZ_NEUTRAL;
	}
	if( right < -dzRadius ) {
		rightNeutral -= POLOLU_DZ_NEUTRAL;
	}
	if( tail > dzRadius ) {
		tailNeutral += POLOLU_DZ_NEUTRAL;
	}
	if( tail < -dzRadius ) {
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
