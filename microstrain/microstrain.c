/******************************************************************************
 *
 *  Title:        microstrain.c
 *
 *  Description:  Sending and receiving data with the MicroStrain IMU. We are
 *                using model 3DM-GX1. The API is described in the document
 *                3DM_GX1_Data_Communication_Protocol_203101.pdf. See
 *                www.microstrain.com for more details.
 *
 *****************************************************************************/


#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "microstrain.h"
#include "serial.h"


/******************************************************************************
 *
 * Title:       int mstrain_setup( char *portname,
 *                             	   int baud
 *                            	  )
 *
 * Description: Initializes communications with the Microstrain IMU. Sets up a
 * 				file descriptor for further communications.
 *
 * Input:       portname: The name of the port that the IMU is plugged into.
 *              baud:     The baud rate to use for the IMU serial port.
 *
 * Output:      fd: A file descriptor for the IMU.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_setup( char *portname,
                   int baud
                 )
{
	int fd = -1;

	if ( portname != NULL ) {
		fd = setup_serial( portname, baud );
	}

	return fd;
} /* end mstrain_setup() */


/******************************************************************************
 *
 * Title:       int mstrain_serial_number( int fd,
 *                                     	   int *serial_number
 *                                   	  )
 *
 * Description: Asks for serial number from IMU.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              serial_number: A pointer to store the serial number value.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_serial_number( int fd,
                           int *serial_number
                         )
{
	int response_length = ( int )IMU_LENGTH_F1;
	int status;
	char response[response_length];
	char cmd = ( char )IMU_SERIAL_NUMBER;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	*serial_number = convert2int( &response[1] );

	return 1;
} /* end mstrain_serial_number() */


/******************************************************************************
 *
 * Title:       int mstrain_temperature( int fd,
 *                                   	 float *temp
 *                                 	    )
 *
 * Description: Asks for temperature from IMU.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              serial_number: A pointer to store the temperature value.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_temperature( int fd,
                         float *temp
                       )
{
	int response_length = ( int )IMU_LENGTH_07;
	int status;
	char response[response_length];
	char cmd = ( char )IMU_TEMPERATURE;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	*temp = convert2int( &response[1] );

	/* This conversion if from the 3DM-GX1 manual. */
	*temp = ( ( ( *temp * 5.0 / 65536.0 ) - 0.5 ) * 100.0 );

	return 1;
} /* end mstrain_temperature() */


/******************************************************************************
 *
 * Title:       int mstrain_orientation( int fd,
 *                                   	 int gyro_stab,
 *                                   	 float *orient[3]
 *                                 		)
 *
 * Description: Asks for the gyro-stabilized orientation matrix from the IMU.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              gyro_stab: Whether to use gyro-stabilized values or not.
 *              orient: A pointer to store the orientation values.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_orientation( int fd,
                         int gyro_stab,
                         float *orient[3][3]
                       )
{
	char cmd;
	int response_length;

	if ( gyro_stab ) {
		cmd = IMU_GYRO_STAB_ORIENT_MATRIX;
		response_length = ( int )IMU_LENGTH_0B;
	}
	else {
		cmd = IMU_INST_ORIENT_MATRIX;
		response_length = ( int )IMU_LENGTH_0A;
	}

	int status;

	int ii;
	int jj;
	char response[response_length];

	/* Conversion factors are from the 3DM-GX1 manual. */
	float orient_convert_factor = 8192.0;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	for ( ii = 0; ii < 3; ii++ ) {
		for ( jj = 0; jj < 3; jj++ ) {
			*orient[jj][ii] = ( float )convert2short( &response[1 + ii * 2] )
			                  / orient_convert_factor;
		}
	}

	return 1;
} /* end mstrain_orientation() */


/******************************************************************************
 *
 * Title:       int mstrain_vectors( int fd,
 *                               	 int gyro_stab,
 *                               	 float **mag,
 *                               	 float **accel,
 *                               	 float **ang_rate
 *                             		)
 *
 * Description: Asks for vectors from IMU.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              gyro_stab: Whether to use gyro-stabilized values or not.
 *              mag: A pointer to store the magnetic values.
 *              accel: A pointer to store the acceleration values.
 *              ang_rate: A pointer to store the angular rate values.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_vectors( int fd,
                     int gyro_stab,
                     float *mag,
                     float *accel,
                     float *ang_rate
                   )
{
	char cmd;
	int response_length;

	if ( gyro_stab ) {
		cmd = IMU_GYRO_STAB_VECTORS;
		response_length = ( int )IMU_LENGTH_02;
	}
	else {
		cmd = IMU_INST_VECTORS;
		response_length = ( int )IMU_LENGTH_03;
	}

	int status;

	int ii;
	char response[response_length];

	/* Conversion factors are from the 3DM-GX1 manual. */
	float mag_convert_factor = 16384.0; //32768000.0 / 2000.0;
	float accel_convert_factor = 4681.142857143; //32768000.0 / 7000.0;
	float ang_rate_convert_factor = 3855.058823529; //32768000.0 / 8500.0;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	for ( ii = 0; ii < 3; ii++ ) {
		mag[ii] = ( float )convert2short( &response[1 + ii * 2] ) /
		          mag_convert_factor;
		accel[ii] = ( float )convert2short( &response[7 + ii * 2] ) /
		            accel_convert_factor;
		ang_rate[ii] = ( float )convert2short( &response[13 + ii * 2] ) /
		               ang_rate_convert_factor;
	}

	return 1;
} /* end mstrain_vectors() */


/******************************************************************************
 *
 * Title:       int mstrain_euler_angles( int fd,
 *                                    	  float *roll,
 *                                    	  float *pitch,
 *                                    	  float *yaw
 *                                  	 )
 *
 * Description: Asks for Euler angles from IMU.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              roll: A pointer to store the roll value.
 *              pitch: A pointer to store the pitch value.
 *              yaw: A pointer to store the yaw value.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_euler_angles( int fd,
                          float *roll,
                          float *pitch,
                          float *yaw
                        )
{
	int response_length = ( int )IMU_LENGTH_0E;
	int status;
	char response[response_length];
	char cmd = ( char )IMU_GYRO_STAB_EULER_ANGLES;
	/* Conversion factor from the 3DM-GX1 manual. */
	float euler_convert_factor = 360.0 / 65536.0;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	*roll  = convert2short( &response[1] ) * euler_convert_factor;

	*pitch = convert2short( &response[3] ) * euler_convert_factor;
	*yaw   = convert2short( &response[5] ) * euler_convert_factor;

	return 1;
} /* end mstrain_euler_angles() */


/******************************************************************************
 *
 * Title:       int mstrain_quaternions( int fd,
 *                                   	 int gyro_stab,
 *                                   	 float *quat[4]
 *                                 	    )
 *
 * Description: Asks for quaternions from IMU. If enable_log is set to TRUE
 *              then write the data to the log file.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              gyro_stab: Whether to use gyro-stabilized values or not.
 *              quat: A pointer to store the quaternion values.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_quaternions( int fd,
                         int gyro_stab,
                         float *quat[4]
                       )
{
	int status;
	int ii;
	int response_length;
	char cmd;
	/* Conversion factor from the 3DM-GX1 manual. */
	float convert_factor = 8192.0;

	if ( gyro_stab ) {
		cmd = IMU_GYRO_STAB_QUAT;
		response_length = ( int )IMU_LENGTH_05;
	}
	else {
		cmd = IMU_INST_QUAT;
		response_length = ( int )IMU_LENGTH_04;
	}

	char response[response_length];

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	for ( ii = 0; ii < 4; ii++ ) {
		*quat[ii] = ( float )convert2short( &response[1 + ii * 2] )
		            / convert_factor;
	}

	return 1;
} /* end mstrain_quaternions() */


/******************************************************************************
 *
 * Title:       int mstrain_quaternions_vectors( int fd,
 *                                           	 float *quat[4],
 *                                           	 float *mag[3],
 *                                           	 float *accel[3],
 *                                           	 float *ang_rate[3]
 *                                          	)
 *
 * Description: Asks for quaternions and vectors from IMU. If enable_log is set
 *              to TRUE then write the data to the log file.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              quat: A pointer to store the quaternion values.
 *              mag: A pointer to store the magnetic values.
 *              accel: A pointer to store the acceleration values.
 *              ang_rate: A pointer to store the angular rate values.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_quaternions_vectors( int fd,
                                 float *quat[4],
                                 float *mag[3],
                                 float *accel[3],
                                 float *ang_rate[3]
                               )
{
	int response_length = ( int )IMU_LENGTH_0C;
	int status;
	int ii;
	char response[response_length];
	char cmd;

	/* Conversion factors are from the 3DM-GX1 manual. */
	float mag_convert_factor = 32768000.0 / 2000.0;
	float accel_convert_factor = 3276800.0 / 7000.0;
	float ang_rate_convert_factor = 32768000.0 / 8500.0;
	float stab_convert_factor = 8192.0;

	cmd = IMU_GYRO_STAB_QUAT_VECTORS;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	for ( ii = 0; ii < 4; ii++ ) {
		*quat[ii] = ( float )convert2short( &response[1 + ii * 2] )
		            / stab_convert_factor;
	}

	for ( ii = 0; ii < 3; ii++ ) {
		*mag[ii]      = ( float )convert2short( &response[9 + ii * 2] )
		                / mag_convert_factor;
		*accel[ii]    = ( float )convert2short( &response[15 + ii * 2] )
		                / accel_convert_factor;
		*ang_rate[ii] = ( float )convert2short( &response[21 + ii * 2] )
		                / ang_rate_convert_factor;
	}

	return 1;
} /* end mstrain_quaternions_vectors() */


/******************************************************************************
 *
 * Title:       int mstrain_euler_vectors( int fd,
 *                                     	   float *roll,
 *                                     	   float *pitch,
 *                                     	   float *yaw,
 *                                     	   float *accel[3],
 *                                     	   float *ang_rate[3]
 *                                   	  )
 *
 * Description: Asks for Euler Angles and vectors from IMU. If enable_log is
 *              set to TRUE then write the data to the log file.
 *
 * Input:       fd: A file descriptor for the IMU port.
 *              roll: A pointer to store the roll value.
 *              pitch: A pointer to store the pitch value.
 *              yaw: A pointer to store the yaw value.
 *              accel: A pointer to store the acceleration values.
 *              ang_rate: A pointer to store the angular rate values.
 *
 * Output:      1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int mstrain_euler_vectors( int fd,
                           float *roll,
                           float *pitch,
                           float *yaw,
                           float *accel,
                           float *ang_rate
                         )
{
	int response_length = ( int )IMU_LENGTH_31;
	int status;
	int ii;
	char response[response_length];
	char cmd;

	/* Conversion factors are from the 3DM-GX1 manual. */
	float accel_convert_factor = 3276800.0 / 7000.0;
	float ang_rate_convert_factor = 32768000.0 / 8500.0;
	float euler_convert_factor = 360.0 / 65536.0;

	cmd = IMU_GYRO_STAB_EULER_VECTORS;

	/* Send request to and receive data from IMU. */
	status = send_serial( fd, &cmd, sizeof( cmd ) );

	if ( status > 0 ) {
		status = serial_bytes_available( fd );

		if ( status < response_length ) {
			usleep( SERIAL_EXTRA_DELAY_LENGTH );
		}

		status = recv_serial( fd, response, response_length );
	}

	if ( status != response_length ) {
		return 0;
	}

	*roll  = convert2short( &response[1] ) * euler_convert_factor;

	*pitch = convert2short( &response[3] ) * euler_convert_factor;
	*yaw   = convert2short( &response[5] ) * euler_convert_factor;

	for ( ii = 0; ii < 3; ii++ ) {
		accel[ii]    = ( float )convert2short( &response[7 + ii * 2] )
		               / accel_convert_factor;
		ang_rate[ii] = ( float )convert2short( &response[13 + ii * 2] )
		               / ang_rate_convert_factor;
	}

	return 1;
} /* end mstrain_euler_vectors() */


/******************************************************************************
 *
 * Title:       int convert2int( char *buffer )
 *
 * Description: Convert two adjacent bytes to an integer.
 *
 * Input:       buffer: Pointer to first byte.
 *
 * Output:      retval: Resulting integer.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int convert2int( char *buffer )
{
	int retval = ( buffer[0] & LSB_MASK ) * 256 + ( buffer[1] & LSB_MASK );

	return retval;
} /* end convert2int() */


/******************************************************************************
 *
 * Title:       short convert2short( char *buffer )
 *
 * Description: Convert two adjacent bytes to a short.
 *
 * Input:       buffer: Pointer to first bytes.
 *
 * Output:      retval: Resulting short integer.
 *
 * Globals:     None.
 *
 *****************************************************************************/

short convert2short( char *buffer )
{
	short retval = ( buffer[0] & LSB_MASK ) * 256 + ( buffer[1] & LSB_MASK );

	return retval;
} /* end convert2short() */
