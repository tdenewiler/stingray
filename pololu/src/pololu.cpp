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
 * int pololu_setup()
 * Initializes communications with the Pololu. Sets up a file descriptor for
 * further communications. Baud rates from 2000 to 40000 Hz are accepted.
 *----------------------------------------------------------------------------*/

int pololu_setup(char *portname, int baud)
{
	/// Declare variables.
	int fd = -1;

	/// Open the port and check for errors.
	if (portname != NULL) {
		fd = setup_serial(portname, baud);
	}
	if (fd < 0) {
		return POLOLU_FAILURE;
	}

	/// Initialize channels and check for errors.
	if (pololu_initialize_channels(fd) < 0) {
		return POLOLU_FAILURE;
	}

	return fd;
} /* end pololu_setup() */


/*------------------------------------------------------------------------------
 * int pololu_set_parameters()
 * Sets the parameters for a channel. If values out of range are given the
 * function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololu_set_parameters(int fd, int channel, int channelOn, int direction, int range)
{
	/// Check ranges.
	if ((fd >= 0) &&
		(channel >= POLOLU_MIN_CHANNEL) &&
	    (channel <= POLOLU_MAX_CHANNEL) &&
	    (range >= POLOLU_MIN_RANGE) &&
	    (range <= POLOLU_MAX_RANGE)) {

		/// Set parameters. Byte 2 is the command number and Byte 3 is the channel number.
		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_CMD_PARAM;
		msg[3] = (unsigned char)channel;

		/// Set up some masks. 0 is default for forward in protocol. 16 is reverse.
		unsigned char bit5 = 0;
		if (!direction) {
			bit5 = 32;
		}
		/// 0 is for off and 32 is for on.
		unsigned char bit6 = 0;
		if (channelOn) {
			bit6 = 64;
		}

		/// Set the servo value.
		msg[4] = (((unsigned char)range & 15) | (bit5 & 32) | (bit6 & 64)) & 127;
		return(send_serial(fd, &msg, 5));
	}

	return POLOLU_FAILURE;
} /* end pololu_set_parameters() */


/*------------------------------------------------------------------------------
 * int pololu_set_speed()
 * Sets the speed of a channel. If values out of range are given the function
 * fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololu_set_speed(int fd, int channel, int speed)
{
	/// Check ranges.
	if ((fd >= 0) &&
		(channel >= POLOLU_MIN_CHANNEL) &&
	    (channel <= POLOLU_MAX_CHANNEL) &&
	    (speed >= POLOLU_MIN_SPEED) &&
	    (speed <= POLOLU_MAX_SPEED)) {

		/// Set parameters. Byte 2 is the command number and Byte 3 is the channel number.
		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_CMD_SPEED;
		msg[3] = (unsigned char)channel;

		/// Send the message.
		msg[4] = (unsigned char)speed & 127;
		return(send_serial(fd, &msg, 5));
	}

	return POLOLU_FAILURE;
} /* end pololu_set_speed() */


/*------------------------------------------------------------------------------
 * int pololu_set_position_7Bit()
 * Sets the relative position of a channel. If values out of range are given the
 * function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololu_set_position_7Bit(int fd, int channel, int position)
{
	/// Check ranges.
	if ((fd >= 0) &&
		(channel >= POLOLU_MIN_CHANNEL) &&
	    (channel <= POLOLU_MAX_CHANNEL) &&
	    (position >= POLOLU_MIN_7BIT) &&
	    (position <= POLOLU_MAX_7BIT)) {

		/// Set 7 bit position.
		unsigned char msg[5];
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_CMD_7BIT;
		msg[3] = (unsigned char)channel;

		/// Send the message.
		msg[4] = (unsigned char)position & 127;
		return(send_serial(fd, &msg, 5));
	}

	return POLOLU_FAILURE;
} /* end pololu_set_position_7Bit() */


/*------------------------------------------------------------------------------
 * int pololu_set_position_8Bit()
 * Sets the relative position of a channel.  If values out of
 * range are given the function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololu_set_position_8Bit(int fd, int channel, int position)
{
	/// Check ranges.
	if ((fd >= 0) &&
	    (channel >= POLOLU_MIN_CHANNEL) &&
	    (channel <= POLOLU_MAX_CHANNEL) &&
	    (position >= POLOLU_MIN_8BIT) &&
	    (position <= POLOLU_MAX_8BIT)) {

		/// Set 8 bit position.
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_CMD_8BIT;
		msg[3] = (unsigned char)channel;

		if ((unsigned char)position & 128) {
			msg[4] = 1;
		}
		else {
			msg[4] = 0;
		}
		msg[5] = ((unsigned char)position & 127);

		/// Send the message.
		return(send_serial(fd, &msg, 6));
	}

	return POLOLU_FAILURE;
} /* end pololu_set_position_8Bit() */


/*------------------------------------------------------------------------------
 * int pololu_set_position_absolute()
 * Sets the absolute position of a channel.  If values out of
 * range are given the function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololu_set_position_absolute(int fd, int channel, int position)
{
	/// Check ranges.
	if ((fd >= 0) &&
	    (channel >= POLOLU_MIN_CHANNEL) &&
	    (channel <= POLOLU_MAX_CHANNEL) &&
	    (position >= POLOLU_MIN_ABS_POS) &&
	    (position <= POLOLU_MAX_ABS_POS)) {

		/// Set absolute position.
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_CMD_ABS_POS;
		msg[3] = (unsigned char)channel;
		msg[4] = ((unsigned char)position >> 7) & 127;
		msg[5] = ((unsigned char)position & 127);

		/// Send the message.
		return(send_serial(fd, &msg, 6));
	}

	return POLOLU_FAILURE;
} /* end pololu_set_position_absolute() */


/*------------------------------------------------------------------------------
 * int pololu_set_neutral()
 * Sets the absolute neutral position of a channel.  If values
 * out of range are given the function fails and returns 0 bytes sent.
 *----------------------------------------------------------------------------*/

int pololu_set_neutral(int fd, int channel, int position)
{
	/// Set neutral. Check ranges.
	if ((fd >= 0) &&
	    (channel >= POLOLU_MIN_CHANNEL) &&
	    (channel <= POLOLU_MAX_CHANNEL) &&
	    (position >= POLOLU_MIN_ABS_POS) &&
	    (position <= POLOLU_MAX_ABS_POS)) {

		/// Set neutral position.
		unsigned char msg[6];
		msg[0] = POLOLU_START_BYTE;
		msg[1] = POLOLU_DEVICE_ID;
		msg[2] = POLOLU_CMD_NEUTRAL;
		msg[3] = (unsigned char)channel;
		msg[4] = ((unsigned char)position >> 7) & 127;
		msg[5] = ((unsigned char)position & 127);

		/// Send the message.
		return(send_serial(fd, &msg, 6));
	}

	return POLOLU_FAILURE;
} /* end pololu_set_neutral() */


/*------------------------------------------------------------------------------
 * int pololu_initialize_channels()
 * Initializes all channels and sets all channels to neutral.
 *----------------------------------------------------------------------------*/

int pololu_initialize_channels(int fd)
{
	int result = 0;

	/// Check for valid port.
	if (fd < 0) {
		return POLOLU_FAILURE;
	}

	/// ======= Left Voith ========
	/// Left Voith Speed
	result += pololu_set_parameters(fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_speed(fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_SPEED_VOITH);
	result += pololu_set_position_7Bit(fd, POLOLU_LEFT_VOITH_MOTOR, POLOLU_NEUTRAL);
	/// Left Voith Servo 1
	result += pololu_set_parameters(fd, POLOLU_LEFT_SERVO1, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE) ;
	result += pololu_set_neutral(fd, POLOLU_LEFT_SERVO1, POLOLU_CH1_NEUTRAL);
	result += pololu_set_position_7Bit(fd, POLOLU_LEFT_SERVO1, POLOLU_NEUTRAL);
	/// Left Voith Servo 2
	result += pololu_set_parameters(fd, POLOLU_LEFT_SERVO2, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_neutral(fd, POLOLU_LEFT_SERVO2, POLOLU_CH2_NEUTRAL);
	result += pololu_set_position_7Bit(fd, POLOLU_LEFT_SERVO2, POLOLU_NEUTRAL);

	/// ======= Right Voith ========
	/// Right Voith Speed
	result += pololu_set_parameters(fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_speed(fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_SPEED_VOITH);
	result += pololu_set_position_7Bit(fd, POLOLU_RIGHT_VOITH_MOTOR, POLOLU_NEUTRAL);
	/// Right Voith Servo 1
	result += pololu_set_parameters(fd, POLOLU_RIGHT_SERVO1, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_neutral(fd, POLOLU_RIGHT_SERVO1, POLOLU_CH4_NEUTRAL);
	result += pololu_set_position_7Bit(fd, POLOLU_RIGHT_SERVO1, POLOLU_NEUTRAL);
	/// Right Voith Servo 2
	result += pololu_set_parameters(fd, POLOLU_RIGHT_SERVO2, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_neutral(fd, POLOLU_RIGHT_SERVO2, POLOLU_CH5_NEUTRAL);
	result += pololu_set_position_7Bit(fd, POLOLU_RIGHT_SERVO2, POLOLU_NEUTRAL);

	/// ======= Attidude and Depth ========
	/// Left Wing Thruster
	result += pololu_set_parameters(fd, POLOLU_LEFT_WING_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_speed(fd, POLOLU_LEFT_WING_MOTOR, POLOLU_SPEED_INSTANT);
	result += pololu_set_position_7Bit(fd, POLOLU_LEFT_WING_MOTOR, POLOLU_NEUTRAL);
	/// Right Wing Thruster
	result += pololu_set_parameters(fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_speed(fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_SPEED_INSTANT);
	result += pololu_set_position_7Bit(fd, POLOLU_RIGHT_WING_MOTOR, POLOLU_NEUTRAL);
	/// Tail Thruster
	result += pololu_set_parameters(fd, POLOLU_TAIL_MOTOR, POLOLU_ON, POLOLU_FORWARD, POLOLU_DEFAULT_RANGE);
	result += pololu_set_speed(fd, POLOLU_TAIL_MOTOR, POLOLU_SPEED_INSTANT);
	result += pololu_set_position_7Bit(fd, POLOLU_TAIL_MOTOR, POLOLU_NEUTRAL);

	/// ======= Other Actuators ========
	/// Dropper Servo
	result += pololu_set_parameters(fd, POLOLU_DROPPER, POLOLU_ON, POLOLU_FORWARD, POLOLU_DROPPER_RANGE);
	result += pololu_set_neutral(fd, POLOLU_DROPPER, POLOLU_DROPPER_NEUTRAL);
	result += pololu_set_position_7Bit(fd, POLOLU_DROPPER, POLOLU_NEUTRAL);

	/// The total number of bytes sent.
	/// 5 for each normal command and 6 for each pololu_set_neutral command.
	if (result == POLOLU_INIT_TOTAL) {
		result = POLOLU_SUCCESS;
	}
	else {
		result = POLOLU_FAILURE;
	}

	/// Needs a little extra sleep in case this function is followed directly by close().
	usleep(POLOLU_SLEEP);

	return result;
} /* end pololu_initialize_channels() */


/*------------------------------------------------------------------------------
 * int pololu_control_voiths()
 * This is a higher level function that controls both VSPs together.
 *----------------------------------------------------------------------------*/

int pololu_control_voiths(int fd, int voith_thrust, float thrust_angle, int thrust, int yaw_torque)
{
	/// The forces need to be in the range [-100,100]. They will then be scaled
	/// to the range [0,128] before being sent to the Pololu. If any force is
	/// outside the acceptable range then an error code will be returned.
	if ((fd < 0) ||
	    (voith_thrust < POLOLU_MIN_THRUST) ||
	    (voith_thrust > POLOLU_SERVO_BOUND) ||
	    (thrust < POLOLU_MIN_THRUST) ||
	    (thrust > POLOLU_SERVO_BOUND) ||
	    (yaw_torque < -1 * POLOLU_SERVO_BOUND) ||
	    (yaw_torque > POLOLU_SERVO_BOUND)) {
		return POLOLU_FAILURE;
	}

	/// Set up angle variables.
	int bytes = 0;

	/// Set up the angle offset in the range [0,360].
	float left_angle_offset = POLOLU_LEFT_ANGLE_OFFSET * (M_PI / 180);
	float right_angle_offset = POLOLU_RIGHT_ANGLE_OFFSET * (M_PI / 180);

	/// Calculate the radius values for where the Voith pins should be.
	int radius1 = yaw_torque * cos(thrust_angle) + thrust;
	int radius2 = yaw_torque * cos(thrust_angle) - thrust;

	/// Correction for yaw when adding thrust has a horizontal component. The yaw torque input is normalized by
	/// POLOLU_MAX_YAW_TORQUE and scaled to POLOLU_YAW_CORRECTION. This result is modulated by the sin of the thrust angle.
	float yaw_angle_correction = yaw_torque * (POLOLU_YAW_CORRECTION / POLOLU_MAX_YAW_TORQUE) * sin(thrust_angle) * (M_PI / 180);

	/// Calculate the commands to send to the servos to control Voith direction.
	int leftCmd1  = (int)(POLOLU_SERVO_NEUTRAL + POLOLU_SERVO_GAIN * radius1 * cos(thrust_angle - yaw_angle_correction + left_angle_offset));
	int leftCmd2  = (int)(POLOLU_SERVO_NEUTRAL + POLOLU_SERVO_GAIN * radius1 * sin(thrust_angle - yaw_angle_correction + left_angle_offset));
	int rightCmd1 = (int)(POLOLU_SERVO_NEUTRAL - POLOLU_SERVO_GAIN * radius2 * cos(thrust_angle + yaw_angle_correction + right_angle_offset));
	int rightCmd2 = (int)(POLOLU_SERVO_NEUTRAL - POLOLU_SERVO_GAIN * radius2 * sin(thrust_angle + yaw_angle_correction + right_angle_offset));

	/// There is differential thrust in the voiths. The left and right scaling done here tries to account for this.
	int voith_thrust_left   = voith_thrust * POLOLU_VOITH_LEFT_SCALE;
	int voith_thrust_right  = voith_thrust * POLOLU_VOITH_RIGHT_SCALE;

	/// Scale the Voith thrust value.
	int scaled_voith_thrust_left  = voith_thrust_left * POLOLU_VOITH_GAIN + POLOLU_VOITH_NEUTRAL;
	int scaled_voith_thrust_right = voith_thrust_right * POLOLU_VOITH_GAIN + POLOLU_VOITH_NEUTRAL;

	/// Send the commands to the Pololu to control servo and motor positions.
	bytes += pololu_set_position_7Bit(fd, POLOLU_LEFT_SERVO1, leftCmd1);
	bytes += pololu_set_position_7Bit(fd, POLOLU_LEFT_SERVO2, leftCmd2);
	bytes += pololu_set_position_7Bit(fd, POLOLU_RIGHT_SERVO1, rightCmd1);
	bytes += pololu_set_position_7Bit(fd, POLOLU_RIGHT_SERVO2, rightCmd2);
	bytes += pololu_set_position_7Bit(fd, POLOLU_LEFT_VOITH_MOTOR, scaled_voith_thrust_left);
	bytes += pololu_set_position_7Bit(fd, POLOLU_RIGHT_VOITH_MOTOR, scaled_voith_thrust_right);

	/// Check the number of bytes sent. There are 5 bytes for each command --> 5 * 6 = 30.
	if (bytes == POLOLU_BYTES_VOITH) {
		return POLOLU_SUCCESS;
	}

	return POLOLU_FAILURE;
} /* end pololu_control_voiths() */


/*------------------------------------------------------------------------------
 * int pololu_control_vertical()
 * This is a higher level function that controls the vertical thrusters together.
 *----------------------------------------------------------------------------*/

int pololu_control_vertical(int fd, int vert_force, int roll_torque, int pitch_torque)
{
	/// The forces need to be in the range [-100,100]. They will then be scaled
	/// to the range [0,128] before being sent to the Pololu. If any force is
	/// outside the acceptable range then an error code will be returned. */
	if ((fd < 0) ||
	    (vert_force < -1 * POLOLU_SERVO_BOUND) ||
	    (vert_force > POLOLU_SERVO_BOUND) ||
	    (roll_torque < -1 * POLOLU_SERVO_BOUND) ||
	    (roll_torque > POLOLU_SERVO_BOUND) ||
	    (pitch_torque < -1 * POLOLU_SERVO_BOUND) ||
	    (pitch_torque > POLOLU_SERVO_BOUND)) {
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
	float dz_radius = POLOLU_DEADZONE;
	float left_neutral = POLOLU_SERVO_NEUTRAL;
	float right_neutral = POLOLU_SERVO_NEUTRAL;
	float tail_neutral = POLOLU_SERVO_NEUTRAL;

	/// Calculate the servo commands.
	left = vert_force + roll_torque;
	right = vert_force - roll_torque;
	tail = pitch_torque;

	/// Check the bounds on the servo commands.
	if (left > POLOLU_SERVO_BOUND) {
		left = POLOLU_SERVO_BOUND;
	}
	if (left < -1 * POLOLU_SERVO_BOUND) {
		left = -1 * POLOLU_SERVO_BOUND;
	}
	if (right > POLOLU_SERVO_BOUND) {
		right = POLOLU_SERVO_BOUND;
	}
	if (right < -1 * POLOLU_SERVO_BOUND) {
		right = -1 * POLOLU_SERVO_BOUND;
	}

	/// Need a very short sleep before sending out commands.
	usleep(POLOLU_SLEEP);

	/// The range [-0.1,0.1] is our new dead zone.
	if (left > dz_radius) {
		left_neutral += POLOLU_DZ_NEUTRAL;
	}
	if (left < -1 * dz_radius) {
		left_neutral -= POLOLU_DZ_NEUTRAL;
	}
	if (right > dz_radius) {
		right_neutral += POLOLU_DZ_NEUTRAL;
	}
	if (right < -1 * dz_radius) {
		right_neutral -= POLOLU_DZ_NEUTRAL;
	}
	if (tail > dz_radius) {
		tail_neutral += POLOLU_DZ_NEUTRAL;
	}
	if (tail < -1 * dz_radius) {
		tail_neutral -= POLOLU_DZ_NEUTRAL;
	}

	/// Actually calculate the values to send to the Pololu now that we have the dead zone accounted for.
	leftCmd = (int)(left_neutral + POLOLU_NEUTRAL_GAIN * left);
	rightCmd = (int)(right_neutral + POLOLU_NEUTRAL_GAIN * right);
	tailCmd = (int)(tail_neutral + POLOLU_NEUTRAL_GAIN * tail);

	/// Send the commands to the Pololu to control servo and motor positions.
	bytes += pololu_set_position_7Bit(fd, POLOLU_LEFT_WING_MOTOR, leftCmd);
	bytes += pololu_set_position_7Bit(fd, POLOLU_RIGHT_WING_MOTOR, rightCmd);
	bytes += pololu_set_position_7Bit(fd, POLOLU_TAIL_MOTOR, tailCmd);

	/// Check the number of bytes sent and return success or failure. There are
	/// 5 bytes for each command --> 5 * 3 = 15.
	if (bytes == POLOLU_BYTES_VERTICAL) {
		return POLOLU_SUCCESS;
	}

	return POLOLU_FAILURE;
} /* end pololu_control_vertical() */
