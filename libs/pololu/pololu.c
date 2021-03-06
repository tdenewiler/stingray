/*------------------------------------------------------------------------------
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
 *----------------------------------------------------------------------------*/

#include "pololu.h"

/*------------------------------------------------------------------------------
 * int pololuSetup()
 * Initializes communications with the Pololu. Sets up a file descriptor for
 * further communications.  Baud rates from 2000 to 40000 Hz are accepted.
 *----------------------------------------------------------------------------*/

int pololuSetup(char *portname, int baud)
{
	/// Declare variables.
	int fd = -1;
	int result = 0;

	if(portname != NULL) {
		fd = setup_serial(portname, baud);
	}

	if(fd < 0) {
	    printf("POLOLU_SETUP: fd = %d\n", fd);
		return POLOLU_FAILURE;
	}

	/// Initialize channels.
	result = pololuInitializeChannels(fd);

	/// If the channels don't initialize then failure.
	if(result < 0) {
		return POLOLU_FAILURE;
	}

	return fd;
} /* end pololu_setup() */


/*------------------------------------------------------------------------------
 * int pololuSetParameters()
 * Sets the parameters for a channel.  If values out of range are given the
 * function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololuSetParameters(int fd, int channel, int channelOn, int direction, int range)
{
	/// Check ranges.
	int result = 0;
	if((fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (range >= 0)
	        && (range <= 15)) {

		/// demote the ints to chars
		unsigned char c = (unsigned char)channel;
		unsigned char r = (unsigned char)range;

		/// set parameters
		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x00;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]

		/// Set up some masks.
		unsigned char bit5 = 0; // 0 is default for forward in protocol

		if(!direction) {
			bit5 = 32;  // 16 is reverse
		}

		unsigned char bit6 = 0; // 0 is for off
		if(channelOn) {
			bit6 = 64;  // 32 is for on
		}

		/// Set the servo value.
		msg[4] = ((r & 15) | (bit5 & 32) | (bit6 & 64)) & 127;
		result = send_serial(fd, &msg, 5);
	}

	return result;
} /* end pololuSetParameters() */


/*------------------------------------------------------------------------------
 * int pololuSetSpeed()
 * Sets the speed of a channel. If values out of range are given the function
 * fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololuSetSpeed(int fd, int channel, int speed)
{
	/// check ranges
	int result = 0;
	if((fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (speed >= 0) &&
	        (speed <= 127)) {

		/// Demote the ints to chars.
		unsigned char c = (unsigned char)channel;
		unsigned char s = (unsigned char)speed;

		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x01;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]

		/// Send the message.
		msg[4] = s & 127;
		result = send_serial(fd, &msg, 5);
	}

	return result;
} /* end pololuSetSpeed() */


/*------------------------------------------------------------------------------
 * int pololuSetPosition7Bit()
 * Sets the relative position of a channel. If values out of range are given the
 * function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololuSetPosition7Bit(int fd, int channel, int position)
{
	/// Check ranges.
	int result = 0;
	if((fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 0) &&
	        (position <= 127)) {

		/// Set position7bit.
		/// Demote the ints to chars.
		unsigned char c = (unsigned char)channel;
		unsigned char p = (unsigned char)position;

		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x02;  // command #  result+=pololuSetPosition7Bit(fd,1,63);
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]

		/// Send the message.
		msg[4] = p & 127;
		result = send_serial(fd, &msg, 5);
	}

	return result;
} /* end pololuSetPosition7Bit() */


/*------------------------------------------------------------------------------
 * int pololuSetPosition8Bit()
 * Sets the relative position of a channel.  If values out of
 * range are given the function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololuSetPosition8Bit(int fd, int channel, int position)
{
	/// Check ranges.
	int result = 0;
	if((fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 0) &&
	        (position <= 255)) {

		/// Set position8bit.
		/// Demote the ints to chars.
		unsigned char c = (unsigned char)channel;
		unsigned char p = (unsigned char)position;
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x03;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		// 0th bit of msg[4] is 7th bit of position

		if(p & 128) {
			msg[4] = 1;
		}
		else {
			msg[4] = 0;
		}

		msg[5] = (p & 127);

		/// Send the message.
		result = send_serial(fd, &msg, 6);
	}

	return result;
} /* end pololuSetPosition8Bit() */


/*------------------------------------------------------------------------------
 * int pololuSetPositionAbsolute()
 * Sets the absolute position of a channel.  If values out of
 * range are given the function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololuSetPositionAbsolute(int fd, int channel, int position)
{
	/// Check ranges.
	int result = 0;
	if((fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 500) &&
	        (position <= 5500)) {

		/// Set positionAbsolute. Demote the ints to chars.
		unsigned char c = (unsigned char)channel;
		unsigned int p = (unsigned int)position;
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x04;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		// msg[4] is upper 7 bits of position
		msg[4] = (p >> 7) & 127;
		msg[5] = (p & 127);

		/// Send the message.
		result = send_serial(fd, &msg, 6);
	}

	return result;
} /* end pololuSetPositionAbsolute() */


/*------------------------------------------------------------------------------
 * int pololuSetNeutral()
 * Sets the absolute neutral position of a channel.  If values
 * out of range are given the function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololuSetNeutral(int fd, int channel, int position)
{
	/// Set neutral. Check ranges.
	int result = 0;
	if((fd >= 0) &&
	        (channel >= 0) &&
	        (channel <= 15) &&
	        (position >= 500) &&
	        (position <= 5500)) {

		/// Demote the ints to chars.
		unsigned char c = (unsigned char)channel;
		unsigned int p = (unsigned int)position;
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE; // start byte
		msg[1] = POLOLU_DEVICE_ID;  // device ID
		msg[2] = 0x05;  // command #
		msg[3] = c; // channel number

		// [128 64 32 16 8 4 2 1]
		// msg[4] is upper 7 bits of position
		msg[4] = (p >> 7) & 127;
		msg[5] = (p & 127);

		/// Send the message.
		result = send_serial(fd, &msg, 6);
	}

	return result;
} /* end pololuSetNeutral() */


/*------------------------------------------------------------------------------
 * int pololuInitializeChannels()
 * Initializes all channels and sets all channels to neutral.
 *----------------------------------------------------------------------------*/

int pololuInitializeChannels(int fd)
{
	int result = 0;

	/// If invalid fd return failure.
	if(fd < 0) {
		return POLOLU_FAILURE;
	}

	/// THIS IS OUR DEFAULT PROFILE

	/// ======= Left Voith ========

	/// Left Voith Speed
	result += pololuSetParameters(fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetSpeed(fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_SPEED_VOITH);
	result += pololuSetPosition7Bit(fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_NEUTRAL);
	/// Left Voith Servo 1
	result += pololuSetParameters(fd, POLOLU_LEFT_SERVO1, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE) ;
	result += pololuSetNeutral(fd, POLOLU_LEFT_SERVO1, POLOLU_CH1_NEUTRAL);
	result += pololuSetPosition7Bit(fd, POLOLU_LEFT_SERVO1, POLOLU_NEUTRAL);
	/// Left Voith Servo 2
	result += pololuSetParameters(fd, POLOLU_LEFT_SERVO2, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetNeutral(fd, POLOLU_LEFT_SERVO2, POLOLU_CH2_NEUTRAL);
	result += pololuSetPosition7Bit(fd, POLOLU_LEFT_SERVO2, POLOLU_NEUTRAL);

	/// ======= Right Voith ========

	/// Right Voith Speed
	result += pololuSetParameters(fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetSpeed(fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_SPEED_VOITH);
	result += pololuSetPosition7Bit(fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_NEUTRAL);
	/// Right Voith Servo 1
	result += pololuSetParameters(fd, POLOLU_RIGHT_SERVO1, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetNeutral(fd, POLOLU_RIGHT_SERVO1, POLOLU_CH4_NEUTRAL);
	result += pololuSetPosition7Bit(fd, POLOLU_RIGHT_SERVO1, POLOLU_NEUTRAL);
	/// Right Voith Servo 2
	result += pololuSetParameters(fd, POLOLU_RIGHT_SERVO2, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetNeutral(fd, POLOLU_RIGHT_SERVO2, POLOLU_CH5_NEUTRAL);
	result += pololuSetPosition7Bit(fd, POLOLU_RIGHT_SERVO2, POLOLU_NEUTRAL);

	/// ======= Attidude and Depth ========

	/// Left Wing Thruster
	result += pololuSetParameters(fd, POLOLU_LEFT_WING_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetSpeed(fd, POLOLU_LEFT_WING_MOTOR, POLOLU_SPEED_INSTANT);
	result += pololuSetPosition7Bit(fd, POLOLU_LEFT_WING_MOTOR, POLOLU_NEUTRAL);
	/// Right Wing Thruster
	result += pololuSetParameters(fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetSpeed(fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_SPEED_INSTANT);
	result += pololuSetPosition7Bit(fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_NEUTRAL);
	/// Tail Thruster
	result += pololuSetParameters(fd, POLOLU_TAIL_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololuSetSpeed(fd, POLOLU_TAIL_MOTOR, POLOLU_SPEED_INSTANT);
	result += pololuSetPosition7Bit(fd, POLOLU_TAIL_MOTOR, POLOLU_NEUTRAL);

	/// ======= Other Actuators ========
	/// Dropper Servo
	result += pololuSetParameters(fd, POLOLU_DROPPER, POLOLU_ON, POLOLU_FORWARD, POLOLU_DROPPER_RANGE);
	result += pololuSetNeutral(fd, POLOLU_DROPPER, POLOLU_DROPPER_NEUTRAL);
	result += pololuSetPosition7Bit(fd, POLOLU_DROPPER, POLOLU_NEUTRAL);

	/// The total number of bytes sent.
	/// 5 for each normal command and 6 for each pololuSetNeutral command.
	if(result == 155) {
		result = POLOLU_SUCCESS;
	}
	else {
		result = POLOLU_FAILURE;
	}

	/// Needs a little extra sleep in case this function is followed directly by close().
	usleep(POLOLU_SLEEP);

	return result;
} /* end pololuInitializeChannels() */


/*************************HIGHER LEVEL CONTROL FUNCTIONS**********************/

/*------------------------------------------------------------------------------
 * int pololuControlVoiths()
 * This is a higher level function that controls both VSPs together.
 *----------------------------------------------------------------------------*/

int pololuControlVoiths(int fd, int voithThrust, float thrustAngle, int thrust, int yawTorque)
{
	/// The forces need to be in the range [-100,100]. They will then be scaled
	/// to the range [0,128] before being sent to the Pololu. If any force is
	/// outside the acceptable range then an error code will be returned.
	if((fd < 0) ||
	    (voithThrust < 0) ||
	    (voithThrust > POLOLU_SERVO_BOUND) ||
	    (thrust < 0) ||
	    (thrust > POLOLU_SERVO_BOUND) ||
	    (yawTorque < -1 * POLOLU_SERVO_BOUND) ||
	    (yawTorque > POLOLU_SERVO_BOUND)) {
		return POLOLU_FAILURE;
	}

	/// Set up angle variables.
	int bytes = 0;
	float angle = 0;
	angle = thrustAngle; // * (M_PI / 180);

	/// Set up the angle offset in the range [0,360].
	float leftAngleOffset = POLOLU_LEFT_ANGLE_OFFSET * (M_PI / 180);
	float rightAngleOffset = POLOLU_RIGHT_ANGLE_OFFSET * (M_PI / 180);

	/// Calculate the radius values for where the Voith pins should be.
	int radius1 = yawTorque * cos(angle) + thrust; // [MIN,MAX] [ -PID_YAW_TORQUE , PID_YAW_TORQUE + POLOLU_SERVO_BOUND ]
	int radius2 = yawTorque * cos(angle) - thrust; // [MIN,MAX] [ -PID_YAW_TORQUE - POLOLU_SERVO_BOUND , PID_YAW_TORQUE ]

	/// Correction for yaw when adding thrust has a horizontal component.
	/// The yaw torque input is normalized by POLOLU_MAX_YAW_TORQUE and scaled
	/// to POLOLU_YAW_CORRECTION. This result is modulated by the sin of the thrust angle.
	float yawAngleCorrection = yawTorque * (POLOLU_YAW_CORRECTION / POLOLU_MAX_YAW_TORQUE) * sin(angle) * (M_PI / 180);

	/// Calculate the commands to send to the servos to control Voith direction.
	int leftCmd1  = (int)(POLOLU_SERVO_NEUTRAL + POLOLU_SERVO_GAIN * radius1 * cos(angle - yawAngleCorrection + leftAngleOffset));
	int leftCmd2  = (int)(POLOLU_SERVO_NEUTRAL + POLOLU_SERVO_GAIN * radius1 * sin(angle - yawAngleCorrection + leftAngleOffset));
	int rightCmd1 = (int)(POLOLU_SERVO_NEUTRAL - POLOLU_SERVO_GAIN * radius2 * cos(angle + yawAngleCorrection + rightAngleOffset));
	int rightCmd2 = (int)(POLOLU_SERVO_NEUTRAL - POLOLU_SERVO_GAIN * radius2 * sin(angle + yawAngleCorrection + rightAngleOffset));

	/// There is differential thrust in the voiths. The left and right scaling
	/// done here tries to account for this.
	int voithThrustLeft   = voithThrust * POLOLU_VOITH_LEFT_SCALE;
	int voithThrustRight  = voithThrust * POLOLU_VOITH_RIGHT_SCALE;

	/// Scale the Voith thrust value.
	int scaledVoithThrustLeft  = voithThrustLeft * POLOLU_VOITH_GAIN + POLOLU_VOITH_NEUTRAL;
	int scaledVoithThrustRight = voithThrustRight * POLOLU_VOITH_GAIN + POLOLU_VOITH_NEUTRAL;

	/// Send the commands to the Pololu to control servo and motor positions.
	bytes += pololuSetPosition7Bit(fd, POLOLU_LEFT_SERVO1, leftCmd1);
	bytes += pololuSetPosition7Bit(fd, POLOLU_LEFT_SERVO2, leftCmd2);
	bytes += pololuSetPosition7Bit(fd, POLOLU_RIGHT_SERVO1, rightCmd1);
	bytes += pololuSetPosition7Bit(fd, POLOLU_RIGHT_SERVO2, rightCmd2);
	bytes += pololuSetPosition7Bit(fd, POLOLU_LEFT_VOITH_MOTOR, scaledVoithThrustLeft);
	bytes += pololuSetPosition7Bit(fd, POLOLU_RIGHT_VOITH_MOTOR, scaledVoithThrustRight);

	/// Check the number of bytes sent and return success or failure. There are
	/// 5 bytes for each command --> 5 * 6 = 30.
	if(bytes == POLOLU_BYTES_VOITH) {
		return POLOLU_SUCCESS;
	}

	return POLOLU_FAILURE;
} /* end pololuControlVoiths() */


/*------------------------------------------------------------------------------
 * int pololuControlVertical()
 * This is a higher level function that controls the vertical thrusters together.
 *----------------------------------------------------------------------------*/

int pololuControlVertical(int fd, int vertForce, int rollTorque, int pitchTorque)
{
	/// The forces need to be in the range [-100,100]. They will then be scaled
	/// to the range [0,128] before being sent to the Pololu. If any force is
	/// outside the acceptable range then an error code will be returned. */
	if((fd < 0) ||
	    (vertForce < -1 * POLOLU_SERVO_BOUND) ||
	    (vertForce > POLOLU_SERVO_BOUND) ||
	    (rollTorque < -1 * POLOLU_SERVO_BOUND) ||
	    (rollTorque > POLOLU_SERVO_BOUND) ||
	    (pitchTorque < -1 * POLOLU_SERVO_BOUND) ||
	    (pitchTorque > POLOLU_SERVO_BOUND)) {
		return POLOLU_FAILURE;
	}

	/// Set up motor command variables.
	int bytes = 0;
	int leftCmd = 0;
	int rightCmd = 0;
	int tailCmd = 0;
	float left = 0;
	float right = 0;
	float tail = 0;

	/// Set up the motor dead zone variables.
	float dzRadius = POLOLU_DEADZONE;
	float leftNeutral = POLOLU_SERVO_NEUTRAL;
	float rightNeutral = POLOLU_SERVO_NEUTRAL;
	float tailNeutral = POLOLU_SERVO_NEUTRAL;

	/// Calculate the servo commands.
	left = vertForce + rollTorque;
	right = vertForce - rollTorque;
	tail = pitchTorque;

	/// Check the bounds on the servo commands.
	if(left > POLOLU_SERVO_BOUND) {
		left = POLOLU_SERVO_BOUND;
	}
	if(left < -1 * POLOLU_SERVO_BOUND) {
		left = -1 * POLOLU_SERVO_BOUND;
	}
	if(right > POLOLU_SERVO_BOUND) {
		right = POLOLU_SERVO_BOUND;
	}
	if(right < -1 * POLOLU_SERVO_BOUND) {
		right = -1 * POLOLU_SERVO_BOUND;
	}

	/// Need a very short sleep before sending out commands.
	usleep(POLOLU_SLEEP);

	/// The range [-0.1,0.1] is our new dead zone.
	if(left > dzRadius) {
		leftNeutral += POLOLU_DZ_NEUTRAL;
	}
	if(left < -1 * dzRadius) {
		leftNeutral -= POLOLU_DZ_NEUTRAL;
	}
	if(right > dzRadius) {
		rightNeutral += POLOLU_DZ_NEUTRAL;
	}
	if(right < -1 * dzRadius) {
		rightNeutral -= POLOLU_DZ_NEUTRAL;
	}
	if(tail > dzRadius) {
		tailNeutral += POLOLU_DZ_NEUTRAL;
	}
	if(tail < -1 * dzRadius) {
		tailNeutral -= POLOLU_DZ_NEUTRAL;
	}

	/// Actually calculate the values to send to the Pololu now that we have the dead zone accounted for.
	leftCmd = (int)(leftNeutral + POLOLU_NEUTRAL_GAIN * left);
	rightCmd = (int)(rightNeutral + POLOLU_NEUTRAL_GAIN * right);
	tailCmd = (int)(tailNeutral + POLOLU_NEUTRAL_GAIN * tail);

	/// Send the commands to the Pololu to control servo and motor positions.
	bytes += pololuSetPosition7Bit(fd, POLOLU_LEFT_WING_MOTOR, leftCmd);
	bytes += pololuSetPosition7Bit(fd, POLOLU_RIGHT_WING_MOTOR, rightCmd);
	bytes += pololuSetPosition7Bit(fd, POLOLU_TAIL_MOTOR, tailCmd);

	/// Check the number of bytes sent and return success or failure. There are
	/// 5 bytes for each command --> 5 * 3 = 15.
	if(bytes == POLOLU_BYTES_VERTICAL) {
		return POLOLU_SUCCESS;
	}

	return POLOLU_FAILURE;
} /* end pololuControlVertical() */
