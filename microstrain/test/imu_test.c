#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"
#include "microstrain.h"


int main()
{
	int imu_fd = -1;
	int gyro_stab = 1;
	int status = -1;
	int ii;
	char port_name[15];
	char *imu_port;

	strncpy( port_name, "/dev/ttyS0", 15 );
	imu_port = port_name;

	/* Don't pass pointers of locals variables because of stack swaps. Instead
	 * use the heap. */
	float *mag          = ( float * )calloc( 3, sizeof( float ) );
	float *accel        = ( float * )calloc( 3, sizeof( float ) );
	float *ang_rate     = ( float * )calloc( 3, sizeof( float ) );
	float *roll         = ( float * )calloc( 1, sizeof( float ) );
	float *pitch        = ( float * )calloc( 1, sizeof( float ) );
	float *yaw          = ( float * )calloc( 1, sizeof( float ) );
	float *temp         = ( float * )calloc( 1, sizeof( float ) );
	float **orient      = ( float ** )calloc( 3, sizeof( float * ) );
	float *orient_mem   = ( float * )calloc( 9, sizeof( float ) );
	int *serial_num     = ( int * )calloc( 1, sizeof( int ) );

	/* Initialize orient in the heap. */

	for ( ii = 0; ii < 3; ii++ ) {
		orient[ii] = orient_mem + 3 * ii;
	}

	/* Baud rate must be 38400 unless IMU is reconfigured. To reconfigure, use
	 * the code that comes with the Microstrain software package. */
	printf( "Trying IMU at /dev/ttyS0 with 38400.\n" );

	imu_fd = mstrain_setup( imu_port, 38400 );

	printf( "imu_fd = %d\n", imu_fd );

	if ( imu_fd < 0 ) {
		printf( "No IMU, quitting...\n" );
		return 0;
	}

	status = mstrain_temperature( imu_fd, temp );

	status = mstrain_serial_number( imu_fd, serial_num );
	printf( "Temp: %f\n SN: %d\n", *temp, *serial_num );

	while ( 1 ) {
		/* Pointers to the heap are safe. */
		/* Get vectors. */
		status = mstrain_vectors( imu_fd, gyro_stab, mag, accel, ang_rate );
		printf( "Mag Vector: %f %f %f\n", mag[0], mag[1], mag[2] );
		printf( "Accel Vector: %f %f %f\n", accel[0], accel[1], accel[2] );
		printf( "Ang Rate Vector: %f %f %f\n", ang_rate[0], ang_rate[1], ang_rate[2] );

		/* Get Euler angles. */
		status = mstrain_euler_angles( imu_fd, roll, pitch, yaw );
		printf( "Roll: %f\nPitch: %f\nYaw: %f\n\n", *roll, *pitch, *yaw );

		/* Get the orientation vectors. */
		status = mstrain_orientation( imu_fd, gyro_stab, orient );
		printf( "%f %f %f\n%f %f %f\n%f %f %f\n"
		        , orient[0][0]
		        , orient[1][0]
		        , orient[2][0]
		        , orient[0][1]
		        , orient[1][1]
		        , orient[2][1]
		        , orient[0][2]
		        , orient[1][2]
		        , orient[2][2]
		      );
	}

	/* Free used heap memory and throw away the keys. */
	free( mag );

	mag = NULL;

	free( accel );

	accel = NULL;

	free( ang_rate );

	ang_rate = NULL;

	free( roll );

	roll = NULL;

	free( pitch );

	pitch = NULL;

	free( yaw );

	yaw = NULL;

	free( temp );

	temp = NULL;

	free( serial_num );

	serial_num = NULL;

	/* Free orient in the heap. */
	free( orient_mem );

	orient_mem = NULL;

	free( orient );

	orient = NULL;

	/* Close the serial port. */
	close( imu_fd );

	return 0;
} /* end main() */
