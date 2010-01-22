/*------------------------------------------------------------------------------
 *
 *  Title:        microstrain.c
 *
 *  Description:  Sending and receiving data with the MicroStrain IMU. We are
 *                using model 3DM-GX1. The API is described in the document
 *                3DM_GX1_Data_Communication_Protocol_203101.pdf. See
 *                www.microstrain.com for more details.
 *
 *----------------------------------------------------------------------------*/

#include "microstrain.h"

/*------------------------------------------------------------------------------
 * int mstrain_setup()
 * Initializes communications with the Microstrain IMU. Sets up a file
 * descriptor for further communications.
 *----------------------------------------------------------------------------*/

int mstrain_setup(char *portname, int baud)
{
	/// Declare variables.
	int fd = -1;

	/// Open serial port.
	if(portname != NULL) {
		fd = setup_serial(portname, baud);
	}

	return fd;
} /* end mstrain_setup() */


/*------------------------------------------------------------------------------
 * int mstrain_serial_number()
 * Asks for serial number from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_serial_number(int fd, int *serial_number)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_F1;
	int status = 0;
	char response[response_length];
	char cmd = (char)IMU_SERIAL_NUMBER;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(SERIAL_EXTRA_DELAY_LENGTH);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	*serial_number = convert2int(&response[1]);

	return 1;
} /* end mstrain_serial_number() */


/*------------------------------------------------------------------------------
 * int mstrain_temperature()
 * Gets temperature from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_temperature(int fd, float *temp)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_07;
	int status = 0;
	char response[response_length];
	char cmd = (char)IMU_TEMPERATURE;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(SERIAL_EXTRA_DELAY_LENGTH);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	*temp = convert2int(&response[1]);

	/// This conversion is from the 3DM-GX1 manual.
	*temp = (((*temp * 5.0 / 65536.0) - 0.5) * 100.0);

	return 1;
} /* end mstrain_temperature() */


/*------------------------------------------------------------------------------
 * int mstrain_orientation()
 * Asks for the gyro-stabilized orientation matrix from the IMU.
 *----------------------------------------------------------------------------*/

int mstrain_orientation(int fd, int gyro_stab, float *orient[3][3])
{
	/// Declare variables.
	char cmd = 0;
	int response_length = 0;

	if(gyro_stab) {
		cmd = IMU_GYRO_STAB_ORIENT_MATRIX;
		response_length = (int)IMU_LENGTH_0B;
	}
	else {
		cmd = IMU_INST_ORIENT_MATRIX;
		response_length = (int)IMU_LENGTH_0A;
	}

	int status;

	int ii;
	int jj;
	char response[response_length];

	/// Conversion factors are from the 3DM-GX1 manual.
	float orient_convert_factor = 8192.0;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(SERIAL_EXTRA_DELAY_LENGTH);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	for(ii = 0; ii < 3; ii++) {
		for(jj = 0; jj < 3; jj++) {
			*orient[jj][ii] = (float)convert2short(&response[1 + ii * 2])
			                  / orient_convert_factor;
		}
	}

	return 1;
} /* end mstrain_orientation() */


/*------------------------------------------------------------------------------
 * int mstrain_vectors()
 * Asks for vectors from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_vectors(int fd, int gyro_stab, float *mag, float *accel, float *ang_rate)
{
	/// Declare variables.
	char cmd = 0;
	int response_length = 0;

	if(gyro_stab) {
		cmd = IMU_GYRO_STAB_VECTORS;
		response_length = (int)IMU_LENGTH_02;
	}
	else {
		cmd = IMU_INST_VECTORS;
		response_length = (int)IMU_LENGTH_03;
	}

	int status = 0;
	int ii = 0;
	char response[response_length];

	/// Conversion factors are from the 3DM-GX1 manual.
	float mag_convert_factor = 16384.0; //32768000.0 / 2000.0;
	float accel_convert_factor = 4681.142857143; //32768000.0 / 7000.0;
	float ang_rate_convert_factor = 3855.058823529; //32768000.0 / 8500.0;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	for(ii = 0; ii < 3; ii++) {
		mag[ii] = (float)convert2short(&response[1 + ii * 2]) /
		          mag_convert_factor;
		accel[ii] = (float)convert2short(&response[7 + ii * 2]) /
		            accel_convert_factor;
		ang_rate[ii] = (float)convert2short(&response[13 + ii * 2]) /
		               ang_rate_convert_factor;
	}

	return 1;
} /* end mstrain_vectors() */


/*------------------------------------------------------------------------------
 * int mstrain_euler_angles()
 * Asks for Euler angles from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_euler_angles(int fd, float *roll, float *pitch, float *yaw)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_0E;
	int status = 0;
	char response[response_length];
	char cmd = (char)IMU_GYRO_STAB_EULER_ANGLES;
	/// Conversion factor from the 3DM-GX1 manual.
	float euler_convert_factor = 360.0 / 65536.0;

	/// Temporary variables for checksum calculation. The checksum is
	/// found by adding up all of the values of the short ints that are
	/// returned by the IMU. */
	short int cs_pitch = 0;
	short int cs_roll = 0;
	short int cs_yaw = 0;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	/// Convert bytes to short ints.
	cs_roll  = convert2short(&response[1]);
	cs_pitch = convert2short(&response[3]);
	cs_yaw   = convert2short(&response[5]);

	/// Set argument pointers to the temp values.
	*roll = cs_roll * euler_convert_factor;
	*pitch = cs_pitch * euler_convert_factor;
	*yaw = cs_yaw * euler_convert_factor;

	/// Convert the Euler angles from (-180,180] to (0,360].
	if(*roll < 0) {
		*roll += 360;
	}
	if(*pitch < 0) {
		*pitch += 360;
	}
	if(*yaw < 0) {
		*yaw += 360;
	}

	return 1;
} /* end mstrain_euler_angles() */


/*------------------------------------------------------------------------------
 * int mstrain_quaternions()
 * Asks for quaternions from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_quaternions(int fd, int gyro_stab, float *quat[4])
{
	/// Declare variables.
	int status = 0;
	int ii = 0;
	int response_length = 0;
	char cmd = 0;
	/// Conversion factor from the 3DM-GX1 manual.
	float convert_factor = 8192.0;

	if(gyro_stab) {
		cmd = IMU_GYRO_STAB_QUAT;
		response_length = (int)IMU_LENGTH_05;
	}
	else {
		cmd = IMU_INST_QUAT;
		response_length = (int)IMU_LENGTH_04;
	}

	char response[response_length];

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));
	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	for(ii = 0; ii < 4; ii++) {
		*quat[ii] = (float)convert2short(&response[1 + ii * 2])
		            / convert_factor;
	}

	return 1;
} /* end mstrain_quaternions() */


/*------------------------------------------------------------------------------
 * int mstrain_quaternions_vectors()
 * Asks for quaternions and vectors from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_quaternions_vectors(int fd, float *quat[4], float *mag[3], float *accel[3], float *ang_rate[3])
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_0C;
	int status = 0;
	int ii = 0;
	char response[response_length];
	char cmd = 0;

	/// Conversion factors are from the 3DM-GX1 manual.
	float mag_convert_factor = 32768000.0 / 2000.0;
	float accel_convert_factor = 3276800.0 / 7000.0;
	float ang_rate_convert_factor = 32768000.0 / 8500.0;
	float stab_convert_factor = 8192.0;

	cmd = IMU_GYRO_STAB_QUAT_VECTORS;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	for(ii = 0; ii < 4; ii++) {
		*quat[ii] = (float)convert2short(&response[1 + ii * 2])
		            / stab_convert_factor;
	}

	for(ii = 0; ii < 3; ii++) {
		*mag[ii]      = (float)convert2short(&response[9 + ii * 2])
		                / mag_convert_factor;
		*accel[ii]    = (float)convert2short(&response[15 + ii * 2])
		                / accel_convert_factor;
		*ang_rate[ii] = (float)convert2short(&response[21 + ii * 2])
		                / ang_rate_convert_factor;
	}

	return 1;
} /* end mstrain_quaternions_vectors() */


/*------------------------------------------------------------------------------
 * int mstrain_euler_vectors()
 * Asks for Euler Angles and vectors from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_euler_vectors(int fd, float *roll, float *pitch, float *yaw, float *accel, float *ang_rate)
{
	/// Declare variables.
	//int response_length = (int)IMU_LENGTH_31; // Gyro-stabilized
	int response_length = (int)IMU_LENGTH_0D; // Instantaneous
	int status = 0;
	int ii = 0;
	char response[response_length];
	char cmd = 0;

	/// Temporary variables for checksum calculation. The checksum is
	/// found by adding up all of the values of the short ints that are
	/// returned by the IMU.
	short int cs_pitch = 0;
	short int cs_roll = 0;
	short int cs_yaw = 0;
	short int cs_accel[3] = {0};
	short int cs_ang_rate[3] = {0};
	short int cs_timer_ticks = 0;
	short int cs_total = 0;

	/// Conversion factors are from the 3DM-GX1 manual.
	float accel_convert_factor = 3276800.0 / 7000.0;
	float ang_rate_convert_factor = 32768000.0 / 8500.0;
	float euler_convert_factor = 360.0 / 65536.0;

	//cmd = IMU_GYRO_STAB_EULER_VECTORS;
	cmd = IMU_INST_EULER_ANGLES;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
		//if(response[0] != IMU_GYRO_STAB_EULER_VECTORS) {
		if(response[0] != cmd) {
			printf("MSTRAIN_EULER_VECTORS: ***** HEADER ERROR *****\n");
			return IMU_ERROR_HEADER;
		}
	}

	if(status != response_length) {
		printf("MSTRAIN_EULER_VECTORS: ***** LENGTH ERROR *****\n");
		return IMU_ERROR_LENGTH;
	}

	/// Convert bytes to short ints.
	cs_roll  = convert2short(&response[1]);
	cs_pitch = convert2short(&response[3]);
	cs_yaw   = convert2short(&response[5]);

	for(ii = 0; ii < 3; ii++) {
		cs_accel[ii]    = convert2short(&response[7 + ii * 2]);
		cs_ang_rate[ii] = convert2short(&response[13 + ii * 2]);
	}

	cs_timer_ticks =  convert2short(&response[19]);

	/// Calculate the checksum.
	cs_total = response[0] + cs_roll + cs_pitch + cs_yaw + cs_accel[0] +
		cs_accel[1] + cs_accel[2] + cs_ang_rate[0] + cs_ang_rate[1] +
		cs_ang_rate[2] + cs_timer_ticks;
	if(cs_total != convert2short(&response[21])) {
		printf("MSTRAIN_EULER_VECTORS: ***** CHECKSUM ERROR *****\n");
		return IMU_ERROR_CHECKSUM;
	}

	/// Try the checksum function.
	if(!mstrain_calc_checksum(response, response_length)) {
		//printf("MSTRAIN_EULER_VECTORS: ***** CHECKSUM FUNCTION *****\n");
		//return IMU_ERROR_CHECKSUM;
	}

	/// Set argument pointers to the temp values.
	*roll = cs_roll * euler_convert_factor;
	*pitch = cs_pitch * euler_convert_factor;
	*yaw = cs_yaw * euler_convert_factor;

	for(ii = 0; ii < 3; ii++) {
		accel[ii]    = (float)cs_accel[ii] / accel_convert_factor;
		ang_rate[ii] = (float)cs_ang_rate[ii] / ang_rate_convert_factor;
	}

	/// Convert the Euler angles from (-180,180] to (0,360].
	if(*roll < 0) {
		*roll += 360;
	}
	if(*pitch < 0) {
		*pitch += 360;
	}
	if(*yaw < 0) {
		*yaw += 360;
	}

	return 1;
} /* end mstrain_euler_vectors() */


/*------------------------------------------------------------------------------
 * int mstrain_set_tare()
 * Set tare for coordinate system.
 *----------------------------------------------------------------------------*/

int mstrain_set_tare(int fd)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_0F;
	int status = 0;
	char response[response_length];
	char cmd[4];

	/// Set the command byte.
	cmd[0] = IMU_TARE_COORDINATE_SYSTEM;

	/// Set the command data bytes.
	cmd[1] = IMU_TARE_BYTE1;
	cmd[2] = IMU_TARE_BYTE2;
	cmd[3] = IMU_TARE_BYTE3;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		status = serial_bytes_available(fd);
		if(status < response_length) {
			usleep(SERIAL_EXTRA_DELAY_LENGTH);
		}
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	return 1;
} /* end mstrain_set_tare() */


/*------------------------------------------------------------------------------
 * int mstrain_remove_tare()
 * Removes tare for coordinate system.
 *----------------------------------------------------------------------------*/

int mstrain_remove_tare(int fd)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_11;
	int status = 0;
	char response[response_length];
	char cmd[4];

	/// Set the command byte.
	cmd[0] = IMU_REMOVE_TARE;

	/// Set the command data bytes.
	cmd[1] = IMU_TARE_BYTE1;
	cmd[2] = IMU_TARE_BYTE2;
	cmd[3] = IMU_TARE_BYTE3;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		status = serial_bytes_available(fd);
		if(status < response_length) {
			usleep(SERIAL_EXTRA_DELAY_LENGTH);
		}
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	return 1;
} /* end mstrain_remove_tare() */


/*------------------------------------------------------------------------------
 * int mstrain_calc_checksum()
 * Calculate the checksum for a message..
 *----------------------------------------------------------------------------*/

int mstrain_calc_checksum(char *buffer, int length)
{
	/// Declare variables.
	int ii = 0;
	short int cs_total = 0;

	/// Set total to the header byte.
	cs_total = buffer[0];

	/// Calculate the values of the remaining bytes in the buffer excluding the
	/// checksum byte.
	for(ii = 1; ii < length - 2; ii++) {
		cs_total += convert2short(&buffer[ii]);
		ii++;
	}

	if(cs_total != convert2short(&buffer[length - 2])) {
		return IMU_ERROR_CHECKSUM;
	}

	return IMU_SUCCESS;
} /* end mstrain_calc_checksum() */


/*------------------------------------------------------------------------------
 * int convert2int()
 * Convert two adjacent bytes to an integer.
 *----------------------------------------------------------------------------*/

int convert2int(char *buffer)
{
	int retval = (buffer[0] & LSB_MASK) * 256 + (buffer[1] & LSB_MASK);

	return retval;
} /* end convert2int() */


/*------------------------------------------------------------------------------
 * short convert2short()
 * Convert two adjacent bytes to a short.
 *----------------------------------------------------------------------------*/

short convert2short(char *buffer)
{
	short retval = (buffer[0] & LSB_MASK) * 256 + (buffer[1] & LSB_MASK);

	return retval;
} /* end convert2short() */


/*------------------------------------------------------------------------------
 * int mstrain_read_system_gains()
 * Asks for Euler angles from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_read_system_gains(int fd, short int *accel_gain, short int *mag_gain, short int *bias_gain)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_25;
	int status = 0;
	char response[response_length];
	char cmd = (char)IMU_READ_SYSTEM_GAINS;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	/// Convert bytes to short ints.
	*accel_gain  = convert2short(&response[1]);
	*mag_gain = convert2short(&response[3]);
	*bias_gain   = convert2short(&response[5]);

	/// Leave this print statement. Sometimes we need to write these values down.
	//printf("response_1=%d, response_3=%d, response_5=%d, accel_gain=%d, mag_gain=%d, bias_gain=%d\n",
	//	response[1], response[3], response[5], accel_gain, mag_gain, bias_gain);

	return 1;
} /* end mstrain_read_system_gains() */


/*------------------------------------------------------------------------------
 * int mstrain_read_system_gains()
 * Asks for Euler angles from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_write_system_gains(int fd, short int accel_gain, short int mag_gain, short int bias_gain)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_24_CMD;
	int status = 0;
	char response[response_length];
	char cmd[7] = {0, 0, 0, 0, 0, 0, 0};

	cmd[0] = 0x24;
	cmd[1] = (char)accel_gain;
	cmd[3] = (char)mag_gain;
	cmd[5] = (char)bias_gain;

	/// Leave this print statement. Sometimes we need to write these values down.
	//printf("accel_gain=%d, mag_gain=%d, bias_gain=%d, cmd_1=%d, cmd_3=%d, cmd_5=%d\n", accel_gain, mag_gain, bias_gain,
	//		cmd[1], cmd[3], cmd[5]);

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	return 1;
} /* end mstrain_write_system_gains() */


/*------------------------------------------------------------------------------
 * int mstrain_read_system_gains()
 * Asks for Euler angles from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_zero_mag_gain(int fd)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_25;
	int status = 0;
	char response[response_length];
	char cmd = (char)IMU_READ_SYSTEM_GAINS;

	int write_response_length = (int)IMU_LENGTH_24_RSP;
	char write_response[write_response_length];
	char cmd_write[7] = {0, 0, 0, 0, 0, 0, 0};

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	cmd_write[0] = 0x24;
	cmd_write[1] = response[1];
	cmd_write[2] = response[2];
	cmd_write[3] = 0x0;
	cmd_write[4] = 0x0;
	cmd_write[5] = response[5];
	cmd_write[6] = response[6];

	/// Send command to set data from IMU.
	status = send_serial(fd, &cmd_write, sizeof(cmd_write));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, write_response, write_response_length);
	}

	if(status != write_response_length) {
		return 0;
	}

	return 1;
} /* end mstrain_zero_mag_gains() */


/*------------------------------------------------------------------------------
 * int mstrain_capture_gyro_bias()
 * Asks for Euler angles from IMU.
 *----------------------------------------------------------------------------*/

int mstrain_capture_gyro_bias(int fd)
{
	/// Declare variables.
	int response_length = (int)IMU_LENGTH_06;
	int status = 0;
	char response[response_length];
	char cmd = (char)IMU_GYRO_BIAS;

	/// Send request to and receive data from IMU.
	status = send_serial(fd, &cmd, sizeof(cmd));

	if(status > 0) {
		usleep(MSTRAIN_SERIAL_DELAY);
		status = recv_serial(fd, response, response_length);
	}

	if(status != response_length) {
		return 0;
	}

	return 1;
} /* end mstrain_capture_gyro_bias() */
