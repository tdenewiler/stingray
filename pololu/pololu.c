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
 * Globals:     None.
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
 * Globals:     None.
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
	usleep( POLOLU_SLEEP );
	// check ranges
	int result = 0;

	if ( ( fd >= 0 ) &&
	        ( channel >= 0 ) &&
	        ( channel <= 15 ) &&
	        ( range >= 0 )
	        && ( range <= 15 ) ) {

		// demote the ints to chars
		unsigned char c = ( unsigned char )channel;
		unsigned char r = ( unsigned char )range;

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
		msg[4] = ( ( r & 15 ) | ( bit5 & 32 ) | ( bit6 & 64 ) ) & 127;

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
 * Globals:     None.
 *
 *****************************************************************************/

int pololuSetSpeed( int fd,
                    int channel,
                    int speed
                  )
{
	// sleep for a bit
	usleep( POLOLU_SLEEP );

	// set speed
	int result = 0;
	// check ranges

	if ( ( fd >= 0 ) &&
	        ( channel >= 0 ) &&
	        ( channel <= 15 ) &&
	        ( speed >= 0 ) &&
	        ( speed <= 127 ) ) {

		// demote the ints to chars
		unsigned char c = ( unsigned char )channel;
		unsigned char s = ( unsigned char )speed;

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
 * Globals:     None.
 *
 *****************************************************************************/

int pololuSetPosition7Bit( int fd,
                           int channel,
                           int position
                         )
{
	// sleep for a bit
	usleep( POLOLU_SLEEP );

	// check ranges
	int result = 0;

	if ( ( fd >= 0 ) &&
	        ( channel >= 0 ) &&
	        ( channel <= 15 ) &&
	        ( position >= 0 ) &&
	        ( position <= 127 ) ) {

		// set position7bit
		// demote the ints to chars
		unsigned char c = ( unsigned char )channel;
		unsigned char p = ( unsigned char )position;

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
 * Globals:     None.
 *
 *****************************************************************************/

int pololuSetPosition8Bit( int fd,
                           int channel,
                           int position
                         )
{
	// sleep for a bit
	usleep( POLOLU_SLEEP );

	// check ranges
	int result = 0;

	if ( ( fd >= 0 ) &&
	        ( channel >= 0 ) &&
	        ( channel <= 15 ) &&
	        ( position >= 0 ) &&
	        ( position <= 255 ) ) {

		// set position8bit
		// demote the ints to chars
		unsigned char c = ( unsigned char )channel;
		unsigned char p = ( unsigned char )position;
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
 * Globals:     None.
 *
 *****************************************************************************/
int pololuSetPositionAbsolute( int fd,
                               int channel,
                               int position
                             )
{
	// sleep for a bit
	usleep( POLOLU_SLEEP );

	// check ranges
	int result = 0;

	if ( ( fd >= 0 ) &&
	        ( channel >= 0 ) &&
	        ( channel <= 15 ) &&
	        ( position >= 500 ) &&
	        ( position <= 5500 ) ) {

		// set positionAbsolute
		// demote the ints to chars
		unsigned char c = ( unsigned char )channel;
		unsigned int p = ( unsigned int )position;
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
 * Globals:     None.
 *
 *****************************************************************************/
int pololuSetNeutral( int fd,
                      int channel,
                      int position
                    )
{
	// sleep for a bit
	usleep( POLOLU_SLEEP );

	// set neutral
	// check ranges
	int result = 0;

	if ( ( fd >= 0 ) &&
	        ( channel >= 0 ) &&
	        ( channel <= 15 ) &&
	        ( position >= 500 ) &&
	        ( position <= 5500 ) ) {

		// demote the ints to chars
		unsigned char c = ( unsigned char )channel;
		unsigned int p = ( unsigned int )position;
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
 * Globals:     None.
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
	result += pololuSetParameters( fd, 9, 1, 1, 15 );
	result += pololuSetSpeed( fd, 7, 0 );
	result += pololuSetSpeed( fd, 8, 0 );
	result += pololuSetSpeed( fd, 9, 0 );
	result += pololuSetPosition7Bit( fd, 7, 63 );
	result += pololuSetPosition7Bit( fd, 8, 63 );
	result += pololuSetPosition7Bit( fd, 9, 63 );
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
 * Title:   int controlVoiths( int voithThrust,
 *                             float thrustAngle,
 *                             int thrust,
 *                             int yawTorque
 *                            )
 *
 * Description: This is a higher level function that controls both
 *              VSPs together.
 *
 * Input:
 *
 *
 * Output:      1 for success and -1 for failure
 *
 * Globals:     None.
 *
 *****************************************************************************/
int controlVoiths( int fd,
                   int voithThrust,
                   float thrustAngle,
                   int thrust,
                   int yawTorque
                 )
{
	// voithThrust: between 0 and 100
	// thrustAngle: between 0 and 360
	// thrust: between -100 and 100
	// yawTorque: between -100 and 100

	// check bounds on all inputs
	// if bounds don't check then we return -1
	//if ( ( fd < 0 ) ||
	        //( voithThrust < 0 ) ||
	        //( voithThrust > 100 ) ||
	        //( thrustAngle <= -M_PI ) ||
	        //( thrustAngle >= M_PI ) ||
	        //( thrust < -100 ) ||
	        //( thrust > 100 ) ||
	        //( yawTorque < -100 ) ||
	        //( yawTorque > 100 ) ) {
		//return POLOLU_FAILURE;
	//}
	if ( ( fd < 0 ) ||
	        ( voithThrust < 0 ) ||
	        ( voithThrust > 100 ) ||
	        ( thrust < -100 ) ||
	        ( thrust > 100 ) ||
	        ( yawTorque < -100 ) ||
	        ( yawTorque > 100 ) ) {
		return POLOLU_FAILURE;
	}
	float angle = 0;

	// unchanging
	angle = thrustAngle; // * ( M_PI / 180 );

	// angle offsets are between 0 and 360
	// 250 original
	float leftAngleOffset = 230 * ( M_PI / 180 );

	// 40 original
	float rightAngleOffset = 40 * ( M_PI / 180 );

	// radius1 and radius2 should be between -100 and 100
	int radius1 = yawTorque + thrust;
	int radius2 = yawTorque - thrust;

	// the commands to send to the servos
	int leftCmd1 = ( int )( 63.5 + 0.2 * radius1 * cos( angle + leftAngleOffset ) );
	int leftCmd2 = ( int )( 63.5 + 0.2 * radius1 * sin( angle + leftAngleOffset ) );
	int rightCmd1 = ( int )( 63.5 - 0.2 * radius2 * cos( angle + rightAngleOffset ) );
	int rightCmd2 = ( int )( 63.5 - 0.2 * radius2 * sin( angle + rightAngleOffset ) );

	// I think integer arithmatic should work -AM
	int scaledVoithThrust = ( voithThrust * 64 ) / 100 + 63;

	// now set all of the positions
	// there should be 5 bytes sent for each command
	int bytes = 0;

	bytes += pololuSetPosition7Bit( fd, 1, leftCmd1 );
	bytes += pololuSetPosition7Bit( fd, 2, leftCmd2 );
	bytes += pololuSetPosition7Bit( fd, 4, rightCmd1 );
	bytes += pololuSetPosition7Bit( fd, 5, rightCmd2 );
	bytes += pololuSetPosition7Bit( fd, 0, scaledVoithThrust );
	bytes += pololuSetPosition7Bit( fd, 3, scaledVoithThrust );

	// check the number of bytes sent and return success or failure
	if ( bytes == 30 ) {
		//fprintf( stderr, "DEBUG controlVoiths SUCCESS, L1 = %d, L2 = %d, R1 = %d, R2 = %d, voithThrust = %d\n"
		         //, leftCmd1
		         //, leftCmd2
		         //, rightCmd1
		         //, rightCmd2
		         //, voithThrust
		       //);
		return POLOLU_SUCCESS;
	}
	else {
		//fprintf( stderr, "DEBUG controlVoiths ERROR, %d bytes sent.\n", bytes );
		return POLOLU_FAILURE;
	}
} /* end controlVoiths() */


/******************************************************************************
 *
 * Title:   int controlVertical( int fd,
 *                               int vertForce,
 *                               int rollTorque,
 *                               int pitchTorque
 *                              )
 *
 * Description: The is a higher level function that controls the vertical
 *              thrusters together.
 *
 * Input:
 *
 * Output:
 *
 * Globals:     None.
 *
 *****************************************************************************/

int controlVertical( int fd,
                     int vertForce,
                     int rollTorque,
                     int pitchTorque
                   )
{
	// between -100 to 100 and will be scaled to 0-128
	// check bounds on all inputs
	// if bounds don't check then we return -1
	if ( ( fd < 0 ) ||
	        ( vertForce < -100 ) ||
	        ( vertForce > 100 ) ||
	        ( rollTorque < -100 ) ||
	        ( rollTorque > 100 ) ||
	        ( pitchTorque < -100 ) ||
	        ( pitchTorque > 100 ) ) {
		return POLOLU_FAILURE;
	}

	int leftCmd = 0;
	int rightCmd = 0;
	int tailCmd = 0;
	float left, right, tail;

	// calculate the servo commands
	left = vertForce + rollTorque;
	right = vertForce - rollTorque;

	// check bounds
	if ( left > 100 ) left = 100;
	if ( left < -100 ) left = -100;
	if ( right > 100 ) right = 100;
	if ( right < -100 ) right = -100;

	usleep( POLOLU_SLEEP );
	tail = pitchTorque;

	// account for dead zone here
	float dzRadius = 0.1;
	float leftNeutral = 63.5;
	float rightNeutral = 63.5;
	float tailNeutral = 63.5;

	// -0.1-0.1 is our new dead zone
	if ( left > dzRadius ) leftNeutral = 67.5;
	if ( left < -dzRadius ) leftNeutral = 59.5;
	if ( right > dzRadius ) rightNeutral = 67.5;
	if ( right < -dzRadius ) rightNeutral = 59.5;
	if ( tail > dzRadius ) tailNeutral = 67.5;
	if ( tail < -dzRadius ) tailNeutral = 59.5;

	leftCmd = ( int )( leftNeutral + 0.605 * left );
	rightCmd = ( int )( rightNeutral + 0.605 * right );
	tailCmd = ( int )( tailNeutral + 0.605 * tail );

	// now set all of the positions
	// there should be 5 bytes sent for each command
	int bytes = 0;
	bytes += pololuSetPosition7Bit( fd, 7, leftCmd );
	bytes += pololuSetPosition7Bit( fd, 8, rightCmd );
	bytes += pololuSetPosition7Bit( fd, 9, tailCmd );

	// check the number of bytes sent and return success or failure
	if ( bytes == 15 ) {
		//fprintf( stderr, "DEBUG: controlVertical SUCCESS, L = %d, R = %d, T = %d\n",
		         //leftCmd,
		         //rightCmd,
		         //tailCmd
		       //);
		return POLOLU_SUCCESS;
	}
	else {
		//fprintf( stderr, "DEBUG: controlVertical ERROR, bytes = %d\n", bytes );
		return POLOLU_FAILURE;
	}
} /* end controlVertical() */
