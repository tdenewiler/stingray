#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"
#include "microstrain.h"


int main( )
{
	int imu_fd = -1;
	int status = -1;
	char port_name[15];
	char *imu_port;

	strncpy( port_name, "/dev/ttyS0", 15 );
	imu_port = port_name;

	/* Baud rate must be 38400 unless IMU is reconfigured. To reconfigure, use
	 * the code that comes with the Microstrain software package. */
	printf("Trying IMU at /dev/ttyS0 with 38400.\n");
	imu_fd = mstrain_setup( imu_port, 38400 );
	printf("imu_fd = %d\n", imu_fd);
	if ( imu_fd < 0 ) {
		printf("No IMU, quitting...\n");
		return 0;
	}

	/* !!!!!!!!!!!!!! Only use one of the two following calls. !!!!!!!!!!!!!!
	 * !!!!!!!!!!!!!! Recompile to use the other call.         !!!!!!!!!!!!!! */
	/* Set the tare for the Microstrain unit. */
	status = mstrain_set_tare( imu_fd );

	/* Remove the tare for the Microstrain unit. */
	//status = mstrain_remove_tare( imu_fd );

	/* Close the serial port. */
	close( imu_fd );

	return 0;
} /* end main() */
